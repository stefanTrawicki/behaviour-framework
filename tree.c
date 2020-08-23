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

void node_running(void *p_node);
void node_failure(void *p_node);
void node_success(void *p_node);

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

    self->state = UNINITIALISED;

    self->parent = p_parent_node;
    self->subject = p_subject;

    self->is_finished = 0;

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
    ((struct node*)p_node)->bt = node->bt;
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
           "subject: %p\n"
           "behaviour_tree: %p\n",
           node->label,
           p_node,
           node->is_running,
           node->parent,
           (node->parent != NULL) ? node->parent->label : NULL,
           node->fn,
           node->cb,
           node->subject,
           (node->bt != NULL) ? node->bt : NULL);
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
                printf("label: %s\n", node->control->child_list[i]->label);
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

void *behaviour_tree_create(void *p_root_node)
{
    struct node *root_node = p_root_node;
    struct behaviour_tree *tree = malloc(sizeof(struct behaviour_tree));
    memset(tree, 0, sizeof(struct behaviour_tree));
    tree->current_node = root_node;
    tree->root_node = tree->current_node;
    tree->tree_started = 0;
    root_node->bt = tree;
    tree->halted = 0;

    return tree;
}

void behaviour_tree_set_current(void *p_behaviour_tree, void *p_node)
{
    struct behaviour_tree *tree = p_behaviour_tree;
    tree->current_node = (struct node *)p_node;
}

void behaviour_tree_tick(void *p_behaviour_tree)
{
    struct behaviour_tree *this = (struct behaviour_tree *)p_behaviour_tree;
    // printf("tree ticked\n");
    struct node *c = this->current_node;
    if (!c->is_finished)
    {
        if (!c->is_running)
        {
            c->cb->start(c);
            c->cb->tick(c);
        }
        else if (c->is_running)
            c->cb->tick(c);
    }
    else
    {
        c->cb->end(c);
        printf("\n");
    }
}

void node_failure(void *p_node)
{
    struct node *n = p_node;
    n->state = FAILED;
    n->fn->failure(p_node);
}

void node_success(void *p_node)
{
    struct node *n = p_node;
    n->state = SUCCESSFUL;
    n->fn->success(p_node);
}

void node_running(void *p_node)
{
    struct node *n = p_node;
    n->fn->running(p_node);
}

void entry_start(void *p_node)
{
    struct node *node = p_node;
    printf("%s started\n", node->label);
    node->is_running = 1;
}

void entry_end(void *p_node)
{
    struct node *node = p_node;
    printf("%s ended\n", node->label);
    node->bt->halted = 1;
    behaviour_tree_set_current(node->bt, node->parent);
}

void entry_tick(void *p_node)
{
    struct node *node = p_node;
    printf("%s ticked\n", node->label);

    if (node->control->child_count > 0)
    {
        struct control_structure *c = node->control;
        if (c->child_list[0]->state == FAILED)
        {
            node_failure(node);
            return;
        }
        else if (c->child_list[0]->state == SUCCESSFUL)
        {
            node_success(node);
            return;
        }
        else if (c->child_index < c->child_count)
        {
            behaviour_tree_set_current(node->bt, c->child_list[0]);
            node_running(node);
            return;
        }
    }
    else
    {
        node_failure(node);
        return;
    }
}

void sequence_start(void *p_node)
{
    struct node *node = p_node;
    printf("%s started\n", node->label);
    node->is_running = 1;
}

void sequence_end(void *p_node)
{
    struct node *node = p_node;
    printf("%s ended\n", node->label);
    behaviour_tree_set_current(node->bt, node->parent);
}

