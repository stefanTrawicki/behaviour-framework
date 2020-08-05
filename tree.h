/*
        Project:  behaviour-framework
        Author:  Stefan Trawicki
        Date:  3rd August 2020
    Language:  Written in C, compiled in clang, debugged in lldb
    To Compile:  'make clean' to remove old files, 'make' to compile and 'make example' to run example code
    Description:  Simple behaviour tree creation framework for later integration into a larger project
*/

#if !defined(TREE_H)
#define TREE_H

// macros for calling callback or operation functions quicky and efficiently
#define call(p_node, callback)                                    \
    do                                                            \
    {                                                             \
        p_node->cbs->callback(p_node, p_node->parent->callbacks); \
    } while (0)
#define act(p_node, action)              \
    do                                   \
    {                                    \
        p_node->actions->action(p_node); \
    } while (0)

// common void-void vfunc, normally for node actions
typedef void (*action)(void *p_node);

// common blocking callback for marking a response
typedef void (*callback)(void *p_node, struct node_callbacks *p_callbacks);

// vtable for the running, success and failure actions of a branch
struct node_actions
{
    action running;
    action success;
    action failure;
    action destruct;
};

// vtable for the tick, enter and failure actions of the branch
struct node_callbacks
{
    callback tick;
    callback enter;
    callback exit;
};

// struct for node
struct node
{
    int is_control;
    struct control_structure control;
    char is_running;

    struct node_actions *actions;
    struct node_callbacks *callbacks;
    struct node *parent;
    void *subject;
};

// enumeration for the control_structure types, loosely based on conventional composite and decorator nodes
enum control_type
{
    ENTRY = 0,
    SEQUENCE,
    SELECTOR,
    INVERTER,
    RANDOM,
    PRIORITY,
    REPEATER_INF,
    REPEATER_FIN
};

// some nodes are controllers, this structure contains that information
struct control_structure
{
    enum control_type type;
    int child_index;
    int child_count;
    struct node **child_list;
};

struct behaviour_tree
{
    struct node root_node;
};

// functions for tree creation/control/destruction
void behaviour_tree_initialiser(); // sets the default callbacks and actions of the tree, as defined in tree.c
void *behaviour_tree_create(void *p_node); // p_node points to entry or "root" node of the tree
void *behaviour_tree_tick(void *p_node); // same as above
void *behaviour_tree_delete(void *p_node); // same as above

// functions for node creation/control/destruction
void *node_create(void *subject, struct node_callbacks *p_node_callbacks); // returns a node pointer and it's callbacks
void node_destruct(void *p_node); // will call the nodes destruct action to remove it
void *node_parent(void *p_node); // gets the nodes parent or controller


#endif // TREE_H