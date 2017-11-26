#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

// global defines

#define TRUE 1
#define FALSE 0

#define PRINTER 1
#define REQUEST 2
#define REPLY 3

#define get_random(min, max) \
	        ((rand() % (int)(((max) + 1) - (min))) + (min))

// global structure

struct Message {
	int to;
	int from;
	int type;
	int request;
	char content[1024];
};

// global functions 

int new_queue(int pKey){
	key_t key = ftok(".", pKey);
	int queue = msgget(key, IPC_CREAT | 0660);
	return queue;
};
