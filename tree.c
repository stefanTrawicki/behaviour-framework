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

static struct functions node_fns[_CONTROL_TYPE_COUNT];
static struct callbacks node_cbs[_CONTROL_TYPE_COUNT];

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
void *node_create(const char *label, void *p_subject, void *p_parent_node, struct callbacks *p_cbs)
{
    struct node *self = malloc(sizeof(struct node));
    memset(self, 0, sizeof(struct node));
    if (label) {
        self->label = malloc(sizeof(char) * strlen(label));
        strcpy(self->label, label);
    }
    self->fn = &node_fns[LEAF];
    self->cb = p_cbs;

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
void *control_node_create(const char *label, void *p_parent_node, enum node_type type)
{
    struct node *self = node_create(label, 0, p_parent_node, &node_cbs[type]);
    self->control = malloc(sizeof(struct control_structure));
    memset(self->control, 0, sizeof(struct control_structure));
    self->fn = &node_fns[type];
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
           (node->parent != NULL) ? node->parent->label : NULL, node->cb,
           node->cb, node->subject);
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

void leaf_failure(void *p_node) {
    printf("node failed (%s)\n", ((struct node *)p_node)->label);
}
void leaf_success(void *p_node) {
    printf("node succeeded (%s)\n", ((struct node *)p_node)->label);
}
void leaf_running(void *p_node) {
    printf("node running (%s)\n", ((struct node *)p_node)->label);
}

void entry_failure(void *p_node) {
    printf("entry failed (%s)\n", ((struct node *)p_node)->label);
}
void entry_success(void *p_node) {
    printf("entry succeeded (%s)\n", ((struct node *)p_node)->label);
}
void entry_running(void *p_node) {
    printf("entry running (%s)\n", ((struct node *)p_node)->label);
}
void entry_start(void *p_node, void *p_subject, struct functions *p_funcs) {
    printf("entry started (%s)\n", ((struct node *)p_node)->label);
}
void entry_end(void *p_node, void *p_subject, struct functions *p_funcs) {
    printf("entry ended (%s)\n", ((struct node *)p_node)->label);
}
void entry_tick(void *p_node, void *p_subject, struct functions *p_funcs) {
    printf("entry ticked (%s)\n", ((struct node *)p_node)->label);
}

void *behaviour_tree_create(void *p_root_node)
{
    struct behaviour_tree *tree = malloc(sizeof(struct behaviour_tree));
    memset(tree, 0, sizeof(struct behaviour_tree));
    tree->current_node = (struct node *)p_root_node;
    tree->root_node = tree->current_node;
    tree->is_running = 0;

    return tree;
}

void behaviour_tree_tick(void *p_behaviour_tree, void *p_subject) {

    struct behaviour_tree *this = (struct behaviour_tree *)p_behaviour_tree;
    if (this->is_running)
    {
        if (this->current_node->parent) {
            this->current_node->parent->fn->running(this->current_node->parent);
        }
    }
    else
    {
        this->is_running = 1;
        this->current_node->subject = p_subject;
        this->root_node->cb->start(this->root_node, p_subject, this->root_node->fn);
        this->root_node->cb->tick(this->root_node, p_subject, this->root_node->fn);
    }
}

void behaviour_tree_initialiser() {
    node_fns[LEAF].failure = &leaf_failure;
    node_fns[LEAF].running = &leaf_running;
    node_fns[LEAF].success = &leaf_success;

    node_fns[ENTRY].failure = &entry_failure;
    node_fns[ENTRY].running = &entry_running;
    node_fns[ENTRY].success = &entry_success;

    node_cbs[ENTRY].start = &entry_start;
    node_cbs[ENTRY].end = &entry_end;
    node_cbs[ENTRY].tick = &entry_tick;
}