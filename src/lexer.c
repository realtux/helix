#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"
#include "constructs.h"
#include "error.h"
#include "regex.h"

extern int line;
extern int chr;

extern char *source;
extern long source_size;

void eat_space(void) {
	infinite {
		if (source[chr] == ' ' || source[chr] == '\t') {
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
			++chr;
			++line;
			continue;
		}

		// check for end declaration
		if (source[chr] == '}' && nested_curly == 0) {
			++chr;
			break;
		}

		// check for new opening lcurly
		if (source[chr] == '{') {
			++chr;
			++nested_curly;
			continue;
		}

		// check for end nested closing rcurly
		if (source[chr] == '}') {
			++chr;
			--nested_curly;
			continue;
		}

		++chr;
	}
}

void lex(void) {
	int num_matches = 0;
	int match_len;
	char **matches;

	infinite {
		// eat space
		if (source[chr] == ' ' || source[chr] == '\t') {
			eat_space();
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

		// returning
		matches = pcre_match(LEXER_RE_RETURN, source + chr, &num_matches);
		if (num_matches > 0) {
			chr += 6;
			free_pcre_matches(matches, num_matches);
			con_return();
			return;
		}

		// reserved keywords
		matches = pcre_match(LEXER_RE_KEYWORDS, source + chr, &num_matches);
		if (num_matches > 0) {
			match_len = strlen(matches[1]);

			// hold the next keyword
			char *keyword = malloc(sizeof(char) * (match_len + 1));

			// copy it in
			strncpy(keyword, matches[1], match_len);
			keyword[match_len] = '\0';

			handle_construct(keyword);

			free(keyword);
			continue;
		}

		// assignment
		matches = pcre_match(LEXER_RE_ASSIGNMENT, source + chr, &num_matches);
		if (num_matches > 0) {
			free_pcre_matches(matches, num_matches);
			con_var();
			continue;
		}

		// std call
		matches = pcre_match(LEXER_RE_STD, source + chr, &num_matches);
		if (num_matches > 0) {
			free_pcre_matches(matches, num_matches);
			evaluate_expression();
			continue;
		}

		// user function call
		matches = pcre_match(LEXER_RE_FN, source + chr, &num_matches);
		if (num_matches > 0) {
			free_pcre_matches(matches, num_matches);
			evaluate_expression();
			continue;
		}

		// eat comments
		if (source[chr] == '/' && source[chr + 1] == '/') {
			eat_comment();
			continue;
		}

		if (source[chr] == '\0') break;

		if (chr < source_size-1) {
			HELIX_PARSE("Syntax error");
		} else {
			break;
		}
	}
}
