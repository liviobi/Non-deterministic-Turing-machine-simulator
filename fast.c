#include <stdio.h>
#include <stdlib.h>
#define FALSE 0
#define TRUE 1
#define STARTINGLENGTH 10
#define REALLOC_INDEX 1.5
#define HASHLEN 1
#define REHASH 0.7
#define REHASH_INDEX 2

typedef struct transition_s{
    char written;
    char headShift;
    int infinite;
    struct state_s*nextState;
    struct transition_s*nextTransition;
}transition;

typedef struct state_s {
    int acceptState;
    int tableDim;
    int tableElements;
    struct container_s**hashTable;
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

typedef  struct container_s{
    char transitionChar;
    transition*transitionForChar;
}container;

typedef struct executionContainer_s{
    execution*executionToDevelop;
    transition*transitionsToApply;
    struct executionContainer_s*nextContainer;
}executionContainer;

int maxIterations; //todo see for long or double
node*root = NULL;
execution*executions;
executionContainer*executionsToRun = NULL;
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

void printStatesTree(node *nodeToPrint);

void freeStatesTree(node *nodeToFree);

execution *createNewExecution(state *state, int cursor, int iteration, int inputStringLen) ;

void printString(execution *executiontoPrint);

void addTransitionToTable(state *stateToUpdate, char redChar, transition *transitionToAdd);

int h(char key, int tableDim, int iteration);

transition *getTransition(state *stateToSearch, char redChar);

void rehash(state *stateToUpdate);

void fastInsertInTable(struct container_s **hashTable, int tableDim, container *containerToInsert);

void freeNodeTree(node *nodeToFree);

void addNewExecutionContainer(execution *executionToDevelop, struct transition_s *transitionsToApply);

void extractExecution();

void freeAllExecutionContainers();

void freeFirstExecutionContainer();

transition*createNewTransition(char written, char headShift, state*nextState, int infinite);

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
        }
        printResult();
        result = -1;
    }

    //printStatesTree(root);
    freeStatesTree(root);

    return 0;

}

void freeNodeTree(node *nodeToFree) {
    if(nodeToFree->left != NULL){
        freeNodeTree(nodeToFree->left);
    }
    if(nodeToFree->right != NULL){
        freeNodeTree(nodeToFree->right);
    }
    free(nodeToFree);
}

void freeStatesTree(node *nodeToFree) {
    if(nodeToFree->left != NULL){
        freeStatesTree(nodeToFree->left);
    }
    if(nodeToFree->right != NULL){
        freeStatesTree(nodeToFree->right);
    }

    for(int i = 0; i < nodeToFree->stateInNode->tableDim; i++) {
        if (nodeToFree->stateInNode->hashTable[i] != NULL) {
            transition *transitionToFree = nodeToFree->stateInNode->hashTable[i]->transitionForChar;
            transition *temp;
            while (transitionToFree != NULL) {
                temp = transitionToFree->nextTransition;
                free(transitionToFree);
                transitionToFree = temp;

            }
            free(nodeToFree->stateInNode->hashTable[i]);
        }
    }
    free(nodeToFree->stateInNode->hashTable);
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
    transition* transitionForRedChar = getTransition(executions->currentState,red);
    int transitionResult;

    if(transitionForRedChar == NULL){
        freeFirstExecution();
        extractExecution();
        return;
    }
    //if I have more than one transition
    if(transitionForRedChar->nextTransition != NULL){
        //todo posso farlo più veloce se controllo se ho solo due transizioni faccio due esecuzione senza fare il container
        execution*clonedExecution = cloneExecution(executions);
        addNewExecutionContainer(clonedExecution,transitionForRedChar->nextTransition);
    }

    transitionResult = makeTransition(executions,transitionForRedChar);
    if(transitionResult == 0){
        result = 0;
        freeFirstExecution();
        extractExecution();
    }else if(transitionResult == 1){
        result = 1;
        freeAllExecutions();
        freeAllExecutionContainers();
        return;
    }

}

