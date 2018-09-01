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
}execution;

typedef  struct container_s{
    char transitionChar;
    transition*transitionForChar;
}container;

typedef struct executionContainer_s{
    struct modifiedCharacter_s*modifiedCharacters;
    state*currentState;
    int iteration;
    int cursor;
    int leftBlanks; //todo can delete them to make it faster
    int rightBlanks;
    transition*transitionsToApply;
    struct executionContainer_s*nextContainer;
}executionContainer;

typedef struct modifiedCharacter_s{
    int index;
    char character;
    struct modifiedCharacter_s*next;
}modifiedCharacter;

int maxIterations;
node*root = NULL;
execution*executions;
executionContainer*containersToRun = NULL;
int result = -1;
char*inputString = NULL;
int inputStringLen;

int leftBlanksLen = 0;
int rightBlanksLen = 0;
char*executionString = NULL;

void setMaxTransitions();

void setAcceptedStates();

void setupStates();

void freeTransition(transition*transitionToFree);

int createFirstExecution();

execution *cloneExecution(execution *executionToClone);

void reAllocInputStringRight(execution *executionToModify);

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

execution *createNewExecution(state *state, int cursor, int iteration) ;

void printString(execution *executiontoPrint);

void addTransitionToTable(state *stateToUpdate, char redChar, transition *transitionToAdd);

int h(char key, int tableDim, int iteration);

transition *getTransition(state *stateToSearch, char redChar);

void rehash(state *stateToUpdate);

void fastInsertInTable(struct container_s **hashTable, int tableDim, container *containerToInsert);

void freeNodeTree(node *nodeToFree);

void extractExecution();

void freeAllExecutionContainers();

void freeFirstExecutionContainer();

int reAllocInputString(int allocMemory);

void cleanInputString();

modifiedCharacter * newModifiedCharacter(int index, char charater);

execution *extractExecutionFromContainer(executionContainer *container);

void addNewExecutionContainer( struct transition_s *transitionsToApply, execution*executionToClone) ;

void freeModifiedCharacters(struct modifiedCharacter_s *modifiedCharacter);

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
    free(executionString);

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
        //todo posso farlo più veloce se controllo se ho solo due transizioni faccio due esecuzione senza fare il container?
        addNewExecutionContainer(transitionForRedChar->nextTransition,executions);
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
    if(containersToRun == NULL){
        return;
    }
    //only one transition left
    while (executions == NULL){
        if(containersToRun->transitionsToApply->nextTransition == NULL){
            executions = extractExecutionFromContainer(containersToRun);
            transitionToApply = containersToRun->transitionsToApply;
            //deleting container with no transitions in it
            freeFirstExecutionContainer();
        }else{
            executions = extractExecutionFromContainer(containersToRun);
            transitionToApply = containersToRun->transitionsToApply;
            containersToRun->transitionsToApply = containersToRun->transitionsToApply->nextTransition;
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
        if(containersToRun == NULL){
            return;
        }
    }

}

execution *extractExecutionFromContainer(executionContainer *container) {
    //todo can avoid to free
    if(executionString != NULL){
        free(executionString);
    }

    executionString = (char*)malloc(sizeof(char)*(inputStringLen+container->rightBlanks+container->leftBlanks));
    leftBlanksLen = container->leftBlanks;
    rightBlanksLen = container -> rightBlanks;
    int i;
    modifiedCharacter *characterDifferentFromInput = container->modifiedCharacters;
    for (i = 0; i < leftBlanksLen; i++) {
        if(characterDifferentFromInput != NULL && i == characterDifferentFromInput->index){
            executionString[i] = characterDifferentFromInput->character;
            characterDifferentFromInput = characterDifferentFromInput->next;
        }else{
            executionString[i] = '_';
        }
    }

    for(; i< leftBlanksLen + inputStringLen; i++){
        if(characterDifferentFromInput != NULL ) {
            if (i == characterDifferentFromInput->index) {
            executionString[i] = characterDifferentFromInput->character;
            characterDifferentFromInput = characterDifferentFromInput->next;
            }
        }else{
            executionString[i] = inputString[i + leftBlanksLen];
        }
    }

    for(; i< leftBlanksLen + inputStringLen + rightBlanksLen; i++){
        if(characterDifferentFromInput != NULL && i == characterDifferentFromInput->index){
            executionString[i] = characterDifferentFromInput->character;
            characterDifferentFromInput = characterDifferentFromInput->next;
        }else{
            executionString[i] = inputString[i + leftBlanksLen + rightBlanksLen];
        }
    }

    return createNewExecution(container->currentState,container->cursor,container->iteration);



}

void freeAllExecutionContainers() {
    while(containersToRun != NULL){
        freeFirstExecutionContainer();
    }

}

void freeFirstExecutionContainer() {
    if(containersToRun != NULL){
        freeModifiedCharacters(containersToRun->modifiedCharacters);
        executionContainer* toDelete = containersToRun;
        containersToRun = toDelete->nextContainer;
        free(toDelete);
    }
}

void freeModifiedCharacters(struct modifiedCharacter_s *toDelete) {
    while (toDelete != NULL){
        struct modifiedCharacter_s *temp = toDelete->next;
        free(toDelete);
        toDelete = temp;
    }

}

