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
#include <string.h>

int hash(char*);

typedef struct _fdata {
	int fd;
	void* buf;
	size_t count;
} fdata;

fdata* new_fdata(int fd, void* buffer, size_t count) {
	fdata* newfdata = (fdata*)malloc(sizeof(fdata));
	newfdata->fd = fd;
	newfdata->buf = buffer;
	newfdata->count = count;
	return newfdata;
}
typedef struct b_tree{
	int key;
	struct b_tree* left;
	struct b_tree* right;
	void* data;
} b_tree;

b_tree* b_init(int key){
	b_tree* newb = (b_tree*)malloc(sizeof(b_tree));
	newb->key = key;
	newb->left = NULL;
	newb->right = NULL;
	newb->data = NULL;
	return newb;
}

void b_insert(b_tree* root, b_tree* newb){
	
}

typedef union {
	int num;
	char str[4];
} h_set;

int hash(char* input) {
	int length = strlen(input);
	int hval = 0;
	h_set hset;
	hset.num = 0;
	int i, j;
	int remain = length % 4;
	for (i = 0; i < (length - remain); i += 4) {
		for (j = 0; j < 4; j++) {
			hset.str[j] = input[i + j];
		}
		hval ^= hset.num;
	}
	if (remain != 0) {
		h_set temp_set;
		temp_set.num = 0;
		for (j = 0; j < remain; j++) {
			temp_set.str[j] = input[i + j];
		}
		hval ^= temp_set.num;
	}
	return hval;
}