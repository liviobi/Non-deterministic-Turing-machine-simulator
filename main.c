#include <stdio.h>
#include <stdlib.h>

int main() {
    int a;
    int b;
    if(scanf("%d %d", &a, &b) > 0){
        int c = a + b;
        printf("%d",c);
    }
    return 0;
}