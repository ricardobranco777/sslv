#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#define _KERNEL
#include <sys/procfs.h>

int
scan_map(pid_t pid, const char *path) {
	char map[128];
	prmap_t entry;
	int found = 0;
	int fd;

	(void)snprintf(map, sizeof(map), "/proc/%d/map", pid);

	if ((fd = open(map, O_RDONLY)) == -1)
		err(1, "open");

	while (read(fd, &entry, sizeof(entry)) == sizeof(entry))
		if (path == NULL || strstr(entry.pr_mapname, path) != NULL) {
			printf("%s\n", entry.pr_mapname);
			found = 1;
		}

	(void)close(fd);
	return (found);
}
