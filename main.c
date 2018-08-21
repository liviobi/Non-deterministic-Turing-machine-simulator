#include <stdio.h>
#include <stdlib.h>
#define FALSE 0
#define TRUE 1
#define STARTINGLENGTH 10
#define CHARACTERS 58
#define REALLOC_INDEX 2

typedef struct transition_s{
    char written;
    char headShift;
    struct state_s*nextState;
    struct transition_s*nextTransition;
}transition;

typedef struct state_s {
    int acceptState;
    transition **transitions;
} state;

typedef struct node_s {
    int stateIndex ;
    state*stateInNode;
    struct node_s*left;
    struct node_s*right;
} node;

typedef  struct execution_s{
    state*currentState;
    int iteration;
    int cursor;
    int inputStringLen;
    char*inputString;
    struct execution_s*nextExecution;
}execution;

int maxIterations; //todo see for long or double
node*root = NULL;
execution*executions;
int result = -1;

void setMaxTransitions();

void setAcceptedStates();

void setupStates();

void freeTransition(transition*transitionToFree);

int createFirstExecution();

execution *cloneExecution(execution *executionToClone);

void reallocInputStringRight(execution *executionToModify);

void reallocInputStringLeft(execution *executionToModify);

int makeTransition(execution*executionToUpdate,transition*transitionToApply);

char readFromTape(execution *runningExecution);

void freeExecution(execution *executionToFree);

void executeAllTransition(char red);

void freeAllExecutions();

void freeFirstExecution();

void printResult();

void setupRoot(int state);

state *searchAddState(node*fatherNode, int stateToFind) ;

node *createNewNode(int stateIndex);

state *createNewState() ;

transition *createNewTransition(char written, char headShift, state*nextState) ;

void printStatesTree(node *nodeToPrint);

void freeStatesTree(node *nodeToFree);

execution *createNewExecution(state *state, int cursor, int iteration, int inputStringLen) ;

void printString(execution *executiontoPrint);

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
            //freeFirstExecution();
        }
        printResult();
        result = -1;
    }

    //printStatesTree(root);
    freeStatesTree(root);

    return 0;

}

void freeStatesTree(node *nodeToFree) {
    if(nodeToFree->left != NULL){
        freeStatesTree(nodeToFree->left);
    }
    if(nodeToFree->right != NULL){
        freeStatesTree(nodeToFree->right);
    }

    for(int i = 0; i < CHARACTERS; i++){
        transition*transitionToFree = nodeToFree->stateInNode->transitions[i];
        transition*temp;
        while (transitionToFree != NULL){
            temp = transitionToFree->nextTransition;
            free(transitionToFree);
            transitionToFree = temp;

        }
    }
    free(nodeToFree->stateInNode->transitions);
    free(nodeToFree->stateInNode);
    free(nodeToFree);
}

void printStatesTree(node *nodeToPrint) {
    if(nodeToPrint->left != NULL){
        printStatesTree(nodeToPrint->left);
    }

    printf("%d\n",nodeToPrint->stateIndex);

    if(nodeToPrint->right != NULL){
        printStatesTree(nodeToPrint->right);
    }

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
    transition* transitionForRedChar = executions->currentState->transitions[(int)red - 65];
    int transitionResult;

    if(transitionForRedChar == NULL){
        freeFirstExecution();
        return;
    }
    while(transitionForRedChar != NULL){
        //whenever I have more than one transition I clone the fsm and execute it
        if(transitionForRedChar->nextTransition != NULL){
            execution*clonedExecution = cloneExecution(executions);
            executions->nextExecution = clonedExecution;
            transitionResult = makeTransition(clonedExecution,transitionForRedChar);
            if(transitionResult == 0){
                result = 0;
                executions->nextExecution = clonedExecution->nextExecution;
                freeExecution(clonedExecution);
            }else if(transitionResult == 1){
                result = 1;
                freeAllExecutions();
                return;
            }
        }else{
            //if I reach the last transition I don't clone it anymore I just update it
            transitionResult = makeTransition(executions,transitionForRedChar);
            if(transitionResult == 0){
                result = 0;
                execution*execToDelete = executions;
                executions = executions->nextExecution;
                freeExecution(execToDelete);
            }else if(transitionResult == 1){
                result = 1;
                freeAllExecutions();
                return;
            }
        }

        transitionForRedChar = transitionForRedChar->nextTransition;
    }
}

