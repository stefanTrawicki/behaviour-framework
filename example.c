#include "tree.h"

int Subject = 0;

void start(void *p_node)
{
    node_t *node = p_node;

    LOG("started leaf node '%s'", node->label);

    SET_FLAG(node->flags, IS_RUNNING);
    behaviour_tree_move(node->tree, node);
}

void stop(void *p_node)
{
    node_t *node = p_node;

    LOG("stopped leaf node '%s'", node->label);

    behaviour_tree_move(node->tree, node->parent);
}

void tick1(void *p_node)
{
    node_t *node = p_node;
    int *p_s = node->subject;
    
    LOG("ticked leaf node '%s'", node->label);

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
    
    LOG("ticked leaf node '%s'", node->label);

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

int run_tree(void *t) {
    behaviour_tree_t *tree = t;

    while (!CHECK_FLAG(tree->flags, IS_HALTED))
    {
        behaviour_tree_tick(tree);
    }

    int out = (tree->root_node->state == SUCCESSFUL);

    behaviour_tree_reset(tree);
    Subject = 0;

    // char node_data[100];
    // sprintf(node_data, "\n\n\n");
    // LOG(tree->root_node, node_data);
    FILE *f = fopen(tree->root_node->tree->log_path, "a");
    LOG("\n\n\n");
    fclose(f);

    return out;
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        log_path = argv[1];
    }

    int options = 3;
    int l[options];

    behaviour_tree_t *tree = behaviour_tree_create("log.txt", IS_LOGGING);
    node_t *entry = node_create("entry", NULL, ENTRY, 0);
    behaviour_tree_set_root(tree, entry);

    node_t *i1 = node_create("inverter", entry, INVERTER, 0);

    node_t *s1 = node_create("s1", i1, SEQUENCE, 0);

    node_t *leaf_1 = node_create("leaf1", s1, LEAF, 0);
    node_t *s2 = node_create("s2", s1, FALLBACK, 0);

    // node_t *s3 = node_create("s3", s1, SEQUENCE, 0);

    node_t *leaf_2 = node_create("leaf2", s2, LEAF, 0);
    node_t *i2 = node_create("inverter 2", s2, INVERTER, 0);
    node_t *leaf_3 = node_create("leaf3", i2, LEAF, 0);

    uint8_t count = 0;
    int passed = 1;

    for (int i = 0; i < pow(2, options); i++) {
        for (int j = 0; j < options; j++) l[j] = CHECK_FLAG(count, j);

        leaf_configure(leaf_1, &Subject, test(l[0]));
        leaf_configure(leaf_2, &Subject, test(l[1]));
        leaf_configure(leaf_3, &Subject, test(l[2]));

        int expected = !( (l[0]) && (l[1] || !l[2]) );
        int actual = run_tree(tree);

        if (actual != expected) {
            printf("Failed (exp %d w precon %d, %d, %d, got %d)\n",
           expected, l[0], l[1], l[2], actual);
           passed = 0;
        }
        count++;

        behaviour_tree_reset(tree);
    }

    if (passed) printf("Passed all %d tests successfully\n", (int)pow(2, options));

    return 0;
}