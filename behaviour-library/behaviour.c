#include "message_assertions_internal.h"
#include "behaviour_node_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TYPE_LABELS \
    (const char *[5]) { "Leaf", "Fallback", "Sequence", "Repeater", "Inverter" }

/* -------------------------------------------------------------------------- */
/*                       behaviour node standard actions                      */
/* -------------------------------------------------------------------------- */

extern int behaviour_node_internal_move_focus(void *node_handle)
{
    Node *node = (Node *)node_handle;
    if (!node->is_root_node)
    {
        ASSERT_MSG(node->root == NULL, "Cannot move root focus as root is unassigned");
    }
    else
    {
        node->currently_executing = node;
        return 1;
    }

    ((Node *)node->root)->currently_executing = node;
    return 1;
}

extern int behaviour_node_internal_standard_start(void *node_handle)
{
    Node *node = (Node *)node_handle;
    NodeType type = node->type;
    switch (type)
    {
    case NT_REPEATER:
        ASSERT_MSG(((RepeaterNode *)node)->starting_repetitions == 0, "Repeater starting repetitions must be >= 0 to execute");
    case NT_INVERTER:
        ASSERT_MSG(((DecoratorNode *)node)->child == NULL, "Cannot execute decorator node with no children");
        break;
    case NT_SEQUENCE:
    case NT_FALLBACK:
        ASSERT_MSG(((CompositeNode *)node)->child_count == 0, "Cannot execute composite node with no children");
        break;
    default:
        ASSERT_MSG(node->tick == NULL, "Cannot execute leaf node will unallocated tick function!");
        break;
    }
    node->state = NS_UNDETERMINED;
    return 1;
}

extern int behaviour_node_internal_decorator_tick(void *node_handle)
{
    Node *node = (Node *)node_handle;
    NodeType node_type = node->type;
    Node *child = ((DecoratorNode *)node)->child;
    if (child->state == NS_PENDING)
    {
        behaviour_node_internal_move_focus(child);
        return 0;
    }
    else if (node_type == NT_REPEATER && node->state == NS_UNDETERMINED)
    {
        if (((RepeaterNode *)node)->starting_repetitions == -1 ||
            ((RepeaterNode *)node)->repetitions > 1)
        {
            void *temp = child->root;
            behaviour_tree_reset(child);
            behaviour_node_internal_recursive_dispatcher(
                behaviour_node_internal_set_root,
                child,
                temp,
                NULL);
            behaviour_node_internal_move_focus(child);
            ((RepeaterNode *)node)->repetitions--;
            node->tick(node);
            return 1;
        }
        else
        {
            node->state = child->state;
            return 1;
        }
    }
    else
    {
        node->state = (child->state == NS_SUCCEEDED) ? NS_FAILED : NS_SUCCEEDED;
        return 1;
    }
    return -1;
}

extern int behaviour_node_internal_composite_tick(void *node_handle)
{
    Node *node = node_handle;
    CompositeNode *comp = node_handle;

    int i;
    for (i = 0; i < comp->child_count; i++)
    {
        Node *child = comp->children[i];

        if (child->state == NS_PENDING)
        {
            behaviour_node_internal_move_focus(child);
            return 1;
        }
        else if (child->state == NS_FAILED && node->type == NT_SEQUENCE)
        {
            node->state = NS_FAILED;
            return 1;
        }
        else if (child->state == NS_SUCCEEDED && node->type == NT_FALLBACK)
        {
            node->state = NS_SUCCEEDED;
            return 1;
        }
    }
    behaviour_node_internal_move_focus(node->parent);
    return node->type == NT_SEQUENCE;
}

/* -------------------------------------------------------------------------- */
/*                      behaviour node internal functions                     */
/* -------------------------------------------------------------------------- */

extern int behaviour_node_internal_is_next_child(CompositeNode *node_handle)
{
    return node_handle->current_child_index + 1 < node_handle->child_count;
}

