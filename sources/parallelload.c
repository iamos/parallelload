#include "parallelload.h"

static b_tree* b_root;
static int last_hash;

void* do_posix_fadvise(void* data) {
	fdata* will_load = (fdata*)data;
	int fd = open(will_load->path, O_RDONLY);
	posix_fadvise(fd, will_load->offset, will_load->length, POSIX_FADV_WILLNEED);
	close(fd);
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
	fdata* data = b_find_f(b_root, *(int*)args);
	if( data != NULL){
		// using thread or not
		int fd = open(data->path, O_RDONLY);
		posix_fadvise(fd, data->offset, data->length, POSIX_FADV_WILLNEED);
		close(fd);
	}
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
		sscanf(line, "%[^,],%zd,%zd", filepath, &offset, &length);
		// printf("%s, %zd, %zd\n", filepath, offset, length);

		int key = hash(filepath);
		fdata* newf = new_fdata(filepath, offset, length);

		// printf("%s, %zd, %zd\n", newf->path, newf->offset, newf->length);
		b_tree* b_temp = b_init(key, newf);
		if (b_root == NULL) {
			b_root = b_temp;
		}
		b_insert(b_root, b_temp);
	}
	// b_travel(b_root);
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
	last_hash = 0;
	pthread_t start_main_thread;
	if ( pthread_create(&start_main_thread, NULL, do_prepare, (void*)0) < 0) {
		perror("Error : read pthread_create");
	}
	pthread_join(start_main_thread, NULL);

	original__libc_start_main = dlsym(RTLD_NEXT, "__libc_start_main");
	return original__libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

ssize_t read(int fd, void *buf, size_t count) {
	ssize_t (*original_read)(int fd, void *buf, size_t count);
	ssize_t ret;

	int pid = getpid();
	char procpath[255];
	char fpath[255];
	memset(procpath, '\0', 255);
	memset(fpath, '\0', 255);
	sprintf(procpath, "/proc/%d/fd/%d", pid, fd);
	if (readlink(procpath, fpath, 255) < 0) {
		perror("readlink Error");
	}
	int my_key = hash(fpath);
	if (last_hash != my_key) {
		last_hash = my_key;
		pthread_t load_thread;
		if ( pthread_create(&load_thread, NULL, do_load, (void*)&my_key ) < 0) {
			perror("Error : pthread_create ( load_thread )");
		}
		// pthread_join(load_thread, NULL);
	}
	original_read = dlsym(RTLD_NEXT, "read");
	ret = (*original_read)(fd, buf, count);
	return ret;
}