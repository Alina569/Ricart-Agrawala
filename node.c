#include "utilities.h"

#define ME 0

// declarations
int reply_queue, printer_queue, request_queue;
int *shared_memory;
// semaphores
sem_t wait_sem, mutex_sem, nodes_sem;

int main(int argc, char **argv){
	// declarations
	int shmem_id, status;
	int request, reply, broadcast; // message communication
	struct Message request_message, received_message;

	pid_t pid = getpid(); // proc id
	int flag = TRUE;

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
	int wait = sem_init(&wait_sem, 1, 1);
	int mutex = sem_init(&mutex_sem, 1, 1);
	int nodes_s = sem_init(&nodes_sem, 1, 1);

	// attach sm
	shared_memory = shmat(shmem_id, (void *) 0, 0);
	shared_memory[ME] = node_id;

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
		status = msgrcv(reply_queue, &request_message, MSG_QUEUE_SIZE, shared_memory[ME], 0);
		if (status == -1) exit(-1);
		printf("Sponsor");

		char* token;
		int i;
		token = strtok(request_message.content, " ");
		sscanf(token, "%d", &i);
		shared_memory[1] = i+1;
		token = strtok(NULL, " ");
		sscanf(token, "%d", &i);
		shared_memory[3] = i-1;

		i = 1;
		token = strtok(NULL, " ");
		while(token != NULL) {
			sscanf(token, "%d", &shared_memory[100+i]);
			token = strtok(NULL, " ");
			i++;
		}

		sem_wait(&nodes_sem); // P
		for (i=1; i < shared_memory[1]; i++){
			send_message(shared_memory[100 + i], request_queue, 100, argv[1], shared_memory);
		}
		printf("ACK %d", shared_memory[1] -1);
		shared_memory[300] = shared_memory[1] -1;
		sem_post(&nodes_sem); // V
	}


	/* =========== request reply print ============= */

	request = fork();

	if (request == 0){
		while(TRUE){
			status = msgrcv(request_queue, &received_message, MSG_QUEUE_SIZE, shared_memory[ME], 0);
			if (status == -1){
				break;
			}
			printf("NODE %ld is requesting access \n", received_message.from);
			switch(received_message.type){
				case 100:
					sem_wait(&nodes_sem); // P
					printf("ACK node: %ld\n", received_message.from);
					shared_memory[100+shared_memory[1]] = atoi(received_message.content);
					shared_memory[1]++;
					send_message(received_message.from, reply_queue, MSG_SET, "", shared_memory);
					sem_post(&nodes_sem); // V
				break;
				default:
					sem_wait(&nodes_sem); // P
					request_handler(received_message.request, received_message.from);
					sem_post(&nodes_sem); // V
				break;
			}
		}
	} else {
		reply = fork();

		if (reply == 0){
			while(TRUE){
                status = msgrcv(reply_queue, &received_message, MSG_QUEUE_SIZE, shared_memory[ME], 0);
				if (status == -1) {
					break;
				}
				switch(received_message.type){
					case MSG_SET:
						printf("NODE %ld agree \n", received_message.from);
						shared_memory[300] = shared_memory[300] -1;
						if (shared_memory[300] == 0){
							printf("READY\n");
						}
					break;
					default:
						printf("REPLY NODE %ld \n", received_message.from);
						reply_handler();
					break;
				}
			}
		} else {
			char buffer[1024];
			broadcast = fork();
			if (broadcast == 0) {
				while(TRUE) {
					status = msgrcv(request_queue, &received_message, MSG_QUEUE_SIZE, 100, 0);
					if (status == -1){
						break;
					}
					if (received_message.from != shared_memory[ME]){
						sem_wait(&nodes_sem); // P
						printf("NEW NODE %ld \n", received_message.from);
						buffer[0] = shared_memory[1] + '0';
						buffer[1] = ' ';
						buffer[2] = shared_memory[3] + '0';
						int position = 3;
						int current;

						for (current=0; current < shared_memory[1]; current++){
							buffer[position] = ' ';
							buffer[position +1] = shared_memory[100 + current] + '0';
							position += 2;
						}
						buffer[position] = '\0';
						sem_post(&nodes_sem); // V
						send_message(received_message.from, reply_queue, 100, buffer, shared_memory);
					}
				}
			} else {
				while(TRUE) {
					if (shared_memory[300] == 0) {
						printf("TRY ENTER\n");
						getchar();
						if (flag){
							printf("WRITE\n");
							shared_memory[300] = 1;
							sem_wait(&nodes_sem); // P
							printer_handler();
							shared_memory[300] = 0;
							sem_post(&nodes_sem); // V

						}
					}
				}
				printf("BYE");
				shmctl(shmem_id, IPC_RMID, (struct shmid_ds *) 0);
				return 0;
			}
		}
	}
	return 0;
}

void reply_handler(){
	shared_memory[4] = shared_memory[4] -1;
	sem_post(&wait_sem); // V
}

void request_handler(int sequence, int r_node){
	int node, defer;
	printf("Node %d request permission", r_node);

	if (sequence > shared_memory[3]){
		shared_memory[3] = sequence;
	}

	sem_wait(&mutex_sem); // P
	node = get_node(r_node, shared_memory);
	defer = shared_memory[5] &&
		((sequence > shared_memory[2]) ||
		(sequence == shared_memory[2] &&
		 r_node > shared_memory[ME]));
	sem_post(&mutex_sem); // V

	if (defer) {
		shared_memory[200 + node] = 1; // set priority 1
	} else {
		send_message(r_node, reply_queue, MSG_REPLY, "", shared_memory);
	}
}

void printer_handler() {
	int i, lines;
	char buffer[1024];

	sem_wait(&mutex_sem); // P
	shared_memory[5] = 1;
	shared_memory[2] = shared_memory[3]++;
	sem_post(&mutex_sem); // V

	shared_memory[4] = shared_memory[1]-1;

	printf("Broadcast for permission\n");
	for(i=1; i < shared_memory[1]; i++){
		send_message(shared_memory[100+i], request_queue, MSG_REQUEST, "", shared_memory);
	}
	printf("Done broadcast\n");

	while(shared_memory[4] != 0) {
		sem_wait(&wait_sem); //P
	}

	printf("Permissions ok\n");

	snprintf(buffer, sizeof(buffer), " #### START OUTPUT NODE %i ####", shared_memory[ME]);
	send_message(PRINTER, printer_queue, MSG_REQUEST, buffer, shared_memory);

	lines = get_random(3, 5);
	int counter = 0;
	while (counter < lines){
		sleep(1);
		memset(buffer, 0, sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "%d: %d line of output. %d total.", shared_memory[ME], counter, lines);
		send_message(PRINTER, printer_queue, MSG_REQUEST, buffer, shared_memory);
		counter++;
	}
	sleep(1);
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), " ==== END OUTPUT FOR NODE %i ====", shared_memory[ME]);
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
