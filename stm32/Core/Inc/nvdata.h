#ifndef NVDATA_H_
#define NVDATA_H_

#include "stm32f0xx_hal.h"

#define MYADDR 1

#if MYADDR == 1
uint8_t const NV_SCHEMA     = 1;
uint8_t const NV_TXPOWER0   = 80; // 0 to 127
uint8_t const NV_CHANNEL0   = 10; // 1 to 69
uint8_t const NV_DATARATE0  = 8;  // DataRate256kbps128kHz
uint8_t const NV_TXPOWER1   = 80; // 0 to 127
uint8_t const NV_CHANNEL1   = 50; // 1 to 69
uint8_t const NV_DATARATE1  = 1;  // DataRate2kbps25kHz
uint8_t const CAL_SCHEMA    = 1;
uint8_t const CAL_MYADDR0   = 1;
uint8_t const CAL_OSCTUNE0  = 103;
uint8_t const CAL_MYADDR1   = 1;
uint8_t const CAL_OSCTUNE1  = 100;
#elif MYADDR == 2
uint8_t const NV_SCHEMA     = 1;
uint8_t const NV_TXPOWER0   = 80; // 0 to 127
uint8_t const NV_CHANNEL0   = 20; // 1 to 69
uint8_t const NV_DATARATE0  = 8;  // DataRate256kbps128kHz
uint8_t const NV_TXPOWER1   = 80; // 0 to 127
uint8_t const NV_CHANNEL1   = 60; // 1 to 69
uint8_t const NV_DATARATE1  = 2;  // DataRate1kbps25kHz
uint8_t const CAL_SCHEMA    = 1;
uint8_t const CAL_MYADDR0   = 2;
uint8_t const CAL_OSCTUNE0  = 101;
uint8_t const CAL_MYADDR1   = 2;
uint8_t const CAL_OSCTUNE1  = 103;
#endif

struct TCalData
{
  uint8_t Schema;
  uint8_t Checksum;
  uint8_t Spare[ 6 ];
  struct
  {
    uint8_t MyAddr;
    uint8_t OscTune;
    int8_t  TempTune;
    uint8_t Spare[ 5 ];
  } Cal433;
  struct
  {
    uint8_t MyAddr;
    uint8_t OscTune;
    int8_t  TempTune;
    uint8_t Spare[ 5 ];
  } Cal868;
};

struct TNvData
{
  uint8_t Schema;
  uint8_t Checksum;
  struct
  {
    uint8_t Channel;
    uint8_t TxPower;
    uint8_t DataRate;
  } Nv433;
  struct
  {
    uint8_t Channel;
    uint8_t TxPower;
    uint8_t DataRate;
  } Nv868;
};

void NvDataInit();
void CalDataInit();
void NvDataUpdate();
void CalDataUpdate();
uint8_t NvDataChecksum();
uint8_t CalDataChecksum();

extern TNvData NvData;
extern TCalData CalData;

#endif // NVDATA_H_
