#include "tree.h"

/* --------------------------------- Logging -------------------------------- */

void log_set_path(char *path)
{
    log_path = malloc(sizeof(char) * strlen(path));
    strcpy(log_path, path);
    CLEAR_LOG();
    return;
}

/* ---------------------------------- List ---------------------------------- */

void list_create(List_t *list)
{
    list->count = 0;
    list->index = -1;
}

void list_add(List_t *list, void *tar_to_add)
{
    list->count++;
    list->arr = realloc(list->arr, sizeof(void *) * (list->count + 1));
    list->arr[list->count - 1] = tar_to_add;
}

void *list_get(List_t *list, uint32_t index)
{
    return list->arr[index];
}

void* list_current(List_t *list) {
    return list_get(list, list->index);
}

void *list_next(List_t *list)
{
    ASSERT_MSG(list->index == list->count, "List element out of range");
    list->index++;
    return list_current;
}

uint32_t list_is_next(List_t *list)
{
    return (list->index + 1) < (list->count);
}

uint32_t list_get_c(List_t *list)
{
    return list->count;
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
        node->children = malloc(sizeof(Node_t));
        list_create(node->children);
        if (t == INVERTER || t == ENTRY)
            SET_FLAG(node->flags, IS_DECORATOR);
        if (t == FALLBACK || t == SEQUENCE)
            SET_FLAG(node->flags, IS_COMPOSITE);

        switch (t)
        {
        case ENTRY:
            node->action_vtable->start = entry_start;
            node->action_vtable->stop = entry_stop;
            node->action_vtable->tick = entry_tick;
            break;
        case SEQUENCE:
            node->action_vtable->start = sequence_start;
            node->action_vtable->stop = sequence_stop;
            node->action_vtable->tick = sequence_tick;
            break;
        case FALLBACK:
            node->action_vtable->start = fallback_start;
            node->action_vtable->stop = fallback_stop;
            node->action_vtable->tick = fallback_tick;
            break;
        case INVERTER:
            node->action_vtable->start = inverter_start;
            node->action_vtable->stop = inverter_stop;
            node->action_vtable->tick = inverter_tick;
            break;
        default:
            break;
        }
    }

    LOG("created node %p of type %d", node, node->type);
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

    ASSERT_MSG(is_decorator && parent->children->count == 1, "Decorator cannot have > 1 child");

    child->parent = parent;

    list_add(parent->children, (void *)child);

    LABEL_LOG(parent, "node %p added child %p", parent, child);
}

/* ---------------------------------- BTree --------------------------------- */

void b_tree_create(BTree_t *tree)
{
    memset(tree, 0, sizeof(BTree_t));

    SET_FLAG(tree->flags, IS_INITIALISED);
    tree->nodes = malloc(sizeof(List_t));
    list_create(tree->nodes);

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

    list_add(tree->nodes, node);
    node->tree = tree;

    LOG("tree %p added node %p", tree, node);
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

    uint32_t i = 0;
    while (list_is_next(tree->nodes))
    {
        Node_t *node = list_next(tree->nodes);

        CLEAR_FLAG(node->flags, IS_RUNNING);
        CLEAR_FLAG(node->flags, IS_FINISHED);

        if (CHECK_FLAG(node->flags, IS_COMPOSITE) || CHECK_FLAG(node->flags, IS_DECORATOR))
        {
            node->children->index = 0;
        }

        LOG("tree %p reset node %p", tree, node);
        i++;
    }
}

void b_tree_move(BTree_t *tree, Node_t *node)
{
    LABEL_LOG(node, "tree %p moved focus to node %p", tree, node);

    tree->current_node = node;
    _b_tree_add_node(tree, node);
}

/* ------------------------ Standard state functions ------------------------ */

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
    LABEL_LOG(node, "node %p was run", node);
}

/* ---------------------------- Action functions ---------------------------- */

/* ---------------------------------- Entry --------------------------------- */

void entry_start(Node_t *node)
{
    ASSERT_MSG(list_get_c(node->children) != 1, "Entry needs children === 1 to start");
    LABEL_LOG(node, "started entry node %p", node);
    SET_FLAG(node->flags, IS_RUNNING);
}

void entry_tick(Node_t *node)
{
    LABEL_LOG(node, "ticked entry node %p", node);

    Node_t *child = list_get(node->children, 0);
    if (child->state == UNDETERMINED)
        b_tree_move(node->tree, child);
    else
        child->state == SUCCESS ? SUCCEED(node) : FAIL(node);
}

void entry_stop(Node_t *node)
{
    LABEL_LOG(node, "stopped entry node %p", node);
    SET_FLAG(node->tree->flags, IS_HALTED);
}

/* -------------------------------- Sequence -------------------------------- */

void sequence_start(Node_t *node)
{
    LABEL_LOG(node, "started sequence node %p", node);
    START(node);
}
void sequence_tick(Node_t *node)
{
    ASSERT_MSG(!(list_get_c(node->children) > 0), "Sequence requires at least 1 child");
    LABEL_LOG(node, "ticked sequence node %p", node);

    for (uint32_t i = 0; i < list_get_c(node->children); i++)
    {
        Node_t *child = list_get(node->children, i);

        if (child->state == FAIL) {
            b_tree_move(node->tree, node->parent);
            FAIL(node);
        } else if (child->state == UNDETERMINED) {
            b_tree_move(node->tree, child);
            RUN(node);
        }
    }

    b_tree_move(node->tree, node->parent);
    SUCCEED(node);
}
void sequence_stop(Node_t *node)
{
    LABEL_LOG(node, "stopped sequence node %p", node);
    b_tree_move(node->tree, node->parent);
}

/* -------------------------------- Fallback -------------------------------- */

void fallback_start(Node_t *node)
{
    LABEL_LOG(node, "started fallback node %p", node);
    START(node);
}
void fallback_tick(Node_t *node)
{
    ASSERT_MSG(!(list_get_c(node->children) > 0), "Fallback requires at least 1 child");
    LABEL_LOG(node, "ticked fallback node %p", node);

    for (uint32_t i = 0; i < list_get_c(node->children); i++)
    {
        Node_t *child = list_get(node->children, i);

        if (child->state == SUCCESS) {
            b_tree_move(node->tree, node->parent);
            SUCCEED(node);
        } else if (child->state == UNDETERMINED) {
            b_tree_move(node->tree, child);
            RUN(node);
        }
    }

    b_tree_move(node->tree, node->parent);
    FAIL(node);
}
void fallback_stop(Node_t *node)
{
    LABEL_LOG(node, "stopped fallback node %p", node);
    b_tree_move(node->tree, node->parent);
}

/* -------------------------------- Inverter -------------------------------- */

void inverter_start(Node_t *node)
{
    ASSERT_MSG(list_get_c(node->children) != 1, "Inverter needs children === 1 to start");
    LABEL_LOG(node, "started inverter node %p", node);
    SET_FLAG(node->flags, IS_RUNNING);
}
void inverter_tick(Node_t *node)
{
    LABEL_LOG(node, "ticked inverter node %p", node);

    Node_t *child = list_get(node->children, 0);
    if (child->state == UNDETERMINED)
        b_tree_move(node->tree, child);
    else
        child->state == SUCCESS ? FAIL(node) : SUCCEED(node);
}
void inverter_stop(Node_t *node)
{
    LABEL_LOG(node, "stopped inverter node %p", node);
    b_tree_move(node->tree, node->parent);
}