#include "phase1.h"
#include <assert.h>
#include <stdio.h>
#include "tester.h"
// Tests P1_Join() on a process that has no children

int Child(void *arg) {
    USLOSS_Console("Child %d\n", (int) arg);
    return (int) arg;
}

int P2_Startup(void *notused)
{
    int status = 0;
    int tag = 0;
    int pid;
    int rc = P1_Join(tag, &pid, &status);
    TEST(rc, P1_NO_CHILDREN);
    PASSED();
    
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
