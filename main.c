#include <stdio.h>
#include <stdlib.h>

//todo check for empty inputs
int main() {
    char strings[100];

    char command[3];
    int state;
    char red;
    char written;
    char headShift;
    int nextState;
    //scan tr
    scanf("%s", command);
    printf("%s\n", command);
    while(scanf("%d %c %c %c %d",&state,&red,&written,&headShift,&nextState) == 5){
        printf("%d %c %c %c %d\n",state,red,written,headShift,nextState);
        //todo setup the fsm
    }
    
    //scan acc
    scanf("%s",command);
    printf("%s\n", command);
    int acceptedState;
    while(scanf("%d", &acceptedState) == 1){
        printf("%d\n",acceptedState);
    }

    //scan max
    scanf("%s",command);
    printf("%s\n", command);
    int maxTransitions; //todo see for long or double, make static
    scanf("%d",&maxTransitions);
    printf("%d\n",maxTransitions);

    //scan r
    scanf("%s",command);
    printf("%s\n", command);
    int i = 15;
    int j;
    while(scanf("%s",strings)==1 && i>0){ //todo make safer
        printf("%s\n",strings);
        i--;
    }

}