#define _GNU_SOURCE

#ifndef __OpenBSD__
#define HAVE_DLINFO
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#ifdef HAVE_DLINFO
#include <link.h>
#endif
#include <dlfcn.h>
#include <err.h>

#include "extern.h"

#define USAGE	"Usage: %s dlopen"

static char *
get_library_path(const char *name)
{
	char *path = NULL;
	void *dlh;
#ifdef RTLD_DI_LINKMAP
	struct link_map *lm;
#else
	Dl_info dli;
#endif

	if ((dlh = dlopen(name, RTLD_LAZY | RTLD_LOCAL)) == NULL) {
		warnx("dlopen: %s", dlerror());
		return (NULL);
	}

#ifdef RTLD_DI_LINKMAP
	if (dlinfo(dlh, RTLD_DI_LINKMAP, &lm) == -1) {
		warnx("dlinfo: %s", dlerror());
		goto out;
	}
	path = strdup(lm->l_name);
#else
	void *sslv = dlsym(dlh, "OpenSSL_version");
	if (sslv == NULL) {
		warnx("dlsym: %s", dlerror());
		goto out;
	}
	if (!dladdr(sslv, &dli)) {
		warnx("dladdr: %s", dlerror());
		goto out;
	}
	path = strdup(dli.dli_fname);
#endif

out:
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
	} else {
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

static pid_t
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
			errx(1, "xdlopen: %s: %s", sopath, dlerror());
		else
			err(1, "xdlopen: %s", sopath);
	}

	sslv = dlsym(dlh, "OpenSSL_version");
	if (sslv == NULL)
		sslv = dlsym(dlh, "SSLeay_version");
	if (sslv == NULL)
		errx(1, "dlsym: %s: %s", sopath, dlerror());

	if (strstr(sslv(0), "SSL") != NULL)
		printf("PASS\n");
	else
		printf("FAIL\n");

	return getpid();
}

static void
print_map(pid_t pid, const char *scan)
{
#ifdef __sun__
	if (!scan_map(pid, scan))
		printf("PASS\n");
#elif defined(__OpenBSD__)
	char cmd[256];

	// This command needs sysctl kern.allowkmem=1
	(void)snprintf(cmd, sizeof(cmd), "doas procmap -l %d | grep %s", pid, scan);
	if (system(cmd) != 0)
		err(1, "system: %s", cmd);
#else
	// This call needs security.bsd.unprivileged_proc_debug=1 on MidnightBSD
	char *map = procmap(pid);
	if (map != NULL) {
		char *line = strtok(map, "\n");
		while (line != NULL) {
			if (scan == NULL)
				printf("%s\n", line);
			else if (strstr(line, scan) != NULL)
				printf("FAIL: %s\n", line);
			line = strtok(NULL, "\n");
		}
		free(map);
	}
#endif
	return;
}

int
main(int argc, char *argv[])
{
	const char *scan;
	pid_t pid;

	if (argc != 2)
		errx(1, USAGE, argv[0]);

	if (!strcmp(argv[1], "dlopen")) {
		pid = test_dlopen();
		scan = "libcrypto.so";
	} else if (!strcmp(argv[1], "map")) {
		print_map(getpid(), NULL);
		return (0);
	} else
		errx(1, USAGE, argv[0]);

	print_map(pid, scan);
}
