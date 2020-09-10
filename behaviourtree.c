#include "behaviourtree.h"

/* --------------------------------- Logging -------------------------------- */

void log_set_path(char *path)
{
    log_path = malloc(sizeof(char) * strlen(path));
    strcpy(log_path, path);
    CLEAR_LOG();
    return;
}

/* ---------------------------------- Nodes --------------------------------- */

void _print_flags(uint8_t flags)
{
    for (uint8_t i = IS_COMPOSITE; i < IS_ROOT_SET; i++)
        printf("%d ", CHECK_FLAG(flags, i));
    printf("\n");
}

void node_create(Node_t *node, NodeType_e t)
{
    memset(node, 0, sizeof(Node_t));

    node->type = t;
    node->state = UNDETERMINED;

    SET_FLAG(node->flags, IS_INITIALISED);

    node->action_vtable = malloc(sizeof(ActionVtable_t));
    node->state_vtable = malloc(sizeof(StateVtable_t));

    node->state_vtable->succeed = std_success;
    node->state_vtable->fail = std_failure;
    node->state_vtable->run = std_running;
    node->action_vtable->std_start = std_start;
    node->action_vtable->std_stop = std_stop;

    if (t != LEAF)
    {
        node->children = create_list();
        switch (t)
        {
        case INVERTER:
        case ENTRY:
        case REPEATER:
            SET_FLAG(node->flags, IS_DECORATOR);
            node->action_vtable->tick = std_decorator_handler;
            break;
        case FALLBACK:
        case SEQUENCE:
            SET_FLAG(node->flags, IS_COMPOSITE);
            node->action_vtable->tick = std_composite_handler;
            break;
        default:
            break;
        }
    }

    LOG("created node %p of type %s", node, TYPE_LABEL[node->type]);
}

void node_set_label(Node_t *node, const char *label)
{
    ASSERT_MSG(!CHECK_FLAG(node->flags, IS_INITIALISED), "Node is not initialised");
    node->label = malloc(sizeof(char) * strlen(label));
    strcpy(node->label, label);
    SET_FLAG(node->flags, IS_LABELLED);

    LABEL_LOG(node, "labelled node %p", node);
}

void node_set_actions(Node_t *node, ActionVtable_t *actions)
{
    ASSERT_MSG(!CHECK_FLAG(node->flags, IS_INITIALISED), "Node is not initialised");
    ASSERT_MSG(node->type != LEAF, "Non-leaf nodes cannot be given actions");

    if (actions->start)
        node->action_vtable->start = actions->start;
    if (actions->stop)
        node->action_vtable->stop = actions->stop;
    if (actions->tick)
        node->action_vtable->tick = actions->tick;
    if (actions->std_start)
        node->action_vtable->std_start = actions->std_start;
    if (actions->std_stop)
        node->action_vtable->std_stop = actions->std_stop;

    LABEL_LOG(node, "node %p actions set to %p", node, actions);
}

void node_set_subject(Node_t *node, void *subject)
{
    ASSERT_MSG(!CHECK_FLAG(node->flags, IS_INITIALISED), "Node is not initialised");
    ASSERT_MSG(node->type != LEAF, "Non-leaf nodes cannot be given subjects");

    node->subject = subject;

    LABEL_LOG(node, "node %p subject set to %p", node, subject);
}

void node_add_child(Node_t *parent, Node_t *child)
{
    ASSERT_MSG(!CHECK_FLAG(parent->flags, IS_INITIALISED), "Parent isn't initialised");
    ASSERT_MSG(!CHECK_FLAG(child->flags, IS_INITIALISED), "Child isn't initialised");
    ASSERT_MSG(child->parent != NULL, "Child already has a parent node");

    uint8_t is_composite = CHECK_FLAG(parent->flags, IS_COMPOSITE);
    uint8_t is_decorator = CHECK_FLAG(parent->flags, IS_DECORATOR);
    ASSERT_MSG(!is_composite && !is_decorator, "Parent is of incorrect type");

    ASSERT_MSG(is_decorator && length(parent->children) == 1, "Decorator cannot have > 1 child");

    child->parent = parent;

    add(parent->children, child);

    LABEL_LOG(parent, "node %p added child %p", parent, child);
}

void node_set_repetitions(Node_t *node, int32_t repetitions)
{
    ASSERT_MSG(node->type != REPEATER, "Cannot set repeitions for none repeater node");
    ASSERT_MSG(repetitions == 0 || repetitions < -1, "Repetitions must be -1 OR > 0");
    node->left_loops = repetitions;
    node->start_loops = repetitions;
}

void node_reset(Node_t *node) {
    CLEAR_FLAG(node->flags, IS_RUNNING);
    CLEAR_FLAG(node->flags, IS_FINISHED);
    node->state = UNDETERMINED;

    if (node->type != LEAF) {
        while (is_next(node->children)) {
            Node_t *n = get_next(node->children);
            node_reset(n);
        }
        if (node->type == REPEATER) node->left_loops = node->start_loops;
        reset_index(node->children);
    }

    LABEL_LOG(node, "reset node %p", node);
}

void node_set_tree(Node_t *node, BTree_t *tree) {
    node->tree = tree;
    SET_FLAG(node->flags, IS_IN_TREE);
    if (node->type != LEAF) {
        while (is_next(node->children)) {
            Node_t *n = get_next(node->children);
            node_set_tree(n, tree);
        }
        reset_index(node->children);
    }

    LABEL_LOG(node, "set node %p tree to %p", node, tree);
}

void *node_get_blackboard(Node_t *node)
{
    ASSERT_MSG(!CHECK_FLAG(node->tree->flags, IS_BLACKBOARD_SET), "Tree blackboard not set");
    ASSERT_MSG(!CHECK_FLAG(node->flags, IS_IN_TREE), "Node does't belong to any tree");
    return node->tree->blackboard;
}

