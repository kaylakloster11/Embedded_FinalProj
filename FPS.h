/*
 * FPS.h
 *
 *  Created on: Dec 10, 2017
 */

#ifndef FPS_H_
#define FPS_H_


#include <stdint.h>
#include "msp.h"

#define bool int
#define true 1
#define false 0


/* CONSTANT DEFINITIONS*/
#define FP_CMD1_CONSTANT (0x55)
#define FP_CMD2_CONSTANT (0xAA)
#define FP_DEVICEID_CONSTANT (0x0001)
#define FP_RSP_ACK  (0x30)
#define FP_RSP_NACK (0x31)

 /* DEFINITIONS FOR PACKET LENGTHS */
#define FP_CMD_PACKET_LENGTH (12)
#define FP_RSP_PACKET_LENGTH (12)
#define FP_DATA_PACKET_LENGTH (30)


 /* DEFs for COMMANDS*/
#define FP_CMD_OPEN (0x0001)
#define FP_CMD_CLOSE (0x0002)
#define FP_CMD_CMOSLED (0x0012)
#define FP_CMD_CAPTURE (0x0060)
#define FP_CMD_ISPRESSED (0x0026)
#define FP_CMD_ENROLL_START (0x22)
#define FP_CMD_ENROLL1 (0x23)
#define FP_CMD_ENROLL2 (0x24)
#define FP_CMD_ENROLL3 (0x25)
#define FP_CMD_CHECK_ENROLL (0x21)
#define FP_CMD_INDENTIFY (0x51)
#define FP_CMD_DELETE_ID (0x40)
#define FP_CMD_DELETE_ALL (0x41)

/* STATES FOR STATE1 MACHINE*/
#define FP_SEND_CMD (0x00)
#define FP_WAIT_RSP (0x01)
#define FP_WAIT_DATA (0x02)
#define FP_TIMEOUT  (0x03)
#define FP_PROCESS_RSP  (0x04)
#define FP_DELETE   (0x05)
#define FP_DONT_CARE (0x06)
#define FP_DELETEALL (0x07)
#define FP_DELETE1 (0x08)
#define FP_ENROLL (0x09)


typedef struct {
	uint8_t start_cmd_1;                                // Command 2 bytes
	uint8_t start_cmd_2;
	uint16_t devid;
	uint8_t param[4];                              // Parameter 4 bytes, changes meaning depending on command
	uint16_t cmd;
	uint16_t checksum;
} __attribute__((packed)) FP_cmd_packet;

/*
    Response_Packet represents the returned data from the finger print scanner
   */
typedef enum{
	NO_ERROR       = 0x0000,   // Default value. no error
	NACK_TIMEOUT                = 0x1001,   // Obsolete, capture timeout
	NACK_INVALID_BAUDRATE       = 0x1002,   // Obsolete, Invalid serial baud rate
	NACK_INVALID_POS            = 0x1003,   // The specified ID is not between 0~199
	NACK_IS_NOT_USED            = 0x1004,   // The specified ID is not used
	NACK_IS_ALREADY_USED        = 0x1005,   // The specified ID is already used
	NACK_COMM_ERR               = 0x1006,   // Communication Error
	NACK_VERIFY_FAILED          = 0x1007,   // 1:1 Verification Failure
	NACK_IDENTIFY_FAILED        = 0x1008,   // 1:N Identification Failure
	NACK_DB_IS_FULL             = 0x1009,   // The database is full
	NACK_DB_IS_EMPTY            = 0x100A,   // The database is empty
	NACK_TURN_ERR               = 0x100B,   // Obsolete, Invalid order of the enrollment (The order was not as: EnrollStart -> Enroll1 -> Enroll2 -> Enroll3)
	NACK_BAD_FINGER             = 0x100C,   // Too bad fingerprint
	NACK_ENROLL_FAILED          = 0x100D,   // Enrollment Failure
	NACK_IS_NOT_SUPPORTED       = 0x100E,   // The specified command is not supported
	NACK_DEV_ERR                = 0x100F,   // Device Error, especially if Crypto-Chip is trouble
	NACK_CAPTURE_CANCELED       = 0x1010,   // Obsolete, The capturing is canceled
	NACK_INVALID_PARAM          = 0x1011,   // Invalid parameter
	NACK_FINGER_IS_NOT_PRESSED  = 0x1012,   // Finger is not pressed
	INVALID                     = 0XFFFF    // Used when parsing fails
}RSP_ERROR_CODE;

typedef struct{
   uint8_t start_cmd_1;
   uint8_t start_cmd_2;
   uint16_t devid;
   uint8_t data[16];
   uint16_t checksum;
  } FP_data;

 typedef struct{
	 uint8_t start_cmd_1;                                // Command 2 bytes
	 uint8_t start_cmd_2;
	 uint16_t devid;
	 uint8_t param[4];                              // Parameter 4 bytes, changes meaning depending on command
	 uint16_t checksum;
	 int ack;         // 0x30 -> ACK TRUE, 0x31 -> NACK (ACK FALSE)
	 RSP_ERROR_CODE error;
 }FP_resp;


//Initialises the device and gets ready for commands
void Open();

void SetLED(int on);

void FP_send_cmd(uint8_t * data, uint32_t length);

uint16_t MergeBytes(uint8_t high, uint8_t low);

void FP_Calculate_checksum_cmd(FP_cmd_packet * cmd);

void ProcessRSP();


void EnrollStart();
void Enroll1();
void Enroll2();
void Enroll3();

RSP_ERROR_CODE GetErrorCode(uint8_t high, uint8_t low);

int CheckEnrolled();
int IsFingerPressed();
void CaptureFinger(int highquality);

void ENROLLPRINT();
void IDtoPARAM(int i);
int ID_print();
int PARAMtoID();
int Available_ID();
int IDENTIFY_PRINT();
int DeleteAll();
int DeleteID(int id);

void FPS_Delete_ID();
void FPS_Delete_ALL();
void FPS_PRINT_ENROLLED();
void WaitResponse();

#endif /* FPS_H_ */
