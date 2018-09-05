#include <stdio.h>
#include <stdlib.h>
#define FALSE 0
#define TRUE 1
#define STARTINGLENGTH 5
#define CHAR_REHASH 2
#define HASHLEN 1
#define REHASH 0.7
#define REHASH_INDEX 2

typedef struct transition_s{
    char written;
    char headShift;
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

typedef struct differentChar_s{
    int index;
    char charater;
}differentChar;

typedef  struct execution_s{
    state*currentState;
    int iteration;
    int cursor;
    int hashTableLen;
    int tableElements;
    differentChar**hashTable;
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
char*inputString = NULL;
int inputStringLen = 0;

void setMaxTransitions();

void setAcceptedStates();

void setupStates();

void freeTransition(transition*transitionToFree);

int createFirstExecution();

execution *cloneExecution(execution *executionToClone);

int makeTransition(execution *executionToUpdate, transition *transitionToApply, char redChar);

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

execution *createNewExecution(state *state, int cursor, int iteration, int hashLen);

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

int reAllocInputString(int allocMemory);

void cleanInputString();

int characterHash(int key, int tableDim, int iteration) ;

differentChar *createNewDiffrentChar(int position, char charToPut);

void rehashDiffrentChar(execution *executionToRehash);

void fastInsertCharacterInTable(execution *executionToRehash, differentChar *diffrenCharToAdd);

void printExecution();

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
    free(inputString);

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

    transitionResult = makeTransition(executions, transitionForRedChar, red);
   //printExecution();
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

void printExecution() {
    for(int i = -10 ; i < inputStringLen+10; i++){
        int iteration = 0;
        int value;
        while (1){
            value = characterHash(i, executions->hashTableLen,iteration);
            if(executions->hashTable[value] == NULL){
                //if I didn't find the character
                if(i < 0 || i >= inputStringLen){
                    printf("_");
                    break;
                }else{
                    printf("%c",inputString[i]);
                    break;
                    }

            }else if(executions->hashTable[value]->index == i){
                printf("%c", executions->hashTable[value]->charater);
                break;
            }else{
                iteration++;
                continue;
            }
        }

    }
    printf("\n");
    /*for(int i = 0; i< executions->hashTableLen; i++){
        if(executions->hashTable[i] != NULL){
            printf("index %d character %c \n", executions->hashTable[i]->index,executions->hashTable[i]->charater);
        }
    }*/


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

        int transitionResult = makeTransition(executions, transitionToApply, 0); //todo extract method
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

void freeAllExecutions() {
    while(executions != NULL){
        freeFirstExecution();
    }

}

void freeFirstExecution() {
    if(executions != NULL){
        execution* toDelete = executions;
        freeExecution(toDelete);
        executions = NULL;
    }
}

void freeExecution(execution *executionToFree) {
    for(int i = 0; i < executionToFree->hashTableLen; i++){
        if(executionToFree->hashTable[i] !=NULL){
            free(executionToFree->hashTable[i]);
        }
    }
    free(executionToFree->hashTable);
    free(executionToFree);
}

char readFromTape(execution *runningExecution) {
    int iteration = 0;
    int value;
    while (1){
        value = characterHash(runningExecution->cursor, runningExecution->hashTableLen,iteration);
        if(runningExecution->hashTable[value] == NULL){
            //if I didn't find the character
            if(runningExecution->cursor < 0 || runningExecution->cursor >= inputStringLen){
                return '_';
            }else{
                return inputString[runningExecution->cursor];
            }

        }else if(runningExecution->hashTable[value]->index == runningExecution->cursor){
            return runningExecution->hashTable[value]->charater; //todo return the struct to avoid searching it again
        }else{
            iteration++;
            continue;
        }
    }
}

void putDiffrentChar(char charToPut, execution *runningExecution ){
    //printf(" putting diffrent char %c ", charToPut);

    int iteration = 0;
    int value;

    while (1){
        value = characterHash(runningExecution->cursor, runningExecution->hashTableLen,iteration);
        if(runningExecution->hashTable[value] == NULL){
            //printf("by creating it");
            runningExecution->hashTable[value] = createNewDiffrentChar(runningExecution->cursor, charToPut);
            runningExecution->tableElements++;
            if(runningExecution->tableElements/runningExecution->hashTableLen>REHASH){
                    rehashDiffrentChar(runningExecution);
                }
                //printf("\n");
            return;
        }else if(runningExecution->hashTable[value]->index == runningExecution->cursor){
                //printf("by updating it");
                runningExecution->hashTable[value]->charater = charToPut;
           // printf("\n");
            return;
        }else{
            iteration++;
            continue;
        }
    }


}


differentChar *createNewDiffrentChar(int position, char charToPut) {
    differentChar*newDiffrentChar = (differentChar*)malloc(sizeof(differentChar));
    newDiffrentChar->charater = charToPut;
    newDiffrentChar->index = position;
    return newDiffrentChar;
}

int makeTransition(execution *executionToUpdate, transition *transitionToApply, char redChar) {
   // printf("making transition red %c cursor %d\n", redChar,executionToUpdate->cursor);
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

    if(redChar != transitionToApply->written){
        putDiffrentChar(transitionToApply->written,executionToUpdate);
    }
    if(transitionToApply->headShift =='L'){
        executionToUpdate->cursor--;
    }else if(transitionToApply->headShift == 'R'){
        executionToUpdate->cursor++;
    }
    return -1;
}

execution *cloneExecution(execution *executionToClone) {
    execution*clonedExecution = createNewExecution(executionToClone->currentState, executionToClone->cursor,
                                                   executionToClone->iteration, executionToClone->hashTableLen);
    for(int i = 0; i<executionToClone->hashTableLen;i++){
        if(executionToClone->hashTable[i] != NULL){
            clonedExecution->hashTable[i] = createNewDiffrentChar(executionToClone->hashTable[i]->index,executionToClone->hashTable[i]->charater);
        }
    }
    clonedExecution->tableElements = executionToClone->tableElements;
    return clonedExecution;
}

int createFirstExecution() {
    char c = '\n';
    int end;
    end = scanf(" %c", &c);
    if(end == EOF){
        return 0;
    }
    int allocatedMemory;
    allocatedMemory = STARTINGLENGTH;
    if(inputStringLen > 0){
        free(inputString);
    }
    inputString = (char*)malloc(sizeof(char)*allocatedMemory);

    for(inputStringLen = 0;c != '\n' && end != EOF;inputStringLen++){
        if(inputStringLen<allocatedMemory){
            inputString[inputStringLen] = c;
        }else{
            allocatedMemory = reAllocInputString(allocatedMemory);
            inputString[inputStringLen] = c;
        }
        end = scanf("%c", &c);
    }
    cleanInputString();
    state*stateZero = searchAddState(root,0);
    execution*firstExec = createNewExecution(stateZero, 0, 0,STARTINGLENGTH);

    executions = firstExec;
    return 1;
}

void cleanInputString() {
    char*oldInputString = inputString;
    inputString = (char*)malloc(sizeof(char)*inputStringLen);
    for(int i = 0; i < inputStringLen; i++){
        inputString[i] = oldInputString[i];
    }
    free(oldInputString);

}

int reAllocInputString(int allocMemory) {
    allocMemory = allocMemory*2;
    char*oldInputString = inputString;
    inputString = (char*)malloc(sizeof(char)*allocMemory);
    for(int i = 0; i< inputStringLen; i++){
        inputString[i] = oldInputString[i];
    }
    free(oldInputString);
    return  allocMemory;
}


execution *createNewExecution(state *state, int cursor, int iteration, int hashLen) {
    execution*newExecution = (execution*)malloc(sizeof(execution));
    newExecution->currentState = state;
    newExecution->cursor = cursor;
    newExecution->iteration = iteration;
    newExecution->tableElements = 0;
    newExecution->hashTableLen = hashLen;
    newExecution->hashTable = (differentChar**)malloc(sizeof(differentChar*)*hashLen);
    for(int i = 0; i< hashLen; i++){ //todo add only in create first exec
        newExecution->hashTable[i] = NULL;
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

    //scan tr
    scanf("%s", command);
    //setup the root
    if(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5){
        setupRoot(currentState);
        state*followingState = searchAddState(root,nextState);
        transition *newTransition = createNewTransition(written, headShift, followingState); //todo check if transition isn't already there
        addTransitionToTable(root->stateInNode,red,newTransition);
    }else{
        return;
    }

    while(scanf("%d %c %c %c %d",&currentState,&red,&written,&headShift,&nextState) == 5) {
        state*followingState = searchAddState(root,nextState);
        state *stateToAddTransition = searchAddState(root,currentState);
        transition *newTransition = createNewTransition(written, headShift, followingState); //todo check if transition isn't already there
        addTransitionToTable(stateToAddTransition,red,newTransition);
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

int h(char key, int tableDim, int iteration) {
    return ((int)key+iteration)%tableDim;
}
int characterHash(int key, int tableDim, int iteration) {
    if(key < 0){
        return ((key*-1)+iteration)%tableDim;
    } else{
        return (key+iteration)%tableDim;
    }
}

void rehashDiffrentChar(execution *executionToRehash) {
    int oldTableDim = executionToRehash->hashTableLen;
    differentChar **oldTable = executionToRehash->hashTable;
    int newTableDim = executionToRehash->hashTableLen*CHAR_REHASH;
    executionToRehash->hashTable = (differentChar **)malloc(sizeof(differentChar *)*newTableDim);
    executionToRehash->hashTableLen = newTableDim;
    //initialize the newly created table
    for(int i = 0; i<newTableDim;i++){
        executionToRehash->hashTable[i] = NULL;
    }

    for(int i = 0; i<oldTableDim;i++){
        if(oldTable[i] != NULL){
            fastInsertCharacterInTable(executionToRehash, oldTable[i]);
        }
    }
    free(oldTable);
}

void fastInsertCharacterInTable(execution *executionToRehash, differentChar *diffrenCharToAdd) {
    int value;
    int iteration = 0;
    while(1) {
        value = characterHash(diffrenCharToAdd->index, executionToRehash->hashTableLen, iteration);
        if(executionToRehash->hashTable[value] == NULL){
            executionToRehash->hashTable[value] = diffrenCharToAdd;
            break;
        }else{
            iteration++;
            continue;
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