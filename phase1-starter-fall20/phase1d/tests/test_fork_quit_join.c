#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "tester.h"
static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    P1_Quit(11);
    // should not return
    assert(0);
    return 0;
}

static int
Parent(void *arg)
{   
    int         pid;
    int         rc;
    int         status;
    int         child;
    P1_ProcInfo info;
    char        *msg = (char *) arg;

    rc = P1_GetProcInfo(P1_GetPid(), &info);
    assert(rc == P1_SUCCESS);
    // child has lower priority.
    rc = P1_Fork("Child", Output, msg, USLOSS_MIN_STACK, info.priority+1, 0, &child);
    assert(rc == P1_SUCCESS);
    rc = P1_Join(0, &pid, &status);
    TEST(rc, P1_SUCCESS);
    TEST(pid, child);
    TEST(status, 11);
    return 13;
}

int
P2_Startup(void *arg)
{
    int         pid;
    int         rc;
    int         child;
    int         status;
    P1_ProcInfo info;

    USLOSS_Console("P2_Startup\n");
    rc = P1_GetProcInfo(P1_GetPid(), &info);
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Parent", Parent, "Hello World!\n", USLOSS_MIN_STACK, info.priority-1, 0, &child);
    assert(rc == P1_SUCCESS);
    rc = P1_Join(0, &pid, &status);
    TEST(rc, P1_SUCCESS);
    TEST(pid, child);
    TEST(status, 13);
    PASSED();
    return 14;
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}
