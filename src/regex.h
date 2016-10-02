#ifndef REGEX_H
#define REGEX_H

char **pcre_match(const char *, const char *, int *);
void free_pcre_matches(char **, int);

#endif
