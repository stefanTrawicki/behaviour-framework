#include "tree.h"

int Subject = 0;

void start(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "started leaf node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->flags, IS_RUNNING);
    behaviour_tree_move(node->tree, node);
}

void stop(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "stopped leaf node '%s'", node->label);
    LOG(node, node_data);

    behaviour_tree_move(node->tree, node->parent);
}

void tick1(void *p_node)
{
    node_t *node = p_node;
    int *p_s = node->subject;
    
    char node_data[100];
    sprintf(node_data, "ticked leaf node '%s'", node->label);
    LOG(node, node_data);

    (*p_s)++;

    if ((*p_s) > 3)
        PASS(node);
    else if ((*p_s) < 0)
        FAIL(node);
    else
        RUN(node);
}

void tick2(void *p_node)
{
    node_t *node = p_node;
    int *p_s = node->subject;
    
    char node_data[100];
    sprintf(node_data, "ticked leaf node '%s'", node->label);
    LOG(node, node_data);

    (*p_s)++;

    FAIL(node);
}

action_vtable_t success_callbacks = {
    .start = &start,
    .stop = &stop,
    .tick = &tick1};

action_vtable_t failure_callbacks = {
    .start = &start,
    .stop = &stop,
    .tick = &tick2};

action_vtable_t *test(int state)
{
    if (state)
        return &success_callbacks;
    else
        return &failure_callbacks;
}

int main(int argc, char **argv)
{
    int l1 = 1;
    int l2 = 0;
    int l3 = 0;
    int l4 = 0;

    behaviour_tree_t *tree = behaviour_tree_create("log.txt", IS_LOGGING);

    node_t *entry = node_create("entry", NULL, ENTRY, 0);

    behaviour_tree_set_root(tree, entry);

    node_t *inverter = node_create("inverter", entry, INVERTER, 0);
    node_t *leaf_1 = node_create("leaf 1", inverter, LEAF, 0);
    leaf_configure(leaf_1, &Subject, test(l1));

    printf("Tree started...\n");

    while (!CHECK_FLAG(tree->flags, IS_HALTED))
    {
        behaviour_tree_tick(tree);
    }

    printf("Tree halted...\n");

    int end_state = CHECK_FLAG(tree->flags, END_STATE);
    int expected = !l1;

    printf("%s (expected %d given preconditions %d%d%d%d)\n",
           ((end_state == expected ? "Passed" : "Failed")),
           expected,
           l1, l2, l3, l4);

    return 0;
}