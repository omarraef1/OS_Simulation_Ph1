#include "phase1.h"
#include <assert.h>
#include <stdio.h>
#include "tester.h"
// Tests P1_Join() with invalid tag -1 for the first child and valid tags for the remaining 9 children.

int Child(void *arg) {
    USLOSS_Console("Child %d\n", (int) arg);
    return (int) arg;
}

int P2_Startup(void *notused)
{
    #define NUM 10
    int status = 0;
    int rc;
    int pids[NUM];
    for (int j = 0; j < NUM; j++) {
        char name[P1_MAXNAME+1];
        snprintf(name, sizeof(name), "Child %d", j);
        rc = P1_Fork(name, Child, (void *) j, USLOSS_MIN_STACK, 1, 0, &pids[j]);
        TEST(rc, P1_SUCCESS);	
    }
    for (int j = 0; j < NUM; j++) {
            int pid;
            if(j == 0){
               rc = P1_Join(-1, &pid, &status);
	       TEST(rc, P1_INVALID_TAG);
	    } else {
               rc = P1_Join(0, &pid, &status);
	       TEST(rc, P1_SUCCESS);
 	    }
    }
    PASSED();
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
