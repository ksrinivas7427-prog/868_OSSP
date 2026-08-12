#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
int main(){
	int pid;
	printf("start\n");
	pid = fork(); //create child process
	if(pid == 0){ // body of child process
		printf("start of child\n");
		sleep(15); // child process sleep for 15sec
		exit(0); // force termination of child without completing child process
		printf("end of child\n");
	}
	else { // body of parent process
		sleep(10); // parent process sleep for 5 sec
		wait(NULL); // parent process wait for child process to complete
		printf("end\n");
	}
		return 0;
}
/* 
OUTPUT:

start
start of child
End of child
end 
*/

