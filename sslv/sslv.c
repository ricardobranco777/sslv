/*
 * Output the OpenSSL version on systems lacking openssl(1)
 */

#define _GNU_SOURCE

#ifndef __OpenBSD__
#define HAVE_DLINFO
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#ifdef HAVE_DLINFO
#include <link.h>
#endif
#include <dlfcn.h>
#include <err.h>

#define file_exists(file)	!access((file), F_OK)

static int
print_info(const char *sopath, int verbose)
{
	char pathname[PATH_MAX+1];
	const char *(*sslv)(int);
	char *path;
	void *dlh;
#ifdef HAVE_DLINFO
	struct link_map *lm;
#else
	Dl_info dli;
#endif

	dlh = dlopen(sopath, RTLD_LAZY | RTLD_LOCAL);
	if (dlh == NULL) {
		warnx("%s: %s", sopath, dlerror());
		return (-1);
	}

	(void) dlerror();
	sslv = dlsym(dlh, "OpenSSL_version");
	if (sslv == NULL) {
		sslv = dlsym(dlh, "SSLeay_version");
	}
	if (sslv == NULL) {
		warnx("dlsym: %s: %s", sopath, dlerror());
		goto bad;
	}

#ifdef HAVE_DLINFO
	if (dlinfo(dlh, RTLD_DI_LINKMAP, &lm) < 0) {
		warnx("%s: %s", sopath, dlerror());
		goto bad;
	}
	sopath = lm->l_name;
#else
	if (!dladdr(sslv, &dli)) {
		warnx("%s: %s", sopath, dlerror());
		goto bad;
	}
	sopath = dli.dli_fname;
#endif

	path = realpath(sopath, pathname);
	if (path != NULL && strcmp(path, sopath) && file_exists(path))
		printf("%s %s:\n", sopath, path);
	else
		printf("%s:\n", sopath);

	printf("\tVersion: %s\n", sslv(0));
	if (verbose)
		for (int i = 1; i < 5; i++) {
			const char *s = sslv(i);
			if (strstr(s, "not available") == NULL)
				printf("\t%s\n", sslv(i));
		}

	(void)dlclose(dlh);
	return (0);

bad:
	(void)dlclose(dlh);
	return (-1);
}

int
main(int argc, char *argv[])
{
	int verbose = 0;
	int errors = 0;
	int ch;

	while ((ch = getopt(argc, argv, "v")) != -1) {
		switch (ch) {
		case 'v':
			verbose++;
			break;
		}
	}

	argv += optind;
	argc -= optind;

	if (*argv) {
		do {
			if (print_info(*argv, verbose) < 0)
				errors++;
		} while (*++argv != NULL);
	} else if (print_info("libcrypto.so", verbose) < 0)
		errors++;

	return (errors ? 1 : 0);
}
