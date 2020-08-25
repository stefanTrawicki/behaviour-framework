#include "tree.h"

/*
    Project:  behaviour-framework
     Author:  Stefan Trawicki
       Date:  3rd August 2020
   Language:  Written in C, compiled in clang, debugged in lldb
 To Compile:  'make clean' to remove old files, 'make' to compile and 'make example' to run example code
Description:  Simple behaviour tree creation framework for later integration into a larger project
*/

void *_node_add_to_parent(void *p_node)
{
    node_t *parent = ((node_t *)p_node)->parent;
    control_structure_t *parent_c = parent->control;

    ASSERT_MSG(!(CHECK_FLAG(parent->flags, IS_CONTROL)), "Cannot add children to leaf node");

    parent_c->child_list = realloc(parent_c->child_list,
                                   parent_c->child_count + 1);

    parent_c->child_list[parent_c->child_count] = p_node;
    parent_c->child_count++;

    char node_data[100];
    sprintf(node_data, "added '%s' to '%s' as child %d", ((node_t *)p_node)->label,
            parent->label,
            parent->control->child_count);
    LOG(parent, node_data);

    node_t *node = p_node;
    node->tree = parent->tree;

    _behaviour_tree_add_child(node->tree, node);

    return (node_t *)p_node;
}

void *node_create(const char *label, void *p_parent_node, enum node_type type, uint8_t flags)
{
    node_t *node = malloc(sizeof(node_t));

    memset(node, 0, sizeof(node_t));

    node->state = UNINITIALISED;

    if (p_parent_node) node->parent = p_parent_node;

    if (type != ENTRY) {
        node->state_fns = &(((node_t *)p_parent_node)->tree->state_vtables[type]);
        node->tree = node->parent->tree;
    }

    node->flags = flags;
    node->type = type;

    if (label)
    {
        node->label = malloc(sizeof(char) * strlen(label));
        memset(node->label, 0, strlen(label));
        strcpy(node->label, label);
    }

    if (type != LEAF)
    {
        node->control = malloc(sizeof(control_structure_t));
        if (type != ENTRY) {
            node->action_fns = &(node->tree->action_vtables[type]);
        }
        SET_FLAG(node->flags, IS_CONTROL);
    }

    if (p_parent_node) _node_add_to_parent(node);

    if (type != ENTRY) {
        char node_data[100];
        sprintf(node_data, "created node '%s'", node->label);
        LOG(node, node_data);
    }

    return node;
}

void leaf_configure(void *p_node, void *p_subject, void *p_action_funcs)
{
    node_t *node = p_node;
    node->subject = p_subject;
    node->action_fns = p_action_funcs;

    char node_data[100];
    sprintf(node_data, "leaf '%s' subject = %p, actions = %p", node->label,
            node->subject,
            node->action_fns);
    LOG(node, node_data);
}

void entry_start(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "started entry node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->flags, IS_RUNNING);
}

void entry_stop(void *p_node)
{
    node_t *node = p_node;
    
    char node_data[100];
    sprintf(node_data, "stopped entry node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->tree->flags, IS_HALTED);
    SET_FLAG(node->flags, HAS_RAN);
    (node->state == SUCCESSFUL) ? SET_FLAG(node->tree->flags, END_STATE) : CLR_FLAG(node->tree->flags, END_STATE);
}

void entry_tick(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "ticked entry node '%s'", node->label);
    LOG(node, node_data);

    if (node->control->child_count > 0)
    {
        control_structure_t *c = node->control;
        if (c->child_list[0]->state == FAILED)
        {
            FAIL(node);
        }
        else if (c->child_list[0]->state == SUCCESSFUL)
        {
            PASS(node);
        }
        else if (c->child_index < c->child_count)
        {
            behaviour_tree_move(node->tree, c->child_list[0]);
            RUN(node);
        }
    }
    else
    {
        FAIL(node);
    }
}

void sequence_start(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "started sequence node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->flags, IS_RUNNING);
}

