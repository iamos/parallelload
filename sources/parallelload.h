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
	char path[256];
	size_t offset;
	size_t length;
} fdata;

fdata* new_fdata(char* path, size_t offset, size_t length) {
	fdata* newfdata = (fdata*)malloc(sizeof(fdata));
	strcpy(newfdata->path, path);
	newfdata->offset = offset;
	newfdata->length = length;
	return newfdata;
}
typedef struct b_tree {
	int key;
	struct b_tree* left;
	struct b_tree* right;
	void* data;
} b_tree;

b_tree* b_init(int key, fdata* data) {
	b_tree* newb = (b_tree*)malloc(sizeof(b_tree));
	newb->key = key;
	newb->left = NULL;
	newb->right = NULL;
	newb->data = (void*) data;
	return newb;
}

void b_insert(b_tree* root, b_tree* newb) {
	int key = newb->key;
	b_tree* me = root;
	while (me != NULL) {
		if (me->key == key) {
			/* if Hole; may cause bugs */
			if ( ((fdata*)(me->data))->offset + ((fdata*)(me->data))->length == ((fdata*)(newb->data))->offset) {
				((fdata*)(me->data))->length += ((fdata*)(newb->data))->length;
			}
			return;
		}
		else if (me->key > key) {
			// printf("%p -> %p\n", me, me->left);
			if ( me->left == NULL) {
				me->left = newb;
				return;
			}
			me = me->left;
		}
		else {
			if ( me->right == NULL) {
				me->right = newb;
				return;
			}
			me = me->right;
		}
	}
}

fdata* b_find_f(b_tree* root, int key){
	b_tree* me = root;
	while( me != NULL){
		if( me->key == key ){
			return (fdata*)(me->data);
		}
		else if( me->key > key){
			me = me->left;
		}
		else {
			me = me->right;
		}
	}
	return NULL;
}

void b_travel(b_tree* root) {
	if (root != NULL) {
		printf("%s, %zu, %zu\n", ((fdata*)(root->data))->path, ((fdata*)(root->data))->offset, ((fdata*)(root->data))->length);
		b_travel(root->left);
		b_travel(root->right);
	}
	// printf("NULL\n");
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
