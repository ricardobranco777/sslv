
`xdlopen` is like `dlopen` but tries to load the shared library in an "anonymous" way.

First we use either `memfd_create` or `shm_open` to create a file in memory,
then we use `fdlopen` if available or `dlopen` the file descriptor in `/proc/<pid>/fd` or `/dev/fd`

| Operating System | `memfd_create` | `fdlopen` |
|---|---|---|
| Linux | [Supported since 3.7 / Glibc 2.27](https://man7.org/linux/man-pages/man2/memfd_create.2.html) | |
| FreeBSD | [Supported since 13.0](https://man.freebsd.org/cgi/man.cgi?memfd_create(3)) | [Supported](https://man.freebsd.org/cgi/man.cgi?fdlopen(3)) |
| NetBSD | [Supported since 11.0](https://man.netbsd.org/memfd_create.2) | |
| DragonflyBSD | | [Supported](https://leaf.dragonflybsd.org/cgi/web-man?command=fdlopen&section=ANY) |
| MidnightBSD | | [Supported](http://man.midnightbsd.org/cgi-bin/man.cgi/fdlopen) |

- On HardenedBSD, [shm_open](https://man.freebsd.org/cgi/man.cgi?shm_open) is restricted if the [hardening.harden_shm sysctl](https://git.hardenedbsd.org/hardenedbsd/HardenedBSD/-/wikis/home#shared-memory-shm-hardening) is set.

## BUGS / Limitations

- On Linux, the entries are seen in `/proc/<pid>/maps` as `/memfd:xxx (deleted)`, just like any JIT compiled program
- On Solaris, the entries are seen as `/proc/<pid>/fd/#` in `pmap -l PID`
- On DragonflyBSD, they're seen as `/var/run/shm/xxx` in `/proc/<pid>/map`
