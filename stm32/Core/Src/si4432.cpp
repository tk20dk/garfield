#include "si4432.h"


#if 0
1 	433.075 	24 	433.650 	47 	434.225
2 	433.100 	25 	433.675 	48 	434.250
3 	433.125 	26 	433.700 	49 	434.275
4 	433.150 	27 	433.725 	50 	434.300
5 	433.175 	28 	433.750 	51 	434.325
6 	433.200 	29 	433.775 	52 	434.350
7 	433.225 	30 	433.800 	53 	434.375
8 	433.250 	31 	433.825 	54 	434.400
9 	433.275 	32 	433.850 	55 	434.425
10 	433.300 	33 	433.875 	56 	434.450
11 	433.325 	34 	433.900 	57 	434.475
12 	433.350 	35 	433.925 	58 	434.500
13 	433.375 	36 	433.950 	59 	434.525
14 	433.400 	37 	433.975 	60 	434.550
15 	433.425 	38 	434.000 	61 	434.575
16 	433.450 	39 	434.025 	62 	434.600
17 	433.475 	40 	434.050 	63 	434.625
18 	433.500 	41 	434.075 	64 	434.650
19 	433.525 	42 	434.100 	65 	434.675
20 	433.550 	43 	434.125 	66 	434.700
21 	433.575 	44 	434.150 	67 	434.725
22 	433.600 	45 	434.175 	68 	434.750
23 	433.625 	46 	434.200 	69 	434.775
#endif


uint8_t const TSi4432::SyncWord[ SYNC_LENGTH ]   = { 0x2d, 0xd4, 0x2d, 0xd4 };


TSi4432::TSi4432(
  char const* const Name,
  GPIO_TypeDef *const SDN_Port,
  uint16_t const SDN_Pin,
  GPIO_TypeDef *const SEL_Port,
  uint16_t const SEL_Pin ) :
  SyncRssi( 0 ),
  PinSDN( SDN_Pin ),
  PinSEL( SEL_Pin ),
  Name( Name ),
  PortSDN( SDN_Port ),
  PortSEL( SEL_Port )
{
}

bool TSi4432::Init()
{
  Reset();

  uint8_t const DeviceType = ReadRegister( 0x00 );
  uint8_t const DeviceVersion = ReadRegister( 0x01 );
  if(( DeviceType != 0x08U ) || ( DeviceVersion != 0x06U ))
  {
    return false;
  }

  WriteRegister( 0x0b, 0xca ); // GPIO 0 = digital output
  WriteRegister( 0x0c, 0xca ); // GPIO 1 = digital output
  WriteRegister( 0x0d, 0xca ); // GPIO 2 = digital output

  WriteRegister( 0x30, 0x8c ); // enable PH+ FIFO, enable crc, msb
  WriteRegister( 0x32, 0xff ); // head = byte 3, 2, 1, 0
  WriteRegister( 0x33, 0x42 ); // sync = byte 3, 2
  WriteRegister( 0x34, 40 );   // preamble for Tx = 40 nibble
  WriteRegister( 0x35, 0x20 ); // preamble detection = 20bit

  WriteRegister( 0x36, SyncWord, sizeof( SyncWord ));

  SetMode( 7 );
  SetTxPower( TxPower1dBm );
  SetChannel( 1 );
  SetOscTune( 76 );
  SetDataRate( DataRate1kbps25kHz );

  ClearRxFifo();
  ClearTxFifo();

  EnableInterrupt(
    INT1_PACKET_SENT | INT1_CRC_ERROR | INT1_PACKET_RECEIVED,
    INT2_SYNC_WORD );

  Standby();
  return true;
}

uint8_t TSi4432::Interrupt( void )
{
  TStatus Status = ReadStatus();

  if( Status.IntStatus1 & INT1_CRC_ERROR )
  {
  }
  if( Status.IntStatus1 & INT1_PACKET_RECEIVED )
  {
  }
  if( Status.IntStatus1 & INT1_PACKET_SENT )
  {
  }
  if( Status.IntStatus2 & INT2_SYNC_WORD )
  {
    SyncRssi = ReadRssi();
  }

  return Status.IntStatus1;
}