void printString(execution *executiontoPrint) {
    for(int i=0; i<executiontoPrint->inputStringLen;i++){
        printf("%c",executiontoPrint->inputString[i]);
    }
    printf("\n");
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
    free(executionToFree);
}

char readFromTape(execution *runningExecution) { //todo see if i must delete it
    return runningExecution->inputString[runningExecution->cursor];
}

int makeTransition(execution*executionToUpdate,transition*transitionToApply) {
    executionToUpdate->iteration++;
    if(executionToUpdate->iteration>maxIterations){
        //exceded maximum iterations
        return 0;
    }
    executionToUpdate->currentState = transitionToApply->nextState;
    if(executionToUpdate->currentState->acceptState){
        //found final currentState
        return 1;
    }

    executionToUpdate->inputString[executionToUpdate->cursor] = transitionToApply->written;
    if(transitionToApply->headShift =='L'){
        executionToUpdate->cursor--;
        if(executionToUpdate->cursor<0){
            reallocInputStringLeft(executionToUpdate);
        }
    }else if(transitionToApply->headShift == 'R'){
        executionToUpdate->cursor++;
        if(executionToUpdate->cursor==executionToUpdate->inputStringLen){
            reallocInputStringRight(executionToUpdate);
        }
    }
    return -1;
}

execution *cloneExecution(execution *executionToClone) {
    execution*clonedExecution = createNewExecution(executionToClone->currentState, executionToClone->cursor,executionToClone->iteration, executionToClone->inputStringLen);
    clonedExecution->nextExecution = executionToClone->nextExecution;
    for(int copiedCharIndex = 0; copiedCharIndex<executionToClone->inputStringLen;copiedCharIndex++){
        clonedExecution->inputString[copiedCharIndex] = executionToClone->inputString[copiedCharIndex];
    }
    return clonedExecution;
}

int createFirstExecution() {
    state*stateZero = searchAddState(root,0);
    execution*firstExec = createNewExecution(stateZero, 0, 0, STARTINGLENGTH);
    char c = '\n';
    int end;
    end = scanf(" %c", &c);
    if(end == EOF){
        freeExecution(firstExec);
        return 0;
    }
    int i;
    for(i = 0;c != '\n' && end != EOF;i++){
        if(i<firstExec->inputStringLen){
            firstExec->inputString[i] = c;
        }else{
            reallocInputStringRight(firstExec);
            firstExec->inputString[i] = c;
        }
        end = scanf("%c", &c);
    }
    //end with blanks
    for(; i<firstExec->inputStringLen;i++){ //todo see if it's necessary
        firstExec->inputString[i] = '_';
    }

    executions = firstExec;
    return 1;
}

