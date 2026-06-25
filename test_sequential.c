#include <stdio.h>
#include <stdlib.h>

int main() {
    int arr[1000];
    
    // sequential access pattern
    for (int i = 0; i < 1000; i++) {
        arr[i] = i * 2;
    }
        
    printf("SEQUENTIAL DONE\n");
    return 0;
}