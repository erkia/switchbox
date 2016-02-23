#ifndef _PWRUSB_H
#define _PWRUSB_H

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
typedef HANDLE pwrusb_fd_t;
#else
typedef int pwrusb_fd_t;
#endif


typedef struct {
    pwrusb_fd_t fd;
} pwrusb_ctx;


extern int pwrusb_search (const char *search_serial, char *buf, size_t buflen);
extern int pwrusb_init (pwrusb_ctx *ctx);
extern int pwrusb_open (pwrusb_ctx *ctx, const char *device);
extern int pwrusb_get (pwrusb_ctx *ctx, int *state);
extern int pwrusb_set (pwrusb_ctx *ctx, int mask, int state);
extern int pwrusb_set_on (pwrusb_ctx *ctx, int mask);
extern int pwrusb_set_off (pwrusb_ctx *ctx, int mask);
extern int pwrusb_close (pwrusb_ctx *ctx);

#endif

