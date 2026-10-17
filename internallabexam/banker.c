#include <stdio.h>

int n, m, a[10][10], mx[10][10], need[10][10];
int av[10], finish[10], seq[10], count = 0;

void safe(int k, int work[]) {
    if (k == n) {
        count++;
        printf("Sequence %d: ", count);
        for (int i = 0; i < n; i++)
            printf("P%d ", seq[i]);
        printf("\n");
        return;
    }

    for (int i = 0; i < n; i++) {
        if (!finish[i]) {
            int j;
            for (j = 0; j < m; j++)
                if (need[i][j] > work[j])
                    break;

            if (j == m) {
                finish[i] = 1;
                seq[k] = i;
                for (j = 0; j < m; j++)
                    work[j] += a[i][j];

                safe(k + 1, work);

                for (j = 0; j < m; j++)
                    work[j] -= a[i][j];
                finish[i] = 0;
            }
        }
    }
}

int main(void) {
    int work[10];

    printf("Enter processes and resources: ");
    scanf("%d%d", &n, &m);

    printf("Enter Allocation matrix:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            scanf("%d", &a[i][j]);

    printf("Enter Maximum matrix:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            scanf("%d", &mx[i][j]);
            need[i][j] = mx[i][j] - a[i][j];
        }

    printf("Enter Available resources:\n");
    for (int j = 0; j < m; j++) {
        scanf("%d", &av[j]);
        work[j] = av[j];
    }

    safe(0, work);

    if (count)
        printf("System is SAFE. Total sequences = %d\n", count);
    else
        printf("System is UNSAFE. No safe sequence.\n");

    return 0;
}
