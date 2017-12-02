#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>

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
#define MSG_QUEUE_SIZE 1024 + sizeof(long int)

#define get_random(min, max) \
	        ((rand() % (int)(((max) + 1) - (min))) + (min))

// global structure

struct Message {
	long int to;
	long int from;
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

char* concant(const char *string1, const char *string2){

	const size_t len_str2 = strlen(string2);

	char *result = malloc(len_str1 + len_str2 + 1);
	memcpy(result, string1, len_str1);
	memcpy(result + len_str1, string2, len_str2 + 1);

	return result;
};

// node and messages functions

int get_node(int id, int *shared_memory){
	int i;
	for (i = 1; i < shared_memory[1]; i++){
		if (shared_memory[200 + i] == id) {
			return i;
		}
	}
	return -1;
}

int send_message(int to, int queue, int type, char buffer[1024], int *shared_memory){
	struct Message message;

	message.to = to;
	message.from = shared_memory[0];
	message.type = type;
	message.request = shared_memory[2];
	strcpy(message.content, buffer);

	return msgsnd(queue, &message, MSG_QUEUE_SIZE, 0);

}
