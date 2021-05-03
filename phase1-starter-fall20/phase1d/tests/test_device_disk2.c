#include <stdio.h>	
#include "phase1.h"
#include "tester.h"
#include <stdlib.h>
#include <assert.h> 

/*    Test Case to test  Interrupt Handling, Disk interrupt Handler, Disk Device Semaphore using Disk Seek, Read and Write
      Output:
	 status from WaitDevice is 0 Request Completed Succesfully 
         status from WaitDevice is 0 Request Completed Succesfully 
	 status from WaitDevice is 0 Request Completed Succesfully 
	 status from WaitDevice is 0 Request Completed Succesfully 
	 buffer read back is a 
	 Write and Read and Seek to the Disk Works Perfectly 
	 TEST PASSED
*/
	
int P2_Startup(void *arg) {
	
	int status;
	int rc;

	// To Seek to a particular track in the  disk	
	USLOSS_DeviceRequest *req = (USLOSS_DeviceRequest *)malloc(sizeof(USLOSS_DeviceRequest)); //Needed to pass to USLOSS_DeviceOutput
	req->opr = USLOSS_DISK_SEEK; // Fill the request indicating the operation to be done on the disk
	req->reg1 =(void *) 2; // Track to be moved. This assumes the disk has atleast 3 tracks.
	status = USLOSS_DeviceOutput(USLOSS_DISK_DEV, 1, req); // This assumes Disk1 has been created as indicated in the comment section above.
	assert(status == USLOSS_ERR_OK);	
	//	Move to Disk, unit 1, Track 2, Request obj..that contains all the information needed to be done on the Disk.
	rc = P1_WaitDevice(USLOSS_DISK_DEV, 1, &status); // Wait for the interrupt to occur
	TEST(rc, P1_SUCCESS);
	USLOSS_Console("status from WaitDevice is %d ", status);// Retrieve the status
	TEST(status, USLOSS_DEV_READY); // For Disk device, this implies the request completed
	USLOSS_Console("Request Completed Succesfully \n");
	
	req = NULL;			
	// To Write to the disk
	req = (USLOSS_DeviceRequest *)malloc(sizeof(USLOSS_DeviceRequest)); //Needed to pass to USLOSS_DeviceOutput
	req->opr = USLOSS_DISK_WRITE; // Fill the request indicating the operation to be done on the disk
	req->reg1 = (void*)1; // Sector to be written 
	char *buf = (char *)malloc(512); // Buffer that holds the characters to be written
	buf[0] = 'a';
	buf[1] = '\0';
	req->reg2 = buf; //Set the buffer in the request
	status = USLOSS_DeviceOutput(USLOSS_DISK_DEV, 1, req); // Write to Disk, unit 1, Request obj..that contains all the information needed to be done on the Disk.
	assert(status == USLOSS_ERR_OK);

	rc = P1_WaitDevice(USLOSS_DISK_DEV, 1, &status); // Wait for the interrupt to occur
	TEST(rc, P1_SUCCESS);
	USLOSS_Console("status from WaitDevice is %d ", status);// Retrieve the status
	TEST(status, USLOSS_DEV_READY); // For Disk device, this implies the request completed
	USLOSS_Console("Request Completed Succesfully \n");
		
	req = NULL;
	buf = NULL;
		
	// To Seek to a particular track in the  disk
	req = (USLOSS_DeviceRequest *)malloc(sizeof(USLOSS_DeviceRequest)); //Needed to pass to USLOSS_DeviceOutput
	req->opr = USLOSS_DISK_SEEK; // Fill the request indicating the operation to be done on the disk
	req->reg1 = (void *)2; // Track to be moved
	status = USLOSS_DeviceOutput(USLOSS_DISK_DEV, 1, req); // Move to Disk, unit 1, Track 2, Request obj..that contains all the information needed to be done on the Disk.
	assert(status == USLOSS_ERR_OK);
	rc = P1_WaitDevice(USLOSS_DISK_DEV, 1, &status); // Wait for the interrupt to occur
	TEST(rc, P1_SUCCESS);
	USLOSS_Console("status from WaitDevice is %d ", status);// Retrieve the status
	TEST(status, USLOSS_DEV_READY); // For Disk device, this implies the request completed
	USLOSS_Console("Request Completed Succesfully \n"); 
		
	req = NULL;
	
	
	//To check and read back the return value
	req = (USLOSS_DeviceRequest *)malloc(sizeof(USLOSS_DeviceRequest)); //Needed to pass to USLOSS_DeviceOutput
	req->opr = USLOSS_DISK_READ; // Fill the request indicating the operation to be done on the disk
	req->reg1 = (void*)1; // Sector to be Read 
	buf = (char *)malloc(512); // Buffer that holds the characters to be read
	req->reg2 = buf;
	status = USLOSS_DeviceOutput(USLOSS_DISK_DEV, 1, req); // Read from Disk, unit 1, Request obj..that contains all the information needed to be done on the Disk.
	assert(status == USLOSS_ERR_OK);
	rc = P1_WaitDevice(USLOSS_DISK_DEV, 1, &status); // Wait for the interrupt to occur
	USLOSS_Console("status from WaitDevice is %d ", status);// Retrieve the status
	TEST(status, USLOSS_DEV_READY); // For Disk device, this implies the request completed
	USLOSS_Console("Request Completed Succesfully \n");
	USLOSS_Console("buffer read back is %s \n", buf);// Retrieve the status
	if(strcmp(buf,"a") == 0) {
		USLOSS_Console("Write and Read and Seek to the Disk Works Perfectly \n");
	}
	else {
		USLOSS_Console("Problems with Disk Operations \n");
		USLOSS_Console("TEST FAILED.\n");
		USLOSS_Halt(0);
	}
	
	PASSED();
	return 0;	
	
}

void test_setup(int argc, char **argv) {
        // Do nothing.
}

void test_cleanup(int argc, char **argv) {
        // Do nothing.
}
