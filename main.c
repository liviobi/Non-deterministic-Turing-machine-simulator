#include <stdio.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1
#define STARTINLEN 50
#define CHARACTERS 27
#define NEXTSTATE

typedef struct transition_s{
    char written;
    char headShift;
    int  nextState;
    struct transition_s*nextTransition;
}transition;

typedef struct state_s {
    int acceptState;
    transition *transitions[CHARACTERS];
} state;

int maxTransitions; //todo see for long or double
state *states[STARTINLEN]; //todo non funziona se gli stati hanno numeri grandi

void setMaxTransitions();

void setAcceptedStates();

void setupStates();

void printFsm();

//todo check for empty inputs
int main() {
    char strings[100];

    setupStates();
    setAcceptedStates();
    setMaxTransitions();
    //scan r
    char command[3];
    scanf("%s",command);
    printf("%s\n", command);
    while(scanf("%s",strings)==1){ //todo make safer
        printf("%s\n",strings);
    }

    printFsm();

}

void printFsm() {
    for(int i =0; i < STARTINLEN; i++){
        if (states[i] != NULL){
            printf("\n%d ",i);
            if(states[i]->acceptState){
                printf("accept \n");
            }
            for(int transitionCharIndex = 0; transitionCharIndex<CHARACTERS;transitionCharIndex++){
                transition *transitionToPrint = states[i]->transitions[transitionCharIndex];
                while (transitionToPrint != NULL){
                    char transitionChar = (char)transitionCharIndex + 95;
                    printf("%c ",transitionChar);
                    printf("%c %c %d \n",transitionToPrint->written,transitionToPrint->headShift,transitionToPrint->nextState);
                    transitionToPrint = transitionToPrint->nextTransition;

                }

            }
        }
    }
}

void setupStates() {
    char command[3];
    int currentState;
    char red;
    char written;
    char headShift;
    int nextState;

    //scan tr
    scanf("%s", command);
    printf("%s\n", command);
    while(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5){
        printf("%d %c %c %c %d\n",currentState,red,written,headShift,nextState);
        if(states[currentState]== NULL){
            state* newState = (state *)malloc(sizeof(state));
            states[currentState] = newState;
            newState->acceptState = FALSE;
            transition* newTransition = (transition *)malloc(sizeof(transition));
            newTransition->nextState = nextState;
            newTransition->written = written;
            newTransition->headShift = headShift;
            newTransition->nextTransition = NULL;
            newState->transitions[red-95] = newTransition;

        }else{
            transition* newTransition = (transition *)malloc(sizeof(transition));
            newTransition->nextState = nextState;
            newTransition->written = written;
            newTransition->headShift = headShift;
            newTransition->nextTransition = states[currentState]->transitions[red-95];
            states[currentState]->transitions[red-95] = newTransition;
            }

    }
}

void setAcceptedStates() {//scan acc
    char command[3];
    scanf("%s",command);
    printf("%s\n", command);
    int acceptedState;
    while(scanf("%d", &acceptedState) == 1){
        printf("%d\n",acceptedState);
        if (states[acceptedState] != NULL){
            states[acceptedState]->acceptState = TRUE; //should never be executed
        }else{
            state* newState = (state *)malloc(sizeof(state));
            states[acceptedState] = newState;
            newState->acceptState = TRUE;
        }
    }
}

void setMaxTransitions() {//scan max
    char command[3];
    scanf("%s",command);
    printf("%s\n", command);
    scanf("%d",&maxTransitions);
    printf("%d\n",maxTransitions);
}