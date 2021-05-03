#include "phase1.h"
#include "usloss.h"
#include <stdlib.h>
#include "tester.h"
// Tests sentinel quitting properly, should print out 
// Sentinel quitting.
// No runnable processes, halting.
static int sem;

int Child(void *arg){
   USLOSS_Console("Child: Child running.\n");
   USLOSS_Console("Child: V(sem)\n");
   int rc = P1_V(sem);
   TEST(rc, P1_SUCCESS);
   USLOSS_Console("Child: Child quitting.\n");
   return 0;
}


int P2_Startup(void *notused) 
{
    int rc;
    int pid;
    USLOSS_Console("P2_Startup: Forks Child with priority 2.\n");
    rc = P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 2 , 0, &pid);
    TEST(rc, P1_SUCCESS);
    USLOSS_Console("P2_Startup: Creating sem with init value 0.\n");
    rc = P1_SemCreate("sem",0, &sem);
    TEST(rc, P1_SUCCESS);
       
    USLOSS_Console("P2_Startup: P(sem).\n");
    rc = P1_P(sem); 
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
