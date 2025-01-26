#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#define _KERNEL
#include <sys/procfs.h>

int
scan_map(const char *path) {
	int found = 0;
	prmap_t entry;
	int fd;

	if ((fd = open("/proc/self/map", O_RDONLY)) == -1)
		err(1, "open");

	while (read(fd, &entry, sizeof(entry)) == sizeof(entry))
		if (strstr(entry.pr_mapname, path) != NULL) {
			printf("%s\n", entry.pr_mapname);
			found = 1;
		}

	(void)close(fd);
	return (found);
}
