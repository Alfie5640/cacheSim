#include <stdio.h>
#include <stdlib.h>

int main() {
    int arr[1000];
    // random access pattern
    for (int i = 0; i < 1000; i++) {
        int index = rand() % 1000;
        arr[index]++;
    }
        
    printf("RANDOM ACCESS DONE\n");
    return 0;
}