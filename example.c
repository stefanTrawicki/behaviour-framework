#include "tree.h"
#include <stdio.h>
#include <unistd.h>

int Subject = 0;

void s1(void *p_node)
{
    struct node *node = p_node;
    printf("\tstarted (%s)\n", node->label);
    node->is_running = 1;
    behaviour_tree_set_current(node->bt, node);
}

void e1(void *p_node)
{
    struct node *node = p_node;
    printf("\tended(%s)\n", node->label);
    behaviour_tree_set_current(node->bt, node->parent);
}

void t1(void *p_node)
{
    struct node *node = p_node;
    int *p_s = node->subject;
    printf("\tticked (%s)\n", node->label);

    (*p_s)++;

    if ((*p_s) > 3) node_success(node);
    else if ((*p_s) < 0) node_failure(node);
    else node_running(node);
}

void s2(void *p_node)
{
    struct node *node = p_node;
    printf("\tstarted (%s)\n", node->label);
    node->is_running = 1;
    behaviour_tree_set_current(node->bt, node);
}

void e2(void *p_node)
{
    struct node *node = p_node;
    printf("\tended(%s)\n", node->label);
    behaviour_tree_set_current(node->bt, node->parent);
}

void t2(void *p_node)
{
    struct node *node = p_node;
    int *p_s = node->subject;
    printf("\tticked (%s)\n", node->label);

    (*p_s)++;

    node_failure(node);
}

struct callbacks success_callbacks = {
    .start = &s1,
    .end = &e1,
    .tick = &t1
};

struct callbacks fail_callbacks = {
    .start = &s2,
    .end = &e2,
    .tick = &t2
};

struct callbacks *test(int state) {
    if (state) return &success_callbacks;
    else return &fail_callbacks;
}

int main(int argc, char **argv)
{
    behaviour_tree_initialiser();

    int l1 = 1;
    int l2 = 1;
    int l3 = 1;
    int l4 = 1;

    struct node *entry_node = control_node_create("E", NULL, ENTRY);
    struct behaviour_tree * bt = behaviour_tree_create(entry_node);

    struct node *selector_1 = control_node_create("SL1", entry_node, SELECTOR);
    struct node *leaf_1 = node_create("L1", &Subject, selector_1, test(l1));
    struct node *leaf_2 = node_create("L2", &Subject, selector_1, test(l2));

    struct node *sequence_1 = control_node_create("S1", selector_1, SEQUENCE);
    struct node *leaf_3 = node_create("L3", &Subject, sequence_1, test(l3));
    struct node *leaf4 = node_create("L4", &Subject, sequence_1, test(l4));

    while (!bt->halted) {
        behaviour_tree_tick(bt);
    }

    printf("Tree terminated\n");

    printf("Expected: %d\n", ((l1 || l2) || (l3 && l4)) );

    return 0;
}