extern Node *behaviour_node_internal_get_next_child(CompositeNode *node_handle)
{
    ASSERT_MSG(node_handle->child_count == 0, "Can only retrieve children if children are assigned to composite node");
    node_handle->current_child_index++;
    return node_handle->children[node_handle->current_child_index];
}

extern int behaviour_node_internal_reset_child_index(CompositeNode *node_handle)
{
    node_handle->current_child_index = -1;
    return 1;
}

extern int behaviour_node_internal_recursive_dispatcher(Job job_handle, Node *node_handle, void *param_v_1, void *param_v_2)
{
    job_handle(node_handle, param_v_1, param_v_1);
    switch (node_handle->type)
    {
    case NT_REPEATER:
    case NT_INVERTER:
        behaviour_node_internal_recursive_dispatcher(
            job_handle,
            ((DecoratorNode *)node_handle)->child,
            param_v_1,
            param_v_2);
        return 1;
    case NT_SEQUENCE:
    case NT_FALLBACK:
        while (behaviour_node_internal_is_next_child((CompositeNode *)node_handle))
        {
            behaviour_node_internal_recursive_dispatcher(
                job_handle,
                behaviour_node_internal_get_next_child((CompositeNode *)node_handle),
                param_v_1,
                param_v_2);
        }
        behaviour_node_internal_reset_child_index((CompositeNode *)node_handle);
        return 1;
    default:
        return 1;
    }
    return 0;
}

extern NodeState behaviour_node_internal_get_state(Node *node_handle)
{
    return node_handle->state;
}

extern Node *behaviour_node_internal_get_parent(Node *node_handle)
{
    return node_handle->parent;
}

extern int behaviour_node_internal_reset_state(Node *node_handle, void *a1, void *a2)
{
    node_handle->state = NS_PENDING;
    node_handle->is_root_node = 0;
    node_handle->root = NULL;
    node_handle->currently_executing = NULL;

    if (node_handle->type == NT_REPEATER)
        ((RepeaterNode *)node_handle)->repetitions = ((RepeaterNode *)node_handle)->starting_repetitions;
    return 1;
}

extern int behaviour_node_internal_set_root(Node *node_handle, void *root_node_handle, void *a2)
{
    node_handle->root = root_node_handle;
    return 1;
}

/* -------------------------------------------------------------------------- */
/*                      behaviour node external functions                     */
/* -------------------------------------------------------------------------- */

extern int behaviour_node_external_run(Node *node_handle)
{
    node_handle->state = NS_UNDETERMINED;
    node_handle->tick(node_handle);
    return 1;
}

extern int behaviour_node_external_fail(Node *node_handle)
{
    node_handle->state = NS_FAILED;
    return 1;
}

extern int behaviour_node_external_succeed(Node *node_handle)
{
    node_handle->state = NS_SUCCEEDED;
    return 1;
}

extern Node *behaviour_node_create(NodeType type)
{
    Node *return_node;

    ASSERT_MSG(type == NT_COUNT, "Cannot assign node of type count, utility enumeration only");

    if (type == NT_LEAF)
    {
        return_node = malloc(sizeof(LeafNode));
    }
    else if (type == NT_INVERTER || type == NT_REPEATER)
    {
        if (type == NT_REPEATER)
        {
            return_node = malloc(sizeof(RepeaterNode));
            ((RepeaterNode *)return_node)->starting_repetitions = 0;
            ((RepeaterNode *)return_node)->repetitions = 0;
        }
        else
            return_node = malloc(sizeof(DecoratorNode));
        return_node->tick = behaviour_node_internal_decorator_tick;
    }
    else
    {
        return_node = malloc(sizeof(CompositeNode));
        ((CompositeNode *)return_node)->child_count = 0;
        ((CompositeNode *)return_node)->current_child_index = -1;
        return_node->tick = behaviour_node_internal_composite_tick;
    }
    return_node->type = type;
    return_node->is_root_node = 0;
    return_node->state = NS_PENDING;
    return_node->start = behaviour_node_internal_standard_start;
    return return_node;
}

