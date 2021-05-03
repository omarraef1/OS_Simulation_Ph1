/*
* Names : Omar R.G. Mohamed, Isabel Campos
* Date: September 16, 2020
* Phase 1 Part A
* Type : Group
*/
#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    USLOSS_Context  *context;
    // you'll need more stuff here
    char *stack;
    int stacksize;
    int free;
} Context;

static Context   contexts[P1_MAXPROC];
static int currentCid = -1;

/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void) {
    assert(contexts[currentCid].startFunc != NULL);
    contexts[currentCid].startFunc(contexts[currentCid].startArg);
}

void P1ContextInit(void) {
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        USLOSS_Console("Called P1ContextInit from user mode Error\n");
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }

    // initialize contexts
    for (int i = 0; i < P1_MAXPROC; i++) {
	Context a = {NULL, NULL, NULL, NULL, USLOSS_MIN_STACK, 1};
	contexts[i] = a;
    }
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        USLOSS_Console("Called P1ContextCreate from user mode Error\n");
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }

    if (stacksize < USLOSS_MIN_STACK) {
        return P1_INVALID_STACK;
    }
    int flag;
    int k = -1;
    for (int i = 0; i < P1_MAXPROC; i++) {
        if (contexts[i].free == 1) {
            k = i;
	    break;
        }
    }
    flag = k;
    if (flag == -1) {
        return P1_TOO_MANY_CONTEXTS;
    }
    *cid = flag;
    contexts[*cid].startFunc = *func;
    contexts[*cid].startArg = arg;
    contexts[*cid].stack = malloc(stacksize *sizeof(char) );
    contexts[*cid].context = malloc(sizeof(USLOSS_Context));
    contexts[*cid].stacksize = stacksize;
    USLOSS_ContextInit(contexts[*cid].context, contexts[*cid].stack, stacksize, P3_AllocatePageTable(cid[0]), launch);
    contexts[*cid].free = 0;
    return P1_SUCCESS;
}

int P1ContextSwitch(int cid) {
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        USLOSS_Console("Called P1ContextSwitch from user mode Error\n");
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }
    int result = P1_SUCCESS;
    // switch to other context
    if ((cid < 0) || (cid >= P1_MAXPROC)){
        result = P1_INVALID_CID;
        }
    else if (contexts[cid].free == 1){
        result = P1_INVALID_CID;
        }
    else {
        int previousCid = currentCid;
        currentCid = cid;
	if (previousCid != -1){ // if previous context
        	USLOSS_ContextSwitch(contexts[previousCid].context, contexts[cid].context);
	}else{ // if first context
		USLOSS_ContextSwitch(NULL, contexts[cid].context);
	}
    }
    return result;
}

int P1ContextFree(int cid) {
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != USLOSS_PSR_CURRENT_MODE) {
        USLOSS_Console("Called P1ContextFree from user mode Error\n");
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }

    if (cid == currentCid) {
        return P1_CONTEXT_IN_USE;
    }

    if ((cid < 0) || (cid >= P1_MAXPROC) || (contexts[cid].free==1)) {
        return P1_INVALID_CID;
    }

    contexts[cid].startFunc = NULL;
    contexts[cid].startArg  = NULL;
    free(contexts[cid].stack);
    contexts[cid].stack = NULL;
    free(contexts[cid].context);
    contexts[cid].stacksize = USLOSS_MIN_STACK;
    contexts[cid].free = 1;
    P3_FreePageTable(cid);
    return P1_SUCCESS;
}


void P1EnableInterrupts(void) {
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0) {
        USLOSS_Console("Called P1EnableInterrupts from user mode Error\n");
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }
    int rc = USLOSS_PsrSet(USLOSS_PsrGet() | (1 << 1)); //set 2nd bit to 1
    assert(rc == USLOSS_DEV_OK);
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int P1DisableInterrupts(void) {
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0) {
        USLOSS_Console("Called P1DisableInterrupts from user mode Error\n");
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
    }
    int enabled = FALSE;
    // set enabled to TRUE if interrupts are already enabled
    // clear the interrupt bit in the PSR
    int psr = USLOSS_PsrGet();
    enabled = (psr>>1)&1; //set TRUE if enabled
    
    int mask = 0xFFFFFFFF^(1<<1); // sets all bits to 1's except for the 2nd bit
    int rc = USLOSS_PsrSet(psr&mask); //clear interrupt bit
    assert(rc == USLOSS_DEV_OK);

    return enabled;
}
