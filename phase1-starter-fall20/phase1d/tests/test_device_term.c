#include <stdio.h>
#include <assert.h>
#include "phase1.h"
#include "phase1Int.h"
#include "tester.h"
/* This should print the first letter in the terminal file term0.in 
   First line is zero:first line
 
   Output:
	z
	TEST PASSED
   
 */

int P2_Startup(void *arg) {
	int     status;
	char ch = '\0';
	int rc;
	status = USLOSS_DeviceOutput(USLOSS_TERM_DEV, 0, (void *) USLOSS_TERM_CTRL_RECV_INT(0));

	assert(status == USLOSS_ERR_OK);

	rc = P1_WaitDevice(USLOSS_TERM_DEV, 0, &status); // Wait for the interrupt to occur
	TEST(rc, P1_SUCCESS);
	if (USLOSS_TERM_STAT_RECV(status) == USLOSS_DEV_BUSY) {
		ch = USLOSS_TERM_STAT_CHAR(status);     // Get the character
		USLOSS_Console("First character of term0.in is:%c \n", ch);
	}
	TEST(ch, 'z');
	PASSED();
	return 0;
}


void test_setup(int argc, char **argv) {
	// Do nothing.
}

void test_cleanup(int argc, char **argv) {
	// Do nothing.
}
