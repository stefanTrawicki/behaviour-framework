# Behaviour

Behaviour is a C library for creating and executing behaviour trees, primarily for use in video games.

## Installation

Use the included 'make install' to install Behaviour.

```bash
make install
```

## Usage
Here's an example of a very simple fallback tree. The entry point in this case is the fallback node itself, but any node can be the target.

```c
#include <behaviour.h>

int tick_example_1(void *node_handle)
 {
     printf("I did a task!\n");
     SUCCEED(node_handle);
 }

 int tick_example_2(void *node_handle)
 {
     printf("I did a task!\n");
     FAIL(node_handle);
 }

int main(int argc, char **argv)
{
     Node *n = behaviour_node_create(NT_FALLBACK);
     Node *i = behaviour_node_create(NT_LEAF);
     Node *l = behaviour_node_create(NT_LEAF);

     behaviour_node_set_action(i, &tick_example_1);
     behaviour_node_set_action(l, &tick_example_2);
     behaviour_node_add_child(n, i);
     behaviour_node_add_child(n, l);

     printf("Tree evaluated to: %d\n", behaviour_tree_run(n));

     return 0;
}
```