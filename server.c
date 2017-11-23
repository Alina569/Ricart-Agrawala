#include "utilities.h"

int main(int argc, char **argv){

	//server globals
	struct Message msg_received;
	int printer_queue, request_queue, reply_queue;
	int status;

	// create the queues
	printer_queue = new_queue(PRINTER);
	request_queue = new_queue(REQUEST);
	reply_queue = new_queue(REPLY);
	
	while(TRUE) {
		status = msgrcv(printer_queue, &msg_received, 1024, PRINTER, 0);
		if (status == -1){
			break;
		}
		printf("%s\n", msg_received);
	}
	
	// delete queue

	return 0;
}
