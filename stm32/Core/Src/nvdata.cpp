#include "nvdata.h"


extern void UartPrintf( char const *const Format, ... );

TNvData NvData;
TCalData CalData;
uint32_t const BASE_NVDATA  = 0x0801F000;
uint32_t const BASE_CALDATA = 0x0801F800;

void NvDataInit()
{
  NvData = *(TNvData*)BASE_NVDATA;

  if( NvDataChecksum() || ( NvData.Schema != NV_SCHEMA ))
  {
    UartPrintf( "NvData checksum error\n" );

    NvData.Schema         = NV_SCHEMA;

    NvData.Nv433.TxPower  = NV_TXPOWER0;
    NvData.Nv433.Channel  = NV_CHANNEL0;
    NvData.Nv433.DataRate = NV_DATARATE0;

    NvData.Nv868.TxPower  = NV_TXPOWER1;
    NvData.Nv868.Channel  = NV_CHANNEL1;
    NvData.Nv868.DataRate = NV_DATARATE1;

    NvDataUpdate();
  }

  UartPrintf( "TxPower0  %u\n", NvData.Nv433.TxPower );
  UartPrintf( "Channel0  %u\n", NvData.Nv433.Channel );
  UartPrintf( "DataRate0 %u\n", NvData.Nv433.DataRate );
  UartPrintf( "TxPower1  %u\n", NvData.Nv868.TxPower );
  UartPrintf( "Channel1  %u\n", NvData.Nv868.Channel );
  UartPrintf( "DataRate1 %u\n", NvData.Nv868.DataRate );
}

void CalDataInit()
{
  CalData = *(TCalData*)BASE_CALDATA;

  if( CalDataChecksum() || ( CalData.Schema != CAL_SCHEMA ))
  {
    UartPrintf( "CalData checksum error\n" );

    CalData.Schema         = CAL_SCHEMA;

    CalData.Cal433.MyAddr  = CAL_MYADDR0;
    CalData.Cal433.OscTune = CAL_OSCTUNE0;

    CalData.Cal868.MyAddr  = CAL_MYADDR1;
    CalData.Cal868.OscTune = CAL_OSCTUNE1;

    CalDataUpdate();
  }

  UartPrintf( "MyAddr0   %u\n", CalData.Cal433.MyAddr );
  UartPrintf( "OscTune0  %u\n", CalData.Cal433.OscTune );
  UartPrintf( "MyAddr1   %u\n", CalData.Cal868.MyAddr );
  UartPrintf( "OscTune1  %u\n", CalData.Cal868.OscTune );
}

uint8_t NvDataChecksum()
{
  uint8_t Sum = 0;
  uint8_t const* Ptr = reinterpret_cast< uint8_t const* >( &NvData );

  for( auto Index = 0U; Index < sizeof( NvData ); Index++ )
  {
    Sum += *Ptr++;
  }

  return Sum;
}

uint8_t CalDataChecksum()
{
  uint8_t Sum = 0;
  uint8_t const* Ptr = reinterpret_cast< uint8_t const* >( &CalData );

  for( auto Index = 0U; Index < sizeof( CalData ); Index++ )
  {
    Sum += *Ptr++;
  }

  return Sum;
}

void NvDataUpdate()
{
  NvData.Checksum = 0;
  NvData.Checksum = -NvDataChecksum();

  auto Status = HAL_FLASH_Unlock();
  assert_param( Status == HAL_OK );

  uint32_t EraseStatus;
  FLASH_EraseInitTypeDef Data = { FLASH_TYPEERASE_PAGES, BASE_NVDATA, 1 };
  Status = HAL_FLASHEx_Erase( &Data, &EraseStatus );
  assert_param( Status == HAL_OK );
  assert_param( EraseStatus == 0xffffffff );

  uint32_t const End = BASE_NVDATA + sizeof( TNvData );
  uint64_t const *Ptr = reinterpret_cast< uint64_t const * >( &NvData );
  for( uint32_t Address = BASE_NVDATA; Address < End; Address += sizeof( uint64_t ))
  {
    Status = HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD, Address, *Ptr++ );
    assert_param( Status == HAL_OK );
  }

  Status = HAL_FLASH_Lock();
  assert_param( Status == HAL_OK );
}

void CalDataUpdate()
{
  CalData.Checksum = 0;
  CalData.Checksum = -CalDataChecksum();

  auto Status = HAL_FLASH_Unlock();
  assert_param( Status == HAL_OK );

  uint32_t EraseStatus;
  FLASH_EraseInitTypeDef Data = { FLASH_TYPEERASE_PAGES, BASE_CALDATA, 1 };
  Status = HAL_FLASHEx_Erase( &Data, &EraseStatus );
  assert_param( Status == HAL_OK );
  assert_param( EraseStatus == 0xffffffff );

  uint32_t const End = BASE_CALDATA + sizeof( TCalData );
  uint64_t const *Ptr = reinterpret_cast< uint64_t const * >( &CalData );
  for( uint32_t Address = BASE_CALDATA; Address < End; Address += sizeof( uint64_t ))
  {
    Status = HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD, Address, *Ptr++ );
    assert_param( Status == HAL_OK );
  }

  Status = HAL_FLASH_Lock();
  assert_param( Status == HAL_OK );
}
