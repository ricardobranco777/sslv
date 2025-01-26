#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dlfcn.h>
#include <err.h>

#include "extern.h"

int
main(int argc, char *argv[])
{
	char *sopath = argv[1];
	void *dlh;

	if (argc != 2)
		errx(1, "Usage: %s /path/to/library.so", argv[0]);

	dlh = xdlopen(sopath, RTLD_LAZY | RTLD_LOCAL);
	if (dlh == NULL) {
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