void TSi4432::SetMode( uint8_t const Mode )
{
  static uint8_t const ModeData[ 7U ] =
  {
    0x30, 0x31, 0x32, 0x33, 0x21, 0x22, 0x23
  };

  WriteRegister( 0x71, ModeData[ Mode ] );

  if( Mode == 0 )
  {
    TX1_RX0();
    SetClearRegister( 0x07, PWRSTATE_TX, PWRSTATE_MASK );
  }
}

int16_t TSi4432::GetTemperature()
{
  return 0;
}

uint16_t TSi4432::GetVoltage()
{
  return 0;
}

void TSi4432::SetMyAddress( uint8_t const Address )
{
  uint8_t const RxMask[ HEADER_LENGTH ]   = { 0xff };
  uint8_t const RxHeader[ HEADER_LENGTH ] = { Address };

  WriteRegister( 0x43, RxMask,   sizeof( RxMask   ));
  WriteRegister( 0x3f, RxHeader, sizeof( RxHeader ));
}

void TSi4432::SetChannel( uint8_t const Channel )
{
	static uint8_t const FreqData[ 69 ][ 4 ] =
	{
	  { 0x53, 0x4C, 0xE0 }, { 0x53, 0x4D, 0x80 }, { 0x53, 0x4E, 0x20 }, { 0x53, 0x4E, 0xC0 },
	  { 0x53, 0x4F, 0x60 }, { 0x53, 0x50, 0x00 }, { 0x53, 0x50, 0xA0 }, { 0x53, 0x51, 0x40 },
	  { 0x53, 0x51, 0xE0 }, { 0x53, 0x52, 0x80 }, { 0x53, 0x53, 0x20 }, { 0x53, 0x53, 0xC0 },
	  { 0x53, 0x54, 0x60 }, { 0x53, 0x55, 0x00 }, { 0x53, 0x55, 0xA0 }, { 0x53, 0x56, 0x40 },
	  { 0x53, 0x56, 0xE0 }, { 0x53, 0x57, 0x80 }, { 0x53, 0x58, 0x20 }, { 0x53, 0x58, 0xC0 },
	  { 0x53, 0x59, 0x60 }, { 0x53, 0x5A, 0x00 }, { 0x53, 0x5A, 0xA0 }, { 0x53, 0x5B, 0x40 },
	  { 0x53, 0x5B, 0xE0 }, { 0x53, 0x5C, 0x80 }, { 0x53, 0x5D, 0x20 }, { 0x53, 0x5D, 0xC0 },
	  { 0x53, 0x5E, 0x60 }, { 0x53, 0x5F, 0x00 }, { 0x53, 0x5F, 0xA0 }, { 0x53, 0x60, 0x40 },
	  { 0x53, 0x60, 0xE0 }, { 0x53, 0x61, 0x80 }, { 0x53, 0x62, 0x20 }, { 0x53, 0x62, 0xC0 },
	  { 0x53, 0x63, 0x60 }, { 0x53, 0x64, 0x00 }, { 0x53, 0x64, 0xA0 }, { 0x53, 0x65, 0x40 },
	  { 0x53, 0x65, 0xE0 }, { 0x53, 0x66, 0x80 }, { 0x53, 0x67, 0x20 }, { 0x53, 0x67, 0xC0 },
	  { 0x53, 0x68, 0x60 }, { 0x53, 0x69, 0x00 }, { 0x53, 0x69, 0xA0 }, { 0x53, 0x6A, 0x40 },
	  { 0x53, 0x6A, 0xE0 }, { 0x53, 0x6B, 0x80 }, { 0x53, 0x6C, 0x20 }, { 0x53, 0x6C, 0xC0 },
	  { 0x53, 0x6D, 0x60 }, { 0x53, 0x6E, 0x00 }, { 0x53, 0x6E, 0xA0 }, { 0x53, 0x6F, 0x40 },
	  { 0x53, 0x6F, 0xE0 }, { 0x53, 0x70, 0x80 }, { 0x53, 0x71, 0x20 }, { 0x53, 0x71, 0xC0 },
	  { 0x53, 0x72, 0x60 }, { 0x53, 0x73, 0x00 }, { 0x53, 0x73, 0xA0 }, { 0x53, 0x74, 0x40 },
	  { 0x53, 0x74, 0xE0 }, { 0x53, 0x75, 0x80 }, { 0x53, 0x76, 0x20 }, { 0x53, 0x76, 0xC0 },
	  { 0x53, 0x77, 0x60 }
	};

  uint8_t const Index = Channel - 1;
  WriteRegister( 0x75, FreqData[ Index ], 3 );
}

