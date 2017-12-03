#include "utilities.h"

// declarations
int reply_queue, printer_queue, request_queue;
int *shared_memory;
// semaphores
int wait_sem, mutex_sem, nodes_sem;

int main(int argc, char **argv){
	// declarations
	int shmem_id, status;
	int request, reply, broadcast; // message communication
	struct Message request_message, received_message;

	pid_t pid = getpid(); // proc id

	//check the number of arguments
	if (argc < 2) {
		printf("Argument --Node number-- expected");
		exit(-1);
	}
	int node_id = atoi(argv[1]);

	// open queues
	reply_queue = new_queue(REPLY);
	printer_queue = new_queue(PRINTER);
	request_queue = new_queue(REQUEST);

	// node exclusive shared mem
	shmem_id = shmget(pid, 1024, IPC_CREAT|0640);

	// create sems
	// send messages connecting ....

	// attach sm
	shared_memory = shmat(shmem_id, (void *) 0, 0);
	shared_memory[0] = node_id;

	// status = send_message(100, request_queue, 100, "");
	shared_memory[100] = node_id;

	status = send_message(100, request_queue, 100, "", shared_memory);
	if (status == -1) {
		printf("Initial message error, check connection");
		exit(-1);
	}

	if (node_id == 1){
		shared_memory[1] = 1;
		shared_memory[3] = 0;
		shared_memory[300] = 0;
	} else {
		status = msgrcv(reply_queue, &received_message, MSG_QUEUE_SIZE, shared_memory[0], 0);
		if (status == -1) exit(-1);
		printf("Sponsor");

		char* token;
		int i;
		token = strtok(received_message.content, " ");
		sscanf(token, "%d", &i);
		shared_memory[3] = i-1;

		i = 1;
		token = strtok(NULL, " ");
		while(token != NULL) {
			sscanf(token, "%d", &shared_memory[100+i]);
			token = strtok(NULL, " ");
			i++;
		}

		sem_post(&nodes_sem);
		for (i=1; i < shared_memory[1] -1; i++){
			send_message(shared_memory[100 + 1], request_queue, 100, argv[1], shared_memory);
		}
		printf("ACK %d", shared_memory[1] -1);
		shared_memory[100] = shared_memory[1] -1;
		sem_wait(&nodes_sem);
	}


	/* =========== request reply print ============= */

	request = fork();

	if (request == 0){
		// some check
	} else {
		reply = fork();

		if (reply == 0){
			// check the reply queue
			// while true
		} else {
			broadcast = fork();
			if (broadcast == 0) {
				// check the broadcast queue
				// while true
			} else {
				// while TRUE
			}
		}
	}
	return 0;
}

void reply_handler(){
	shared_memory[4] = shared_memory[4] -1;
	sem_wait(&wait_sem);
}

void request_handler(int sequence, int r_node){
	int node, defer;
	printf("Node %d request permission", r_node);

	if (sequence > shared_memory[3]){
		shared_memory[3] = sequence;
	}

	sem_post(&mutex_sem);
	node = get_node(r_node, shared_memory);
	defer = shared_memory[5] && 
		((sequence > shared_memory[2]) ||
		(sequence == shared_memory[2] && 
		 r_node > shared_memory[0]));
	sem_wait(&mutex_sem);

	if (defer) {
		shared_memory[200 + node] = 1; // set priority 1
	} else {
		send_message(r_node, reply_queue, MSG_REPLY, "", shared_memory);
	}
}

void print_handler() {
	int i, lines;
	char buffer[1024];

	sem_post(&mutex_sem);
	shared_memory[5] = 1;
	shared_memory[2] = shared_memory[3]++;
	sem_wait(&mutex_sem);

	shared_memory[4] = shared_memory[1]-1;

	printf("Broadcast for permission\n");
	for(i=1; i < shared_memory[1]; i++){
		send_message(shared_memory[100+i], request_queue, MSG_REQUEST, "", shared_memory);
	}
	printf("Done broadcast\n");

	while(shared_memory[4] != 0) {
		sem_wait(&wait_sem);
	}

	printf("Permissions ok\n");

	snprintf(buffer, sizeof(buffer), " #### START OUTPUT NODE %i ####", shared_memory[0]);
	send_message(PRINTER, printer_queue, MSG_REQUEST, buffer, shared_memory);

	lines = get_random(3, 5);
	int counter = 0;
	while (counter < lines){
		sleep(1);
		memset(buffer, 0, sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "%d: %d line of output. %d total.", shared_memory[0], counter, lines);
		send_message(PRINTER, printer_queue, MSG_REQUEST, buffer, shared_memory);
		counter++;
	}
	sleep(1);
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), " ==== END OUTPUT FOR NODE %i ====", shared_memory[0]);
	send_message(PRINTER, printer_queue, MSG_REQUEST, buffer, shared_memory);

	shared_memory[5] = 0;

	for(i=1; i < shared_memory[1]; i++) {
		if (shared_memory[200 + i]) {
			shared_memory[200 + i] = 0;
			send_message(shared_memory[100 + i], reply_queue, MSG_REPLY, "", shared_memory);
		}
	}
	printf("DONE\n");
}

