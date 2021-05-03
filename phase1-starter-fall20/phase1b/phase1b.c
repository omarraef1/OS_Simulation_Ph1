/*
Phase 1b
Omar, Isabel
*/

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef struct PCB {
    int             cid;                // context's ID
    int             cpuTime;            // process's running time
    char            name[P1_MAXNAME+1]; // process's name
    int             priority;           // process's priority
    P1_State        state;              // state of the PCB
    // more fields here

    int (*startFunc)(void*); //fix: changed void to int
    void *startArg;
    int sid;
    int tag;
    int parentID;
    int childIDs[P1_MAXPROC];
    int childCount;
    int childQuit;
    int status;


} PCB;

struct Node{
    int pid;
    struct Node *nxt;
};
static struct Node* processTracker[6];
static int currPid = -1;
static int pidFirst = -1;
static PCB processTable[P1_MAXPROC];   // the process table
static int timeElapsed = 0;


int fetchHP();

int peekHP();

int removeEl(int pid);

void addToTracker(int pid, int priority);

void launch(void* arg) {
    assert(processTable[currPid].startFunc != NULL);

    // enable interrupts and run function. If returns, call quit
    P1EnableInterrupts();
    int sfv = processTable[currPid].startFunc(arg);
    P1_Quit(sfv);
}

void P1ProcInit(void)
{
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }

    P1ContextInit();
    // initialize everything including the processTable
    for (int i = 0; i< P1_MAXPROC; i++){
        processTable[i].state = P1_STATE_FREE;
        processTable[i].cid = -1;
        processTable[i].cpuTime = 0;
        processTable[i].name[0] = '\0';
        processTable[i].priority = -1;
        //processTable[i].state = P1_STATE_FREE;
        processTable[i].startFunc = NULL;
        processTable[i].startArg = NULL;
        processTable[i].sid = -1;
        processTable[i].tag = -1;
        processTable[i].parentID = -1;
        processTable[i].childCount = 0;
        processTable[i].childQuit = 0;
        processTable[i].status = 0;
    }

    for (int i = 0; i < 6; i++){
        processTracker[i] = NULL;
    }

}

int P1_GetPid(void) 
{
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }
    return currPid;
}

int P1_Fork(char *name, int (*func)(void*), void *arg, int stacksize, int priority, int tag, int *pid ) 
{
    int result = P1_SUCCESS;

    // check for kernel mode
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }
    // disable interrupts
    int disableInterrupts;
    disableInterrupts = P1DisableInterrupts();
    // check all parameters
    if (tag != 0 && tag != 1) {
        result = P1_INVALID_TAG;
        return result;
    }

    if (priority < 1 || priority > 6 || (priority == 6 && pidFirst != -1)) {
        result = P1_INVALID_PRIORITY;
        return result;
    }

    if (stacksize < USLOSS_MIN_STACK) {
        result = P1_INVALID_STACK;
        return result;
    }

    int nameFlag = 0;
    int flagger7 = 0;
    for (int i = 0; i<P1_MAXPROC; i++){
        if ((processTable[i].state != P1_STATE_FREE && strcmp(processTable[i].name, name) == 0) && flagger7==0) {
            nameFlag = 1;
            flagger7=1;
        }
    }
    if(nameFlag==1){
        result = P1_DUPLICATE_NAME;
        return result;
    }

    if (name == NULL) {
        result = P1_NAME_IS_NULL;
        return result;
    }

    if (strlen(name) > P1_MAXNAME) {
        result = P1_NAME_TOO_LONG;
        return result;
    }

    int tooManyFlag = -1;
    int flagger3 = 0;
    for(int i = 0; i<P1_MAXPROC;i++){
        if((processTable[i].state == P1_STATE_FREE) && flagger3==0){
            tooManyFlag = i;
            flagger3 = 1;
        }
    }
    if(tooManyFlag == -1){
        result = P1_TOO_MANY_PROCESSES;
        return result;
    }
    *pid = tooManyFlag;
    // create a context using P1ContextCreate
    int currentID;
    int contextCreation = P1ContextCreate(launch, arg, stacksize, &currentID);
    if(contextCreation != P1_SUCCESS){
        USLOSS_Halt(0);
    }
    // allocate and initialize PCB
    processTable[*pid].cid = currentID;
    processTable[*pid].cpuTime = 0;
    strcpy(processTable[*pid].name, name);
    processTable[*pid].priority = priority;
    processTable[*pid].state = P1_STATE_READY;
    processTable[*pid].startFunc = func;
    processTable[*pid].startArg = arg;
    processTable[*pid].tag = tag;
    // if this is the first process or this process's priority is higher than the 
    if (currPid == -1) {
        pidFirst = *pid;
    } else {
        processTable[*pid].parentID = currPid;
        processTable[currPid].childIDs[processTable[currPid].childCount] = *pid;
        processTable[currPid].childCount++;
    }
    addToTracker(*pid, priority);
    //    currently running process call P1Dispatch(FALSE)
    if (currPid == -1 || priority < processTable[currPid].priority) {
        P1Dispatch(FALSE);
    }
    // re-enable interrupts if they were previously enabled
    if(disableInterrupts){
        P1EnableInterrupts();
    }
    result = P1_SUCCESS;
    return result;
}

