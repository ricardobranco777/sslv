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

#if defined(__FreeBSD__) || defined(__DragonFly__)
#define PROC_MAP    "/proc/%d/map"
#elif defined(__NetBSD__)
#define PROC_MAP    "/proc/%d/maps"
#elif defined(__linux__)
#define PROC_MAP    "/proc/%d/maps"
#endif

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

static void
test_dlopen(void)
{
	const char *(*sslv)(int);
	char *sopath;
	void *dlh;

	if ((sopath = get_library_path2("libcrypto.so")) == NULL)
		errx(1, "get_library_path2");

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
}

static void
print_map(void)
{
	char cmd[256];

#ifdef __sun__
	(void)snprintf(cmd, sizeof(cmd), "pmap -l %d", getpid());
#elif defined(__OpenBSD__)
	// This command needs sysctl kern.allowkmem=1
	(void)snprintf(cmd, sizeof(cmd), "doas procmap -l %d", getpid());
#else
	char path[128];
	(void)snprintf(path, sizeof(path), PROC_MAP, getpid());
	(void)snprintf(cmd, sizeof(cmd), "cat %s", path);
#endif

	if (system(cmd) != 0)
		err(1, "system: %s", cmd);
}

int
main(void)
{
	test_dlopen();
	print_map();
}