void extractExecution() {
    transition*transitionToApply;
    if(executionsToRun == NULL){
        return;
    }
    //only one transition left
    while (executions == NULL){
        if(executionsToRun->transitionsToApply->nextTransition == NULL){
            executions = executionsToRun->executionToDevelop;
            transitionToApply = executionsToRun->transitionsToApply;
            //deleting container with no transitions in it
            executionContainer*toDelete = executionsToRun;
            executionsToRun = executionsToRun->nextContainer;
            free(toDelete);
        }else{
            executions = cloneExecution(executionsToRun->executionToDevelop);
            transitionToApply = executionsToRun->transitionsToApply;
            executionsToRun->transitionsToApply = executionsToRun->transitionsToApply->nextTransition;
        }

        int transitionResult = makeTransition(executions,transitionToApply); //todo extract method
        if(transitionResult == 0){
            result = 0;
            freeFirstExecution();
        }else if(transitionResult == 1){
            result = 1;
            freeAllExecutions();
            freeAllExecutionContainers();
            return;
        }
        //if there are no more containers
        if(executionsToRun == NULL){
            return;
        }
    }

}

void freeAllExecutionContainers() {
    while(executionsToRun != NULL){
        freeFirstExecutionContainer();
    }

}

void freeFirstExecutionContainer() {
    if(executionsToRun != NULL){
        executionContainer* toDelete = executionsToRun;
        executionsToRun = toDelete->nextContainer;
        freeExecution(toDelete->executionToDevelop);
        free(toDelete);
    }
}

void addNewExecutionContainer(execution *executionToDevelop, struct transition_s *transitionsToApply) {
    executionContainer*newContainer = (executionContainer*)malloc(sizeof(executionContainer));
    newContainer->executionToDevelop = executionToDevelop;
    newContainer->transitionsToApply = transitionsToApply;
    newContainer->nextContainer = executionsToRun;
    executionsToRun = newContainer;
}

