#include "tree.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    struct node *entry_node = control_node_create("root", NULL, ENTRY);

    struct node *repeater_node = control_node_create("repeater 1", entry_node, REPEATER);

    struct node *reg_node = node_create("leaf 1", NULL, repeater_node, NULL);
    struct node *reg_node2 = node_create("leaf 2", NULL, repeater_node, NULL);
    node_print(entry_node);
    node_print(repeater_node);
    return 0;
}