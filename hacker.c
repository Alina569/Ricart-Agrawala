#include "utilities.h"

int main(int argc, char **argv){

	struct Message message;
	int status;
	int printer_queue = new_queue(PRINTER);
	if (printer_queue == -1) exit(-1);

	char buffer[1024] = {"Hacker ja ja ja"};

	printf("Hacker up: ID: %d", getpid());
	fflush(stdout);

	while(TRUE){
		int sleep_time = get_random(5,10);

		message.to = PRINTER;
		message.from = getpid();
		message.type = 0; // fixed values for the hacker messages
		message.request = 0; // same here

		sleep(sleep_time); // sleep random time

		strcpy(message.content, buffer);
		status = msgsnd(printer_queue, &message, 1024, 0);

		if (status == -1){
			printf("Failed message");
			exit(-1);
		}
		printf(".");
	}
	return 0;
}
