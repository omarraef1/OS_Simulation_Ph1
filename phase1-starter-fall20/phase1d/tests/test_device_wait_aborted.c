#include <stdio.h>
#include <assert.h>
#include "phase1.h"
#include "phase1Int.h"
#include "tester.h"
 /*
   Tests aborting wait device 
   Output:
	Waking up term device with abort=TRUE
	P1_WaitDevice returns P1_WAIT_ABORTED
	TEST PASSED
 
 */

int P2_Startup(void *arg) {
	int status = 0;
	int rc;
	USLOSS_Console("Waking up term device with abort=TRUE\n");
	rc = P1_WakeupDevice(USLOSS_TERM_DEV, 0, status, 1);
	rc = P1_WaitDevice(USLOSS_TERM_DEV, 0, &status); // Wait for the interrupt to occur
	USLOSS_Console("P1_WaitDevice returns P1_WAIT_ABORTED\n");
	TEST(rc, P1_WAIT_ABORTED);
	PASSED();
	return 0;
}


void test_setup(int argc, char **argv) {
	// Do nothing.
}

void test_cleanup(int argc, char **argv) {
	// Do nothing.
}
