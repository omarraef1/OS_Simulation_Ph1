#include <stdio.h>	
#include "phase1.h"
#include "tester.h"
#include <stdlib.h>
#include <assert.h> 
/* Test Case to test Basic Interrupt Handling, Disk interrupt Handler, Disk Device Semaphore using Disk Seek
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
	req->opr = USLOSS_DISK_SEEK; // Fill the request indicating the operation to be done on the disk
	req->reg1 = (void*)2; // Track to be moved. This assumes the disk has atleast 3 tracks.
	status = USLOSS_DeviceOutput(USLOSS_DISK_DEV, 1, req); // This assumes Disk1 has been created as indicated in the comment section above.
	assert(status == USLOSS_ERR_OK);	
        // Move to Disk, unit 1, Track 2, Request obj..that contains all the information needed to be done on the Disk.
	rc = P1_WaitDevice(USLOSS_DISK_DEV, 1, &status); // Wait for the interrupt to occur
	TEST(rc, P1_SUCCESS);
	USLOSS_Console("status from WaitDevice is %d \n", status);// Retrieve the status
	TEST(status, USLOSS_DEV_READY); // For Disk device, this implies the request completed
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
