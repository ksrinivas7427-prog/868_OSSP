#include <stdio.h>
#include <string.h>
int main() {
	char command[100];
	while(1) {
		//Dispaly prompt
		printf("myshell> ");
		fflush(stdout);
		// Read user input
		fgets(command, sizeof(command), stdin);
		//remove newline
		command[strcspn(command, "\n")] = '\0';
		//Check exit condition
		if (strcmp(command, "exit") == 0) {
			printf("Exiting shell...\n");
			break;
		}
		//Prcess command
		printf("You enterd: %s\n", command);
	}
	return 0;
/*
OUTPUT: 
myshell> hello
You enterd: hello
myshell> exit
Exiting shell...*/
	
}
