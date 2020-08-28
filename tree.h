#if !defined(TREE_H)
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

/* Macros for failing, running and succeeding a given node. */
#define FAIL(NODE) ({NODE->state = FAILED; NODE->state_fns->failure(NODE); return; })
#define RUN(NODE) ({NODE->state_fns->running(NODE); return;})
#define PASS(NODE) ({NODE->state = SUCCESSFUL; NODE->state_fns->success(NODE); return; })

/* Macro to get and set a given field flag. */
#define CHECK_FLAG(FLAG, FIELD) ( (FLAG >> FIELD) & 0x1 )
#define SET_FLAG(FLAG, FIELD) ( FLAG = FLAG | ((uint8_t)0x01 << FIELD) )
#define CLR_FLAG(FLAG, FIELD) ( FLAG = FLAG & ~((uint8_t)0x01 << FIELD) )

/* Assertion macro to display a message as well as exiting the program. */
#define ASSERT_MSG(COND, MSG) ({if(COND){printf(MSG "\n"); exit(EXIT_FAILURE);} })

/* Node Flags 
    IS_CONTROL: node is considered a control node (not a leaf).
    IS_RUNNING: node is currently running.
    HAS_RAN: node has ran previously.
    HAS_LABEL: node has a valid label.
*/
#define IS_CONTROL 0x01
#define IS_RUNNING 0x02
#define HAS_RAN 0x03
#define HAS_LABEL 0x04

/* Tree Flags
    IS_HALTED: tree has been halted.
    IS_LOGGIN: tree is being logged given passed parameter
*/
#define IS_HALTED 0x05
#define IS_LOGGING 0x06
#define ROOT_SET 0x07
#define END_STATE 0x08

/* Macro to check logging is enabled, and to log a string to the trees file. */

char* log_path;
#define CLEAR_LOG() ( {if(log_path) { FILE *f = fopen(log_path, "w"); fprintf(f, ""); fclose(f);}})
#define LOG(...) ( {if(log_path) { const char* lp = (const char*)log_path; FILE *f = fopen(lp, "a"); fprintf(f, __VA_ARGS__); fprintf(f, "\n"); fclose(f); }} )

typedef void (*function_t)(void *p_node);

typedef struct action_vtable
{
    function_t tick;
    function_t start;
    function_t stop;
} action_vtable_t;

typedef struct state_vtable
{
    function_t running;
    function_t success;
    function_t failure;
} state_vtable_t;

enum node_state
{
    FAILED,
    SUCCESSFUL,
    UNINITIALISED
};

enum node_type
{
    ENTRY = 0,
    SEQUENCE,
    FALLBACK,
    INVERTER,
    RANDOM,
    REPEATER,
    LEAF,
    _CONTROL_TYPE_COUNT
};

typedef struct control_structure control_structure_t;
typedef struct node node_t;
typedef struct behaviour_tree behaviour_tree_t;

typedef struct node
{
    enum node_state state;
    action_vtable_t *action_fns;
    state_vtable_t *state_fns;

    uint8_t flags;
    enum node_type type;
    control_structure_t *control;

    void *subject;
    node_t *parent;
    behaviour_tree_t *tree;

    char *label;
} node_t;

typedef struct control_structure
{
    int child_index;
    int child_count;
    node_t **child_list;
} control_structure_t;

typedef struct behaviour_tree
{
    action_vtable_t action_vtables[_CONTROL_TYPE_COUNT];
    state_vtable_t state_vtables[_CONTROL_TYPE_COUNT];

    uint8_t flags;
    char* log_path;

    int node_c;
    node_t **nodes;

    node_t *root_node;
    node_t *current_node;
} behaviour_tree_t;

/* Tree Functions
    behaviour_tree_create: creates a tree with a given root node, returns pointer.
    behaviour_tree_tick: ticks a tree, i.e performing the current nodes next action.
    behaviour_tree_move_current_node: moves the behaviour tree to a new node.
    behaviour_tree_enable_logging: enables logging of a tree.
*/
void *behaviour_tree_create(const char* log_path, uint8_t flags);
void behaviour_tree_set_root(void *p_behaviour_tree, void *p_root_node);
void behaviour_tree_move(void *p_behaviour_tree, void *p_node);
void behaviour_tree_tick(void *p_behaviour_tree);
void _behaviour_tree_enable_logging(void *p_behaviour_tree, const char* path);
void _behaviour_tree_add_child(void *p_behaviour_tree, void *p_node);
void behaviour_tree_reset(void *p_behaviour_tree);

/* Node Functions
    node_create: creates a node, configures it, returns pointer.
    leaf_configure: sets the subject and the action functions for a leaf node.
    _node_add_to_parent: adds a child to a given control node, returns pointer to child.
*/
void *node_create(const char *label, void *p_parent_node, enum node_type type, uint8_t flags);
void leaf_configure(void *p_node, void *p_subject, void *p_action_funcs);
void *_node_add_to_parent(void *p_node);

void general_failure(void *p_node);
void general_success(void *p_node);
void general_running(void *p_node);

void entry_start(void *p_node);
void entry_stop(void *p_node);
void entry_tick(void *p_node);

void sequence_start(void *p_node);
void sequence_stop(void *p_node);
void sequence_tick(void *p_node);

void fallback_start(void *p_node);
void fallback_stop(void *p_node);
void fallback_tick(void *p_node);

void inverter_start(void *p_node);
void inverter_stop(void *p_node);
void inverter_tick(void *p_node);

#endif // TREE_H