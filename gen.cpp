#include <stdio.h>
#include <stdlib.h>

#define WIDTH 256

int main(){
    FILE *f = fopen("gen.txt", "wb");

    for (int i = 0; i < 100000000; i++) {
        char aplha = rand() % ('z' - 'a') + 'a';
        putc(aplha, f);
    }
}