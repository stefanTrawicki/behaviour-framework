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

static struct node_actions standard_node_actions;
static struct node_callbacks callback_masterlist[_CONTROL_TYPE_COUNT];

void *node_create(void *p_subject, void *p_parent_node, struct node_callbacks *p_node_callbacks)
{
    struct node *self = malloc(sizeof(struct node));
    memset(self, 0, sizeof(struct node));

    self->callbacks = p_node_callbacks;
    self->actions = &standard_node_actions;
    self->parent = p_parent_node;
    self->subject = p_subject;

    return self;
}

void *control_node_create(void *p_parent_node, struct node_callbacks *p_node_callbacks, enum control_type type)
{
    struct node *self = node_create(0, p_parent_node, p_node_callbacks);
    self->control = malloc(sizeof(struct control_structure));
    memset(self->control, 0, sizeof(struct control_structure));

    self->callbacks = &callback_masterlist[type];
    self->actions = &standard_node_actions;

    self->is_control = 1;
    self->control->type = type;
    self->parent = p_parent_node;

    return self;
}

void set_repeater_limit(void *p_node, int limit)
{
    struct node *n = (struct node *)p_node;
    struct control_structure *c = n->control;
    if (c->type == REPEATER)
    {
        c->repetitions = limit;
    }
}

void node_print(void *p_node)
{
    struct node *node = (struct node *)p_node;
    printf("\
        p_node: %p\nis_running: %d\n \
        parent: %p\nactions: %p\n \
        callbacks: %p\nsubject: %p\n",
           node, node->is_running,
           node->parent, node->actions,
           node->callbacks, node->subject);
    if (node->is_control)
    {
        printf("\
        type: %d\nchild_count: %d\n \
        child_index: %d\nrepetitions: %d\n",
               node->control->type, node->control->child_count,
               node->control->child_index, node->control->repetitions);
    }
}

void node_destruct(void *p_node)
{
    struct node *node = (struct node *)p_node;
    node->actions->destruct(node);
    return;
}

void *node_parent(void *p_node)
{
    struct node *node = (struct node *)p_node;
    return node->parent;
}
