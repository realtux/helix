#include <stdio.h>

#include "core.h"

/**
 * signature: void output::print(string value);
 */
void output_print(helix_val *val) {
    if (val->type == HELIX_VAL_STRING) {
        printf("%s", val->d.val_string);
    } else if (val->type == HELIX_VAL_INT) {
        printf("%lld", val->d.val_int);
    }
}
