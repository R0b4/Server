#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *f = fopen("compressed.txt", "wb");
    for (int i = 0; i < 1000000; i++) {
        fprintf(f, "%i ", rand());
    }
    fflush(f);
    fclose(f);
}