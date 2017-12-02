#include "utilities.h"

int main(int argc, char **argv){
	// declarations
	int reply_queue, printer_queue, request_queue;
	int *shared_memory, shmem_id, status;
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

	if (node_id == 1){
		// do something
	} else {
		// do something with the rest of the nodes
	}

	request = fork();

	if (request == 0){
		// check the request queue
		// while TRUE
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

