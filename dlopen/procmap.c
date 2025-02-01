#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "extern.h"

#if defined(__FreeBSD__) || defined(__DragonFly__)
#define PATH	"/proc/%d/map"
#elif defined(__NetBSD__)
#define PATH	"/proc/%d/maps"
#elif defined(__linux__)
#define PATH	"/proc/%d/maps"
#endif

char *
procmap(pid_t pid)
{
	ssize_t size = 65536;
	char *buf = NULL;
	char path[128];
	ssize_t n;
	int fd;

	(void)snprintf(path, sizeof(path), PATH, pid);

	if ((fd = open(path, O_RDONLY)) == -1)
		return (NULL);

	/* This file must be read in one go */
	while (1) {
		if ((buf = calloc(1, size)) == NULL)
			goto bad;
		n = read(fd, buf, size);
		if ((n == -1 && errno == EFBIG) || n == size) {
			size <<= 1;
			if (size < 0)
				goto bad;
			if (lseek(fd, 0, SEEK_SET) == -1)
				goto bad;
			free(buf);
		}
		else if (n == -1)
			goto bad;
		else
			break;
	}

	buf = realloc(buf, n + 1);
	buf[n] = '\0';
	(void)close(fd);
	return (buf);

bad:
	if (buf != NULL)
		free(buf);
	(void)close(fd);
	return (NULL);
}
