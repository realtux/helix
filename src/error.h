#ifndef ERROR_H
#define ERROR_H

#define HELIX_GENERAL(msg) helix_log_error("General", msg, line, "", 0)
#define HELIX_CORE(msg) helix_log_error("Core", msg, line, "", 1)
#define HELIX_FATAL(msg) helix_log_error("Fatal", msg, line, "", 1)
#define HELIX_PARSE(msg) helix_log_error("Parse", msg, line, "", 1)
#define HELIX_WARNING(msg) helix_log_error("Warning", msg, line, "", 1)

void stack_trace(void);
void helix_log_error(const char *, const char *, int, const char *, int);
char *get_timestamp(void);

#endif
