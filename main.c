#include <stdio.h>
#include <stdlib.h>
#define FALSE 0
#define TRUE 1
#define STARTINGLEN 50
#define STRINGLEN 10
#define CHARACTERS 58

typedef  struct execution_s{
    int state;
    int iteration;
    int cursor;
    int inputStringLen;
    int leftBlanksLen;
    char*inputString;
    char*leftBlanks;
    struct execution_s*nextExecution;
}execution;

typedef struct transition_s{
    char written;
    char headShift;
    int  nextState;
    struct transition_s*nextTransition;
}transition;

typedef struct state_s {
    int acceptState;
    transition **transitions;
} state;

int maxIterations; //todo see for long or double
state **states;
execution*executions;
int result = -1;

void setMaxTransitions();

void setAcceptedStates();

void setupStates();

void printFsm();

void freeTransition(transition*transitionToFree);


void freeStatesBlocks();

execution *newExecution(int,int,int,int,int);

int createFirstExecution();

execution *cloneExecution(execution *executionToClone);

void reallocInputString(execution *executionToModify);

void reallocLeftBlanks(execution *executionToModify);

int makeTransition(execution*executionToUpdate,transition*transitionToApply);

char readFromTape(execution *runningExecution);

void freeExecution(execution *executionToFree);

void executeAllTransition(char red);

void freeAllExecutions();

void freeFirstExecution();

void printResult();

transition *createNewTransition(char written, char headShift, int nextState);

state *createNewState(int stateToCreate);

//todo check for empty inputs
int main() {
    setupStates();
    setAcceptedStates();
    setMaxTransitions();

    //scan r
    char command[3];
    scanf("%s",command);
    //printf("%s\n", command);

    while(createFirstExecution() == 1) {
        while (executions != NULL) {
            char redChar = readFromTape(executions);
            executeAllTransition(redChar);
            //printf("ended trasition for %c\n\n", redChar);
            freeFirstExecution();
        }
        printResult();
        result = -1;
    }

   //printFsm();

    freeStatesBlocks();
    return 0;

}

void printResult() {
    if(result == 1){
        printf("1\n");
    }else if(result == 0){
        printf("U\n");
    }else{
        printf("0\n");
    }
}

void executeAllTransition(char red) {
    transition* transitionForRedChar = states[executions->state]->transitions[(int)red - 65];
    while(transitionForRedChar != NULL){
        execution*clonedExecution = cloneExecution(executions);
        executions->nextExecution = clonedExecution;
        //printf("char red %c starting from state %d iteration %d cursor %d\n",red,clonedExecution->state,clonedExecution->iteration,clonedExecution->cursor);
        int transitionResult = makeTransition(clonedExecution,transitionForRedChar);
        if(transitionResult == 0){
            result = 0;
            executions->nextExecution = clonedExecution->nextExecution;
            freeExecution(clonedExecution);
        }else if(transitionResult == 1){
            result = 1;
            freeAllExecutions();
            return;
        }
        transitionForRedChar = transitionForRedChar->nextTransition;
    }
}

void freeAllExecutions() {
    while(executions != NULL){
        freeFirstExecution();
    }

}

void freeFirstExecution() {
    if(executions != NULL){
        execution* toDelete = executions;
        executions = toDelete->nextExecution;
        freeExecution(toDelete);
    }
}

void freeExecution(execution *executionToFree) {
    free(executionToFree->inputString);
    free(executionToFree->leftBlanks);
    free(executionToFree);
}

char readFromTape(execution *runningExecution) {
    if(runningExecution->cursor >= 0){
        return runningExecution->inputString[runningExecution->cursor];
    }else{
        return runningExecution->leftBlanks[runningExecution->cursor * (-1)];
    }
}

