#ifndef _EXTERN_H
#define _EXTERN_H

#ifdef __cplusplus
extern "C" {
#endif

char *procmap(void);
int scan_map(const char *);

void *xdlopen(const char *, int);
int xexecve(const char *, char *const __argv[], char *const __envp[]);

#ifdef __cplusplus
}
#endif

#endif
