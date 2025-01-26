#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dlfcn.h>
#include <err.h>

#include "extern.h"

#define USAGE	"Usage: %s dlopen LIBRARY | execve ARGV..."

static int
test_dlopen(const char *sopath)
{
	void *dlh;

	if ((dlh = xdlopen(sopath, RTLD_LAZY | RTLD_LOCAL)) == NULL) {
		char *error = dlerror();
		if (error != NULL)
			errx(1, "%s: %s", sopath, dlerror());
		else
			err(1, "xdlopen: %s", sopath);
	}

#ifndef __sun__
	// This call needs security.bsd.unprivileged_proc_debug=1 on MidnightBSD
	char *map = procmap();
	if (map != NULL && strstr(map, sopath) != NULL)
		printf("%s\n", map);
	else
		printf("PASS\n");
	free(map);
#else
	if (!scan_map(sopath))
		printf("PASS\n");
#endif

	(void)dlclose(dlh);
	return (0);
}

static int
test_execve(char *argv[], char *envp[])
{
	// XXX fork
	xexecve(argv[0], argv, envp);
	return (-1);
}

int
main(int argc, char *argv[], char *envp[])
{
	if (argc < 3)
		errx(1, USAGE, argv[0]);

	if (!strcmp(argv[1], "dlopen"))
		return (test_dlopen(argv[2]));
	else if (!strcmp(argv[1], "execve")) {
		argv += 2;
		return (test_execve(argv, envp));
	} else
		errx(1, USAGE, argv[0]);
}
