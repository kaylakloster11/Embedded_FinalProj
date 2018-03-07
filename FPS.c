/*
 * FPS.c
 *
 *  Created on: Dec 10, 2017
 *      Author: admin
 */

#include "FPS.h"
#include "UART.h"
#include "timers.h"

volatile extern FP_cmd_packet cp;
volatile extern uint8_t FP_state;
volatile extern uint8_t data_databuf[30];
volatile extern uint8_t rsp_databuf[12];
volatile extern FP_resp rp;

extern volatile uint8_t FP_state;
extern volatile uint8_t state3;
extern volatile uint8_t freqDiv;
extern volatile uint8_t countTime;
extern volatile uint8_t count_time_fall;
extern volatile uint8_t timer_exit;
extern volatile uint8_t FPS_byte_count;


void FP_send_cmd(uint8_t * data, uint32_t length){
	int i;
	for(i = 0; i<length; i++){
	   UART_send_byteA2(*data);
	   //UART_send_byteA3(*location);
		data++;
	}
}

//Initializes the device and gets ready for commands
void Open()
	{
	   cp.start_cmd_1 = FP_CMD1_CONSTANT;
	   cp.start_cmd_2 = FP_CMD2_CONSTANT;
	   cp.devid = FP_DEVICEID_CONSTANT;
	   cp.cmd = FP_CMD_OPEN;
	   cp.param[0] = 0;
	   cp.param[1] = 0;
	   cp.param[2] = 0;
	   cp.param[3] = 0;
	   FP_Calculate_checksum_cmd(&cp);
	   FP_state = FP_WAIT_RSP;
	   FP_send_cmd(&cp, 12);
	   WaitResponse();
	   return;
	}

