#ifndef _PWRUSB_INTERNAL_H
#define _PWRUSB_INTERNAL_H

#if BUILD_LIBPWRUSB && HAVE_VISIBILITY
    #define LIBPWRUSB_DLL_EXPORTED __attribute__((__visibility__("default")))
#elif BUILD_LIBPWRUSB && defined _WIN32
    #define LIBPWRUSB_DLL_EXPORTED __declspec(dllexport)
#elif defined _WIN32
    #define LIBPWRUSB_DLL_EXPORTED __declspec(dllimport)
#else
    #define LIBPWRUSB_DLL_EXPORTED
#endif

#include "pwrusb.h"

int pwrusb_get_state (pwrusb_ctx *ctx, int *state);
int pwrusb_set_state (pwrusb_ctx *ctx, int *state);

#endif