/* ---------------------------------- BTree --------------------------------- */

void b_tree_create(BTree_t *tree)
{
    memset(tree, 0, sizeof(BTree_t));

    SET_FLAG(tree->flags, IS_INITIALISED);
    tree->nodes = create_list();

    LOG("tree %p created", tree);
}

int32_t b_tree_tick(BTree_t *tree)
{
    // return -1 if fail, 0 if running, 1 if successful
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree is not initialised");
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_ROOT_SET), "Tree root not set");

    LOG("tree %p ticked on node %p", tree, tree->current_node);

    if (!CHECK_FLAG(tree->flags, IS_HALTED))
    {
        Node_t *c = tree->current_node;
        uint8_t f = tree->current_node->flags;

        if (!CHECK_FLAG(f, IS_FINISHED))
        {
            if (!CHECK_FLAG(f, IS_RUNNING))
            {
                c->action_vtable->std_start(c);
                if (c->action_vtable->start)
                    c->action_vtable->start(c);
            }
            else
                c->action_vtable->tick(c);
        }
        else
        {
            if (c->action_vtable->stop)
                c->action_vtable->stop(c);
            c->action_vtable->std_stop(c);
            
            if (c->type == ENTRY) {
                return (c->state == SUCCESS) ? 1: -1;
            }
        }
    }
    return 0;
}

int32_t b_tree_is_running(BTree_t *tree) {
    return (!CHECK_FLAG(tree->flags, IS_HALTED));
}

int32_t b_tree_run(BTree_t *tree)
{
    int32_t end_state = 0;
    while (end_state == 0) {
        end_state = b_tree_tick(tree);
    }
    return end_state;
}

void b_tree_set_root(BTree_t *tree, Node_t *node)
{
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree is not initialised");
    ASSERT_MSG(CHECK_FLAG(tree->flags, IS_ROOT_SET), "Tree root node already set");

    SET_FLAG(tree->flags, IS_ROOT_SET);
    tree->root_node = node;
    node_set_tree(node, tree);
    b_tree_move(tree, node);

    LOG("tree %p set root at %p", tree, node);
}

void b_tree_reset(BTree_t *tree)
{
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree is not initialised");

    LOG("tree %p reset", tree);

    CLEAR_FLAG(tree->flags, IS_HALTED);

    node_reset(tree->root_node);
}

void b_tree_move(BTree_t *tree, Node_t *node)
{
    LABEL_LOG(node, "tree %p moved focus to node %p", tree, node);
    tree->current_node = node;
}

void b_tree_set_blackboard(BTree_t *tree, void *blackboard)
{
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree isn't initialised yet");
    tree->blackboard = blackboard;
    SET_FLAG(tree->flags, IS_BLACKBOARD_SET);
}

/* --------------------------- Standard functions --------------------------- */

void std_success(Node_t *node)
{
    LABEL_LOG(node, "node %p succeeded", node);
    FINISH(node);
}
void std_failure(Node_t *node)
{
    LABEL_LOG(node, "node %p failed", node);
    FINISH(node);
}
void std_running(Node_t *node)
{
    LABEL_LOG(node, "node %p is running", node);
}

void std_start(Node_t *node)
{
    if (CHECK_FLAG(node->flags, IS_DECORATOR))
        ASSERT_MSG(length(node->children) != 1, "Decorators need children === 1 to start");
    else if (CHECK_FLAG(node->flags, IS_COMPOSITE))
        ASSERT_MSG(length(node->children) <= 1, "Composites need children > 1 to start");
    LABEL_LOG(node, "started %s node %p", TYPE_LABEL[node->type], node);
    START(node);
}

void std_stop(Node_t *node)
{
    LABEL_LOG(node, "stopped %s node %p", TYPE_LABEL[node->type], node);
    FINISH(node);
    if (node->type != ENTRY)
        b_tree_move(node->tree, node->parent);
    else
        SET_FLAG(node->tree->flags, IS_HALTED);
}

void std_decorator_handler(Node_t *node)
{
    LABEL_LOG(node, "ticked %s node %p", TYPE_LABEL[node->type], node);
    Node_t *child = get(node->children, 0);
    if (child->state == UNDETERMINED)
    {
        b_tree_move(node->tree, child);
        RUN(node);
    }
    else if (node->type == REPEATER &&
             (node->left_loops == -1 || node->left_loops > 1))
    {
        node_reset(child);
        b_tree_move(node->tree, child);
        if (node->left_loops != -1)
            node->left_loops--;
        LABEL_LOG(node, "repeater %p has %d loops left", node, node->left_loops);
        RUN(node);
    }
    else
    {
        uint8_t passed = (child->state == SUCCESS);
        if (node->type == ENTRY || node->type == REPEATER)
            passed ? SUCCEED(node) : FAIL(node);
        else
            passed ? FAIL(node) : SUCCEED(node);
    }
}

void std_composite_handler(Node_t *node)
{
    LABEL_LOG(node, "ticked %s node %p", TYPE_LABEL[node->type], node);

    for (uint32_t i = 0; i < length(node->children); i++)
    {
        Node_t *child = get(node->children, i);

        if (child->state == UNDETERMINED)
        {
            b_tree_move(node->tree, child);
            RUN(node);
        }
        else if (child->state == FAIL && node->type == SEQUENCE)
        {
            b_tree_move(node->tree, node->parent);
            FAIL(node);
        }
        else if (child->state == SUCCESS && node->type == FALLBACK)
        {
            b_tree_move(node->tree, node->parent);
            SUCCEED(node);
        }
    }
    b_tree_move(node->tree, node->parent);
    (node->type == SEQUENCE) ? SUCCEED(node) : FAIL(node);
}