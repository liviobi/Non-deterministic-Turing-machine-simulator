#include <stdio.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1
#define STARTINGLEN 50
#define CHARACTERS 27

typedef struct transition_s{
    char written;
    char headShift;
    int  nextState;
    struct transition_s*nextTransition;
}transition;

typedef struct state_s {
    int acceptState;
    transition **transitions; //todo can make it
} state;

int maxTransitions; //todo see for long or double
state **states; //todo non funziona se gli stati hanno numeri grandi

void setMaxTransitions();

void setAcceptedStates();

void setupStates();

void printFsm();

void freeTransition(transition*transitionToFree);


void freeStatesBlocks();

//todo check for empty inputs
int main() {
    char strings[100];
    states = (state **)malloc(STARTINGLEN*sizeof(state));
    for (int stateToInit = 0; stateToInit < STARTINGLEN; stateToInit++) {
        states[stateToInit] = NULL;
    }

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

    freeStatesBlocks();


}

void freeStatesBlocks() {
    for(int stateIndex = 0; stateIndex < STARTINGLEN; stateIndex++){
        if(states[stateIndex]!= NULL){
            if(states[stateIndex]->transitions != NULL){
                for(int transitionCharacterIndex = 0; transitionCharacterIndex < CHARACTERS;transitionCharacterIndex++){
                   freeTransition(states[stateIndex]->transitions[transitionCharacterIndex]);
                }
                free(states[stateIndex]->transitions);
            }
            free(states[stateIndex]);

        }
    }
    free(states);
}

void freeTransition(transition*transitionToFree){
    if(transitionToFree == NULL) {
        return;
    }else{
        freeTransition(transitionToFree->nextTransition);
        free(transitionToFree);
    }

}

void printFsm() {
    for(int i =0; i < STARTINGLEN; i++){
        if (states[i] != NULL){
            printf("\n%d ",i);
            if(states[i]->acceptState){
                printf("accept");
            }
            printf("\n");
            if(states[i]->transitions !=NULL){
                for(int transitionCharIndex = 0; transitionCharIndex<CHARACTERS;transitionCharIndex++){
                    transition *transitionToPrint = states[i]->transitions[transitionCharIndex];
                    while (transitionToPrint != NULL){
                        char transitionChar = (char)transitionCharIndex + 95;
                        printf("%c %c %c %d \n",transitionChar,transitionToPrint->written,transitionToPrint->headShift,transitionToPrint->nextState);
                        transitionToPrint = transitionToPrint->nextTransition;

                    }

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
            newState->acceptState = FALSE;
            newState->transitions = (transition**)malloc(CHARACTERS*sizeof(transition*));
            for(int i = 0;i<CHARACTERS;i++){
                newState->transitions[i] = NULL;
            }
            transition* newTransition = (transition *)malloc(sizeof(transition));
            newTransition->nextState = nextState;
            newTransition->written = written;
            newTransition->headShift = headShift;
            newTransition->nextTransition = NULL;
            newState->transitions[red-95] = newTransition;
            states[currentState] = newState;

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
            newState->transitions = NULL;
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