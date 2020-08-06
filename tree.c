#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Project:  behaviour-framework
     Author:  Stefan Trawicki
       Date:  3rd August 2020
   Language:  Written in C, compiled in clang, debugged in lldb
 To Compile:  'make clean' to remove old files, 'make' to compile and 'make example' to run example code
Description:  Simple behaviour tree creation framework for later integration into a larger project
*/

static struct node_actions std_node_act;
static struct node_actions ctrl_act_ml[_CONTROL_TYPE_COUNT];
static struct node_callbacks ctrl_cb_ml[_CONTROL_TYPE_COUNT];

/*
    * Creates a standard leaf node in the system, no control characteristics
    * or children.
    * 
    * inputs:
    * const char *label- a friendly name for the node when debugging.
    * void *p_subject- pointer to the nodes subject, typically a player or NPC.
    * void *p_parent_node- pointer to the parent node, a non-leaf type.
    * struct node_callbacks *p_node_callbacks- a structure containing the fp's to
    * call when ticking, starting and finishing the node, the leaf nodes functionality.
    * 
    * returns:
    * pointer to the node created.
*/
void *node_create(const char *label, void *p_subject, void *p_parent_node, struct node_callbacks *p_node_callbacks)
{
    struct node *self = malloc(sizeof(struct node));
    memset(self, 0, sizeof(struct node));
    if (label) {
        self->label = malloc(sizeof(char) * strlen(label));
        strcpy(self->label, label);
    }
    self->callbacks = p_node_callbacks;
    self->actions = &std_node_act;
    self->parent = p_parent_node;
    self->subject = p_subject;

    if (p_parent_node != NULL) self->parent->control->added_child(self);
    return self;
}

/*
    * fp called by the controller node to add a child to the tree, based
    * on the parent of the parameter.
    * 
    * inputs:
    * void* p_node- the node to add as a child.
*/
void node_add_child(void* p_node) {
    struct node *node = ((struct node *)p_node)->parent;
    struct control_structure *c_s = node->control;
    c_s->child_count++;
    c_s->child_list = realloc(c_s->child_list, c_s->child_count);
    c_s->child_list[c_s->child_count-1] = (struct node *)p_node;
}

/*
    * Creates a control node (i.e a composite or decorator node), and assign
    * its type. Doesn't require a subject as a control node doesn't manipulate
    * the subjects state. Doesn't require callbacks because they are assigned
    * by the behaviour_tree_initialiser function.
    * 
    * inputs:
    * const char *label- a friendly name for the node when debugging.
    * void *p_parent_node- pointer to the parent node, a non-leaf type.
    * enum control_type type- the type of control node, assigns the callbacks
    * based on the type.
    * 
    * returns:
    * pointer to the control node created
*/
void *control_node_create(const char *label, void *p_parent_node, enum control_type type)
{
    struct node *self = node_create(label, 0, p_parent_node, &ctrl_cb_ml[type]);
    self->control = malloc(sizeof(struct control_structure));
    memset(self->control, 0, sizeof(struct control_structure));
    self->actions = &ctrl_act_ml[type];
    self->is_control = 1;
    self->control->type = type;
    self->control->added_child = node_add_child;
    return self;
}

/*
    * Sets the limit of a repeating type, 0 for infinite.
    * 
    * inputs:
    * void *p_node- pointer to the control node to modify.
    * int limit- the limit of repetitions to for the repeater node.
*/
void set_repeater_limit(void *p_node, int limit)
{
    struct node *n = (struct node *)p_node;
    struct control_structure *c = n->control;
    if (c->type == REPEATER)
    {
        c->repetitions = limit;
    }
}

/*
    * Prints out debug information for the node passed. If a control
    * node, prints out the extra information and any children.
    * 
    * inputs:
    * void *p_node- the node to print information for.
*/
void node_print(void *p_node)
{
    struct node *node = (struct node *)p_node;
    printf("label: '%s'\n"
           "p_node: %p\n"
           "is_running: %d\n"
           "parent: %p\n"
           "parent label: '%s'\n"
           "actions: %p\n"
           "callbacks: %p\n"
           "subject: %p\n",
           node->label, node,
           node->is_running, node->parent,
           (node->parent != NULL) ? node->parent->label : NULL, node->actions,
           node->callbacks, node->subject);
    if (node->is_control)
    {
        char* types[_CONTROL_TYPE_COUNT] = {"ENTRY", "SEQUENCE", "SELECTOR", "INVERTER", "RANDOM", "REPEATER"};
        printf("type: %s\n"
               "child_count: %d\n"
               "child_index: %d\n"
               "repetitions: %d\n",
               types[node->control->type], node->control->child_count,
               node->control->child_index, node->control->repetitions);

        if (node->control->child_count > 0) {
            printf("Children:\n");
            for (size_t i = 0; i < node->control->child_count; i++)
            {
                printf("\tlabel: %s %c", node->control->child_list[i]->label,
                (i+1 == node->control->child_count || i % 3 == 0) ? '\n':'\t');
            }
        }
        
    }
    printf("\n");
}

/*
    * Call to delete the node, will likely be assigned 'free'.
    * 
    * inputs:
    * void *p_node- the node to delete
*/
void node_destruct(void *p_node)
{
    struct node *node = (struct node *)p_node;
    node->actions->destruct(node);
    return;
}


/*
    * Gets the parent of the node so we don't have to cast anything.
    * 
    * inputs:
    * void *p_node- the node to return the parent of.
    * 
    * returns:
    * pointer to p_node's parent.
*/
void *node_parent(void *p_node)
{
    struct node *node = (struct node *)p_node;
    return node->parent;
}