//Initializes the device and gets ready for commands
void Close()
   {
      cp.start_cmd_1 = FP_CMD1_CONSTANT;
      cp.start_cmd_2 = FP_CMD2_CONSTANT;
      cp.devid = FP_DEVICEID_CONSTANT;
      cp.cmd = FP_CMD_OPEN;
      cp.param[0] = 0;
      cp.param[1] = 0;
      cp.param[2] = 0;
      cp.param[3] = 0;
      FP_Calculate_checksum_cmd(&cp);
      FP_state = FP_WAIT_RSP; // Set some state here for receiving data
     }


   void FP_Calculate_checksum_cmd(FP_cmd_packet * cmd){
       int i = 0;
       uint16_t checksumFULL = 0;
       uint8_t * bptr = cmd;

       for (i=0;i<10;i++) {
           checksumFULL += *bptr++;
        }
       cp.checksum = checksumFULL;
   }


   // Turns on or off the LED backlight
      // Parameter: true turns on the backlight, false turns it off
      // Returns: True if successful, false if not
      void SetLED(int on)
      {
          cp.cmd = FP_CMD_CMOSLED;
          if (on)
              {
                cp.param[0] = 0x01;
              }
          else
              {
                 cp.param[0] = 0x00;
              }
          cp.param[1] = 0x00;
          cp.param[2] = 0x00;
          cp.param[3] = 0x00;
          FP_Calculate_checksum_cmd(&cp);
          FP_state = FP_WAIT_RSP;
          FP_send_cmd(&cp, 12);
          WaitResponse();
          return;
      };

      void CaptureFinger(int highquality){
          cp.cmd = FP_CMD_CAPTURE;
          if(highquality){
              cp.param[0] = 0x01; //best image but slow
          }
          else{
              cp.param[0] = 0x00; //not best image, but fast
          }
          cp.param[1] = 0x00;
          cp.param[2] = 0x00;
          cp.param[3] = 0x00;
          FP_Calculate_checksum_cmd(&cp);
          FP_state = FP_WAIT_RSP;
          FP_send_cmd(&cp, 12);
          WaitResponse();
          return;
      }

  int IsFingerPressed(){
      int retval = 0;
      cp.cmd = FP_CMD_ISPRESSED;
      cp.param[0] = 0x00;
      cp.param[1] = 0x00;
      cp.param[2] = 0x00;
      cp.param[3] = 0x00;
      FP_Calculate_checksum_cmd(&cp);
      FP_state = FP_WAIT_RSP;
      FP_send_cmd(&cp, 12);
      WaitResponse();
      int pval;
      pval = rp.param[0];
      pval += rp.param[1];
      pval += rp.param[2];
      pval += rp.param[3];
      if (pval == 0){
          retval = 1; //Finger is pressed
      }
      return retval;
  }

  void EnrollStart(int id){
      cp.cmd = FP_CMD_ENROLL_START;
      cp.param[0] = (id & 0x000000ff);
      cp.param[1] = (id & 0x0000ff00) >> 8;
      cp.param[2] = (id & 0x00ff0000) >> 16;
      cp.param[3] = (id & 0xff000000) >> 24;
      FP_Calculate_checksum_cmd(&cp);
      FP_state = FP_WAIT_RSP;
      FP_send_cmd(&cp, 12);
      WaitResponse();
  }

  void Enroll1(){
      cp.cmd = FP_CMD_ENROLL1;
      cp.param[0] = 0x00;
      cp.param[1] = 0x00;
      cp.param[2] = 0x00;
      cp.param[3] = 0x00;
      FP_Calculate_checksum_cmd(&cp);
      FP_state = FP_WAIT_RSP;
      FP_send_cmd(&cp, 12);
      WaitResponse();
  }

  void Enroll2(){
        cp.cmd = FP_CMD_ENROLL2;
        cp.param[0] = 0x00;
        cp.param[1] = 0x00;
        cp.param[2] = 0x00;
        cp.param[3] = 0x00;
        FP_Calculate_checksum_cmd(&cp);
        FP_state = FP_WAIT_RSP;
        FP_send_cmd(&cp, 12);
        WaitResponse();
    }

  void Enroll3(){
        cp.cmd = FP_CMD_ENROLL3;
        cp.param[0] = 0x00;
        cp.param[1] = 0x00;
        cp.param[2] = 0x00;
        cp.param[3] = 0x00;
        FP_Calculate_checksum_cmd(&cp);
        FP_state = FP_WAIT_RSP;
        FP_send_cmd(&cp, 12);
        WaitResponse();
    }

  void ProcessRSP(){
      rp.start_cmd_1 = rsp_databuf[0];
      rp.start_cmd_2 = rsp_databuf[1];
      rp.devid = MergeBytes(rsp_databuf[3], rsp_databuf[2]);
      rp.checksum = MergeBytes(rsp_databuf[11], rsp_databuf[10]);
      rp.param[0] = rsp_databuf[4];
      rp.param[1] = rsp_databuf[5];
      rp.param[2] = rsp_databuf[6];
      rp.param[3] = rsp_databuf[7];
      if(rsp_databuf[8] == 0x30){
          rp.ack = 1;
      }
      else{
          rp.ack = 0;
      }
      rp.error = GetErrorCode(rsp_databuf[5], rsp_databuf[4]);
  }

  RSP_ERROR_CODE GetErrorCode(uint8_t high, uint8_t low){
        RSP_ERROR_CODE e;
        if(high != 0x00){
            e = INVALID;
        }
        else{
            switch(low)
              {
                  case 0x00: e = NO_ERROR; break;
                  case 0x01: e = NACK_TIMEOUT; break;
                  case 0x02: e = NACK_INVALID_BAUDRATE; break;
                  case 0x03: e = NACK_INVALID_POS; break;
                  case 0x04: e = NACK_IS_NOT_USED; break;
                  case 0x05: e = NACK_IS_ALREADY_USED; break;
                  case 0x06: e = NACK_COMM_ERR; break;
                  case 0x07: e = NACK_VERIFY_FAILED; break;
                  case 0x08: e = NACK_IDENTIFY_FAILED; break;
                  case 0x09: e = NACK_DB_IS_FULL; break;
                  case 0x0A: e = NACK_DB_IS_EMPTY; break;
                  case 0x0B: e = NACK_TURN_ERR; break;
                  case 0x0C: e = NACK_BAD_FINGER; break;
                  case 0x0D: e = NACK_ENROLL_FAILED; break;
                  case 0x0E: e = NACK_IS_NOT_SUPPORTED; break;
                  case 0x0F: e = NACK_DEV_ERR; break;
                  case 0x10: e = NACK_CAPTURE_CANCELED; break;
                  case 0x11: e = NACK_INVALID_PARAM; break;
                  case 0x12: e = NACK_FINGER_IS_NOT_PRESSED; break;
              }
          }
          return e;
      }


  uint16_t MergeBytes(uint8_t high, uint8_t low){
      uint16_t makeOne = high;
      makeOne = makeOne << 8;
      makeOne += low;
      return makeOne;
  }


  //True if the ID number is enrolled, false if not
 int CheckEnrolled(int enrolled){
     cp.cmd = FP_CMD_CHECK_ENROLL;
     IDtoPARAM(enrolled);
     FP_Calculate_checksum_cmd(&cp);
     FP_state = FP_WAIT_RSP;
     FP_send_cmd(&cp, 12);
     WaitResponse();
     return rp.ack;
 }

 void IDtoPARAM(int i)
 {
     cp.param[0] = (i & 0x000000ff);
     cp.param[1] = (i & 0x0000ff00) >> 8;
     cp.param[2] = (i & 0x00ff0000) >> 16;
     cp.param[3] = (i & 0xff000000) >> 24;
 }

 int ID_print(){
      //  0-19: Verified against the specified ID (found, and here is the ID number)
      //  20: Failed
          cp.cmd = FP_CMD_INDENTIFY;
          cp.param[0] = 0x00;
          cp.param[1] = 0x00;
          cp.param[2] = 0x00;
          cp.param[3] = 0x00;
          FP_Calculate_checksum_cmd(&cp);
          FP_state = FP_WAIT_RSP;
          FP_send_cmd(&cp, 12);
          WaitResponse();
          int print_ID = PARAMtoID();
          if(print_ID > 20){
              print_ID = 20;
          }
          return print_ID;
      }


  int PARAMtoID()
  {
      int ID = 0;
      ID = rp.param[3];
      ID = (ID << 8) + rp.param[2];
      ID = (ID << 8) + rp.param[1];
      ID = (ID << 8) + rp.param[0];
      return ID;
  }

  int Available_ID(){
      int enrollid = 0;
      int used = 1;
          while (used == 1)
          {
              used = CheckEnrolled(enrollid);
              if (used==1) enrollid++;
          }
        return enrollid;
  }

  // Deletes the specified ID (enrollment) from the database
  // Parameter: 0-19 (id number to be deleted)
  // Returns: true if successful, false if position invalid
  int DeleteID(int id)
  {
      cp.cmd = FP_CMD_DELETE_ID;
      IDtoPARAM(id);
      FP_Calculate_checksum_cmd(&cp);
      FP_state = FP_WAIT_RSP;
      FP_send_cmd(&cp, 12);
      WaitResponse();
      return rp.ack;
   }

  // Deletes all IDs (enrollments) from the database
  // Returns: true if successful, false if db is empty
  int DeleteAll()
  {
      cp.cmd = FP_CMD_DELETE_ALL;
      cp.param[0] = 0x00;
      cp.param[1] = 0x00;
      cp.param[2] = 0x00;
      cp.param[3] = 0x00;
      FP_Calculate_checksum_cmd(&cp);
      FP_state = FP_WAIT_RSP;
      FP_send_cmd(&cp, 12);
      WaitResponse();
      return rp.ack;
  }

 void ENROLLPRINT(){
     UART_send_A3("ENROLLING A PRINT\n", 18);
     freqDiv = 5;
     int8_t enroll1[] ="Press finger to Enroll\n";
     uint32_t len1 = strlen(enroll1);
     uint8_t remove[] = "Remove finger\n";
     uint32_t remLen = strlen(remove);
     uint8_t enroll2[] = "Press same finger again\n";
     uint32_t len2 = strlen(enroll2);
     uint8_t enroll3[] = "Press same finger yet again\n";
     uint32_t len3 = strlen(enroll3);
     uint8_t success[] = "Enrolling Successful\n";
     uint32_t finLen = strlen(success);
     uint8_t error1[] = "Failed to capture first finger\n";
     uint32_t errlen1 = strlen(error1);
     uint8_t error2[] = "Failed to capture second finger\n";
     uint32_t errlen2 = strlen(error2);
     uint8_t error3[] = "Failed to capture third finger\n";
     uint32_t errlen3 = strlen(error3);

     int enrollid = 0;
     enrollid = Available_ID();

        EnrollStart(enrollid);
        UART_send_A3(enroll1, len1);
        while(IsFingerPressed() == 0);

        CaptureFinger(1);
                if (rp.ack == 1)
                {
                    UART_send_A3(remove, remLen);
                    Enroll1();
                    while(IsFingerPressed() == 1);

                    UART_send_A3(enroll2, len2);
                    while(IsFingerPressed() == 0);

                    CaptureFinger(1);
                    if (rp.ack ==1)
                    {
                        UART_send_A3(remove, remLen);
                        Enroll2();
                        while(IsFingerPressed() == 1);
                        UART_send_A3(enroll3, len3);
                        while(IsFingerPressed() == 0);

                       CaptureFinger(1);
                        if (rp.ack == 1)
                        {
                           UART_send_A3(remove, remLen);
                           Enroll3();
                           if(rp.ack ==1){
                               UART_send_A3(success, finLen);
                           }
                           else{
                               UART_send_A3("Enrolling Failed\n", 17);
                           }
                         }
                        else {
                            UART_send_A3(error3, errlen3);
                        }
                    }
                    else {
                        UART_send_A3(error2, errlen2);
                    }
                }
           else{
               UART_send_A3(error1, errlen1);
           }
     }


  int IDENTIFY_PRINT(){
          freqDiv = 50;
  // Identify fingerprint test

           while(IsFingerPressed() == 0){
               TIMER_A2->CCR[0] = 50000;
               TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE;   // Capture/Compare interrupt enabled @ peripheral  (CCIE flag enabled)
            }
           TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;

           CaptureFinger(1);
           int id = ID_print();
           char strID[3] = 0;
           itoa(id, strID);
           uint32_t lenID = strlen((char *)strID);

           if (id <20)
           {//if the fingerprint matches, provide the matching template ID
               UART_send_A3("Verified ID: ", 13);
               UART_send_A3(&strID,lenID);
               UART_send_byteA3(0x0A);
           }
           else
           {//if unable to recognize
               UART_send_A3("Finger not found\n", 17);
           }
           return id;
      }

  void FPS_Delete_ID(){
      UART_send_A3("DELETE ONE PRINT\n", 17);
      int i = 0;
      int no_ID = 1;
      int ID = 0;
      while(no_ID){
              ID = IDENTIFY_PRINT();
                 if(ID < 20){
                     no_ID = 0;
                 }
                 else{
                     no_ID = 1;
                     i++;
                 }
             if(i > 6){
                 no_ID = 0;
             }
           }

          if(DeleteID(ID)){
              UART_send_A3("Fingerprint Deleted\n", 20);
          }
          else{
              UART_send_A3("Finger to Delete Not Found\n", 27);
          }
  }


  void FPS_Delete_ALL(){
      UART_send_A3("DELETING ALL PRINTS\n", 20);
     if(DeleteAll()){
         UART_send_A3("All Fingers Deleted\n", 20);
     }
    else{
        UART_send_A3("No Prints Found\n", 16);
     }
  }

  void FPS_PRINT_ENROLLED(){
      int checkNum = 0;
      int ID = 0;
      int num_Enroll = 0;
      num_Enroll = Available_ID();
      char strNum[3] = 0;
      itoa(num_Enroll, strNum);
      uint32_t numLen = strlen((char *)strNum);
      UART_send_A3("Number of Stored Prints: ", 25);
      UART_send_A3(&strNum, numLen);
  }

  void WaitResponse(){
      FPS_byte_count = 0;
           while(FP_state == FP_WAIT_RSP);
             FPS_byte_count = 0;
           if(FP_state == FP_PROCESS_RSP){
               ProcessRSP();
           }
            FPS_byte_count = 0;
       }

