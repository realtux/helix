#include <stdlib.h>
#include <stdio.h>

#include "core.h"

extern int line_number;
extern int chr;

extern char *source;

void lex(void) {
	infinite {
		if (source[chr] == ' ') {
			goto next;
		}

		if (source[chr] == '\n') {
			++line_number;
			goto next;
		}

		// done with the source file
		if (source[chr] == '\0') break;

		// move to next character
		next:
		++chr;
	}
}