void 
P1_Quit(int status) 
{
    // check for kernel mode
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }
    // disable interrupts
    int disableInterrupts;
    disableInterrupts = P1DisableInterrupts();
    // remove from ready queue, set status to P1_STATE_QUIT
    processTable[currPid].state = P1_STATE_QUIT;
    processTable[currPid].status = status;
    // if first process verify it doesn't have children, otherwise give children to first process
    if(processTable[currPid].childCount != 0){
        if (currPid == pidFirst) {
            USLOSS_Console("First process quitting with children, halting.\n");
            USLOSS_Halt(1);
        } else {
            for(int i = 0; i <processTable[currPid].childCount; i++){
                processTable[pidFirst].childIDs[processTable[pidFirst].childCount] = processTable[currPid].childIDs[i];
                processTable[pidFirst].childCount++;
            }
        }
    }
    // add ourself to list of our parent's children that have quit
    processTable[processTable[currPid].parentID].childQuit++;
    // if parent is in state P1_STATE_JOINING set its state to P1_STATE_READY
    if (processTable[processTable[currPid].parentID].state == P1_STATE_JOINING) {
        processTable[processTable[currPid].parentID].state = P1_STATE_READY;
    }
    P1Dispatch(FALSE);
    // should never get here
    assert(0);
}


int 
P1GetChildStatus(int tag, int *cpid, int *status) 
{
    // check for kernel mode
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }

    int result = P1_SUCCESS;
    // do stuff here
    if (tag != 0 && tag != 1) {
        result = P1_INVALID_TAG;
        return result;
    }
    if (processTable[currPid].childQuit == 0 && processTable[currPid].childCount != 0) {
        result = P1_NO_QUIT;
        return result;
    }

    for (int i = 0; i<processTable[currPid].childCount; i++){
        if (processTable[processTable[currPid].childIDs[i]].state == P1_STATE_QUIT 
            && processTable[processTable[currPid].childIDs[i]].tag == tag){
                *cpid = processTable[currPid].childIDs[i];
                *status = processTable[*cpid].status;

                int k = 0;
                int buffer[P1_MAXPROC];
                for(int j = 0; j< processTable[currPid].childCount; j++){
                    if(processTable[currPid].childIDs[j] != *cpid){
                        buffer[k] = processTable[currPid].childIDs[j];
                        k++;
                    }
                }
                processTable[currPid].childCount = k;
                for (int j = 0; j<k;j++){
                    processTable[currPid].childIDs[j] = buffer[j];
                }

                assert(P1ContextFree(processTable[*cpid].cid) == P1_SUCCESS);
                for (int j = 0; j <P1_MAXPROC;j++){
                    processTable[*cpid].state = P1_STATE_FREE;
                    processTable[*cpid].cid          = -1;
                    processTable[*cpid].cpuTime      = 0;
                    processTable[*cpid].name[0]      = '\0';
                    processTable[*cpid].priority     = -1;
                    processTable[*cpid].state        = P1_STATE_FREE;
                    processTable[*cpid].startFunc    = NULL;
                    processTable[*cpid].startArg     = NULL;
                    processTable[*cpid].parentID       = -1;
                    processTable[*cpid].childCount  = 0;
                    processTable[*cpid].childQuit = 0;
                    processTable[*cpid].status       = 0;
                    processTable[*cpid].sid          = -1;
                    processTable[*cpid].tag          = -1;
                }
                result = P1_SUCCESS;
                return result;
            }
    }
    result = P1_NO_CHILDREN;
    return result;
}

int
P1_GetProcInfo(int pid, P1_ProcInfo *info)
{
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }
    int result = P1_SUCCESS;
    if (pid >= P1_MAXPROC || pid < 0 || processTable[pid].state == P1_STATE_FREE) {
        result = P1_INVALID_PID;
        return result;
    }
    // fill in info here
    strcpy(info[0].name, processTable[pid].name);
    info[0].state = processTable[pid].state;
    info[0].sid = processTable[pid].sid;
    info[0].priority = processTable[pid].priority;
    info[0].tag = processTable[pid].tag;
    info[0].cpu = processTable[pid].cpuTime;
    info[0].parent = processTable[pid].parentID;
    info[0].numChildren = processTable[pid].childCount;
    int i;
    for (i = 0; i < info[0].numChildren; i++) {
        info[0].children[i] = processTable[pid].childIDs[i];
    }
    result = P1_SUCCESS;
    return result;
}