void sequence_stop(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "stopped sequence node '%s'", node->label);
    LOG(node, node_data);

    behaviour_tree_move(node->tree, node->parent);
}

void sequence_tick(void *p_node)
{
    node_t *node = p_node;
    
    char node_data[100];
    sprintf(node_data, "ticked sequence node '%s'", node->label);
    LOG(node, node_data);

    if (node->control->child_count > 0)
    {
        control_structure_t *c = node->control;

        int flag = 1;
        for (int i = 0; i < c->child_count; i++)
        {
            if (c->child_list[i]->state == FAILED)
            {
                FAIL(node);
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
                    PASS(node);
                }
            }
        }
        behaviour_tree_move(node->tree, c->child_list[c->child_index]);
        c->child_index++;
        RUN(node);
    }
    else RUN(node);
}

void fallback_start(void *p_node)
{
    node_t *node = p_node;
    
    char node_data[100];
    sprintf(node_data, "started fallback node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->flags, IS_RUNNING);
}

void fallback_stop(void *p_node)
{
    node_t *node = p_node;
    
    char node_data[100];
    sprintf(node_data, "stopped fallback node '%s'", node->label);
    LOG(node, node_data);

    behaviour_tree_move(node->tree, node->parent);
}

void fallback_tick(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "ticked fallback node '%s'", node->label);
    LOG(node, node_data);

    if (node->control->child_count > 0)
    {
        control_structure_t *c = node->control;

        int failed_children = 0;

        for (int i = 0; i < c->child_count; i++)
        {
            if (c->child_list[i]->state == SUCCESSFUL)
            {
                PASS(node);
            }
            else if (c->child_list[i]->state == FAILED)
            {
                failed_children++;
            }
        }
        if (failed_children == c->child_count)
        {
            FAIL(node);
        }
        behaviour_tree_move(node->tree, c->child_list[c->child_index]);
        c->child_index++;
        RUN(node);
    }
}

void inverter_start(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "started inverter node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->flags, IS_RUNNING);
}

void inverter_stop(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "stopped inverter node '%s'", node->label);
    LOG(node, node_data);

    behaviour_tree_move(node->tree, node->parent);
}

void inverter_tick(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "ticked inverter node '%s'", node->label);
    LOG(node, node_data);

    if (node->control->child_count > 0)
    {
        control_structure_t *c = node->control;
        if (c->child_list[0]->state == FAILED)
        {
            PASS(node);
        }
        else if (c->child_list[0]->state == SUCCESSFUL)
        {
            FAIL(node);
        }
        else if (c->child_list[0]->state == UNINITIALISED)
        {
            behaviour_tree_move(node->tree, c->child_list[0]);
            return;
        }
    }
    else
    {
        FAIL(node);
    }
}

void general_failure(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "failed leaf node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->flags, HAS_RAN);
    CLR_FLAG(node->flags, IS_RUNNING);
}

void general_success(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "successful leaf node '%s'", node->label);
    LOG(node, node_data);

    SET_FLAG(node->flags, HAS_RAN);
    CLR_FLAG(node->flags, IS_RUNNING);
}

void general_running(void *p_node)
{
    node_t *node = p_node;

    char node_data[100];
    sprintf(node_data, "running node '%s'", node->label);
    LOG(node, node_data);
}