int32_t TSi4432::GetAfcFreqOffset()
{
  int32_t Correction = (int8_t)ReadRegister( 0x2b );
  Correction *= 4;
  Correction |= ReadRegister( 0x2c ) >> 6;
  return ( Correction * 15625 ) / 100;
}

void TSi4432::SetDataRate( uint8_t const DataRate )
{
  static uint8_t const RateRegs[ 18 ] =
  {
      0x6E, 0x6F, 0x70, 0x58, 0x72, 0x71, 0x1C, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x1D, 0x1E, 0x2A, 0x1F, 0x69
  };
  static uint8_t const RateData[ DataRateNoOf ][ 18 ] =
  {
    { 0x08, 0x31, 0x2C, 0x80, 0x28, 0x23, 0x2E, 0xE8, 0x60, 0x20, 0xC5, 0x00, 0x05, 0x44, 0x0A, 0x1E, 0x03, 0x60 }, // DataRate1kbpx25kHz
    { 0x10, 0x62, 0x2C, 0x80, 0x28, 0x23, 0x15, 0xE8, 0x60, 0x20, 0xC5, 0x00, 0x07, 0x44, 0x0A, 0x1E, 0x03, 0x60 }, // DataRate2kbps25kHz
    { 0x20, 0xC5, 0x2C, 0x80, 0x28, 0x23, 0x15, 0xF4, 0x20, 0x41, 0x89, 0x00, 0x17, 0x44, 0x0A, 0x1E, 0x03, 0x60 }, // DataRate4kbps25kHz
    { 0x41, 0x89, 0x2C, 0x80, 0x28, 0x23, 0x16, 0xFA, 0x00, 0x83, 0x12, 0x00, 0x56, 0x44, 0x0A, 0x1E, 0x03, 0x60 }, // DataRate8kbps25kHz
    { 0x83, 0x12, 0x2C, 0x80, 0x28, 0x23, 0x17, 0x7D, 0x01, 0x06, 0x25, 0x01, 0x52, 0x44, 0x0A, 0x1E, 0x03, 0x60 }, // DataRate16kbps25kHz
    { 0x08, 0x31, 0x0C, 0x80, 0x33, 0x23, 0x05, 0x7D, 0x01, 0x06, 0x25, 0x02, 0x0E, 0x44, 0x0A, 0x20, 0x03, 0x60 }, // DataRate32kbps32kHz
    { 0x10, 0x62, 0x0C, 0x80, 0x33, 0x23, 0x07, 0x3F, 0x02, 0x0C, 0x4A, 0x07, 0xFF, 0x44, 0x0A, 0x31, 0x03, 0x60 }, // DataRate64kbps32kHz
    { 0x20, 0xC5, 0x0C, 0xC0, 0x66, 0x23, 0x83, 0x5E, 0x01, 0x5D, 0x86, 0x05, 0x74, 0x44, 0x0A, 0x50, 0x03, 0x60 }, // DataRate128kbps64kHz
    { 0x41, 0x89, 0x0C, 0xED, 0xCD, 0x23, 0x8C, 0x2F, 0x02, 0xBB, 0x0D, 0x07, 0xFF, 0x44, 0x02, 0x50, 0x03, 0x60 }  // DataRate256kbps128kHz
  };

  uint8_t const *Ptr = RateData[ DataRate ];
  for( uint8_t Reg : RateRegs )
  {
    WriteRegister( Reg, *Ptr++ );
  }
}

TSi4432::TStatus TSi4432::ReadStatus()
{
  uint8_t Buffer[ 7 ];
  ReadRegister( 0x02, Buffer, sizeof( Buffer ));
  return TStatus( Buffer );
}

