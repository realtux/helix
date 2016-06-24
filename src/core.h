#ifndef CORE_H
#define CORE_H

#define BLANG_GENERAL(msg) printf("Blang General: %s\n", msg);

#define BLANG_CORE(msg) printf("Blang Core: %s at line %d\n", msg, line);
#define BLANG_FATAL(msg) printf("Blang Fatal: %s at line %d\n", msg, line);
#define BLANG_PARSE(msg) printf("Blang Parse: %s at line %d\n", msg, line);
#define BLANG_WARNING(msg) printf("Blang Warning: %s at line %d\n", msg, line);

#define true 1
#define false 0

#define infinite for(;;)

typedef struct {
    char *name;
    int char_pos;
    int line_pos;
    union {
        int return_int;
        int return_bool;
        double return_double;
        char *return_char;
    } return_val;
} stack_frame;

void stack_push(stack_frame);
void stack_destroy(void);

#endif
