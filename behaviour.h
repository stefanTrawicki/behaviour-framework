#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

#define RUN(node) {behaviour_node_external_run(node); return 1;}
#define FAIL(node) {behaviour_node_external_fail(node); return 1;}
#define SUCCEED(node) {behaviour_node_external_succeed(node); return 1;}

typedef enum
{
    NT_LEAF = 0,
    NT_FALLBACK,
    NT_SEQUENCE,
    NT_REPEATER,
    NT_INVERTER,
    NT_COUNT
} NodeType;

typedef struct n Node;
typedef int (*Action)(void *node_handle);

/* ------------------------- external tree functions ------------------------ */

extern int       behaviour_tree_reset(Node *root_node_handle);
extern int       behaviour_tree_tick(Node *root_node_handle);
extern int       behaviour_tree_get_state(Node *root_node_handle);
extern int       behaviour_tree_run(Node *root_node_handle);

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

#endif // !BEHAVIOUR_H
