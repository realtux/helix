#ifndef CORE_H
#define CORE_H

#define HELIX_GENERAL(msg) printf("HELIX General: %s\n", msg);
#define HELIX_CORE(msg) \
    printf("Helix Core: %s at line %d\n", msg, line); \
    stack_trace();
#define HELIX_FATAL(msg) \
    printf("Helix Fatal: %s at line %d\n", msg, line); \
    stack_trace();
#define HELIX_PARSE(msg) \
    printf("Helix Parse: %s at line %d\n", msg, line); \
    stack_trace();
#define HELIX_WARNING(msg) \
    printf("Helix Warning: %s at line %d\n", msg, line); \
    stack_trace();

#define true 1
#define false 0

#define infinite for(;;)

const char *re_keywords;
const char *re_integers;
const char *re_std;

#define HELIX_VAL_INT 1
#define HELIX_VAL_FLOAT 2
#define HELIX_VAL_STRING 3
#define HELIX_VAL_BOOL 4

typedef struct {
    int type;
    union {
        int val_int;
        double val_float;
        char *val_string;
        int val_bool;
        void *something;
    } d;
} helix_val;

typedef struct {
    char *name;
    int char_pos;
    int line_pos;
    union {
        int return_int;
        double return_float;
        char *return_string;
        int return_bool;
        void *something;
    } return_val;
} stack_frame;

#define EXPAND_STRING_BY(var, type, amt) var = realloc(var, sizeof(type) * (strlen(var) + amt + 1));
#define SHRINK_STRING_BY(var, type, amt) var = realloc(var, sizeof(type) * (strlen(var) + 1 - amt));

void stack_push(stack_frame);
void stack_destroy(void);

void free_helix_val(helix_val*);

#endif
