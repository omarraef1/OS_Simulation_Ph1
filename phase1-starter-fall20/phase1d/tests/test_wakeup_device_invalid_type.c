#include <stdio.h>
#include <assert.h>
#include "phase1.h"
#include "phase1Int.h"
#include "tester.h"
 /*
   Tests wakeup device with invalid type 
   Output:
	TEST PASSED
 
 */

int P2_Startup(void *arg) {
	int rc;
	rc = P1_WakeupDevice(4, 0, 0 ,0); // Wait for the interrupt to occur
	TEST(rc, P1_INVALID_TYPE);
	PASSED();
	return 0;
}


void test_setup(int argc, char **argv) {
	// Do nothing.
}

void test_cleanup(int argc, char **argv) {
	// Do nothing.
}
