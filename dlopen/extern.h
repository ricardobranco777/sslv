#ifndef _EXTERN_H
#define _EXTERN_H

#ifdef __cplusplus
extern "C" {
#endif

char *procmap(void);
void *xdlopen(const char *, int);
int scan_map(const char *);

#ifdef __cplusplus
}
#endif

#endif
