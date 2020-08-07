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

// vtable for the running, success and failure actions of a branch
struct node_actions
{
    action running;
    action success;
    action failure;
    action destruct;
};

// common blocking callback for marking a response
typedef void (*callback)(void *p_node, void *subject, struct node_actions *p_callback_actions);

// vtable for the tick, start and failure actions of the branch
struct node_callbacks
{
    callback tick;
    callback start;
    callback finish;
};

// struct for node
struct node
{
    char* label;
    int is_control;
    char is_running;
    struct node_actions *actions;
    struct node_callbacks *callbacks;
    struct control_structure *control;

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
    REPEATER,
    _CONTROL_TYPE_COUNT // must be last to keep count of types
};

// some nodes are controllers, this structure contains that information
struct control_structure
{
    enum control_type type;
    int child_index;
    int child_count;
    action added_child;
    struct node **child_list;
    int repetitions;
};

struct behaviour_tree
{
    struct node *root_node;
    struct node *current_node;
};

void node_add_child(void* p_node);

void behaviour_tree_initialiser();
void *behaviour_tree_create(void *p_root_node);
void *behaviour_tree_tick(void *p_node);
void *behaviour_tree_delete(void *p_node);

void node_print(void *p_node);
void *node_create(const char *label, void *p_subject, void *p_parent_node, struct node_callbacks *p_node_callbacks);
void node_destruct(void *p_node);
void *node_parent(void *p_node);

void *control_node_create(const char *label, void *p_parent_node, enum control_type type);
void set_repeater_limit(void *p_control_node, int limit);

#endif // TREE_H