extern int behaviour_node_add_child(Node *parent_node_handle, Node *child_node_handle)
{
    ASSERT_MSG(parent_node_handle->type == NT_LEAF, "Cannot add a child to a leaf node");

    if (parent_node_handle->type == NT_REPEATER || parent_node_handle->type == NT_INVERTER)
    {
        ASSERT_MSG(((DecoratorNode *)parent_node_handle)->child != NULL, "Decorator nodes cannot have > 1 child");
        ((DecoratorNode *)parent_node_handle)->child = child_node_handle;
    }
    else
    {
        CompositeNode *composite = (CompositeNode *)parent_node_handle;
        if (composite->child_count % COMPOSITE_NODE_ARRAY_BUFFER_INCREMENT == 0)
        {
            Node **temp = realloc(composite->children,
                                  composite->child_count + COMPOSITE_NODE_ARRAY_BUFFER_INCREMENT);
            ASSERT_MSG(temp == NULL, "Composite node memory allocation failed");
            free(composite->children);
            composite->children = temp;
        }
        composite->child_count++;
        composite->children[composite->child_count - 1] = child_node_handle;
    }
    child_node_handle->parent = parent_node_handle;
    return 1;
}

extern int behaviour_node_set_start(Node *node_handle, Action start_action_handle)
{
    ASSERT_MSG(node_handle->type != NT_LEAF, "Only leaf nodes can have start actions configured");
    ((LeafNode *)node_handle)->configured_start = start_action_handle;
    return 1;
}

extern int behaviour_node_set_action(Node *node_handle, Action tick_action_handle)
{
    ASSERT_MSG(node_handle->type != NT_LEAF, "Only leaf nodes can have tick actions configured");
    node_handle->tick = tick_action_handle;
    return 1;
}

extern int behaviour_node_set_stop(Node *node_handle, Action stop_action_handle)
{
    ASSERT_MSG(node_handle->type != NT_LEAF, "Only leaf nodes can have stop actions configured");
    ((LeafNode *)node_handle)->configured_stop = stop_action_handle;
    return 1;
}

extern int behaviour_node_set_label(Node *node_handle, char *node_label, int node_label_length)
{
    if (node_handle->label != NULL)
        free(node_handle->label);

    node_handle->label = malloc((sizeof *node_label) * node_label_length);
    memcpy(node_handle->label, node_label, node_label_length);
    return node_label_length;
}

extern int behaviour_node_set_subject(Node *node_handle, void *subject_handle)
{
    ASSERT_MSG(node_handle->type != NT_LEAF, "Only leaf nodes can have a subject assigned!");
    ((LeafNode *)node_handle)->subject = subject_handle;
    return 1;
}

extern void * behaviour_node_get_subject(Node *node_handle)
{
    ASSERT_MSG(node_handle->type != NT_LEAF, "Non-leaf nodes have no subject");
    ASSERT_MSG(((LeafNode *)node_handle)->subject == NULL, "Node subject is null");
    return ((LeafNode *)node_handle)->subject;
}

extern void * behaviour_node_get_blackboard(Node *node_handle)
{
    ASSERT_MSG(node_handle->type != NT_LEAF, "Non-leaf nodes have no blackboard");
    ASSERT_MSG(((LeafNode *)node_handle)->blackboard == NULL, "Node blackboard is null");
    return ((LeafNode *)node_handle)->blackboard;
}


extern int behaviour_node_set_repetitions(Node *node_handle, int repetitions)
{
    ASSERT_MSG(node_handle->type != NT_REPEATER, "Only repeater nodes can have repetitions configured");
    ((RepeaterNode *)node_handle)->repetitions = repetitions;
    ((RepeaterNode *)node_handle)->starting_repetitions = repetitions;
    return 1;
}

