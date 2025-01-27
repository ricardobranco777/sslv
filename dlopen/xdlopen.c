#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "extern.h"

static ssize_t
Write(int fd, const void *buf, size_t nbytes)
{
	const unsigned char *ptr = (const unsigned char *)buf;
	size_t nleft = nbytes;
	ssize_t n;

	while (nleft > 0) {
		if ((n = write(fd, ptr, nleft)) == -1) {
			if (errno == EINTR)
				continue;
			else
				return (-1);
		}
		nleft -= (size_t) n;
		ptr += (size_t) n;
	}

	return (nbytes - nleft);
}

void *
xdlopen(const char *path, int mode)
{
	struct stat sb;
	void *buf;
	int save_errno;
	void *handle = NULL;
	int fd, mfd;
#if !defined(__FreeBSD__) && !defined(__DragonFly__)
	char fdpath[4096];
#endif

	fd = mfd = -1;

	if ((fd = open(path, O_RDONLY)) == -1)
		return (NULL);

	if (fstat(fd, &sb) == -1)
		goto out;

	if ((buf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		goto out;

#if defined(__DragonFly__) || defined(__sun__) || (defined(__NetBSD__) && __NetBSD_Version__ < 1099000000)
	if ((mfd = shm_open("/xxx", O_RDWR | O_CREAT | O_EXCL, 0700)) == -1)
		goto out;
#elif defined(__MidnightBSD__)
	if ((mfd = shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL, 0700)) == -1)
		goto out;
#else
	if ((mfd = memfd_create("xxx", 0)) == -1)
		goto out;
#endif

	if (ftruncate(mfd, sb.st_size) == -1)
		goto out;

	if (Write(mfd, buf, sb.st_size) == -1)
		goto out;

	(void)munmap(buf, sb.st_size);

#if defined(__FreeBSD__) || defined(__DragonFly__)
	handle = fdlopen(mfd, mode);
#else
#if defined(__NetBSD__)
	// XXX On NetBSD this file descriptor is not visible in /proc/<pid>/fd
	(void)snprintf(fdpath, sizeof(fdpath), "/dev/fd/%d", mfd);
#elif defined(__linux__)
	(void)snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", mfd);
#else
	(void)snprintf(fdpath, sizeof(fdpath), "/proc/%d/fd/%d", getpid(), mfd);
#endif
	handle = dlopen(fdpath, mode);
#endif

out:
	save_errno = errno;
	if (mfd != -1) {
#if defined(__DragonFly__) || defined(__sun__) || (defined(__NetBSD__) && __NetBSD_Version__ < 1099000000)
		(void)shm_unlink("/xxx");
#elif !defined(__MidnightBSD__)
		(void)shm_unlink("memfd:xxx");
#endif
		(void)close(mfd);
	}
	if (fd != -1)
		(void)close(fd);
	errno = save_errno;
	return (handle);
}
