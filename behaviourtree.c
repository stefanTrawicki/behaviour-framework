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

    if (t != LEAF)
    {
        node->children = create_list();

        if (t == INVERTER || t == ENTRY)
        {
            SET_FLAG(node->flags, IS_DECORATOR);
            node->action_vtable->tick = std_decorator_handler;
        }
        if (t == FALLBACK || t == SEQUENCE)
        {
            SET_FLAG(node->flags, IS_COMPOSITE);
            node->action_vtable->tick = std_composite_handler;
        }
        node->action_vtable->start = std_start;
        node->action_vtable->stop = std_stop;
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

    node->action_vtable = actions;

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

/* ---------------------------------- BTree --------------------------------- */

void b_tree_create(BTree_t *tree)
{
    memset(tree, 0, sizeof(BTree_t));

    SET_FLAG(tree->flags, IS_INITIALISED);
    tree->nodes = create_list();

    LOG("tree %p created", tree);
}

uint32_t b_tree_run(BTree_t *tree)
{
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree is not initialised");
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_ROOT_SET), "Tree root not set");

    LOG("tree %p started with entry node %p", tree, tree->current_node);

    uint8_t state = 0;
    while (!CHECK_FLAG(tree->flags, IS_HALTED))
    {
        Node_t *c = tree->current_node;
        uint8_t f = tree->current_node->flags;

        if (!CHECK_FLAG(f, IS_FINISHED))
        {
            if (!CHECK_FLAG(f, IS_RUNNING))
                c->action_vtable->start(c);
            else
                c->action_vtable->tick(c);
        }
        else
            c->action_vtable->stop(c);
        state = (c->state == SUCCESS);
    }

    return state;
}

void _b_tree_add_node(BTree_t *tree, Node_t *node)
{
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree is not initialised");
    if (!CHECK_FLAG(node->flags, IS_IN_TREE))
    {
        add(tree->nodes, node);
        node->tree = tree;
        SET_FLAG(node->flags, IS_IN_TREE);
        LOG("tree %p added node %p", tree, node);
    }
}

void b_tree_set_root(BTree_t *tree, Node_t *node)
{
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree is not initialised");
    ASSERT_MSG(CHECK_FLAG(tree->flags, IS_ROOT_SET), "Tree root node already set");

    SET_FLAG(tree->flags, IS_ROOT_SET);
    tree->root_node = node;
    node->tree = tree;
    b_tree_move(tree, node);

    LOG("tree %p set root at %p", tree, node);
}

void b_tree_reset(BTree_t *tree)
{
    ASSERT_MSG(!CHECK_FLAG(tree->flags, IS_INITIALISED), "Tree is not initialised");

    LOG("tree %p reset", tree);

    reset_index(tree->nodes);
    CLEAR_FLAG(tree->flags, IS_HALTED);

    while (is_next(tree->nodes))
    {
        Node_t *node = get_next(tree->nodes);

        CLEAR_FLAG(node->flags, IS_RUNNING);
        CLEAR_FLAG(node->flags, IS_FINISHED);
        node->state = UNDETERMINED;

        if (CHECK_FLAG(node->flags, IS_COMPOSITE) || CHECK_FLAG(node->flags, IS_DECORATOR))
        {
            reset_index(node->children);
        }

        LOG("tree %p reset node %p", tree, node);
    }
}

void b_tree_move(BTree_t *tree, Node_t *node)
{
    LABEL_LOG(node, "tree %p moved focus to node %p", tree, node);
    tree->current_node = node;
}

void_list_t *b_tree_discover(BTree_t *tree)
{
    if (CHECK_FLAG(tree->flags, IS_ROOT_SET))
    {
        void_stack_t *stack = create_stack();
        void_list_t *visited_nodes = create_list();

        push(stack, tree->root_node);
        while (stack_size(stack) > 0)
        {
            Node_t *node = stack_head(stack);
            if (node->type == LEAF || !is_next(node->children)) {
                add(visited_nodes, pop(stack));
                continue;
            }
            while (is_next(node->children)) {
                Node_t *child = get_next(node->children);
                push(stack, child);
            }
        }

        for (int i = 0; i < length(visited_nodes); i++) {
            Node_t *n = get(visited_nodes, i);
            _b_tree_add_node(tree, n);
            if (n->children) reset_index(n->children);
        }
        return visited_nodes;

    }
    else
    {
        printf("Tree must have root set to discover nodes\n");
        return (void *)0;
    }
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
    else
    {
        uint8_t passed = (child->state == SUCCESS);
        if (node->type == ENTRY)
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