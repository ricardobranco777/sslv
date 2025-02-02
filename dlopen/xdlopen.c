#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

#ifndef MFD_EXEC
#define MFD_EXEC	0
#endif

#define SHM_NAME	"test"

static int
get_memfd(const char *path)
{
	void *buf, *buf2;
	struct stat sb;
	int save_errno;
	int fd, memfd;

	buf = buf2 = MAP_FAILED;
	fd = memfd = -1;

	if ((fd = open(path, O_RDONLY)) == -1)
		return (-1);

	if (fstat(fd, &sb) == -1)
		goto out;

	if ((buf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		goto out;

#ifdef MFD_CLOEXEC
	if ((memfd = memfd_create(SHM_NAME, MFD_CLOEXEC | MFD_EXEC)) == -1)
		goto out;
#elif defined(SHM_ANON)
	if ((memfd = shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR)) == -1)
		goto out;
#else
	(void)shm_unlink("/" SHM_NAME);
	if ((memfd = shm_open("/" SHM_NAME, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR)) == -1)
		goto out;
#endif

	if (ftruncate(memfd, sb.st_size) == -1)
		goto out;

	if ((buf2 = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0)) == MAP_FAILED)
		goto out;

	memcpy(buf2, buf, sb.st_size);

out:
	save_errno = errno;

	if (buf2 != MAP_FAILED)
		(void)munmap(buf2, sb.st_size);
	if (buf != MAP_FAILED)
		(void)munmap(buf, sb.st_size);

	(void)close(fd);

	if (memfd != -1) {
#if !defined(__linux__) && !defined(SHM_ANON)
		(void)shm_unlink("/" SHM_NAME);
#endif
		return (memfd);
	}

	errno = save_errno;
	return (-1);
}

void *
xdlopen(const char *path, int mode)
{
	void *handle = NULL;
	int memfd;

	if ((memfd = get_memfd(path)) == -1)
		return (NULL);

#if defined(__FreeBSD__) || defined(__DragonFly__)
	handle = fdlopen(memfd, mode);
#else
	char fdpath[128];
	(void)snprintf(fdpath, sizeof(fdpath), "/dev/fd/%d", memfd);
	handle = dlopen(fdpath, mode);
#endif

	return (handle);
}