int makeTransition(execution*executionToUpdate,transition*transitionToApply) {
    executionToUpdate->iteration++;
    if(executionToUpdate->iteration>maxIterations){
        //exceded maximum iterations
        //printf("\n\n\nexceded maximum iterations\n");
        return 0;
    }
    executionToUpdate->state = transitionToApply->nextState;
    if(states[executionToUpdate->state]->acceptState){
        //found final state
       // printf("\n\n\nfound final staten\n");
        return 1;
    }

    if(executionToUpdate->cursor>=0){
        executionToUpdate->inputString[executionToUpdate->cursor] = transitionToApply->written;
        if(transitionToApply->headShift =='L'){
            executionToUpdate->cursor--;
            //printf("state %d iteration %d cursor %d written %c moving L\n",executionToUpdate->state,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }else if(transitionToApply->headShift == 'R'){
            executionToUpdate->cursor++;
           // printf("state %d iteration %d cursor %d written %c moving R\n",executionToUpdate->state,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
            if(executionToUpdate->cursor==executionToUpdate->inputStringLen){
                reallocInputString(executionToUpdate);
            }
        }else{
            //printf("state %d iteration %d cursor %d written %c not moving\n",executionToUpdate->state,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }
    }else{
        executionToUpdate->leftBlanks[executionToUpdate->cursor*(-1)] = transitionToApply->written;
        if(transitionToApply->headShift =='L'){
            executionToUpdate->cursor--;
           // printf("state %d iteration %d cursor %d written %c moving blank L\n",executionToUpdate->state,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
            if(executionToUpdate->cursor*(-1) == executionToUpdate->leftBlanksLen){
                reallocLeftBlanks(executionToUpdate);
            }
        }else if(transitionToApply->headShift == 'R'){
            executionToUpdate->cursor++;
            //printf("state %d iteration %d cursor %d written %c moving blank R\n",executionToUpdate->state,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }else{
            //printf("state %d iteration %d cursor %d written %c leaving blank\n",executionToUpdate->state,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }
    }
    return -1;
}

execution *cloneExecution(execution *executionToClone) {
    execution*clonedExecution = newExecution(executionToClone->state,executionToClone->cursor,executionToClone->iteration,executionToClone->inputStringLen,executionToClone->leftBlanksLen);
    clonedExecution->nextExecution = executionToClone->nextExecution;
    for(int copiedCharIndex = 0; copiedCharIndex<executionToClone->inputStringLen;copiedCharIndex++){
        clonedExecution->inputString[copiedCharIndex] = executionToClone->inputString[copiedCharIndex];
    }
    for(int copiedBlank = 0; copiedBlank < executionToClone->leftBlanksLen;copiedBlank++){
        clonedExecution->leftBlanks[copiedBlank] = executionToClone->leftBlanks[copiedBlank];
    }
    return clonedExecution;
}

int createFirstExecution() {
    execution*firstExec = newExecution(0, 0, 0, STRINGLEN, STRINGLEN);
    char c;
    int end;
    end = scanf(" %c", &c);
    for(int i = 0;c != '\n' && end != EOF;i++){
        if(i<firstExec->inputStringLen){
            firstExec->inputString[i] = c;
        }else{
            reallocInputString(firstExec);
            firstExec->inputString[i] = c;

        }
        end = scanf("%c", &c);
    }
    if(end == EOF){
        return 0;
    }
    executions = firstExec;
    return 1;
}

void reallocInputString(execution *executionToModify) {
    char*oldInputString = executionToModify ->inputString;
    int newInputStringLen = executionToModify->inputStringLen*2;
    executionToModify->inputString = (char*)malloc((newInputStringLen)*sizeof(char));
    for(int copiedCharIndex = 0;copiedCharIndex<executionToModify->inputStringLen;copiedCharIndex++){
                executionToModify->inputString[copiedCharIndex] = oldInputString[copiedCharIndex];
            }
    for(int blankIndex = executionToModify->inputStringLen;blankIndex<newInputStringLen;blankIndex++){
                executionToModify->inputString[blankIndex] = '_';
            }
    free(oldInputString);
    executionToModify->inputStringLen = newInputStringLen;
}

void reallocLeftBlanks(execution *executionToModify) {
    char*oldLeftBlanks = executionToModify ->leftBlanks;
    int newLeftBlanksLen = executionToModify->leftBlanksLen*2;
    executionToModify->leftBlanks = (char*)malloc((newLeftBlanksLen)*sizeof(char));
    for(int copiedCharIndex = 0;copiedCharIndex<executionToModify->leftBlanksLen;copiedCharIndex++){
        executionToModify->leftBlanks[copiedCharIndex] = oldLeftBlanks[copiedCharIndex];
    }
    for(int blankIndex = executionToModify->leftBlanksLen;blankIndex<newLeftBlanksLen;blankIndex++){
        executionToModify->leftBlanks[blankIndex] = '_';
    }
    free(oldLeftBlanks);
    executionToModify->leftBlanksLen = newLeftBlanksLen;
}

execution *newExecution(int state,int cursor,int iteration,int inputStringLen, int leftBlanksLen) {
    execution*newExecution = (execution*)malloc(sizeof(execution));
    newExecution->state = state;
    newExecution->cursor = cursor;
    newExecution->iteration = iteration;
    newExecution->nextExecution = NULL;
    newExecution->inputStringLen = inputStringLen;
    newExecution->leftBlanksLen = leftBlanksLen;
    newExecution->inputString = (char*)malloc(newExecution->inputStringLen*sizeof(char));
    newExecution->leftBlanks = (char*)malloc(newExecution->leftBlanksLen*sizeof(char));
    for(int i = 0; i<newExecution->inputStringLen;i++){
        newExecution->inputString[i] = '_';
    }
    for(int i = 0; i<newExecution->leftBlanksLen;i++){
        newExecution->leftBlanks[i] = '_';
    }
    return newExecution;
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
                        char transitionChar = (char)transitionCharIndex + 65;
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

    states = (state **)malloc(STARTINGLEN*sizeof(state));  //todo non funziona se gli stati hanno numeri grandi
    for (int stateToInit = 0; stateToInit < STARTINGLEN; stateToInit++) {
        states[stateToInit] = NULL;
    }

    //scan tr
    scanf("%s", command);
    //printf("%s\n", command);
    while(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5){
        //printf("%d %c %c %c %d\n",currentState,red,written,headShift,nextState);
        int pos = red-65 ;
        if(states[currentState]== NULL){
            state *newState = createNewState(currentState);
            transition *newTransition = createNewTransition(written, headShift, nextState);
            newState->transitions[pos] = newTransition;
        }else{
            transition* newTransition = createNewTransition(written, headShift, nextState);
            newTransition->nextTransition = states[currentState]->transitions[pos];
            states[currentState]->transitions[pos] = newTransition;
            }
    }
}

state *createNewState(int stateToCreate) {
    state* newState = (state *)malloc(sizeof(state));
    newState->acceptState = FALSE;
    newState->transitions = (transition**)malloc(CHARACTERS*sizeof(transition*));
    for(int i = 0;i<CHARACTERS;i++){
        newState->transitions[i] = NULL;
    }
    states[stateToCreate] = newState;
    return newState;
}

transition *createNewTransition(char written, char headShift, int nextState) {
    transition* newTransition = (transition *)malloc(sizeof(transition));
    if(states[nextState] == NULL){
        createNewState(nextState);
    }
    newTransition->nextState = nextState;
    newTransition->written = written;
    newTransition->headShift = headShift;
    newTransition->nextTransition = NULL;
    return newTransition;
}

void setAcceptedStates() {//scan acc
    char command[3];
    scanf("%s",command);
    //printf("%s\n", command);
    int acceptedState;
    while(scanf("%d", &acceptedState) == 1){
        //printf("%d\n",acceptedState);
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
    //printf("%s\n", command);
    scanf("%d",&maxIterations);
    //printf("%d\n",maxIterations);
}