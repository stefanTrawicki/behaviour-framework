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
}

void t2(void *p_node)
{
    struct node *node = p_node;
    int *p_s = node->subject;
    printf("\tticked (%s)\n", node->label);

    (*p_s)++;

    node_failure(node);
}

int main(int argc, char **argv)
{
    behaviour_tree_initialiser();

    struct callbacks l1_callbacks = {
        .start = &s1,
        .end = &e1,
        .tick = &t1
    };

    struct callbacks l2_callbacks = {
        .start = &s2,
        .end = &e2,
        .tick = &t2
    };

    struct node *entry_node = control_node_create("E", NULL, ENTRY);
    struct behaviour_tree * bt = behaviour_tree_create(entry_node);

    struct node *sequence_node = control_node_create("S", entry_node, SEQUENCE);

    struct node *leaf1 = node_create("L1", &Subject, sequence_node, &l1_callbacks);
    struct node *leaf2 = node_create("L2", &Subject, sequence_node, &l2_callbacks);

    while (!bt->halted) {
        behaviour_tree_tick(bt);
        // usleep(500000);
    }

    printf("Tree terminated\n");

    return 0;
}