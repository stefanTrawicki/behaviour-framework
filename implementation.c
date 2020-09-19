#include <stdio.h>
#include "behaviour.h"

int tick(void *node_handle)
{
    FAIL(node_handle);
}

int main(int argc, char **argv)
{
    int test = 20;
    Node *i_arr[test];
    for (int i = 0; i < test; i++)
        i_arr[i] = behaviour_node_create(NT_INVERTER);

    Node *leaf = behaviour_node_create(NT_LEAF);
    behaviour_node_set_action(leaf, &tick);

    for (int i = 0; i < test-1; i++)
        behaviour_node_add_child(i_arr[i], i_arr[i+1]);
    
    behaviour_node_add_child(i_arr[test-1], leaf);

    printf("Tree evaluated to: %d\n", behaviour_tree_run(i_arr[0]));

    printf("Tree evaluated to: %d\n", behaviour_tree_run(i_arr[1]));


    return 0;
}