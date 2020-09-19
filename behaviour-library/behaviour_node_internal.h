#ifndef BEHAVIOUR_INTERNAL_H
#define BEHAVIOUR_INTERNAL_H

/*
    When adding children to composites, we allocate in blocks of 4 to reduce calls realloc.
    */
#define COMPOSITE_NODE_ARRAY_BUFFER_INCREMENT 4

/*
    Action: Function pointer for node actions.
        Tick, start and stop are examples of this.
    
    Job: Function pointer for a recursive job.
        These are used with the internal_recursive_job_dispatcher. The job function is called
        on a node and all of its children. Currently used with reset and set root.
        Massively simplifies and DRY's the code involved.
     */
typedef int (*Action)(void *node_handle);

/*
    Enumeration of the various types of node.
        Gives a nice label for the user.
    */
typedef enum
{
    NT_LEAF,
    NT_FALLBACK,
    NT_SEQUENCE,
    NT_REPEATER,
    NT_INVERTER,
    NT_COUNT
} NodeType;

/* 
    Enumeration of the different node states.
        Used for switching internally depending on the state of a node.
    */
typedef enum
{
    NS_FAILED = -1,
    NS_SUCCEEDED,
    NS_PENDING,
    NS_UNDETERMINED
} NodeState;

/* 
    Structure for the head of a node. Essentially the base class of all nodes.
    All other nodes include this header as a commonality that they can be casted as.

        type- the type of node (leaf, repeater, sequence etc).
        *parent- pointer to this nodes parent.
        *root- the root node of the tree thats executing. Used to set the focus of the tree root.
        *currently_executing- if the node is a root, the node it's currently executing the start()
            or tick() function for will be here.
        is_root_node- flag telling us whether this node is the root of the tree.
        state- the state of the tree.
        start- function pointer to the main start function of this node.
        tick- function pointer to the tick action of this node.
        label- a label used for printing out node information and eventually logging.
    */
typedef struct nodehead
{
    NodeType type;
    void *parent;
    void *root;
    void *currently_executing;
    int is_root_node;
    NodeState state;
    Action start;
    Action tick;
    char *label;
} Node;

/* 
    Structure for the leaf node. "inherits" the head structure of the node, and adds some extra leaf-specific fields:
        head- the base class of the leaf node.
        configured_start- leaf nodes can be configured with extra start function, for setting up pre-conditions etc.
        configured_stop- leaves can also have stop functions, to free() subjects or delete characters for example.
        *subject- pointer to the subject of a leaf.
        *blackboard- pointer to a common set of data shared between nodes, i.e player position etc.
    */
typedef struct leafnode_t
{
    Node head;
    Action configured_start;
    Action configured_stop;
    void *subject;
    void *blackboard;
} LeafNode;

/*
    The structure of a decorator node. This is part of the repeater and inverter nodes.
        head- base class of the decorator nodes.
        *child- pointer to the child of the node. Decorators only have 1 child.
    */
typedef struct decoratornode_t
{
    Node head;
    Node *child;
} DecoratorNode;

/*
    Structure of the repeater node, is a decorator node but also stores repetitions.
        head- base class of the repeater node.
        repetitions- the number of repetitions remaining on a repeater node.
        starting repetitions- the number of repetitions a node started with. When a repeater node itself is reset,
            the repetitions are reset to this value.
    */
typedef struct repeaternode_t
{
    Node head;
    void *spacer; //When casting repeater to decorator, child is overwritten. This prevents that.
    unsigned int repetitions;
    unsigned int starting_repetitions;
} RepeaterNode;

/*
    Structure of a composite node, identical between fallback and sequence.
        child_count- number of children a node has. Used for space allocation, iteration and assertions.
        current_child_index- when iterating over the internal child array, this keeps track of where it's up to.
        **children- the internal child array.
    */
typedef struct compositenode_t
{
    Node head;
    int child_count;
    int current_child_index;
    Node **children;
} CompositeNode;

typedef int (*Job)(Node *node_handle, void *param_v_1, void *param_v_2);

/* --------------------------- internal functions --------------------------- */

// takes a composite node and returns 1 if there is another child in its array, 0 if not.
extern int       behaviour_node_internal_is_next_child(CompositeNode *node_handle);
// takes a composite node and returns the next child in its array.
extern Node *    behaviour_node_internal_get_next_child(CompositeNode *node_handle);
// takes a composite node and resets its index to -1 (the start).
extern int       behaviour_node_internal_reset_child_index(CompositeNode *node_handle);

