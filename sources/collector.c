#include "parallelload.h"

static int log_fd;

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

	if ( (strncmp(fpath, "anon_inode:", 11) != 0
	        && strncmp(fpath, "socket:", 7) != 0
	        && strncmp(fpath, "pipe:", 5) != 0) ) {
		dprintf(log_fd, "%s,%zu,%zu\n", fpath, f_offset, count);
	}
	original_read = dlsym(RTLD_NEXT, "read");
	ret = (*original_read)(fd, buf, count);
	return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t (*original_fread)(void *ptr, size_t size, size_t nmemb, FILE * stream);
	size_t ret;

	pid_t pid = getpid();
	char proc_path[255];
	char fpath[255];
	loff_t stream_offset;
	int fd = fileno(stream);

	sprintf(proc_path, "/proc/%d/fd/%d", pid, fd);
	memset(fpath, '\0', sizeof(fpath));

	readlink(proc_path, fpath, sizeof(fpath));

	if (fseek(stream, 0, SEEK_CUR) == 0)
		stream_offset = ftell(stream);

	original_fread = dlsym(RTLD_NEXT, "fread");
	ret = (*original_fread)(ptr, size, nmemb, stream);

	if (!feof(stream)) {
		if ( (strncmp(fpath, "anon_inode:", 11) != 0
		        && strncmp(fpath, "socket:", 7) != 0
		        && strncmp(fpath, "pipe:", 5) != 0) ) {
			dprintf(log_fd, "%s,%zu,%zu\n", fpath, stream_offset, ret * size);
		}
	}

	return ret;
}