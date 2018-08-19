#include <stdio.h>
#include <stdlib.h>
#define FALSE 0
#define TRUE 1
#define STRINGLEN 10
#define BLANKLEN 2
#define CHARACTERS 58

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
    int leftBlanksLen;
    char*inputString;
    char*leftBlanks;
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

void reallocInputString(execution *executionToModify);

void reallocLeftBlanks(execution *executionToModify);

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

execution *createNewExecution(state *state, int cursor, int iteration, int inputStringLen, int leftBlanksLen) ;

void freeStatesTree(node *nodeToFree);

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
    while(transitionForRedChar != NULL){
        execution*clonedExecution = cloneExecution(executions);
        executions->nextExecution = clonedExecution;
        //printf("char red %c starting from currentState %d iteration %d cursor %d\n",red,clonedExecution->currentState,clonedExecution->iteration,clonedExecution->cursor);
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
    executionToUpdate->currentState = transitionToApply->nextState;
    if(executionToUpdate->currentState->acceptState){
        //found final currentState
       // printf("\n\n\nfound final staten\n");
        return 1;
    }

    if(executionToUpdate->cursor>=0){
        executionToUpdate->inputString[executionToUpdate->cursor] = transitionToApply->written;
        if(transitionToApply->headShift =='L'){
            executionToUpdate->cursor--;
            //printf("currentState %d iteration %d cursor %d written %c moving L\n",executionToUpdate->currentState,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }else if(transitionToApply->headShift == 'R'){
            executionToUpdate->cursor++;
           // printf("currentState %d iteration %d cursor %d written %c moving R\n",executionToUpdate->currentState,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
            if(executionToUpdate->cursor==executionToUpdate->inputStringLen){
                reallocInputString(executionToUpdate);
            }
        }else{
            //printf("currentState %d iteration %d cursor %d written %c not moving\n",executionToUpdate->currentState,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }
    }else{
        executionToUpdate->leftBlanks[executionToUpdate->cursor*(-1)] = transitionToApply->written;
        if(transitionToApply->headShift =='L'){
            executionToUpdate->cursor--;
           // printf("currentState %d iteration %d cursor %d written %c moving blank L\n",executionToUpdate->currentState,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
            if(executionToUpdate->cursor*(-1) == executionToUpdate->leftBlanksLen){
                reallocLeftBlanks(executionToUpdate);
            }
        }else if(transitionToApply->headShift == 'R'){
            executionToUpdate->cursor++;
            //printf("currentState %d iteration %d cursor %d written %c moving blank R\n",executionToUpdate->currentState,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }else{
            //printf("currentState %d iteration %d cursor %d written %c leaving blank\n",executionToUpdate->currentState,executionToUpdate->iteration,executionToUpdate->cursor,transitionToApply->written);
        }
    }
    return -1;
}

execution *cloneExecution(execution *executionToClone) {
    execution*clonedExecution = createNewExecution(executionToClone->currentState, executionToClone->cursor,
                                                   executionToClone->iteration, executionToClone->inputStringLen,
                                                   executionToClone->leftBlanksLen);
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
    state*stateZero = searchAddState(root,0);
    execution*firstExec = createNewExecution(stateZero, 0, 0, STRINGLEN, BLANKLEN);
    char c = '\n';
    int end;
    end = scanf(" %c", &c);
    if(end == EOF){
        freeExecution(firstExec);
        return 0;
    }
    for(int i = 0;c != '\n' && end != EOF;i++){
        if(i<firstExec->inputStringLen){
            firstExec->inputString[i] = c;
        }else{
            reallocInputString(firstExec);
            firstExec->inputString[i] = c;
        }
        end = scanf("%c", &c);
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

execution *createNewExecution(state *state, int cursor, int iteration, int inputStringLen, int leftBlanksLen) {
    execution*newExecution = (execution*)malloc(sizeof(execution));
    newExecution->currentState = state;
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