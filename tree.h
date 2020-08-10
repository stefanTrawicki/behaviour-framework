#if !defined(TREE_H)
#define TREE_H

#define run(p_node, p_func) (p_func(p_node))

struct functions;
struct callbacks;

typedef void (*function_t)(void *p_node);
typedef void (*callback_t)(void *p_node, void *p_subject, struct functions *p_funcs);

struct callbacks {
    callback_t tick;
    callback_t start;
    callback_t end;
};

struct functions {
    function_t running;
    function_t success;
    function_t failure;
};

// struct for node
struct node
{
    char* label;
    int is_control;
    char is_running;
    struct functions *fn;
    struct callbacks *cb;
    struct control_structure *control;

    struct node *parent;
    void *subject;
};

// enumeration for the control_structure types, loosely based on conventional composite and decorator nodes
enum node_type
{
    ENTRY = 0,
    SEQUENCE,
    SELECTOR,
    INVERTER,
    RANDOM,
    REPEATER,
    LEAF,
    _CONTROL_TYPE_COUNT // must be last to keep count of types
};

// some nodes are controllers, this structure contains that information
struct control_structure
{
    enum node_type type;
    int child_index;
    int child_count;
    function_t added_child;
    struct node **child_list;
    int repetitions;
};

struct behaviour_tree
{
    int is_running;
    struct node *root_node;
    struct node *current_node;
};


void behaviour_tree_initialiser();
void *behaviour_tree_create(void *p_root_node);
void *behaviour_tree_destruct(void *p_node);
void behaviour_tree_tick(void *p_behaviour_tree, void *p_subject);

void node_print(void *p_node);
void *node_create(const char *label, void *p_subject, void *p_parent_node, struct callbacks *p_cbs);
void *node_parent(void *p_node);
void node_add_child(void* p_node);
void *control_node_create(const char *label, void *p_parent_node, enum node_type type);
void set_repeater_limit(void *p_control_node, int limit);

void leaf_failure(void *p_node);
void leaf_success(void *p_node);
void leaf_running(void *p_node);

void entry_failure(void *p_node);
void entry_success(void *p_node);
void entry_running(void *p_node);

#endif // TREE_H