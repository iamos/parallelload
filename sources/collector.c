#include "parallelload.h"

static int log_fd;

/*
void* do_log(void* data) {
	fdata* args = (fdata*)data;
	size_t f_offset = lseek(args->fd, 0, SEEK_CUR);
	int pid = getpid();
	char procpath[255];
	char fpath[255];
	memset(procpath, '\0', 255);
	memset(fpath, '\0', 255);
	sprintf(procpath, "/proc/%d/fd/%d", pid, args->fd);
	if (readlink(procpath, fpath, 255) < 0) {
		perror("readlink Error");
	}

	dprintf(log_fd, "%s,%zu,%zu\n", fpath, f_offset, args->count);
	pthread_exit((void*)0);
}
*/
int __libc_start_main(int (*main) (int, char **, char **), int argc, char **ubp_av,
                      void (*init) (void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end)) {
	int (*original__libc_start_main)(int (*main) (int, char **, char **), int argc, char **ubp_av,
	                                 void (*init) (void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end));
	mlockall(MCL_CURRENT | MCL_FUTURE);
	log_fd = open("/home/shared/parallelload/collect_log", O_WRONLY | O_APPEND | O_CREAT, 00666);
	if (log_fd < 0) {
		printf("Error, open log_fd\n");
	}

	original__libc_start_main = dlsym(RTLD_NEXT, "__libc_start_main");
	return original__libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

ssize_t read(int fd, void *buf, size_t count) {
	ssize_t (*original_read)(int fd, void *buf, size_t count);
	ssize_t ret;

	size_t f_offset = lseek(fd, 0, SEEK_CUR);
	int pid = getpid();
	char procpath[255];
	char fpath[255];
	memset(procpath, '\0', 255);
	memset(fpath, '\0', 255);
	sprintf(procpath, "/proc/%d/fd/%d", pid, fd);
	if (readlink(procpath, fpath, 255) < 0) {
		perror("readlink Error");
	}

	dprintf(log_fd, "%s,%zu,%zu\n", fpath, f_offset, count);

	original_read = dlsym(RTLD_NEXT, "read");
	ret = (*original_read)(fd, buf, count);
	return ret;
}