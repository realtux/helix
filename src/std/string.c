#include <stdio.h>

#include "core.h"

/**
 * signature: int string::len(string value);
 */
int string_len(helix_val *val) {
    return 0;
    if (val->type == HELIX_VAL_STRING) {
        printf("%s", val->d.val_string);
    } else if (val->type == HELIX_VAL_INT) {
        printf("%lld", val->d.val_int);
    }

    free_helix_val(val);
}
