#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "extern.h"

#if defined(__FreeBSD__) || defined(__DragonFly__)
#define PATH	"/proc/curproc/map"
#elif defined(__NetBSD__)
#define PATH	"/proc/curproc/maps"
#elif defined(__linux__)
#define PATH	"/proc/self/maps"
#elif defined(__sun__)
#define PATH	"/proc/self/map"
#endif

char *
procmap(void)
{
	ssize_t size = 65536;
	char *buf = NULL;
	ssize_t n;
	int fd;

	if ((fd = open(PATH, O_RDONLY)) == -1)
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

	buf = realloc(buf, n);
	(void)close(fd);
	return (buf);

bad:
	if (buf != NULL)
		free(buf);
	(void)close(fd);
	return (NULL);
}