void *behaviour_tree_create(const char *log_path, uint8_t flags)
{
    behaviour_tree_t *tree = malloc(sizeof(behaviour_tree_t));
    memset(tree, 0, sizeof(behaviour_tree_t));

    tree->node_c = 0;

    memset(tree->action_vtables, 0, sizeof(action_vtable_t) * _CONTROL_TYPE_COUNT);
    memset(tree->state_vtables, 0, sizeof(state_vtable_t) * _CONTROL_TYPE_COUNT);

    tree->flags = flags;
    SET_FLAG(tree->flags, IS_LOGGING);
    if (CHECK_FLAG(tree->flags, IS_LOGGING)) _behaviour_tree_enable_logging(tree, log_path);

    tree->action_vtables[ENTRY].start = entry_start;
    tree->action_vtables[ENTRY].stop = entry_stop;
    tree->action_vtables[ENTRY].tick = entry_tick;

    tree->action_vtables[SEQUENCE].start = sequence_start;
    tree->action_vtables[SEQUENCE].stop = sequence_stop;
    tree->action_vtables[SEQUENCE].tick = sequence_tick;

    tree->action_vtables[FALLBACK].start = fallback_start;
    tree->action_vtables[FALLBACK].stop = fallback_stop;
    tree->action_vtables[FALLBACK].tick = fallback_tick;

    tree->action_vtables[INVERTER].start = inverter_start;
    tree->action_vtables[INVERTER].stop = inverter_stop;
    tree->action_vtables[INVERTER].tick = inverter_tick;

    for (int i = 0; i < _CONTROL_TYPE_COUNT; i++)
    {
        tree->state_vtables[i].running = general_running;
        tree->state_vtables[i].failure = general_failure;
        tree->state_vtables[i].success = general_success;
    }

    return tree;
}

void _behaviour_tree_add_child(void *p_behaviour_tree, void *p_node) {
    behaviour_tree_t *tree = p_behaviour_tree;

    tree->nodes = realloc(tree->nodes, sizeof(node_t *) * (tree->node_c + 1));
    tree->nodes[tree->node_c] = p_node;
    tree->node_c++;
}

void behaviour_tree_reset(void *p_behaviour_tree) {
    behaviour_tree_t *tree = p_behaviour_tree;
    for(int i = 0; i < tree->node_c; i++) {
        node_t *n = tree->nodes[i];
        if (CHECK_FLAG(n->flags, IS_CONTROL)) n->control->child_index = 0;
        if (n->type == ENTRY) CLR_FLAG(n->flags, IS_HALTED);
        n->state = UNINITIALISED;
        CLR_FLAG(n->flags, HAS_RAN);
        CLR_FLAG(n->flags, IS_RUNNING);
    }
    CLR_FLAG(tree->flags, IS_HALTED);
}

void behaviour_tree_move(void *p_behaviour_tree, void *p_node)
{
    behaviour_tree_t *tree = p_behaviour_tree;
    tree->current_node = (node_t *)p_node;
}

void behaviour_tree_set_root(void *p_behaviour_tree, void *p_root_node)
{
    behaviour_tree_t *tree = p_behaviour_tree;

    tree->root_node = p_root_node;
    tree->root_node->tree = tree;

    tree->root_node->state_fns = &(tree->state_vtables[ENTRY]);
    tree->root_node->action_fns = &(tree->action_vtables[ENTRY]);

    behaviour_tree_move(tree, p_root_node);
    SET_FLAG(tree->flags, ROOT_SET);

    _behaviour_tree_add_child(tree, p_root_node);
}

void behaviour_tree_tick(void *p_behaviour_tree)
{
    behaviour_tree_t *tree = p_behaviour_tree;

    ASSERT_MSG(!CHECK_FLAG(tree->flags, ROOT_SET), "Error- No root node set for tree");

    node_t *c = tree->current_node;
    uint8_t f = tree->current_node->flags;

    if (!CHECK_FLAG(f, HAS_RAN))
    {
        if (!CHECK_FLAG(f, IS_RUNNING))
        {
            c->action_fns->start(c);
            c->action_fns->tick(c);
        }
        else
            c->action_fns->tick(c);
    }
    else
        c->action_fns->stop(c);
}

void _behaviour_tree_enable_logging(void *p_behaviour_tree, const char *path)
{
    behaviour_tree_t *tree = p_behaviour_tree;
    SET_FLAG(tree->flags, IS_LOGGING);
    tree->log_path = malloc(sizeof(char) * strlen(path));
    memset(tree->log_path, 0, strlen(path));
    strcpy(tree->log_path, path);
}