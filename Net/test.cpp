#include <stdio.h>

int main() {
    for (int t, a = 1, b = 1; 
            a > 0; 
            t = b, b = a + b, a = t) 
                printf("%i\n", a);
}