transition *getTransition(state *stateToSearch, char redChar) {
    int i = 1;
    int value;
    if(stateToSearch->hashTable == NULL){
        return NULL;
    }
    while(1) {
        value = h(redChar,stateToSearch->tableDim,i);
        //if there's no container i create it and add the transition
        if (stateToSearch->hashTable[value] == NULL) {
            return NULL;
        } else {
            //if I already have a container for that character I first check if the container is right then I add the transition
            if (stateToSearch->hashTable[value]->transitionChar == redChar) {
                return stateToSearch->hashTable[value]->transitionForChar;
            }
            i++;
        }
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
    if(executionToUpdate->iteration == maxIterations){
        //exceded maximum iterations
        return 0;
    }
    executionToUpdate->currentState = transitionToApply->nextState;
    if(executionToUpdate->currentState->acceptState){
        //found final currentState
        return 1;
    }

    if(transitionToApply->infinite){
        //will proceed till the end of transitions
        return 0;
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
    //printf(" old = %d ",executionToModify->inputStringLen);
    int newInputStringLen = executionToModify->inputStringLen*REALLOC_INDEX;
    //printf("new = %d\n",newInputStringLen);
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

    int blanksEnd = newInputStringLen - oldInputStringLen;
    for(int blankIndex = 0; blankIndex < blanksEnd; blankIndex++){
        executionToModify->inputString[blankIndex] = '_';
    }

    for(int copiedCharIndex = blanksEnd; copiedCharIndex < newInputStringLen; copiedCharIndex++){
        executionToModify->inputString[copiedCharIndex] = oldInputString[copiedCharIndex - blanksEnd];
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

    //scan tr
    scanf("%s", command);
    //setup the root
    if(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5){
        setupRoot(currentState);
        state*followingState = searchAddState(root,nextState);
        transition *newTransition = createNewTransition(written, headShift, followingState,0); //todo check if transition isn't already there
        addTransitionToTable(root->stateInNode,red,newTransition);
    }else{
        return;
    }

    while(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5) {
        if(red == written && currentState == nextState && headShift == 'S'){
            state*followingState = searchAddState(root,nextState);
            state *stateToAddTransition = searchAddState(root,currentState);
            transition *newTransition = createNewTransition(written, headShift, followingState,1); //todo check if transition isn't already there
            addTransitionToTable(stateToAddTransition,red,newTransition);
        }else{
            state*followingState = searchAddState(root,nextState);
            state *stateToAddTransition = searchAddState(root,currentState);
            transition *newTransition = createNewTransition(written, headShift, followingState,0); //todo check if transition isn't already there
            addTransitionToTable(stateToAddTransition,red,newTransition);
        }
    }
}

void addTransitionToTable(state *stateToUpdate, char redChar, transition *transitionToAdd) {
    if(stateToUpdate->hashTable == NULL){
        stateToUpdate->tableDim = HASHLEN;
        stateToUpdate->tableElements = 0;
        stateToUpdate->hashTable = (container**)malloc(sizeof(container*)*stateToUpdate->tableDim);
        for(int i = 0;i<stateToUpdate->tableDim;i++){
            stateToUpdate->hashTable[i] = NULL;
        }
    }
    int i = 1;
    int value;
    while(1) {
        value = h(redChar,stateToUpdate->tableDim,i);
        //if there's no container i create it and add the transition
        if (stateToUpdate->hashTable[value] == NULL) {
            stateToUpdate->hashTable[value] = (container *) malloc(sizeof(container));
            stateToUpdate->tableElements++;
            stateToUpdate->hashTable[value]->transitionChar = redChar;
            transitionToAdd->nextTransition = NULL;
            stateToUpdate->hashTable[value]->transitionForChar = transitionToAdd;
            //if the table is too crowded
            if(stateToUpdate->tableElements/stateToUpdate->tableDim>REHASH){
                rehash(stateToUpdate);
            }
            break;
        } else {
            //if I already have a container for that character I first check if the container is right then I add the transition
            if (stateToUpdate->hashTable[value]->transitionChar == redChar) {
                transitionToAdd->nextTransition = stateToUpdate->hashTable[value]->transitionForChar;
                stateToUpdate->hashTable[value]->transitionForChar = transitionToAdd;
                break;
            }
            i++;
        }
    }
}

void rehash(state *stateToUpdate) {
    int oldTableDim = stateToUpdate->tableDim;
    container**oldTable = stateToUpdate->hashTable;
    int newTableDim = stateToUpdate->tableDim*REHASH_INDEX;
    stateToUpdate->hashTable = (container**)malloc(sizeof(container*)*newTableDim);
    stateToUpdate->tableDim = newTableDim;
    //initialize the newly created table
    for(int i = 0; i<newTableDim;i++){
        stateToUpdate->hashTable[i] = NULL;
    }
    for(int i = 0; i<oldTableDim;i++){
        if(oldTable[i] != NULL){
            fastInsertInTable(stateToUpdate->hashTable, newTableDim, oldTable[i]);
        }
    }
    free(oldTable);
}

//funcion use only when rehashing
void fastInsertInTable(struct container_s **hashTable, int tableDim, container *containerToInsert) {
    int i = 1;
    int value;
    while(1) {
        value = h(containerToInsert->transitionChar,tableDim,i);
        if (hashTable[value] == NULL) {
            hashTable[value] = containerToInsert;
            break;
        }
        i++;
    }
}

int h(char key, int tableDim, int iteration) {
    return ((int)key+iteration)%tableDim;
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
    newState->tableDim = 0;
    newState->tableElements = 0;
    newState->hashTable = NULL;
    return newState;
}

transition *createNewTransition(char written, char headShift, state*nextState, int infinite) {
    transition* newTransition = (transition *)malloc(sizeof(transition));
    newTransition->nextState = nextState;
    newTransition->written = written;
    newTransition->headShift = headShift;
    newTransition->nextTransition = NULL;
    newTransition->infinite = infinite;
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