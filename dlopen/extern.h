#ifndef _EXTERN_H
#define _EXTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __sun__
int scan_map(pid_t, const char *);
#else
char *procmap(pid_t);
#endif

void *xdlopen(const char *, int);
int xexecve(const char *, char *const __argv[], char *const __envp[]);

#ifdef __cplusplus
}
#endif

#endif
