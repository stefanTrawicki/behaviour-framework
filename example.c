#include <behaviourtree.h>

void start(Node_t *node) {
    LABEL_LOG(node, "node %p started", node);
    START(node);
}

void stop(Node_t *node) {
    LABEL_LOG(node, "node %p stopped", node);
    FINISH(node);
    b_tree_move(node->tree, node->parent);
}

void tick_succeed(Node_t *node) {
    SUCCEED(node);
}

void tick_fail(Node_t *node) {
    FAIL(node);
}

ActionVtable_t leaf_success = {
    .start = &start,
    .tick = &tick_succeed,
    .stop = &stop
};

ActionVtable_t leaf_failure = {
    .start = &start,
    .tick = &tick_fail,
    .stop = &stop
};

int main(int argc, char **argv) {

    if (argc > 1) log_set_path(argv[1]);
    else printf("No log found, logging disabled\n");

    Node_t leaf1;
    node_create(&leaf1, LEAF);
    node_set_actions(&leaf1, &leaf_success);

    Node_t leaf2;
    node_create(&leaf2, LEAF);
    node_set_actions(&leaf2, &leaf_failure);

    Node_t entry;
    node_create(&entry, ENTRY);

    Node_t fallback;
    node_create(&fallback, FALLBACK);

    node_add_child(&entry, &fallback);
    node_add_child(&fallback, &leaf1);
    node_add_child(&fallback, &leaf2);

    BTree_t tree;
    b_tree_create(&tree);
    b_tree_set_root(&tree, &entry);
    b_tree_discover(&tree);

    printf("tree evaluated to %d\n", b_tree_run(&tree));

    b_tree_reset(&tree);

    printf("tree evaluated to %d\n", b_tree_run(&tree));

    return 0;
}