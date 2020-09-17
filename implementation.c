#include <stdio.h>
#include "behaviour.h"

int tick_example_1(void *node_handle)
{
    printf("I did a task!\n");
    SUCCEED(node_handle);
}

int tick_example_2(void *node_handle)
{
    printf("I did a task!\n");
    SUCCEED(node_handle);
}

int decorator() {
    Node *n = behaviour_node_create(NT_REPEATER);
    behaviour_node_set_repetitions(n, 4);
    Node *i = behaviour_node_create(NT_INVERTER);
    Node *l = behaviour_node_create(NT_LEAF);

    behaviour_node_set_action(l, &tick_example_1);
    behaviour_node_add_child(i, n);
    behaviour_node_add_child(n, l);
    
    printf("Tree evaluated to: %d\n", behaviour_tree_run(i));

    while (behaviour_tree_tick(i) == -1)
    {
        behaviour_tree_tick(i);
    }
    printf("Tree evaluated to: %d\n", behaviour_tree_get_state(i));
    behaviour_tree_reset(i);

    printf("Tree evaluated to: %d\n", behaviour_tree_run(n));

    printf("Tree evaluated to: %d\n", behaviour_tree_run(l));

    return 0;
}

int composite() {
    Node *n = behaviour_node_create(NT_FALLBACK);
    Node *i = behaviour_node_create(NT_LEAF);
    Node *l = behaviour_node_create(NT_LEAF);

    behaviour_node_set_action(i, &tick_example_1);
    behaviour_node_set_action(l, &tick_example_2);
    behaviour_node_add_child(n, i);
    behaviour_node_add_child(n, l);
    
    printf("Tree evaluated to: %d\n", behaviour_tree_run(n));

    while (behaviour_tree_tick(i) == -1)
    {
        behaviour_tree_tick(i);
    }
    printf("Tree evaluated to: %d\n", behaviour_tree_get_state(i));
    behaviour_tree_reset(i);

    printf("Tree evaluated to: %d\n", behaviour_tree_run(i));

    printf("Tree evaluated to: %d\n", behaviour_tree_run(n));

    return 0;
}

int main()
{
    return composite();
}