#include <regex.h>
#include <stdlib.h>
#include <stdio.h>

#include "core.h"

extern int line;
extern int chr;

extern char *source;

void eat_space(void) {
	infinite {
		if (source[chr] == ' ') {
			++chr;
			continue;
		}

		break;
	}
}

void eat_comment(void) {
	infinite {
		if (source[chr] == '\n' ||
			source[chr] == '\0') {
			++line;
			break;
		}

		++chr;
	}
}

void eat_braced_block(void) {
	// push past the true block
	int nested_curly = 0;

	// push past lcurly
	++chr;

	infinite {
		// fail on endline
		if (source[chr] == '\0') {
			BLANG_PARSE("Unclosed brace block");
		}

		if (source[chr] == '\n') {
			++line;
		}

		// check for end declaration
		if (source[chr] == '}' && nested_curly == 0) {
			break;
		}

		// check for new opening lcurly
		if (source[chr] == '{') {
			++nested_curly;
		}

		// check for end nested closing rcurly
		if (source[chr] == '}') {
			--nested_curly;
		}

		++chr;
	}

	++chr;
}

void lex(void) {
	infinite {
		// eat space
		if (source[chr] == ' ') {
			eat_space();
			goto next;
		}

		// eat semi colon
		if (source[chr] == ';') {
			goto next;
		}

		// eat newlines
		if (source[chr] == '\n') {
			++line;
			goto next;
		}

		// eat comments
		if (source[chr] == '/' && source[chr + 1] == '/') {
			eat_comment();
			goto next;
		}

		// done with the source file
		if (source[chr] == '\0') break;

		// move to next character
		next:
		++chr;
	}
}
