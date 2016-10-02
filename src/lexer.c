#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"
#include "constructs.h"
#include "error.h"
#include "tpv/slre.h"

extern int line;
extern int chr;

extern char *source;

void eat_space(void) {
	infinite {
		if (source[chr] == ' ' || source[chr] == '\t') {
			++chr;
			continue;
		}

		if (source[chr] == '\n') {
			++line;
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
			++chr;
			++line;
			break;
		}

		++chr;
	}
}

void eat_braced_block(void) {
	eat_space();

	// push past the true block
	int nested_curly = 0;

	// push past lcurly
	++chr;

	infinite {
		// fail on endline
		if (source[chr] == '\0') {
			HELIX_PARSE("Unclosed brace block");
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

	infinite {
		// eat space
		if (source[chr] == ' ' || source[chr] == '\t') {
			eat_space();
			continue;
		}

		// eat semi colon
		if (source[chr] == ';') {
			++chr;
			continue;
		}

		// eat newlines
		if (source[chr] == '\n') {
			++line;
			++chr;
			continue;
		}

		// end braced block
		if (source[chr] == '}') {
			++chr;
			return;
		}

		// reserved keywords
		match = slre_match(LEXER_RE_KEYWORDS, source + chr, 32, cap, 1, 0);

		if (match >= 0) {
			// hold the next keyword
			char *keyword = malloc(sizeof(char) * (cap->len + 1));

			// copy it in
			strncpy(keyword, cap->ptr, cap->len);
			keyword[cap->len] = '\0';

			handle_construct(keyword);

			free(keyword);

			continue;
		}

		// assignment
		match = slre_match(LEXER_RE_ASSIGNMENT, source + chr, 32, cap, 1, 0);

		if (match >= 0) {
			con_var();
			continue;
		}

		// std call
		match = slre_match(LEXER_RE_STD, source + chr, 32, cap, 1, 0);

		if (match >= 0) {
			evaluate_expression();
			continue;
		}

		// eat comments
		if (source[chr] == '/' && source[chr + 1] == '/') {
			eat_comment();
			continue;
		}

		// done with the source file
		if (source[chr] == '\0') break;

		HELIX_PARSE("Syntax error");
	}
}
