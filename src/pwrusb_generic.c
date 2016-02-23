#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pwrusb_internal.h"


LIBPWRUSB_DLL_EXPORTED int pwrusb_get (pwrusb_ctx *ctx, int *state)
{
    return pwrusb_get_state (ctx, state);
}


LIBPWRUSB_DLL_EXPORTED int pwrusb_set (pwrusb_ctx *ctx, int mask, int state)
{
    int old_state;

    if (mask != 0xFF) {
        if (pwrusb_get_state (ctx, &old_state) != 0) {
            return -1;
        }

        state &= mask;
        state |= (old_state & ~mask);
    }

    if (pwrusb_set_state (ctx, &state) != 0) {
        return -1;
    }

    return 0;
}


LIBPWRUSB_DLL_EXPORTED int pwrusb_set_on (pwrusb_ctx *ctx, int mask)
{
    return pwrusb_set (ctx, mask, 0xFF);
}


LIBPWRUSB_DLL_EXPORTED int pwrusb_set_off (pwrusb_ctx *ctx, int mask)
{
    return pwrusb_set (ctx, mask, 0x00);
}

