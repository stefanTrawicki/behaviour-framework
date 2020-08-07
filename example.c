#include "tree.h"
#include <stdio.h>

int Subject = 0;

void s(void *node, void *object, struct node_actions *calls)
{
    int *ctx = (int *)object;
    *ctx = *ctx + 1;
    printf("example_enter %d\n", *ctx);
}

void f(void *node, void *object, struct node_actions *calls)
{
    int *ctx = (int *)object;
    *ctx = *ctx + 1;
    printf("example_exit %d\n", *ctx);
}

void t(void *node, void *object, struct node_actions *calls)
{
    int *ctx = (int *)object;
    *ctx = *ctx + 1;
    printf("example_tick %d\n", *ctx);
    struct node *n = (struct node *)node;

    if (n->is_control) {
        if (n->control->type == ENTRY)
        {
            calls->failure(node_control(node));
            return;
        }
    }

    if (*ctx < 10) calls->running(node_control(node));
    else calls->success(node_control(node));
}

int main(int argc, char **argv)
{
    struct node_callbacks x = {
        .start = s,
        .finish = f,
        .tick = t
    };
    
    struct node *entry_node = control_node_create("root", NULL, ENTRY);

    struct node *repeater_node = control_node_create("repeater 1", entry_node, REPEATER);

    struct node *reg_node = node_create("leaf 1", NULL, repeater_node, NULL);
    struct node *reg_node2 = node_create("leaf 2", NULL, repeater_node, NULL);
    node_print(entry_node);
    node_print(repeater_node);
    return 0;
}