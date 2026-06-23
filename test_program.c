#include <stdio.h>
#include <stdlib.h>

int main() {
    int arr[1000];
    
    // sequential access pattern
    for (int i = 0; i < 1000; i++) {
        arr[i] = i * 2;
    }
    
    // strided access pattern
    for (int i = 0; i < 1000; i += 7) {
        arr[i] = arr[i] + 1;
    }

    // random access pattern
    for (int i = 0; i < 1000; i++) {
        int index = rand() % 1000;
        arr[index]++;
    }
        
    printf("Done\n");
    return 0;
}