void sequence_tick(void *p_node)
{
    struct node *node = p_node;
    printf("%s ticked\n", node->label);
    if (node->control->child_count > 0)
    {
        struct control_structure *c = node->control;

        int flag = 1;
        for (int i = 0; i < c->child_count; i++)
        {
            if (c->child_list[i]->state == FAILED)
            {
                node_failure(node);
                return;
            }
            if (c->child_index == c->child_count)
            {
                for (int j = 0; j < c->child_count; j++)
                {
                    if (c->child_list[j]->state != SUCCESSFUL)
                        flag = 0;
                }
                if (flag)
                {
                    node_success(node);
                    return;
                }
            }
        }
        behaviour_tree_set_current(node->bt, c->child_list[c->child_index]);
        c->child_index++;
        node_running(node);
        return;
    }
    else
    {
        node_failure(node);
        return;
    }
}

void selector_start(void *p_node)
{
    struct node *node = p_node;
    printf("%s started\n", node->label);
    node->is_running = 1;
}

void selector_end(void *p_node)
{
    struct node *node = p_node;
    printf("%s ended\n", node->label);
    behaviour_tree_set_current(node->bt, node->parent);
}

void selector_tick(void *p_node)
{
    struct node *node = p_node;
    printf("%s ticked\n", node->label);
    if (node->control->child_count > 0)
    {
        struct control_structure *c = node->control;

        int failed_children = 0;

        for (int i = 0; i < c->child_count; i++)
        {
            if (c->child_list[i]->state == SUCCESSFUL)
            {
                node_success(node);
                return;
            }
            else if (c->child_list[i]->state == FAILED)
            {
                failed_children++;
            }
        }
        if (failed_children == c->child_count)
        {
            node_failure(node);
            return;
        }
        behaviour_tree_set_current(node->bt, c->child_list[c->child_index]);
        c->child_index++;
        node_running(node);
        return;
    }
}

void inverter_start(void *p_node)
{
    struct node *node = p_node;
    printf("%s started\n", node->label);
    node->is_running = 1;
}

void inverter_end(void *p_node)
{
    struct node *node = p_node;
    printf("%s ended\n", node->label);
    behaviour_tree_set_current(node->bt, node->parent);
}

void inverter_tick(void *p_node)
{
    struct node *node = p_node;
    if (node->control->child_count > 0)
    {
        struct control_structure *c = node->control;
        if (c->child_list[0]->state == FAILED)
        {
            node_success(node);
            return;
        }
        else if (c->child_list[0]->state == SUCCESSFUL)
        {
            node_failure(node);
            return;
        }
        else if (c->child_list[0]->state == UNINITIALISED)
        {
            behaviour_tree_set_current(node->bt, c->child_list[0]);
            return;
        }
    }
    else
    {
        node_failure(node);
    }
}

void leaf_failure(void *p_node)
{
    struct node *node = p_node;
    printf("%s failed\n", node->label);
    node->is_finished = 1;
    node->is_running = 0;
}

void leaf_success(void *p_node)
{
    struct node *node = p_node;
    printf("%s success\n", node->label);
    node->is_finished = 1;
    node->is_running = 0;
}

void leaf_running(void *p_node)
{
    struct node *node = p_node;
    printf("%s running\n", node->label);
}

void behaviour_tree_initialiser()
{
    node_cbs[ENTRY].start = &entry_start;
    node_cbs[ENTRY].end = &entry_end;
    node_cbs[ENTRY].tick = &entry_tick;

    node_cbs[SEQUENCE].start = &sequence_start;
    node_cbs[SEQUENCE].end = &sequence_end;
    node_cbs[SEQUENCE].tick = &sequence_tick;

    node_cbs[SELECTOR].start = &selector_start;
    node_cbs[SELECTOR].end = &selector_end;
    node_cbs[SELECTOR].tick = &selector_tick;

    node_cbs[INVERTER].start = &inverter_start;
    node_cbs[INVERTER].end = &inverter_end;
    node_cbs[INVERTER].tick = &inverter_tick;

    for (int i = 0; i < _CONTROL_TYPE_COUNT; i++)
    {
        node_fns[i].running = &leaf_running;
        node_fns[i].failure = &leaf_failure;
        node_fns[i].success = &leaf_success;
    }
}