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

static int
get_memfd(const char *path)
{
	void *buf = MAP_FAILED;
	struct stat sb;
	int save_errno;
	int fd, memfd;

	fd = memfd = -1;
	if ((fd = open(path, O_RDONLY)) == -1)
		return (-1);

	if (fstat(fd, &sb) == -1)
		goto out;

	if ((buf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		goto out;

#ifdef MFD_CLOEXEC
	if ((memfd = memfd_create("xxx", MFD_CLOEXEC)) == -1)
		goto out;
#elif defined(SHM_ANON)
	if ((memfd = shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0700)) == -1)
		goto out;
#else
	if ((memfd = shm_open("/xxx", O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0700)) == -1)
		goto out;
#endif

	if (ftruncate(memfd, sb.st_size) == -1)
		goto out;

	if (Write(memfd, buf, sb.st_size) == -1)
		goto out;

out:
	save_errno = errno;

	if (buf != MAP_FAILED)
		(void)munmap(buf, sb.st_size);
	(void)close(fd);

	if (memfd != -1) {
		(void)shm_unlink("/xxx");
		return (memfd);
	}

	errno = save_errno;
	return (-1);
}

void *
xdlopen(const char *path, int mode)
{
	void *handle = NULL;
#if !defined(__FreeBSD__) && !defined(__DragonFly__)
	char fdpath[4096];
#endif
	int memfd;

	if ((memfd = get_memfd(path)) == -1)
		return (NULL);

#if defined(__FreeBSD__) || defined(__DragonFly__)
	handle = fdlopen(memfd, mode);
#else
#if defined(__NetBSD__)
	// XXX On NetBSD this file descriptor is not visible in /proc/<pid>/fd
	(void)snprintf(fdpath, sizeof(fdpath), "/dev/fd/%d", memfd);
#elif defined(__linux__)
	(void)snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", memfd);
#else
	(void)snprintf(fdpath, sizeof(fdpath), "/proc/%d/fd/%d", getpid(), memfd);
#endif
	handle = dlopen(fdpath, mode);
#endif

	return (handle);
}

int
xexecve(const char *path, char *const argv[], char *const envp[])
{
	int memfd;

	if ((memfd = get_memfd(path)) == -1)
		return (-1);

	fexecve(memfd, argv, envp);

	(void)close(memfd);
	return (-1);
}
