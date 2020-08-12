#include "tree.h"
#include <stdio.h>

int Subject = 0;

void s(void *p_node)
{
    printf("example_enter\n");
}

void f(void *p_node)
{
    printf("example_exit\n");
}

void t(void *p_node)
{
    printf("example_tick\n");
    struct node *n = (struct node *)p_node;

    if (n->is_control) {
        struct control_structure *c = n->control;
        if (c->type == ENTRY)
        {
            node_success(n);
            return;
        }
    }

    if (*((int *)n->subject) < 10) node_running(n);
    else node_failure(n);
}

int main(int argc, char **argv)
{
    behaviour_tree_initialiser();

    struct callbacks l1_callbacks = {
        .start = &s,
        .end = &f,
        .tick = &t
    };

    struct callbacks l2_callbacks = {
        .start = &s,
        .end = &f,
        .tick = &t
    };

    struct node *entry_node = control_node_create("E", NULL, ENTRY);
    struct behaviour_tree * bt = behaviour_tree_create(entry_node);
    entry_node->subject = &Subject;
    struct node *sequence = control_node_create("S", entry_node, SEQUENCE);

    struct node *leaf1 = node_create("L1", &Subject, sequence, &l1_callbacks);
    struct node *leaf2 = node_create("L2", &Subject, sequence, &l2_callbacks);

    while (!bt->halted) {
        behaviour_tree_tick(bt);
    }

    return 0;
}