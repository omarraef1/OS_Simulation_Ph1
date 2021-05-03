// isabel
// omar
// group
// Phase 1 part c 
// October 14, 2020
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"

typedef struct Sem
{
    char        name[P1_MAXNAME+1];
    u_int       value;
    // more fields here
	int free ;
	int wait[P1_MAXPROC]; 
} Sem;

static Sem sems[P1_MAXSEM];

void 
P1SemInit(void) 
{
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_IllegalInstruction();
	}
    P1ProcInit();
    for (int i = 0; i < P1_MAXSEM; i++) {
        sems[i].name[0] = '\0';
	sems[i].free = 0;
        // initialize rest of sem here
	for (int a = 0; a < P1_MAXPROC; a++){
		sems[i].wait[a] = -1;
	}
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid)
{
    // check for kernel mode
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_IllegalInstruction();
	}
    // disable interrupts
	int undo = P1DisableInterrupts();
    // check parameters
	if (name == NULL){
		return P1_NAME_IS_NULL;
	}
	if(strlen(name) > P1_MAXNAME){
		return P1_NAME_TOO_LONG;
	}
    // find a free Sem and initialize it
	int semNo = -1;
	for (int i = 0; i< P1_MAXSEM; i++){
		if (strcmp(sems[i].name, name) == 0){
			return P1_DUPLICATE_NAME;
		}
		if (sems[i].free == 0 && semNo == -1){
			semNo = i;
		}
	}
	if (semNo == -1){
		return P1_TOO_MANY_SEMS;
	}
	sems[semNo].free = 1; // not free
	sems[semNo].value = value;
	*sid = semNo;
	strcpy(sems[semNo].name, name);
    // re-enable interrupts if they were previously enabled
	if (undo){
		P1EnableInterrupts();
	}
    return P1_SUCCESS;
}

int P1_SemFree(int sid) 
{
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_IllegalInstruction();
	}
	if ((sid < 0) || (sid >= P1_MAXSEM) || (sems[sid].free==0)){ // not active 
		return P1_INVALID_SID;
	}

	for(int i = 0; i < P1_MAXPROC ; i++){
		if (sems[sid].wait[i] != -1){
			return P1_BLOCKED_PROCESSES;
		}
	}

	sems[sid].free = 0;
	sems[sid].name[0] = '\0';
	sems[sid].value = 0; 
	//free frist process in semaphore
    return P1_SUCCESS;
}

int P1_P(int sid) 
{
    // check for kernel mode
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_IllegalInstruction();
	}
    // disable interrupts
	if ((sid < 0) || (sid >= P1_MAXSEM) || (sems[sid].free == 0)){
		return P1_INVALID_SID;
	}
	int undo = P1DisableInterrupts();
	int pid = P1_GetPid(); 
	
	while (sems[sid].value == 0){
		 assert (P1_SUCCESS == P1SetState( pid , P1_STATE_BLOCKED, sid));
         	for (int i = 0; i < P1_MAXPROC; i++){
			if (sems[sid].wait[i] == -1){
		    		sems[sid].wait[i] = pid;
		    		break;
	    		}
    		}
		 P1Dispatch(FALSE);
	}
	sems[sid].value--;
    // re-enable interrupts if they were previously enabled
	if (undo){
		P1EnableInterrupts();
	}
    return P1_SUCCESS;
}

int P1_V(int sid) 
{
    // check for kernel mode
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_IllegalInstruction();
	}
	if ((sid < 0) || (sid >= P1_MAXSEM) || (sems[sid].free == 0)){
		return P1_INVALID_SID;
	}
    // disable interrupts
	int undo = P1DisableInterrupts();
	sems[sid].value ++;
    // if a process is waiting for this semaphore
	int pid = sems[sid].wait[0]; // first process
	if(pid != -1) { //if waiting
		assert(P1_SUCCESS == P1SetState(pid, P1_STATE_READY, sid));
		P1Dispatch(FALSE);
	}
	// move to left
	for (int i = 0; i < P1_MAXPROC - 1; i++){
		sems[sid].wait[i] = sems[sid].wait[i+1];
	}
	sems[sid].wait[P1_MAXPROC-1] = -1;
    // re-enable interrupts if they were previously enabled
	if (undo){
		P1EnableInterrupts();
	}
    return P1_SUCCESS;
}

int P1_SemName(int sid, char *name) {
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_IllegalInstruction();
	}
	if (((sid < 0) && (sid >= P1_MAXSEM) ) || (sems[sid].free == 0)){ // not active sid
		return P1_INVALID_SID;
	}	
	if (name == NULL){
		return P1_NAME_IS_NULL;
	}
	strcpy(name, sems[sid].name);
    return P1_SUCCESS;
}