void addNewExecutionContainer( struct transition_s *transitionsToApply, execution*executionToClone) {
    executionContainer*newContainer = (executionContainer*)malloc(sizeof(executionContainer));
    newContainer->iteration = executionToClone->iteration;
    newContainer->cursor = executionToClone->cursor;
    newContainer->currentState = executionToClone->currentState;
    newContainer->leftBlanks = leftBlanksLen;
    newContainer->rightBlanks = rightBlanksLen;
    newContainer->transitionsToApply = transitionsToApply;
    newContainer->nextContainer = containersToRun;
    containersToRun = newContainer;
    newContainer->modifiedCharacters = NULL;

    modifiedCharacter*newModifiedChar = NULL;
    int i;
    for(i = 0; i < leftBlanksLen; i++){
        if(executionString[i] != '_'){
            if(newModifiedChar == NULL){
                newModifiedChar = newModifiedCharacter(i,executionString[i]);
                newContainer->modifiedCharacters = newModifiedChar;
            }else{
                newModifiedChar->next = newModifiedCharacter(i,executionString[i]);
                newModifiedChar = newModifiedChar->next;
            }
        }
    }

    int index;
    for(i = 0; i < inputStringLen; i++){
        index = i + leftBlanksLen;
        if(executionString[index] != inputString[i]){
            if(newModifiedChar == NULL){
                newModifiedChar = newModifiedCharacter(index,executionString[index]);
                newContainer->modifiedCharacters = newModifiedChar;
            }else{
                newModifiedChar->next = newModifiedCharacter(index,executionString[index]);
                newModifiedChar = newModifiedChar->next;
            }
        }
    }

    for(i = 0; i < rightBlanksLen; i++){
        index = i + leftBlanksLen + inputStringLen;
        if(executionString[index] != '_'){
            if(newModifiedChar == NULL){
                newModifiedChar = newModifiedCharacter(index,executionString[index]);
                newContainer->modifiedCharacters = newModifiedChar;
            }else{
                newModifiedChar->next = newModifiedCharacter(index,executionString[index]);
                newModifiedChar = newModifiedChar->next;
            }
        }
    }

    if(newModifiedChar != NULL){
        newModifiedChar->next = NULL;
    }
}

modifiedCharacter * newModifiedCharacter(int index, char character) {
    modifiedCharacter*newContainer = (modifiedCharacter*)malloc(sizeof(modifiedCharacter));
    newContainer->character = character;
    newContainer->index = index;
    newContainer->next = NULL;
    return newContainer;

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
    free(executionToFree);
}

char readFromTape(execution *runningExecution) { //todo see if i must delete it
    return executionString[runningExecution->cursor];
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

    executionString[executionToUpdate->cursor] = transitionToApply->written;
    if(transitionToApply->headShift =='L'){
        executionToUpdate->cursor--;
        if(executionToUpdate->cursor<0){
            reallocInputStringLeft(executionToUpdate);
        }
    }else if(transitionToApply->headShift == 'R'){
        executionToUpdate->cursor++;
        if(executionToUpdate->cursor==leftBlanksLen+inputStringLen+rightBlanksLen){ //todo can make as an index instead of length
            reAllocInputStringRight(executionToUpdate);
        }
    }
    return -1;
}

int createFirstExecution() {
    char c = '\n';
    int end;
    end = scanf(" %c", &c);
    if(end == EOF){
        return 0;
    }
    if(inputString !=NULL){
        free(inputString);
    }
    int allocatedMemory;
    allocatedMemory = STARTINGLENGTH;
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
    execution*firstExec = (execution*)malloc(sizeof(execution));
    firstExec->currentState = stateZero;
    firstExec->cursor = 0;
    firstExec->iteration = 0;
    if(executionString != NULL) {
        free(executionString);
    }
    executionString = (char *) malloc(sizeof(char) * inputStringLen);
    for(int i = 0; i < inputStringLen; i++){
       executionString[i] = inputString[i];
    }

    containersToRun = NULL;
    leftBlanksLen = 0;
    rightBlanksLen = 0;
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

void reAllocInputStringRight(execution *executionToModify) {
    char*oldInputString = executionString;
    int oldInputStringLen = leftBlanksLen+inputStringLen+rightBlanksLen;
    if(rightBlanksLen == 0){
        rightBlanksLen = STARTINGLENGTH;
    }else{
        rightBlanksLen = leftBlanksLen*REALLOC_INDEX;
    }
    int newInputStringLen = leftBlanksLen+inputStringLen+rightBlanksLen;
    executionString = (char*)malloc((newInputStringLen)*sizeof(char));
    for(int copiedCharIndex = 0;copiedCharIndex<oldInputStringLen;copiedCharIndex++){
        executionString[copiedCharIndex] = oldInputString[copiedCharIndex];
    }
    for(int blankIndex = oldInputStringLen;blankIndex<newInputStringLen;blankIndex++){
        executionString[blankIndex] = '_';
    }
    free(oldInputString);
}

void reallocInputStringLeft(execution *executionToModify) {
    char*oldInputString = executionString;
    int oldInputStringLen = leftBlanksLen+inputStringLen+rightBlanksLen;
    if(leftBlanksLen == 0){
        leftBlanksLen = STARTINGLENGTH;
    }else{
        leftBlanksLen = leftBlanksLen*REALLOC_INDEX;
    }
    int newInputStringLen = leftBlanksLen+inputStringLen+rightBlanksLen;
    executionString = (char*)malloc((newInputStringLen)*sizeof(char));

    int blanksEnd = newInputStringLen - oldInputStringLen;
    for(int blankIndex = 0; blankIndex < blanksEnd; blankIndex++){
        executionString[blankIndex] = '_';
    }

    for(int copiedCharIndex = blanksEnd; copiedCharIndex < newInputStringLen; copiedCharIndex++){
        executionString[copiedCharIndex] = oldInputString[copiedCharIndex - blanksEnd];
    }
    free(oldInputString);
    executionToModify->cursor = newInputStringLen - oldInputStringLen -1;
}

execution *createNewExecution(state *state, int cursor, int iteration) {
    execution*newExecution = (execution*)malloc(sizeof(execution));
    newExecution->currentState = state;
    newExecution->cursor = cursor;
    newExecution->iteration = iteration;
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