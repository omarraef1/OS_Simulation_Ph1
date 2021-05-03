#include "phase1.h"
#include <assert.h>
#include <stdio.h>
#include "tester.h"
#include "phase1Int.h"

// Tests P1_Join() with 10 children Child priority 1, each has 1 child ChildChild priority 3.
// Child has to wait for ChildChild to quit before it can quit even though Child has the higher priority.

#define NUM 10

int ChildChild(void *arg){
    USLOSS_Console("Child%dChild running\n", (int) arg);
    USLOSS_Console("Child%dChild quitting\n", (int) arg);
    P1_Quit((int) arg);
    return 0;
}


int Child(void *arg) {
    int rc;
    char name[P1_MAXNAME+1];
    int childpid;
    int status;
    USLOSS_Console("Child %d: running\n", (int) arg);
    
    snprintf(name, sizeof(name), "Child%dChild", (int) arg);
    USLOSS_Console("Child %d: forking %s\n", (int) arg, name);
    rc = P1_Fork(name, ChildChild, arg, USLOSS_MIN_STACK, 3, 1, &childpid);
    TEST(rc, P1_SUCCESS);

    rc = P1_Join(1, &childpid, &status);
    TEST(rc, P1_SUCCESS);
    TEST(status, (int) arg); 
    USLOSS_Console("Child %d quitting\n", (int) arg);
    return (int) arg;
}

int P2_Startup(void *notused)
{
    int status = 0;
    int rc;
    int pids[NUM];
    for (int j = 0; j < NUM; j++) {
        char name[P1_MAXNAME+1];
        snprintf(name, sizeof(name), "Child %d", j);
        rc = P1_Fork(name, Child, (void *) j, USLOSS_MIN_STACK, 1, 0, &pids[j]);
        TEST(rc, P1_SUCCESS);	
    }
    int pid;
    for (int j = 0; j < NUM; j++) {
            rc = P1_Join(0, &pid, &status);
	    TEST(rc, P1_SUCCESS);
    }
    TEST(status, 9);
    PASSED();
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
