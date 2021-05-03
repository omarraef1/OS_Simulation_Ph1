#include "phase1.h"
#include <stdio.h>
#include "phase1Int.h"
#include "usloss.h"
#include <assert.h>

// Tests invoking a system call
// Output should be something like:
// System call handlers has not been inplemented yet! (the content of this line depends on the implementation of dummy handler)
// Not in kernel mode!
// Aborted

int P2_Startup(void *arg){
        
        int rc = USLOSS_PsrSet(USLOSS_PsrGet() & 0xfe);    //switch to user mode
        assert(rc == USLOSS_DEV_OK);
        USLOSS_Syscall(NULL);
        return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}                                                                                                                                   
