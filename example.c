#include "tree.h"
#include <stdio.h>

int Subject = 0;

void s(void *p_node, void* p_subject, struct functions *fns)
{
    int *ctx = (int *)p_subject;
    *ctx = *ctx + 1;
    printf("example_enter %d\n", *ctx);
}

void f(void *p_node, void* p_subject, struct functions *fns)
{
    int *ctx = (int *)p_subject;
    *ctx = *ctx + 1;
    printf("example_exit %d\n", *ctx);
}

void t(void *p_node, void* p_subject, struct functions *fns)
{
    int *ctx = (int *)p_subject;
    *ctx = *ctx + 1;
    printf("example_tick %d\n", *ctx);
    struct node *n = (struct node *)p_node;

    if (n->is_control) {
        struct control_structure *c = n->control;
        if (c->type == ENTRY)
        {
            run(p_node, fns->success);
            return;
        }
    }

    if (*ctx < 10) run(p_node, fns->running);
    else run(p_node, fns->failure);
}

int main(int argc, char **argv)
{
    behaviour_tree_initialiser();

    struct callbacks x = {
        .start = &s,
        .end = &f,
        .tick = &t
    };

    struct node *entry_node = control_node_create("root", NULL, ENTRY);

    struct node *repeater_node = control_node_create("repeater 1", entry_node, REPEATER);
    struct node *reg_node = node_create("leaf 1", &Subject, repeater_node, &x);
    struct node *reg_node2 = node_create("leaf 2", NULL, repeater_node, NULL);

    struct behaviour_tree * bt = behaviour_tree_create(entry_node);

    reg_node->cb->tick(reg_node, &Subject, reg_node->fn);

    return 0;
}