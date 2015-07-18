
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "serial.h"
#include "zigbee.h"
#include "unused.h"

void display_frame(uint8_t* frame, uint32_t size);
void main_first_test(int argc, char* argv[]);

int main(int argc, char** argv)
{
  fprintf(stdout, "Essai lib zigbee\n");
  uint8_t buffer[1024];
  uint32_t frameSize;
  
  UNUSED(argc);
  UNUSED(argv);

  //zigbee_panID panID = { 0, 0, 0, 0 , 0, 0 , 0 , 0};
  frameSize = zigbee_encode_SetPanID(buffer, 1024, 1, (zigbee_panID*) &zigbee_randomPanID);
  display_frame(buffer, frameSize);
  
  frameSize = zigbee_encode_setScanChannelBitmask(buffer, 1024, 1, ZIGGEE_DEFAULT_BITMASK);
  display_frame(buffer, frameSize);

  frameSize = zigbee_encode_setScanDurationExponent(buffer, 1024, 1, ZIGBEE_DEFAULT_SCAN_DURATION_EXPONENT);
  display_frame(buffer, frameSize);
  
  frameSize = zigbee_encode_setStackProfile(buffer, 1024, 1, ZIGBEE_DEFAULT_STACK_PROFILE);
  display_frame(buffer, frameSize);
  
  frameSize = zigbee_encode_setEncryptionEnabled(buffer, 1024, 1, false);
  display_frame(buffer, frameSize);

  frameSize = zigbee_encode_setNetworkEncryptionKey(buffer, 1024, 1, (zigbee_encryptionKey*) &zigbee_randomEncryptionKey);
  display_frame(buffer, frameSize);

  frameSize = zigbee_encode_setLinkKey(buffer, 1024, 1, (zigbee_linkKey*) &zigbee_noLinkKey);
  display_frame(buffer, frameSize);
  
  frameSize = zigbee_encode_setEncryptionOptions(buffer, 1024, 1, ZIGBEE_DEFAULT_ENCRYPTION_OPTION);
  display_frame(buffer, frameSize);

  frameSize = zigbee_encode_SetJoinTime(buffer, 1024, 1, ZIGBEE_JOINING_ALWAYS_ACTIVATED);
  display_frame(buffer, frameSize);

  frameSize = zigbee_encode_getAssociationIndication(buffer, 1024, 1);
  display_frame(buffer, frameSize);
  
  frameSize = zigbee_encode_getHardwareVersion(buffer, 1024, 1);
  display_frame(buffer, frameSize);
  
  frameSize = zigbee_encode_getFirmwareVersion(buffer, 1024, 1);
  display_frame(buffer, frameSize);
  
  
  uint8_t test_frame[9] = { 0x7E, 0x00, 0x05, 0x88, 0x01, 'B', 'D', 0x00, 0xF0 };
  zigbee_decodedFrame decodedData;
  bool bCorrectlyDecoded;
  uint16_t sizeOfExpectedData;
  bCorrectlyDecoded = zigbee_decodeHeader(test_frame, 3, &sizeOfExpectedData);
  fprintf(stdout, "bCorrectlyDecoded = %d\n", bCorrectlyDecoded);
  fprintf(stdout, "sizeOfExpectedData = %d\n", sizeOfExpectedData);
  
  bCorrectlyDecoded = zigbee_decodeFrame(&test_frame[3], sizeOfExpectedData + 1, &decodedData); // 1 for chechsum
  fprintf(stdout, "bCorrectlyDecoded = %d\n", bCorrectlyDecoded);
  fprintf(stdout, "decodedData.type = %.2x\n", decodedData.type);
  fprintf(stdout, "decodedData.frameID = %d\n", decodedData.atCmd.frameID);
  fprintf(stdout, "decodedData.atCmd.ATcmd = %c %c\n", decodedData.atCmd.ATcmd[0], decodedData.atCmd.ATcmd[1]);
  fprintf(stdout, "decodedData.atCmd.status = %d\n", decodedData.atCmd.status);  
  fprintf(stdout, "decodedData.atCmd.data = %p\n", decodedData.atCmd.data);
  fprintf(stdout, "decodedData.atCmd.size = %d\n", decodedData.atCmd.size);

  return EXIT_SUCCESS;
}

void main_first_test(int argc, char* argv[])
{
  uint8_t buffer[1024];
  UNUSED(argc);
  UNUSED(argv);  
  
  int32_t fd, nbRead, nbWrite;
  fd = serial_setup(argv[1], 9600, SERIAL_PARITY_OFF, SERIAL_RTSCTS_OFF, SERIAL_STOPNB_1);
  if (fd >= 0)
  {
    fprintf(stdout, "serial correctly configurated fd = %d\n", fd);
    nbWrite = serial_write(fd, (uint8_t*) "+++", 3);
    fprintf(stdout, "nbWrite = %d\n", nbWrite);

    fd_set rfs;
    struct timeval waitTime;
    waitTime.tv_sec = 2;
    waitTime.tv_usec = 0;
    FD_ZERO(&rfs);
    FD_SET(fd, &rfs);
    //retry:
    if ((select(fd + 1, &rfs, NULL, NULL, &waitTime) > 0))
    {
      if (FD_ISSET(fd, &rfs))
      {
        nbRead = read(fd, buffer, 1024);
        while (nbRead > 0)
        {
          fprintf(stdout, "nbRead = %d\n", nbRead);
          for (int i = 0; i < nbRead; i++)
          {
            fprintf(stdout, "0x%x (%c) ", buffer[i], buffer[i]);
          }
          fprintf(stdout, "\n");
          nbRead = read(fd, buffer, 1024);
          //   goto retry;
        }
      }
    }
  }  
}

void display_frame(uint8_t* frame, uint32_t size)
{
  fprintf(stdout, "frame : \n");
  for (unsigned int i = 0; i < size; i++)
  {
    if ((i == 5) || (i == 6))
      fprintf(stdout, "%c ", frame[i]);
    else
      fprintf(stdout, "%.2x ", frame[i]);
  }
  fprintf(stdout, "\n");
}

