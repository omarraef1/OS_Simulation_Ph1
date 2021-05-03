#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "usloss.h"
#include "phase1.h"
#include "phase1Int.h"

static void DeviceHandler(int type, void *arg);
static void SyscallHandler(int type, void *arg);
static void IllegalInstructionHandler(int type, void *arg);

static int sentinel(void *arg);

// struct to hold the status od deivce 
typedef struct INFO{
	int statusNum;
	int abortNum;
} INFO;

int track[4][4];
INFO devices[4];
int count ;
int sems;


void 
startup(int argc, char **argv)
{
    int pid;
    P1SemInit();

    // initialize device data structures
	//track = new int[4][4];
	for (int i = 0; i < 4; i++){
		for(int m = 0; m < 4; m++ ){
			track[i][m] = -1; // no semaphore
		}
	}
	count = 1;
	sems = 0;
		
	USLOSS_IntVec[USLOSS_ALARM_INT] = DeviceHandler; // ?
	USLOSS_IntVec[USLOSS_TERM_INT] = DeviceHandler; // ??
	USLOSS_IntVec[USLOSS_DISK_INT] = DeviceHandler; // ???
	USLOSS_IntVec[USLOSS_CLOCK_INT] = DeviceHandler;// the clock time?
    // put device interrupt handlers into interrupt vector
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = SyscallHandler;

    /* create the sentinel process */
    int rc = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6 , 0, &pid);
    assert(rc == P1_SUCCESS);
    // should not return
    assert(0);
    return;

} /* End of startup */

char * createName(char* string, int num){
	static char name[P1_MAXNAME+1];
	snprintf(name, sizeof(name),"%s%d", string, num);
	return name;
}

int 
P1_WaitDevice(int type, int unit, int *status) 
{
    // disable interrupts
	int check = P1DisableInterrupts();
    // check kernel mode
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_Console("Called in user mode.\n");
		USLOSS_IllegalInstruction();
	}

	if ((type != USLOSS_CLOCK_DEV) &&( type != USLOSS_ALARM_DEV) &&
		( type != USLOSS_DISK_DEV) &&( type != USLOSS_TERM_DEV)){
		return P1_INVALID_TYPE;
	}

	if ((type == USLOSS_CLOCK_DEV) && (unit >( USLOSS_CLOCK_UNITS-1) || unit < 0)){
		return P1_INVALID_UNIT;
	}else if ((type == USLOSS_ALARM_DEV) && (unit > (USLOSS_ALARM_UNITS-1) || unit < 0)){
		return P1_INVALID_UNIT;
	}else if ((type == USLOSS_DISK_DEV) && (unit >( USLOSS_DISK_UNITS-1) || unit < 0)){
		return P1_INVALID_UNIT;
	} else if ((type == USLOSS_TERM_DEV) && (unit >( USLOSS_TERM_UNITS-1) || unit < 0)){
		return P1_INVALID_UNIT;
	}
    // P device's semaphore
	int sem = track[type][unit];// probably not. Only one semaphore for each device unit?
	if (sem == -1){
		//create semaphore
		int save ;
		assert(P1_SUCCESS == P1_SemCreate(createName("wait", sems), 0, &save));
		sems +=1;
		track[type][unit] = save;
		sem = save;
	}
	assert(P1_SUCCESS == P1_P(sem));
    // set *status to device's status
	*status = devices[type].statusNum; // unit doesn't matter ? just the device itself.
    // restore interrupts
	if (check ){
		P1EnableInterrupts();
	}
	if (devices[type].abortNum ){ // if true?
		return P1_WAIT_ABORTED;
	}
    return P1_SUCCESS;
}

int 
P1_WakeupDevice(int type, int unit, int status, int abort) 
{
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE ) != USLOSS_PSR_CURRENT_MODE){
		USLOSS_IllegalInstruction();
	}
    // disable interrupts
	int check = P1DisableInterrupts();
    // check kernel mode
	if ((type != USLOSS_CLOCK_INT) && (type != USLOSS_ALARM_INT )&&( 
		type != USLOSS_DISK_INT) &&( type != USLOSS_TERM_INT)){
		return P1_INVALID_TYPE;
	}

	if (type == USLOSS_CLOCK_DEV && (unit > (USLOSS_CLOCK_UNITS -1)|| unit < 0)){
		return P1_INVALID_UNIT;
	}else if (type == USLOSS_ALARM_DEV && (unit > (USLOSS_ALARM_UNITS-1) || unit < 0)){
		return P1_INVALID_UNIT;
	}else if (type == USLOSS_DISK_DEV && (unit > (USLOSS_DISK_UNITS-1) || unit < 0)){
		return P1_INVALID_UNIT;
	}else if (type == USLOSS_TERM_DEV && (unit >( USLOSS_TERM_UNITS-1) || unit < 0)){
		return P1_INVALID_UNIT;
	}

    // save device's status to be used by P1_WaitDevice
	devices[type].statusNum = status;
    // save abort to be used by P1_WaitDevice
	devices[type].abortNum = abort;
    // V device's semaphore 
	int sem = track[type][unit];
	if (sem == -1){
		int save;
		assert(P1_SUCCESS == P1_SemCreate(createName("wakeup", sems), 0, &save));
		track[type][unit] = save;
		sems+=1;
		sem = save;
	}
	assert( P1_SUCCESS == P1_V(sem));
    // restore interrupts
	if (check){
		P1EnableInterrupts();
	}
    return P1_SUCCESS;
}

