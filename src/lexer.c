#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"
#include "std/constructs.h"
#include "std/error.h"
#include "tpv/slre.h"

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
	int match;
	struct slre_cap cap[1];
	char *keyword;

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

		// end braced block
		if (source[chr] == '}') {
			return;
		}

		// reserved keywords
		match = slre_match(re_keywords, source + chr, 32, cap, 1, 0);

		if (match >= 0) {
			// hold the next keyword
			keyword = malloc(sizeof(char) * (cap->len + 1));

			// copy it in
			strncpy(keyword, cap->ptr, cap->len);
			keyword[cap->len] = '\0';

			handle_construct(keyword);

			free(keyword);
			goto next;
		}

		// function call
		match = slre_match(re_functions, source + chr, 32, cap, 1, 0);

		if (match >= 0) evaluate_expression();

		// method call
		match = slre_match(re_methods, source + chr, 32, cap, 1, 0);

		if (match >= 0) evaluate_expression();

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
