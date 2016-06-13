
#define _GNU_SOURCE
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/mman.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <dlfcn.h>

void* do_posix_fadvise(void* data) {
	printf("Prefetch\n");
	pthread_exit((void*)0);
}

void* do_hi(void* args){
	printf("HI, i'm child\n");
	pthread_exit((void*)0);
	return NULL;
}

void* do_load(void* args){
	// Find targets by args
	// IF Find; do_posix_fadvise
	// Else; pthread_exit();
	pthread_exit(0);
}

int __libc_start_main(int (*main) (int, char **, char **), int argc, char **ubp_av,
	void (*init) (void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end)) {
	int (*original__libc_start_main)(int (*main) (int, char **, char **), int argc, char **ubp_av,
	                                 void (*init) (void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end));
	mlockall(MCL_CURRENT | MCL_FUTURE);

	// fd_log = open("/home/shared/prefetch/logs/log", O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 00666);
	pthread_t start_main_thread;
	if( pthread_create(&start_main_thread, NULL, do_hi, (void*)0) < 0){
		perror("Error : read pthread_create");
	}
	pthread_join(start_main_thread,NULL);
	
	original__libc_start_main = dlsym(RTLD_NEXT, "__libc_start_main");
	return original__libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	ssize_t (*original_read)(int fildes, void *buf, size_t nbyte);
	ssize_t ret;
	pthread_t read_thread;
	if( pthread_create(&read_thread, NULL, do_hi, (void*)0) < 0){
		perror("Error : read pthread_create");
	}
	pthread_join(read_thread,NULL);
	original_read = dlsym(RTLD_NEXT, "read");
	ret = (*original_read)(fildes, buf, nbyte);
	return ret;
}