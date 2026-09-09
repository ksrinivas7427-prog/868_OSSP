#include <stdio.h>
#include <stdlib.h>

int main() {
    int choice;
    while (1) {
     printf("CPU Scheduling Algorithms\n");
     printf("1. FCFS \n");
     printf("2. SJF \n");
     printf("3. SRTF\n");
     printf("4. Priority Non-preemptive\n");
     printf("5. Priority preemptive\n");
     printf("6. Round Robin\n");
     printf("7. Exit\n");
     printf("Select one of the CPU Scheduling Algorithms\n");
     scanf("%d", &choice);
     switch(choice) {
        case 1:
         printf("Operations of FCFS\n");
         break;
        case 2:
         printf("Operations of SJF\n");
         break;
        case 3:
         printf("Operations of SRTF\n");
         break;
        case 4:
         printf("Operations of Priority Non-preemptive\n");
          break;
        case 5:
         printf("Operations of Priority preemptive\n");
         break;
        case 6:
         printf("Operations of Round Robin\n");
         break;
        case 7:
         exit(0);
        }
     }
     return 0;
    }
