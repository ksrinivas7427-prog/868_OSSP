
#include <stdio.h>

int main(void)
{
    int n, f;
    int pages[100], frames[20], last[20];
    int faults = 0, hits = 0, time = 0;

    printf("Enter number of pages: ");
    scanf("%d", &n);

    printf("Enter page reference string:\n");
    for (int i = 0; i < n; i++)
        scanf("%d", &pages[i]);

    printf("Enter number of frames: ");
    scanf("%d", &f);

    if (n < 1 || n > 100 || f < 1 || f > 20) {
        printf("Invalid input.\n");
        return 1;
    }

    for (int i = 0; i < f; i++) {
        frames[i] = -1;
        last[i] = -1;
    }

    printf("\nPage\tFrames\t\tStatus\n");

    for (int i = 0; i < n; i++) {
        int found = -1;

        for (int j = 0; j < f; j++) {
            if (frames[j] == pages[i]) {
                found = j;
                break;
            }
        }

        time++;

        if (found != -1) {
            hits++;
            last[found] = time;
        } else {
            faults++;
            int pos = -1;

            for (int j = 0; j < f; j++) {
                if (frames[j] == -1) {
                    pos = j;
                    break;
                }
            }

            if (pos == -1) {
                pos = 0;

                for (int j = 1; j < f; j++) {
                    if (last[j] < last[pos])
                        pos = j;
                }
            }

            frames[pos] = pages[i];
            last[pos] = time;
        }

        printf("%d\t", pages[i]);

        for (int j = 0; j < f; j++) {
            if (frames[j] == -1)
                printf("- ");
            else
                printf("%d ", frames[j]);
        }

        if (found != -1)
            printf("\tHit\n");
        else
            printf("\tPage Fault\n");
    }

    printf("\nTotal Page Faults = %d\n", faults);
    printf("Total Hits = %d\n", hits);
    printf("Page Fault Ratio = %.2f\n", (float)faults / n);

    return 0;
}