void reallocInputStringRight(execution *executionToModify) {
    char*oldInputString = executionToModify ->inputString;
    int newInputStringLen = executionToModify->inputStringLen*REALLOC_INDEX;
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

void reallocInputStringLeft(execution *executionToModify) {
    char*oldInputString = executionToModify ->inputString;
    int newInputStringLen = executionToModify->inputStringLen*REALLOC_INDEX;
    int oldInputStringLen = executionToModify->inputStringLen;
    executionToModify->inputString = (char*)malloc((newInputStringLen)*sizeof(char));

    for(int blankIndex = 0; blankIndex < oldInputStringLen; blankIndex++){
        executionToModify->inputString[blankIndex] = '_';
    }

    for(int copiedCharIndex = oldInputStringLen; copiedCharIndex < newInputStringLen; copiedCharIndex++){
        executionToModify->inputString[copiedCharIndex] = oldInputString[copiedCharIndex - oldInputStringLen];
    }
    free(oldInputString);
    executionToModify->cursor = newInputStringLen - oldInputStringLen -1;
    executionToModify->inputStringLen = newInputStringLen;
}

execution *createNewExecution(state *state, int cursor, int iteration, int inputStringLen) {
    execution*newExecution = (execution*)malloc(sizeof(execution));
    newExecution->currentState = state;
    newExecution->cursor = cursor;
    newExecution->iteration = iteration;
    newExecution->nextExecution = NULL;
    newExecution->inputStringLen = inputStringLen;
    newExecution->inputString = (char*)malloc(newExecution->inputStringLen*sizeof(char));
    return newExecution;
}

void freeTransition(transition*transitionToFree){
    if(transitionToFree == NULL) {
        return;
    }else{
        freeTransition(transitionToFree->nextTransition);
        free(transitionToFree);
    }

}

void setupStates() {
    char command[3];
    int currentState;
    char red;
    char written;
    char headShift;
    int nextState;
    int pos;

    //scan tr
    scanf("%s", command);
    //setup the root
    if(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5){
        setupRoot(currentState);
        state*followingState = searchAddState(root,nextState);
        pos = red - 65;
        transition *newTransition = createNewTransition(written, headShift, followingState); //todo check if transition isn't already there
        newTransition->nextTransition = root->stateInNode->transitions[pos];
        root->stateInNode->transitions[pos] = newTransition;
    }else{
        return;
    }

    while(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5) {
        state*followingState = searchAddState(root,nextState);
        state *stateToAddTransition = searchAddState(root,currentState);
        pos = red - 65;
        transition *newTransition = createNewTransition(written, headShift, followingState); //todo check if transition isn't already there
        newTransition->nextTransition = stateToAddTransition->transitions[pos];
        stateToAddTransition->transitions[pos] = newTransition;
    }
    }

state *searchAddState(node*fatherNode, int stateToFind) {
    if(fatherNode->stateIndex == stateToFind){
        return fatherNode->stateInNode;
    }else if(fatherNode->stateIndex > stateToFind){
        if(fatherNode->left != NULL){
            searchAddState(fatherNode->left,stateToFind);
        }else{
            fatherNode->left = createNewNode(stateToFind);
            return fatherNode->left->stateInNode;
        }
    }else if(fatherNode->stateIndex < stateToFind){
        if(fatherNode->right != NULL) {
            searchAddState(fatherNode->right, stateToFind);
        }else{
            fatherNode->right = createNewNode(stateToFind);
            return fatherNode->right->stateInNode;
        }
    }
}

void setupRoot(int state) { root = createNewNode(state); }

node *createNewNode(int stateIndex) {
    node*newNode = (node*)malloc(sizeof(node));
    newNode->stateIndex = stateIndex;
    newNode->stateInNode = createNewState();
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

state *createNewState() {
    state* newState = (state *)malloc(sizeof(state));
    newState->acceptState = FALSE;
    newState->transitions = (transition**)malloc(CHARACTERS*sizeof(transition*));
    for(int i = 0;i<CHARACTERS;i++){
        newState->transitions[i] = NULL;
    }
    return newState;
}

transition *createNewTransition(char written, char headShift, state*nextState) {
    transition* newTransition = (transition *)malloc(sizeof(transition));
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
    int acceptedStateIndex;
    while(scanf("%d", &acceptedStateIndex) == 1){
        state*acceptedState = searchAddState(root,acceptedStateIndex);
        acceptedState->acceptState = TRUE;
    }
}

void setMaxTransitions() {//scan max
    char command[3];
    scanf("%s",command);
    //printf("%s\n", command);
    scanf("%d",&maxIterations);
    //printf("%d\n",maxIterations);
}