extern int behaviour_node_get_information(Node *node_handle)
{
    printf("Type: %s\nParent: %p\nRoot: %p\nCurrently executing: %p\nIs root: %d\nState: %d\nStart: %p\nTick: %p\nLabel: %s\n",
           TYPE_LABELS[node_handle->type],
           node_handle->parent,
           node_handle->root,
           node_handle->currently_executing,
           node_handle->is_root_node,
           node_handle->state,
           node_handle->start,
           node_handle->tick,
           node_handle->label ? node_handle->label : "n/a");

    switch (node_handle->type)
    {
    case NT_LEAF:
        printf("Conf_start: %p\nConf_stop: %p\nSubject: %p\n\n",
               ((LeafNode *)node_handle)->configured_start,
               ((LeafNode *)node_handle)->configured_stop,
               ((LeafNode *)node_handle)->subject);
        return 1;
    case NT_REPEATER:
        printf("Starting repetitions: %d\nRepetitions: %d\nChild: %p\n\n",
               ((RepeaterNode *)node_handle)->starting_repetitions,
               ((RepeaterNode *)node_handle)->repetitions,
               ((DecoratorNode *)node_handle)->child);
        return 1;
    case NT_INVERTER:
        printf("Child: %p\n\n",
               ((DecoratorNode *)node_handle)->child);
        return 1;
    case NT_FALLBACK:
    case NT_SEQUENCE:
        printf("Child_count: %d\nCurrent_index: %d\n\n",
               ((CompositeNode *)node_handle)->child_count,
               ((CompositeNode *)node_handle)->current_child_index);
        return 1;
    default:
        printf("\n");
        return 0;
    }
}

/* -------------------------------------------------------------------------- */
/*                      behaviour tree external functions                     */
/* -------------------------------------------------------------------------- */

extern int behaviour_tree_reset(Node *root_node_handle)
{
    behaviour_node_internal_recursive_dispatcher(
        behaviour_node_internal_reset_state,
        root_node_handle,
        NULL,
        NULL);
    return 1;
}

extern int behaviour_tree_tick(Node *root_node_handle)
{
    if (root_node_handle->state == NS_UNDETERMINED)
    {
        Node *focus = root_node_handle->currently_executing;

        switch (focus->state)
        {
        case NS_PENDING:
            focus->start(focus);
            if (focus->type == NT_LEAF)
            {
                if (((LeafNode *)focus)->configured_start != NULL)
                    ((LeafNode *)focus)->configured_start(focus);
            }
            break;
        case NS_UNDETERMINED:
            focus->tick(focus);
            break;
        default:
            if (focus->type == NT_LEAF)
            {
                if (((LeafNode *)focus)->configured_stop != NULL)
                    ((LeafNode *)focus)->configured_stop(focus);
            }
            if (!focus->is_root_node)
            {
                behaviour_node_internal_move_focus(focus->parent);
            }
            break;
        }
    }
    else if (root_node_handle->state == NS_PENDING)
    {
        behaviour_tree_reset(root_node_handle);
        behaviour_node_internal_recursive_dispatcher(
            behaviour_node_internal_set_root,
            root_node_handle,
            root_node_handle,
            NULL);
        root_node_handle->is_root_node = 1;
        root_node_handle->start(root_node_handle);
        behaviour_node_internal_move_focus(root_node_handle);
    }
    return behaviour_tree_get_state(root_node_handle);
}

extern int behaviour_tree_get_state(Node *root_node_handle)
{
    if (root_node_handle->state == NS_SUCCEEDED)
        return 1;
    else if (root_node_handle->state == NS_FAILED)
        return 0;
    else
        return -1;
}

extern int behaviour_tree_run(Node *root_node_handle)
{
    while (behaviour_tree_get_state(root_node_handle) == -1)
    {
        behaviour_tree_tick(root_node_handle);
    }
    int evaluation = behaviour_tree_get_state(root_node_handle);
    behaviour_tree_reset(root_node_handle);
    return evaluation;
}

extern int behaviour_node_set_blackboard(Node *node_handle, void *blackboard_handle)
{
    ASSERT_MSG(node_handle->type != NT_LEAF, "Only leaf nodes can have a blackboard assigned!");
    ((LeafNode *)node_handle)->blackboard = blackboard_handle;
    return 1;
}