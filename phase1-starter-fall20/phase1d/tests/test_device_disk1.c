#include <stdio.h>	
#include "phase1.h"
#include "tester.h"
#include <stdlib.h>
#include <assert.h> 
/* Test Case to test Basic Interrupt Handling, Disk interrupt Handler, Disk Device Semaphore using Disk Write
   Output:
	status from WaitDevice is 0
        Request Completed Succesfully 
	TEST PASSED
*/ 

	
int P2_Startup(void *arg) {
	
	
	// To Seek to a particular track in the  disk
	int status;
	int rc;
	USLOSS_DeviceRequest *req = (USLOSS_DeviceRequest *)malloc(sizeof(USLOSS_DeviceRequest)); //Needed to pass to USLOSS_DeviceOutput
	req->opr = USLOSS_DISK_WRITE; // Fill the request indicating the operation to be done on the disk
	req->reg1 = (void*)1; //sector to be written.
	char *buf = (char *)malloc(512); // Buffer that holds the characters to be written
	buf[0] = 'a';
	buf[1] = '\0';
	req->reg2 = buf; //Set the buffer in the request
	status = USLOSS_DeviceOutput(USLOSS_DISK_DEV, 1, req); // Write to Disk, unit 1, Request obj..that contains all the information needed to be done on the Disk.
	assert(status == USLOSS_ERR_OK);
	rc = P1_WaitDevice(USLOSS_DISK_DEV, 1, &status); // Wait for the interrupt to occur
	TEST(rc, P1_SUCCESS);
	USLOSS_Console("status from WaitDevice is %d \n", status);// Retrieve the status
	TEST(status, USLOSS_DEV_READY) // For Disk device, this implies the request completed
	USLOSS_Console("Request Completed Succesfully \n");
        PASSED();	
        return 0;	
	
}

void test_setup(int argc, char **argv) {
        // Do nothing.
}

void test_cleanup(int argc, char **argv) {
        // Do nothing.
}