int
P1SetState(int pid, P1_State state, int sid) 
{
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }
    int result = P1_SUCCESS;
    // do stuff here
    if ((pid >= P1_MAXPROC) || (pid < 0) || (processTable[pid].state == P1_STATE_FREE)) {
        result = P1_INVALID_PID;
        return result;
    }
    if ((state != P1_STATE_READY) && (state != P1_STATE_JOINING) && (state != P1_STATE_BLOCKED) && (state != P1_STATE_QUIT)) {
        result = P1_INVALID_STATE;
        return result;
    }
    if (state == P1_STATE_JOINING && processTable[pid].childQuit != 0) {
        result = P1_CHILD_QUIT;
        return result;
    }
    if (processTable[pid].state == P1_STATE_QUIT) {
        result = P1_SUCCESS;
        return result;
    }

    int prev = processTable[pid].state;
    processTable[pid].sid = sid;
    processTable[pid].state = state;
    if(prev==P1_STATE_READY && state != P1_STATE_READY){
        assert(removeEl(pid) == TRUE);
    }
    if(prev != P1_STATE_READY && state == P1_STATE_READY){
        processTable[pid].priority--;
        struct Node* curr2 = processTracker[processTable[pid].priority];
        int nullFlagger = 0;
        if(curr2 == NULL){
            processTracker[processTable[pid].priority] = malloc(sizeof(struct Node));
            processTracker[processTable[pid].priority]->pid = pid;
            processTracker[processTable[pid].priority]->nxt = NULL;
            nullFlagger = 1;
        }
        if(nullFlagger == 0){
            while(curr2->nxt != NULL){
                curr2 = curr2->nxt;
            }
            curr2->nxt = malloc(sizeof(struct Node));
            curr2->nxt->pid = pid;
            curr2->nxt->nxt = NULL;
        }
    }
    result = P1_SUCCESS;
    return result;
}

void
P1Dispatch(int rotate)
{
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        P1_Quit(1024);
        USLOSS_IllegalInstruction();
    }
    // record time
    int time;
    int currStat = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &time);
    if (currStat != USLOSS_DEV_OK) {
        USLOSS_Halt(currStat);
    }
    if (currPid != -1) {
        processTable[currPid].cpuTime += time - timeElapsed;
    }
    timeElapsed = time;

    // halt if no processes are running
    if (processTable[currPid].state != P1_STATE_RUNNING && peekHP() == -1) {
        USLOSS_Console("No runnable processes, halting.\n");
        USLOSS_Halt(0);
    }
    // select the highest-priority runnable process
    if ((peekHP() < processTable[currPid].priority) || (rotate && peekHP() == processTable[currPid].priority)) {
        processTable[currPid].state = P1_STATE_READY;
        addToTracker(currPid, processTable[currPid].priority);
    }
    currPid = fetchHP();
    // call P1ContextSwitch to switch to that process
    if (processTable[currPid].state != P1_STATE_RUNNING) {
        processTable[currPid].state = P1_STATE_RUNNING;
        assert(P1ContextSwitch(processTable[currPid].cid) == P1_SUCCESS);
    }
}

int removeEl(int pid) {
    struct Node *curr;
    for (int i = 0; i < 6; i++) {
        if (processTracker[i] != NULL && processTracker[i]->pid == pid) {
            processTracker[i] = processTracker[i]->nxt;
            return 1;
        } 
        else if (processTracker[i] != NULL) {
            curr = processTracker[i];
            while (curr->nxt != NULL && curr->nxt->pid != pid) {
                curr = curr->nxt;
            }
            if (curr->nxt!= NULL) {
                curr->nxt = curr->nxt->nxt;
                return 1;
            }
        }
    }
    return 0;
}
int fetchHP() {
    int result = -1;
    for (int i = 0; i < 6; i++) {
        if (processTracker[i] != NULL) {
            result = processTracker[i]->pid;
            processTracker[i] = processTracker[i]->nxt;
            return result;
        }
    }
    return result;
}

int peekHP() {
    for (int i = 0; i < 6; i++) {
        if (processTracker[i] != NULL) {
            return processTable[processTracker[i]->pid].priority;
        }
    }
    return -1;
}

void addToTracker(int pid, int priority) {
    priority--;
    struct Node* curr = processTracker[priority];
    if (curr == NULL) {
        processTracker[priority] = malloc(sizeof(struct Node));
        processTracker[priority]->pid = pid;
        processTracker[priority]->nxt = NULL;
        return;
    }
    while (curr->nxt != NULL) {
        curr = curr->nxt;
    }
    curr->nxt = malloc(sizeof(struct Node));
    curr->nxt->pid = pid;
    curr->nxt->nxt = NULL;
}