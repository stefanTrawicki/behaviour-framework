#ifndef BEHAVIOUR_INTERNAL_H
#define BEHAVIOUR_INTERNAL_H

#define COMPOSITE_NODE_ARRAY_BUFFER_INCREMENT 4

typedef int (*Action)(void *node_handle);

typedef enum
{
    NT_LEAF,
    NT_FALLBACK,
    NT_SEQUENCE,
    NT_REPEATER,
    NT_INVERTER,
    NT_COUNT
} NodeType;

typedef enum
{
    NS_FAILED = -1,
    NS_SUCCEEDED,
    NS_PENDING,
    NS_UNDETERMINED
} NodeState;

typedef struct nodehead{
    NodeType type;
    void *parent;
    void *root;
    void *currently_executing;
    int is_root_node;
    NodeState state;
    Action start;
    Action tick;
    char *label;
}Node;

typedef struct leafnode_t{
    Node head;
    Action configured_start;
    Action configured_stop;
    void *subject;
    void *blackboard;
}LeafNode;

typedef struct decoratornode_t{
    Node head;
    Node *child;
}DecoratorNode;

typedef struct repeaternode_t{
    Node head;
    void *spacer; //When casting repeater to decorator, child is overwritten. This prevents that.
    unsigned int repetitions;
    unsigned int starting_repetitions;
}RepeaterNode;

typedef struct compositenode_t{
    Node head;
    int child_count;
    int current_child_index;
    Node **children;
}CompositeNode;

/* --------------------------- internal functions --------------------------- */

extern int       behaviour_node_internal_is_next_child(CompositeNode *node_handle);
extern Node *    behaviour_node_internal_get_next_child(CompositeNode *node_handle);
extern int       behaviour_node_internal_reset_child_index(CompositeNode *node_handle);

typedef int (*Job)(Node *node_handle, void *param_v_1, void *param_v_2);
extern int       behaviour_node_internal_recursive_dispatcher(Job job_handle, Node *node_handle, void *param_v_1, void *param_v_2);
extern NodeState behaviour_node_internal_get_state(Node *node_handle);
extern Node *    behaviour_node_internal_get_parent(Node *node_handle);
extern int       behaviour_node_internal_reset_state(Node *node_handle, void *a1, void *a2);
extern int       behaviour_node_internal_set_root(Node *node_handle, void *root_node_handle, void *a2);

extern int       behaviour_node_internal_move_focus(void *node_handle);
extern int       behaviour_node_internal_standard_start(void *node_handle);
extern int       behaviour_node_internal_decorator_tick(void *node_handle);
extern int       behaviour_node_internal_composite_tick(void *node_handle);

/* ------------------------- external tree functions ------------------------ */

extern int       behaviour_tree_reset(Node *root_node_handle);
extern int       behaviour_tree_tick(Node *root_node_handle);
extern int       behaviour_tree_get_state(Node *root_node_handle);
extern int       behaviour_tree_run(Node *root_node_handle);

/* ------------------------- internal tree functions ------------------------ */

extern int       behaviour_tree_internal_root_reset(Node *root_node_handle);

/* ------------------------- external node functions ------------------------ */

extern int       behaviour_node_external_run(Node *node_handle);
extern int       behaviour_node_external_fail(Node *node_handle);
extern int       behaviour_node_external_succeed(Node *node_handle);

extern Node *    behaviour_node_create(NodeType type);
extern int       behaviour_node_add_child(Node *parent_node_handle, Node *child_node_handle);
extern int       behaviour_node_set_start(Node *node_handle, Action start_action_handle);
extern int       behaviour_node_set_action(Node *node_handle, Action tick_action_handle);
extern int       behaviour_node_set_stop(Node *node_handle, Action stop_action_handle);
extern int       behaviour_node_set_label(Node *node_handle, char *node_label, int node_label_length);
extern int       behaviour_node_set_subject(Node *node_handle, void *subject_handle);
extern int       behaviour_node_set_blackboard(Node *node_handle, void *blackboard_handle);
extern void *    behaviour_node_get_subject(Node *node_handle);
extern void *    behaviour_node_get_blackboard(Node *node_handle);
extern int       behaviour_node_set_repetitions(Node *node_handle, int repetitions);
extern int       behaviour_node_get_information(Node *node_handle);

#endif // !BEHAVIOUR_INTERNAL_H