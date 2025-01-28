#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <link.h>
#include <dlfcn.h>
#include <err.h>

#include "extern.h"

#define USAGE	"Usage: %s dlopen|execve"

static char *
get_library_path(const char *name)
{
	struct link_map *lm;
	char *path;
	void *dlh;

	if ((dlh = dlopen(name, RTLD_LAZY | RTLD_LOCAL)) == NULL) {
		warnx("dlopen: %s", dlerror());
		return (NULL);
	}

	if (dlinfo(dlh, RTLD_DI_LINKMAP, &lm) == -1) {
		warnx("dlinfo: %s", dlerror());
		return (NULL);
	}

	path = strdup(lm->l_name);
	(void)dlclose(dlh);

	return (path);
}

/*
 * Calls get_library_path() in a child process
 * because we can't unload a shared library
 */
static char *
get_library_path2(const char *name)
{
	int pipefd[2];
	pid_t pid;

	if (pipe(pipefd) == -1)
		err(1, "pipe");

	if ((pid = fork()) == -1)
		err(1, "fork");
	else if (!pid) {
		char *path;

		if ((path = get_library_path(name)) == NULL)
			_exit(1);
		(void)write(pipefd[1], path, strlen(path)+1);
		free(path);
		_exit(0);
	}
	else {
		char path[PATH_MAX];
		int status;

		(void)close(pipefd[1]);
		if (read(pipefd[0], path, sizeof(path)) == -1)
			err(1, "read");
		(void)close(pipefd[0]);
		if (wait(&status) == -1)
			err(1, "wait");
		if (!WIFEXITED(status) || WEXITSTATUS(status))
			errx(1, "get_library_path failed");

		return strdup(path);
	}
}

static int
test_dlopen(void)
{
	const char *(*sslv)(int);
	char *sopath;
	void *dlh;

	if ((sopath = get_library_path2("libcrypto.so")) == NULL)
		return (-1);

	if ((dlh = xdlopen(sopath, RTLD_LAZY | RTLD_LOCAL)) == NULL) {
		char *error = dlerror();
		if (error != NULL)
			errx(1, "%s: %s", sopath, dlerror());
		else
			err(1, "xdlopen: %s", sopath);
	}

	sslv = dlsym(dlh, "OpenSSL_version");
	if (sslv == NULL)
		sslv = dlsym(dlh, "SSLeay_version");
	if (sslv == NULL)
		errx(1, "dlsym: %s: %s", sopath, dlerror());

	if (strstr(sslv(0), "OpenSSL") != NULL)
		printf("PASS\n");
	else
		printf("FAIL\n");

#ifndef __sun__
	// This call needs security.bsd.unprivileged_proc_debug=1 on MidnightBSD
	char *map = procmap();
	if (map != NULL && strstr(map, "libcrypto.so") != NULL)
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
	if (argc != 2)
		errx(1, USAGE, argv[0]);

	if (!strcmp(argv[1], "dlopen"))
		return (test_dlopen());
	else if (!strcmp(argv[1], "execve")) {
		argv += 2;
		return (test_execve(argv, envp));
	} else
		errx(1, USAGE, argv[0]);
}
