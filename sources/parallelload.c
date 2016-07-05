#include "parallelload.h"

static b_tree* b_root;

void* do_posix_fadvise(void* data) {
	printf("Prefetch\n");
	pthread_exit((void*)0);
}

void* do_hi(void* args) {
	printf("HI, i'm child\n");
	pthread_exit((void*)0);
	return NULL;
}

void* do_load(void* args) {
	// Find targets by args
	// IF Find; do_posix_fadvise
	// Else; pthread_exit();
	pthread_exit(0);
}

void* do_prepare(void* args) {
	FILE* stream = fopen("/home/shared/parallelload/collect_log", "r");
	char* line = NULL;
	size_t len = 0;
	size_t read = 0;
	if (stream == NULL) {
		perror("do_prepare :");
	}
	while ( (read = getline(&line, &len, stream)) != -1 ) {
		char filepath[255];
		memset(filepath, '\0', 255);
		size_t offset;
		size_t length;
		sscanf(line,"%[^,],%zd,%zd", filepath, &offset, &length);
		// printf("%s, %zd, %zd\n", filepath, offset, length);
		
		int key = hash(filepath);
		fdata* newf = new_fdata(filepath, offset, length);
		if(b_root == NULL){
			b_init(b_root);
		}
		/*
			Make B-tree [ key : path, offset, length ]
		*/
		// if (b_root != NULL){

		// }
		// else{
			// b_root->key = key;
			// b_root->data = newf;
		// }
	}
	free(line);
	fclose(stream);
	pthread_exit(0);
}

int __libc_start_main(int (*main) (int, char **, char **), int argc, char **ubp_av,
                      void (*init) (void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end)) {
	int (*original__libc_start_main)(int (*main) (int, char **, char **), int argc, char **ubp_av,
	                                 void (*init) (void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end));
	mlockall(MCL_CURRENT | MCL_FUTURE);

	/* Do my things here */
	b_root = NULL;
	pthread_t start_main_thread;
	if ( pthread_create(&start_main_thread, NULL, do_prepare, (void*)0) < 0) {
		perror("Error : read pthread_create");
	}
	pthread_join(start_main_thread, NULL);

	original__libc_start_main = dlsym(RTLD_NEXT, "__libc_start_main");
	return original__libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	ssize_t (*original_read)(int fildes, void *buf, size_t nbyte);
	ssize_t ret;
	// pthread_t read_thread;
	// if ( pthread_create(&read_thread, NULL, do_hi, (void*)0) < 0) {
	// 	perror("Error : read pthread_create");
	// }
	// pthread_join(read_thread, NULL);
	/* 
		if last == hash_key
			NULL
		else
			do_prefetch
	*/
	original_read = dlsym(RTLD_NEXT, "read");
	ret = (*original_read)(fildes, buf, nbyte);
	return ret;
}