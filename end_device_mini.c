

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "serial.h"

#define ZIGBEE_START_DELIMITER    (0x7E)
#define ZIGBEE_HEADER_SIZE          (3)
#define ZIGBEE_UNKNOWN_16B_ADDR    (0xFFFE)

typedef enum
{
  ZIGBEE_API_AT_CMD           = 0x08,
  ZIGBEE_API_TRANSMIT_REQUEST = 0x10,
  ZIGBEE_AT_COMMAND_RESPONSE  = 0x88,
  ZIGBEE_MODEM_STATUS         = 0x8A,
  ZIGBEE_TRANSMIT_STATUS      = 0x8B,
  ZIGBEE_RECEIVE_PACKET       = 0x90,
} zigbee_frameType;

const uint8_t frameAuthentificationInd[] = 
{
  ZIGBEE_START_DELIMITER,
  0, //size
  4, //size without the checksum
  ZIGBEE_API_AT_CMD,
  1, // frameID
  'A',  //AT 1
  'I', // AT 2
  //no data
  0x6c, //checksum
};

//AI association indication

#define SENSOR_PROTOCOL_DATA_TYPE     (0x00)
#define SENSOR_PROTOCOL_DBG_TYPE      (0x01)
#define SENSOR_HYT221_TEMP            (0x01)
#define SENSOR_HYT221_HUM             (0x02)
#define SENSOR_VOLTAGE                (0x03)


#define OFFSET_SIZE         (1)
#define OFFSET_FRAMEID      (4)
#define OFFSET_TEMPERATURE (20)
#define OFFSET_HUMIDITY    (23)
#define OFFSET_VOLTAGE     (26)


const uint8_t frameData[] = 
{
  ZIGBEE_START_DELIMITER,
  0, //size
  4, //size without the checksum
  ZIGBEE_API_TRANSMIT_REQUEST,
  2, //frameID
  0x00, //coordinator @1 
  0x00, //coordinator @2 
  0x00, //coordinator @3
  0x00, //coordinator @4
  0x00, //coordinator @5
  0x00, //coordinator @6
  0x00, //coordinator @7
  0x00, //coordinator @8
  0xFF, //coord 16bits ZIGBEE_UNKNOWN_16B_ADDR 
  0xFE, //coord 16bits ZIGBEE_UNKNOWN_16B_ADDR
  0x00, //broadcast radius
  0x00, //option
  //payload
  SENSOR_PROTOCOL_DATA_TYPE,
  0x03,
  SENSOR_HYT221_TEMP,
  0x00,
  0x00,
  SENSOR_HYT221_HUM,
  0x00,
  0x00,
  SENSOR_VOLTAGE,
  0x00,
  0x00  
  //and checksum
};

typedef struct
{
  uint8_t type;
  uint8_t status;
  uint8_t AT[2];
  uint8_t* data;
  uint8_t size;
  uint8_t frameID;
} zigbee_decodedFrame;


enum
{
  ZB_STATUS_IDLE,
  ZB_STATUS_JOINED,
  ZB_STATUS_IN_ERROR
};

static void display_frame(char* displayTextHeader, uint8_t* frame, uint32_t size);
static bool zb_decodage(uint8_t* frame, uint8_t frameSize, zigbee_decodedFrame* decodedFrame);


static uint8_t zigbee_doChecksum(uint8_t* frame, uint8_t size)
{
  uint8_t checksum;

  checksum = 0xFF;
  for (uint32_t i = 0; i < size; i++)
  {
    checksum -= frame[i];
  }

  return checksum;
}


static void zigbee_appendChecksum(uint8_t* buffer, uint8_t* sizeFrame)
{
  buffer[*sizeFrame] = zigbee_doChecksum(&buffer[ZIGBEE_HEADER_SIZE], &buffer[*sizeFrame] - &buffer[ZIGBEE_HEADER_SIZE]);
  (*sizeFrame)++;
}

uint8_t zb_frameToSend[50];
uint8_t zb_frameReceived[50];
uint8_t zb_nbBytes;

