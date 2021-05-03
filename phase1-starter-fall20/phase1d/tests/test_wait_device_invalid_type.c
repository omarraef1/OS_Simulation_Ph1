#include <stdio.h>
#include <assert.h>
#include "phase1.h"
#include "phase1Int.h"
#include "tester.h"
 /*
   Tests wait device with invalid type 
   Output:
	TEST PASSED
 
 */

int P2_Startup(void *arg) {
	int status = 0;
	int rc;
	rc = P1_WaitDevice(4, 0, &status); // Wait for the interrupt to occur
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
