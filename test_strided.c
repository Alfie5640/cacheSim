#include <stdio.h>
#include <stdlib.h>

int main() {
    int arr[1000];
    // strided access pattern
    for (int i = 0; i < 1000; i += 7) {
        arr[i] = arr[i] + 1;
    }
        
    printf(" STRIDED DONE\n");
    return 0;
}