#include <stdio.h>
#include <stdlib.h>

int main() {
    int n, arr[10];

    printf("Enter the size of array: ");
    scanf("%d", &n);


    for (int i = 0; i < n; i++) {
        printf("Enter %d element: ", i + 1);
        scanf("%d", &arr[i]);
    }

    for (int i = 0; i < n; i++) {
        printf("%d element is: %d\n", i + 1, arr[i]);
    }

    return 0;
}
