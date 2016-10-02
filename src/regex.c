#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcre.h"

char **pcre_match(const char *regex, const char *str, int *num_matches) {
    const char *error;
    int erroffset;
    pcre *re;
    int rc;
    int total_matches = 0;
    int ovector[100];

    re = pcre_compile(regex, 0, &error, &erroffset, 0);

    unsigned int offset = 0;
    unsigned int len = strlen(str);

    char **matches = malloc(sizeof(char *) * 1);

    while (offset < len && (rc = pcre_exec(re, 0, str, len, offset, 0,
                                    ovector, sizeof(ovector))) >= 0) {

       for(int i = 0; i < rc; ++i) {
           int cap_len = ovector[2*i+1] - ovector[2*i];

           char *match = malloc(sizeof(char) * (cap_len + 1));

           strncpy(match, str + ovector[2*i], cap_len);
           match[cap_len] = '\0';

           matches = realloc(matches, sizeof(char *) * (i + 1));
           matches[i] = match;

           ++total_matches;
       }

       offset = ovector[1];
    }

    *num_matches = total_matches;

    if (total_matches == 0) {
        free(matches);
        return NULL;
    }

    return matches;
}

void free_pcre_matches(char **matches, int num_matches) {
    for (int i = 0; i < num_matches; ++i) {
        free(matches[i]);
    }
    free(matches);
}