int main(int argc, char* argv[])
{
  int32_t fd;
  uint8_t zb_status = ZB_STATUS_IDLE;
  
  if (argc != 2)
  {
    fprintf(stderr, "Usage : %s tty_device\n", argv[0]);
  }
 
  fd = serial_setup(argv[1], 9600, SERIAL_PARITY_OFF, SERIAL_RTSCTS_OFF, SERIAL_STOPNB_1);
  if (fd < 0)
  {
    fprintf(stderr, "not possible to configurate serial line\n");
    return EXIT_FAILURE;
  }

  bool bSuccess = true;
  uint16_t sizeOfNextData;
  uint8_t frameID = 2;
  while (1)
  {
    switch (zb_status)
    {
      case ZB_STATUS_IDLE:
        serial_write(fd, (uint8_t*) frameAuthentificationInd, sizeof(frameAuthentificationInd));
        break;
        
      case ZB_STATUS_JOINED:
        memcpy(zb_frameToSend, frameData, sizeof(frameData));
        zb_frameToSend[OFFSET_FRAMEID] = frameID++;
        zb_frameToSend[OFFSET_TEMPERATURE]   = 0x55;
        zb_frameToSend[OFFSET_TEMPERATURE+1] = 0xAA;    
        zb_frameToSend[OFFSET_HUMIDITY]   = 0xAA;
        zb_frameToSend[OFFSET_HUMIDITY+1] = 0x55;
        zb_frameToSend[OFFSET_VOLTAGE]   = 0xFF;
        zb_frameToSend[OFFSET_VOLTAGE+1] = 0xFF;
        zb_frameToSend[OFFSET_SIZE] = ((sizeof(frameData)-ZIGBEE_HEADER_SIZE) & 0xFF00)>>8;
        zb_frameToSend[OFFSET_SIZE+1] = ((sizeof(frameData)-ZIGBEE_HEADER_SIZE) & 0x00FF);
        
        uint8_t frameSize = sizeof(frameData);
        zigbee_appendChecksum(zb_frameToSend, &frameSize);
        serial_write(fd, (uint8_t*) zb_frameToSend, frameSize);
        display_frame("send Frame", zb_frameToSend, frameSize); 
        break;
        
      case ZB_STATUS_IN_ERROR:
        break;
        
      default:
        break;
    }
    
   // 
    bSuccess = serial_read(fd, zb_frameReceived, ZIGBEE_HEADER_SIZE);
    if (bSuccess)
    {
      if (zb_frameReceived[0] == ZIGBEE_START_DELIMITER)
      {
        sizeOfNextData = (((uint16_t) zb_frameReceived[1]) << 8) | (zb_frameReceived[2]);
        bSuccess = serial_read(fd, zb_frameReceived+ZIGBEE_HEADER_SIZE, sizeOfNextData+1); //+1 for the checksum 
        if (bSuccess)
        {
          display_frame("received Frame", zb_frameReceived, sizeOfNextData + 1 + ZIGBEE_HEADER_SIZE); 
          zigbee_decodedFrame decodedFrame;
          bool bOK;
          bOK = zb_decodage(zb_frameReceived+ZIGBEE_HEADER_SIZE, sizeOfNextData + 1, &decodedFrame);
          if (bOK)
          {
            switch (decodedFrame.type)
            {
              case ZIGBEE_MODEM_STATUS:
                if (decodedFrame.status == 0x02)
                  zb_status = ZB_STATUS_JOINED;
                else 
                  zb_status = ZB_STATUS_IDLE;
                break;
                
              case ZIGBEE_AT_COMMAND_RESPONSE:
                if ((decodedFrame.AT[0] == 'A') && (decodedFrame.AT[1] == 'I'))
                {
                  if (decodedFrame.status == 0)
                  {
                    fprintf(stdout, "size = %d, data = %p\n", decodedFrame.size, decodedFrame.data);
                    if (decodedFrame.data[0] == 0)
                    {                      
                      zb_status = ZB_STATUS_JOINED;
                      fprintf(stdout, "AI respond OK\n");
                    }
                  }
                }
                break;
                
              case ZIGBEE_TRANSMIT_STATUS:
                if (decodedFrame.status == 0)
                {
                  fprintf(stdout, "transmit OK\n");
                }
                
                break;
                
              default:
                break;
            }
          }
        }
      }
    }
    
    struct timespec waitTime;
    waitTime.tv_sec = 1;
    waitTime.tv_nsec = 0;//500000000;
    nanosleep(&waitTime, NULL);
    
  }

  
  return EXIT_SUCCESS;
}


static bool zb_decodage(uint8_t* frame, uint8_t frameSize, zigbee_decodedFrame* decodedFrame)
{
  bool bCorrectlyDecoded;
  uint8_t checksum;
  bCorrectlyDecoded = false;
  
  if (frameSize >= 1)
  {
    checksum = zigbee_doChecksum(frame, frameSize-1);
    if (checksum != frame[frameSize-1])
    {
      //fprintf(stdout, "Checksum KO\n");
      bCorrectlyDecoded = false;
    }
    else
    {
      bCorrectlyDecoded = true;
    }
  }

  if (bCorrectlyDecoded)
  {
    decodedFrame->type = frame[0];
    switch (frame[0])
    {
      case ZIGBEE_AT_COMMAND_RESPONSE:
        decodedFrame->frameID = frame[1];
        decodedFrame->AT[0] = frame[2];
        decodedFrame->AT[1] = frame[3];
        decodedFrame->status = frame[4];
        decodedFrame->size = frameSize - 6;
        if (decodedFrame->size == 0)
          decodedFrame->data = NULL;    
        else
          decodedFrame->data = &frame[5];
        break;
        
      case ZIGBEE_MODEM_STATUS:
        decodedFrame->status = frame[1];
        bCorrectlyDecoded = true;
        break;
        
      case ZIGBEE_TRANSMIT_STATUS:
        decodedFrame->status /* deliveryStatus */ = frame[5];
        break;
        
      default:
        bCorrectlyDecoded = false;
        break;
    }
  }
  
  return bCorrectlyDecoded;
}

static void display_frame(char* displayTextHeader, uint8_t* frame, uint32_t size)
{
  static uint8_t mark = 0;
  bool bAtCommandResponse = false;
  fprintf(stdout, "(%s)-%d frame : \n", displayTextHeader, mark++);
  for (uint32_t i = 0; i < size; i++)
  {
    if ((i == 3) && ((frame[i] == ZIGBEE_AT_COMMAND_RESPONSE) || (frame[i] == ZIGBEE_API_AT_CMD)))
      bAtCommandResponse = true;
    
    if ((bAtCommandResponse == true) && ((i == 5) || (i == 6)))
      fprintf(stdout, "'%c' ", frame[i]);
    else
      fprintf(stdout, "%.2x ", frame[i]);
  }
  fprintf(stdout, "\n");
}
