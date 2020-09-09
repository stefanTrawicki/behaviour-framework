#include <behaviourtree.h>

struct blackboard_example {
    int x;
    char y;
    float z;
};

void tick_succeed(Node_t *node) {
    struct blackboard_example *b = node_get_blackboard(node);
    printf("x %d\n", b->x);
    b->x += 20;
    SUCCEED(node);
}

void tick_fail(Node_t *node) {
    struct blackboard_example *b = node_get_blackboard(node);
    printf("x %d\n", b->x);
    FAIL(node);
}

ActionVtable_t leaf_success = {
    .tick = &tick_succeed,
};

ActionVtable_t leaf_failure = {
    .tick = &tick_fail,
};

int main(int argc, char **argv) {

    if (argc > 1) log_set_path(argv[1]);

    Node_t leaf1;
    node_create(&leaf1, LEAF);
    node_set_actions(&leaf1, &leaf_success);

    Node_t leaf2;
    node_create(&leaf2, LEAF);
    node_set_actions(&leaf2, &leaf_failure);

    Node_t repeater;
    node_create(&repeater, REPEATER);
    node_set_repetitions(&repeater, 10);

    Node_t entry;
    node_create(&entry, ENTRY);

    Node_t fallback;
    node_create(&fallback, SEQUENCE);

    node_add_child(&entry, &repeater);
    node_add_child(&repeater, &fallback);
    node_add_child(&fallback, &leaf1);
    node_add_child(&fallback, &leaf2);

    struct blackboard_example b = {.x = 5, .y = 10};

    BTree_t tree;
    b_tree_create(&tree);
    b_tree_set_root(&tree, &entry);
    b_tree_set_blackboard(&tree, &b);

    int tree_out = b_tree_run(&tree);
    printf("tree evaluated to %d\n", tree_out);

    return 0;
}