static void
DeviceHandler(int type, void *arg) 
{
    // if clock device
	if (type == USLOSS_CLOCK_DEV){
    //      P1_WakeupDevice every 5 ticks
		if (count == 5){//???
			//type unit status abort
			int stat;
			assert( P1_SUCCESS ==  USLOSS_DeviceInput(type, 0, &stat));
		//	USLOSS_Console("return from getting stat %d and stat %d\n", saveIt, stat);
			assert(P1_SUCCESS == P1_WakeupDevice(type, 0, stat, FALSE));
			//P1_WakeupDevice();
			//reset?
			count = 0;
		}else if (count == 4){
			P1Dispatch(TRUE);
		}
    //      P1Dispatch(TRUE) every 4 ticks
    // else
	}else{
    //      P1_WakeupDevice
		// type, unit, status, abort
		
		int stat2;
		assert( P1_SUCCESS ==USLOSS_DeviceInput(type,(int) arg , &stat2));
		assert(P1_SUCCESS == P1_WakeupDevice(type, (int)arg, stat2, FALSE)); // ?
	}
	count+=1;
}

static int
sentinel (void *notused)
{
    int     pid;
    int     rc;

    /* start the P2_Startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 2 , 0, &pid);
    assert(rc == P1_SUCCESS);

    // enable interrupts
	P1EnableInterrupts(); //
    // while sentinel has children
	int id;
	int stat;
	int val = 1;
	int ret1 = 0;
	int ret2 = 0;
	while(val!= 0 ){
    //      get children that have quit via P1GetChildStatus (either tag)
		//USLOSS_Console("in while loop\n");
		// call P1GetChildStatus();
		ret1= P1GetChildStatus(0, &id, &stat);
		ret2=P1GetChildStatus(1, &id, &stat);
		if ((ret1 == ret2) &&(ret1 == P1_NO_CHILDREN)){
			val = 0; // done no children with either tag not waiting to quit
		}
    //      wait for an interrupt via USLOSS_WaitInt
		USLOSS_WaitInt(); // ???????????
	}
    USLOSS_Console("Sentinel quitting.\n");
    return 0;
} /* End of sentinel */

int 
P1_Join(int tag, int *pid, int *status) 
{
	if (tag != 0 && tag != 1){
		return P1_INVALID_TAG;
	}
    // disable interrupts
	int back =  P1DisableInterrupts();
    // kernel mode
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)!= USLOSS_PSR_CURRENT_MODE){
		USLOSS_Console("Invoked in user mode.\n");
		USLOSS_IllegalInstruction();
	}
    // do
	int keep = 1;
	int val = P1_SUCCESS;
	do{
    //     use P1GetChildStatus to get a child that has quit  
		val = P1GetChildStatus( tag, pid, status);
    //     if no children have quit
		if (val == P1_NO_QUIT){ // has children but none have quit
    //        set state to P1_STATE_JOINING vi P1SetState
			int sid ; // create semaphore
			assert (P1_SUCCESS == P1_SemCreate(createName("join", sems),0 , &sid));
			sems +=1;
			assert (P1_SUCCESS == P1SetState(P1_GetPid(), P1_STATE_JOINING, sid)); //
    //        P1Dispatch(FALSE)
			P1Dispatch(FALSE);
		} else if (val == P1_NO_CHILDREN || val ==P1_SUCCESS){ // has no children with specified tag or a child has quit
			keep = 0;
		}
	}while(keep == 1) ;
    // until either a child quit or there are no more children
	if (back){
		P1EnableInterrupts();//????????
	}
    return val;
}

static void
SyscallHandler(int type, void *arg) 
{
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE){

	
    		USLOSS_Console("System call %d not implemented.\n", (int) arg);
 	   	USLOSS_IllegalInstruction();
	}
}

void finish(int argc, char **argv) {}