// takes a job function, a target node, and 2 variable void pointers for arguments. Runs the job on the node and its subtree.
extern int       behaviour_node_internal_recursive_dispatcher(Job job_handle, Node *node_handle, void *param_v_1, void *param_v_2);
// takes a node and returns its state.
extern NodeState behaviour_node_internal_get_state(Node *node_handle);
// takes a node and returns its parent.
extern Node *    behaviour_node_internal_get_parent(Node *node_handle);
// takes a node and resets its state to NS_PENDING, the pre-start state.
extern int       behaviour_node_internal_reset_state(Node *node_handle, void *a1, void *a2);
// a job function that when called in the recursive dispatcher, sets the entire subtree root to root_node_handle.
extern int       behaviour_node_internal_set_root(Node *node_handle, void *root_node_handle, void *a2);

// takes a node and sets its root nodes focus to the pointer passed.
extern int       behaviour_node_internal_move_focus(void *node_handle);
// the standard node start function. Performs lots of assertions then sets state to NS_UNDETERMINED.
extern int       behaviour_node_internal_standard_start(void *node_handle);
// decorator handler. takes a decorator node and determines what to do depending on type and child state.
extern int       behaviour_node_internal_decorator_tick(void *node_handle);
// composite handles. takes a composite node and determines what to do depending on type and child state.
extern int       behaviour_node_internal_composite_tick(void *node_handle);

/* ------------------------- external tree functions ------------------------ */

// resets the behaviour tree to default nodes with no root affiliation.
extern int       behaviour_tree_reset(Node *root_node_handle);
// ticks the behaviour tree, for use in game loops to step through tree one instruction at a time.
extern int       behaviour_tree_tick(Node *root_node_handle);
// returns the node state of the tree at that moment.
extern int       behaviour_tree_get_state(Node *root_node_handle);
// executes the entire tree in one go.
extern int       behaviour_tree_run(Node *root_node_handle);

/* ------------------------- external node functions ------------------------ */

// The run, fail and succeed functions for the nodes. Sets the internal state of a passed node to NS_UNDETERMINED, SUCCEEDED OR FAILED.
extern int       behaviour_node_external_run(Node *node_handle);
extern int       behaviour_node_external_fail(Node *node_handle);
extern int       behaviour_node_external_succeed(Node *node_handle);

// Creates a behaviour node based on a passed type.
extern Node *    behaviour_node_create(NodeType type);
// Adds a child to a given node and sets the child's parent to the given node.
extern int       behaviour_node_add_child(Node *parent_node_handle, Node *child_node_handle);
// Sets the start action of a leaf node. Takes the leaf node and the function pointer to the start action.
extern int       behaviour_node_set_start(Node *node_handle, Action start_action_handle);
// Sets the tick action of a leaf node. Takes the leaf node and the function pointer to the tick action.
extern int       behaviour_node_set_action(Node *node_handle, Action tick_action_handle);
// Sets the stop action of a leaf node. Takes the leaf node and the function pointer to the stop action.
extern int       behaviour_node_set_stop(Node *node_handle, Action stop_action_handle);
// Labels a leaf node, for use in loggin and visualising the tree. Takes the node, label and label length.
extern int       behaviour_node_set_label(Node *node_handle, char *node_label, int node_label_length);
// Sets the subject of a node for use in manipulation, of a player for example. Takes a node and a subject pointer.
extern int       behaviour_node_set_subject(Node *node_handle, void *subject_handle);
// Sets the nodes blackboard, for use with multiple leafs with subjects to share values. Takes a node and a blackboard pointer.
extern int       behaviour_node_set_blackboard(Node *node_handle, void *blackboard_handle);
// Gets the subject of a behaviour node. The internal node structure is hidden, so a function is necessary. Takes a node pointer.
extern void *    behaviour_node_get_subject(Node *node_handle);
// Gets the blackboard of a behaviour node. Takes a node pointer.
extern void *    behaviour_node_get_blackboard(Node *node_handle);
// Sets the repetitions of a repeater node. Takes a node and a number of repetitions.
extern int       behaviour_node_set_repetitions(Node *node_handle, int repetitions);
// Prints some debug information on a node, used for santity checking.
extern int       behaviour_node_get_information(Node *node_handle);

#endif // !BEHAVIOUR_INTERNAL_H