void TSi4432::Reset()
{
  SEL_H();
  SDN_H();
  HAL_Delay( 10 );
  SDN_L();
  HAL_Delay( 100 );
}

void TSi4432::Idle()
{
  TX0_RX0();
  SetClearRegister( 0x07, PWRSTATE_READY, PWRSTATE_MASK );
}

void TSi4432::Standby()
{
  TX0_RX0();
  SetClearRegister( 0x07, PWRSTATE_STANDBY, PWRSTATE_MASK );
}

void TSi4432::Receive()
{
  TX0_RX1();
  SetClearRegister( 0x07, PWRSTATE_RX, PWRSTATE_MASK );
}

void TSi4432::Transmit( void const *const Buffer, uint8_t const Length )
{
  uint8_t const* const Ptr = reinterpret_cast< uint8_t const* >( Buffer );

  WriteRegister( TX_HDR, Ptr, HEADER_LENGTH );
  WriteRegister( TX_PKT_LEN, Length - HEADER_LENGTH );
  WriteRegister( FIFO, &Ptr[ HEADER_LENGTH ], Length - HEADER_LENGTH );

  TX1_RX0();
  SetClearRegister( 0x07, PWRSTATE_TX, PWRSTATE_MASK );
}

uint8_t TSi4432::ReadRxPacket( uint8_t *const Buffer, uint8_t const MaxLen )
{
  uint8_t const Length = ReadRegister( RX_PKT_LEN );
  if(( Length == 0U ) || ( Length > ( MaxLen - HEADER_LENGTH )))
  {
    ClearRxFifo();
    return 0U;
  }

  ReadRegister( RX_HDR, Buffer, HEADER_LENGTH );
  ReadRegister( FIFO, &Buffer[ HEADER_LENGTH ], Length );

  return Length + HEADER_LENGTH;
}

void TSi4432::ClearRxFifo()
{
  uint8_t const Data = ReadRegister( 0x08 );
  WriteRegister( 0x08, Data | 0x02 );
  WriteRegister( 0x08, Data );
}

void TSi4432::ClearTxFifo()
{
  uint8_t const Data = ReadRegister( 0x08 );
  WriteRegister( 0x08, Data | 0x01 );
  WriteRegister( 0x08, Data );
}

void TSi4432::EnableInterrupt( uint8_t const Mask1, uint8_t const Mask2 )
{
  uint8_t Buffer[ 2 ];
  ReadRegister( 0x05, Buffer, sizeof( Buffer ));
  Buffer[ 0 ] |= Mask1;
  Buffer[ 1 ] |= Mask2;
  WriteRegister( 0x05, Buffer, sizeof( Buffer ));
}

void TSi4432::DisableInterrupt( uint8_t const Mask1, uint8_t const Mask2 )
{
  uint8_t Buffer[ 2 ];
  ReadRegister( 0x05, Buffer, sizeof( Buffer ));
  Buffer[ 0 ] &= ~Mask1;
  Buffer[ 1 ] &= ~Mask1;
  WriteRegister( 0x05, Buffer, sizeof( Buffer ));
}

void TSi4432::SetClearRegister( uint8_t const Register, uint8_t const Set, uint8_t const Clear )
{
  uint32_t Data = ReadRegister( Register );
  Data &= ~Clear;
  Data |= Set;
  WriteRegister( Register, Data );
}

uint8_t TSi4432::ReadRegister( uint8_t const Register )
{
  SEL_L();
  Spi.Write( Register );
  uint8_t const Data = Spi.Read();
  SEL_H();

  return Data;
}

void TSi4432::WriteRegister( uint8_t const Register, uint8_t const Data )
{
  SEL_L();
  Spi.Write( 0x80 | Register );
  Spi.Write( Data );
  SEL_H();
}

void TSi4432::ReadRegister( uint8_t const Register, uint8_t *const Buffer, uint8_t const Length )
{
  SEL_L();
  Spi.Write( Register );
  Spi.Read( Buffer, Length );
  SEL_H();
}

void TSi4432::WriteRegister( uint8_t const Register, uint8_t const *const Buffer, uint8_t const Length )
{
  SEL_L();
  Spi.Write( 0x80 | Register );
  Spi.Write( Buffer, Length );
  SEL_H();
}
