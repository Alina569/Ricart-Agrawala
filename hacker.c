#include "utilities.h"

int main(int argc, char **argv){
	srand(time(NULL));

	struct Message message;
	int status;
	int printer_queue = new_queue(PRINTER);

	char buffer[1024] = {"Hacker ja ja ja"};

	while(TRUE){
		int sleep_time = get_random(2,4);

		message.to = PRINTER;
		message.from = getpid();
		message.type = 0;
		message.request = 0;

		sleep(sleep_time);

		strcpy(message.content, buffer);
		status = msgsnd(printer_queue, &message, 1024, 0);

		if (status == -1){
			printf("Failed message");
			exit(-1);
		}
		printf("Message sent");
	}
	return 0;
}
