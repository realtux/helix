#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core.h"

extern int line;
extern int chr;

extern char *source;

extern int stack_size;
extern stack_frame **stack;

extern int env_debug;

char *get_timestamp(void) {
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	char *stamp = malloc(sizeof(char) * 22);

	sprintf(stamp, "%d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900,
		tm->tm_mon + 1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);

	return stamp;
}

void stack_trace(void) {
	FILE *log = fopen("/var/log/helix.log", "a");

	if (log == NULL) return;

	char *stamp = get_timestamp();

	fprintf(log, "%s: Stack Trace:\n", stamp);

	if (stack_size > 1) {
		int i;
		for (i = stack_size-1; i > 0; --i) {
			if (i == stack_size-1) {
				fprintf(log, "%s:   -> err:%d\n", stamp, line);
				fprintf(log, "%s:   -> %s():%d\n", stamp, stack[i]->name, stack[i]->line_pos);
			} else {
				fprintf(log, "%s:   -> %s():%d\n", stamp, stack[i]->name, stack[i]->line_pos);
			}
		}
	} else {
		fprintf(log, "%s:   -> err:%d\n", stamp, line);
	}

	fclose(log);
	free(stamp);
}

void helix_log_error(const char *type, const char *msg, int line,
	const char *file, int dump_stack) {

    FILE *log = fopen("/var/log/helix.log", "a");

	char *stamp = get_timestamp();

    if (log == NULL) {
		//free(stamp);
		return;
	}

    fprintf(log, "%s: Helix %s: %s at line %d in %s\n", stamp, type, msg, line, file);

	fclose(log);
	free(stamp);

	if (dump_stack) stack_trace();

	if (strcmp(type, "Parse") == 0) goto blackhole;
	if (strcmp(type, "Fatal") == 0) goto blackhole;
	if (strcmp(type, "Core") == 0) goto blackhole;

	if (1==2) {
		blackhole:
		// clean up
		stack_destroy();
		free(source);

		// final debugging output, if any
		if (env_debug) {
			// execution details
			printf("\n---->\n");
			printf("Lines Read: %d\n", line);
			printf("---->\n");
		}

		// always
		printf("\n");
		exit(EXIT_FAILURE);
	}
}
