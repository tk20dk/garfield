#include "si4463.h"


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


extern uint8_t const* RADIO_CONFIG_B1_1_25;
extern uint8_t const* RADIO_CONFIG_B1_2_25;
extern uint8_t const* RADIO_CONFIG_B1_4_25;
extern uint8_t const* RADIO_CONFIG_B1_8_25;
extern uint8_t const* RADIO_CONFIG_B1_16_25;
extern uint8_t const* RADIO_CONFIG_B1_32_32;
extern uint8_t const* RADIO_CONFIG_B1_64_32;
extern uint8_t const* RADIO_CONFIG_B1_128_64;
extern uint8_t const* RADIO_CONFIG_B1_256_128;
extern uint8_t const* RADIO_CONFIG_B1_CW;
extern uint8_t const* RADIO_CONFIG_C2_1_25;
extern uint8_t const* RADIO_CONFIG_C2_2_25;
extern uint8_t const* RADIO_CONFIG_C2_4_25;
extern uint8_t const* RADIO_CONFIG_C2_8_25;
extern uint8_t const* RADIO_CONFIG_C2_16_25;
extern uint8_t const* RADIO_CONFIG_C2_32_32;
extern uint8_t const* RADIO_CONFIG_C2_64_32;
extern uint8_t const* RADIO_CONFIG_C2_128_64;
extern uint8_t const* RADIO_CONFIG_C2_256_128;
extern uint8_t const* RADIO_CONFIG_C2_CW;

static uint8_t const* const RADIO_CONFIG_B1[ TSi4463::DataRateNoOf ] =
{
  RADIO_CONFIG_B1_1_25,
  RADIO_CONFIG_B1_2_25,
  RADIO_CONFIG_B1_4_25,
  RADIO_CONFIG_B1_8_25,
  RADIO_CONFIG_B1_16_25,
  RADIO_CONFIG_B1_32_32,
  RADIO_CONFIG_B1_64_32,
  RADIO_CONFIG_B1_128_64,
  RADIO_CONFIG_B1_256_128,
  RADIO_CONFIG_B1_CW
};
static uint8_t const* const RADIO_CONFIG_C2[ TSi4463::DataRateNoOf ] =
{
  RADIO_CONFIG_C2_1_25,
  RADIO_CONFIG_C2_2_25,
  RADIO_CONFIG_C2_4_25,
  RADIO_CONFIG_C2_8_25,
  RADIO_CONFIG_C2_16_25,
  RADIO_CONFIG_C2_32_32,
  RADIO_CONFIG_C2_64_32,
  RADIO_CONFIG_C2_128_64,
  RADIO_CONFIG_C2_256_128,
  RADIO_CONFIG_C2_CW
};

TSi4463::TSi4463( char const* const Name, GPIO_TypeDef *const SDN_Port, uint16_t const SDN_Pin, GPIO_TypeDef *const SEL_Port, uint16_t const SEL_Pin ) :
  Channel( 0 ),
  SyncRssi( 0 ),
  AfcFreqOffset( 0 ),
  PinSDN( SDN_Pin ),
  PinSEL( SEL_Pin ),
  Name( Name ),
  PortSDN( SDN_Port ),
  PortSEL( SEL_Port )
{
}


bool TSi4463::Init()
{
  Reset();

  if( GetDeviceType() != 0x4463 )
  {
    return false;
  }

  return true;
}

void TSi4463::Reset()
{
  SEL_H();
  SDN_H();
  HAL_Delay( 10 );
  SDN_L();
  HAL_Delay( 100 );
}

void TSi4463::Standby()
{
}

void TSi4463::Receive()
{
  uint8_t const Param[ 1 ] = { 59 };
  SetProperties( PROPERTY_PKT_FIELD_3_LENGTH_7_0, Param, sizeof( Param ));

  uint8_t const Data[] = { Channel, CONDITION_RX_START_IMMEDIATE, 0x00, 0x00, DEVICE_STATE_RX, DEVICE_STATE_READY, DEVICE_STATE_RX };
  Command( CMD_START_RX, Data, sizeof( Data ), nullptr, 0 );
}

void TSi4463::Transmit0()
{

  uint8_t const Params[ 2 ] = { Channel, DEVICE_STATE_READY << 4 };
  Command( CMD_START_TX, Params, sizeof( Params ), nullptr, 0 );
}

void TSi4463::Transmit( void const *const Data, uint8_t const Length )
{
  SEL_L();
  Spi.Write( CMD_TX_FIFO_WRITE );
  Spi.Write( Data, 4 );
  Spi.Write( Length - 4 );
  Spi.Write( ((uint8_t*)Data) + 4, Length - 4 );
  SEL_H();

  uint8_t const Param[ 1 ] = { static_cast< uint8_t >( Length - 4 ) };
  SetProperties( PROPERTY_PKT_FIELD_3_LENGTH_7_0, Param, sizeof( Param ));
  Transmit0();
}

void TSi4463::SetMode( uint8_t const Mode )
{
}

void TSi4463::SetChannel( uint8_t const Channel )
{
  this->Channel = Channel;
}

void TSi4463::SetDataRate( uint8_t const DataRate )
{
  LoadRadioConfig( DataRate );
}

void TSi4463::SetMyAddress( uint8_t const Address )
{
  uint8_t const Data[] = { Address, 0xff, 0x60 };
  SetProperties( PROPERTY_MATCH_VALUE_1, Data, sizeof( Data ));
}

void TSi4463::ClearRxFifo()
{
  uint8_t const Data[] = { 0x02 };
  Command( CMD_FIFO_INFO, Data, sizeof( Data ), nullptr, 0 );
}

void TSi4463::ClearTxFifo()
{
  uint8_t const Data[] = { 0x01 };
  Command( CMD_FIFO_INFO, Data, sizeof( Data ), nullptr, 0 );
}

uint8_t TSi4463::Interrupt()
{
  uint8_t Result = 0;

  uint8_t Status[ 8 ];
  Command( CMD_GET_INT_STATUS, nullptr, 0, Status, sizeof( Status ));

  if( Status[ 0 ] & INT_STATUS_PH_INT_STATUS )
  {
    if( Status[ 2 ] & INT_STATUS_FILTER_MATCH )
    {
    }
    if( Status[ 2 ] & INT_STATUS_FILTER_MISS )
    {
    }
    if( Status[ 2 ] & INT_STATUS_PACKET_SENT )
    {
      Result |= INT1_PACKET_SENT;
    }
    if( Status[ 2 ] & INT_STATUS_PACKET_RX )
    {
      uint8_t Buffer[ 8 ];
      Command( CMD_GET_MODEM_STATUS, nullptr, 0, Buffer, sizeof( Buffer ));
      SyncRssi = Buffer[ 3 ];
      AfcFreqOffset = (( Buffer[ 6 ] << 8 ) | Buffer[ 7 ] );
      Result |= INT1_PACKET_RECEIVED;
    }
    if( Status[ 2 ] & INT_STATUS_CRC_ERROR )
    {
      ClearRxFifo();
      UartPrintf( "%s CRC_ERROR\n", Name );
    }
    if( Status[ 2 ] & INT_STATUS_TX_FIFO_ALMOST_EMPTY )
    {
    }
    if( Status[ 2 ] & INT_STATUS_RX_FIFO_ALMOST_FULL )
    {
    }
  }

  if( Status[ 0 ] & INT_STATUS_MODEM_INT_STATUS )
  {
    if( Status[ 4 ] & INT_STATUS_INVALID_SYNC )
    {
    }
    if( Status[ 4 ] & INT_STATUS_RSSI_JUMP )
    {
    }
    if( Status[ 4 ] & INT_STATUS_RSSI )
    {
    }
    if( Status[ 4 ] & INT_STATUS_INVALID_PREAMBLE )
    {
    }
    if( Status[ 4 ] & INT_STATUS_PREAMBLE_DETECT )
    {
    }
    if( Status[ 4 ] & INT_STATUS_SYNC_DETECT )
    {
    }
  }

  if( Status[ 0 ] & INT_STATUS_CHIP_INT_STATUS )
  {
    if( Status[ 6 ] & INT_STATUS_CAL )
    {
      UartPrintf( "%s CAL\n", Name );
    }
    if( Status[ 6 ] & INT_STATUS_FIFO_UNDERFLOW_OVERFLOW_ERROR )
    {
      UartPrintf( "%s FIFO_UNDERFLOW_OVERFLOW_ERROR\n", Name );
    }
    if( Status[ 6 ] & INT_STATUS_STATE_CHANGE )
    {
    }
    if( Status[ 6 ] & INT_STATUS_CMD_ERROR )
    {
      UartPrintf( "%s CMD_ERROR\n", Name );
    }
    if( Status[ 6 ] & INT_STATUS_CHIP_READY )
    {
      UartPrintf( "%s Chip ready\n", Name );
    }
    if( Status[ 6 ] & INT_STATUS_LOW_BATT )
    {
      UartPrintf( "%s LOW_BATT\n", Name );
    }
    if( Status[ 6 ] & INT_STATUS_WUT )
    {
      UartPrintf( "%s WUT\n", Name );
    }
  }

  return Result;
}

uint8_t TSi4463::ReadRxPacket( uint8_t *const Buffer, uint8_t const MaxLen )
{
  uint8_t Data[ 2 ];
  Command( CMD_FIFO_INFO, nullptr, 0, Data, sizeof( Data ));

  auto const Length = Data[ 0 ];
  if(( Length == 0 ) || ( Length > MaxLen ))
  {
    ClearRxFifo();
    return 0;
  }

  SEL_L();
  Spi.Write( CMD_RX_FIFO_READ );
  Spi.Read( Buffer, Length );
  SEL_H();

  return Length;
}

int16_t TSi4463::GetAfcFreqOffset()
{
  return AfcFreqOffset;
}

void TSi4463::SetTxPower( uint8_t const TxPower )
{
}

void TSi4463::SetOscTune( uint8_t const Tune )
{
  uint8_t const Buffer[ 1 ] = { Tune };
  SetProperties( PROPERTY_GLOBAL_XO_TUNE, Buffer, sizeof( Buffer ));
}

uint16_t TSi4463::GetDeviceType()
{
  uint8_t Buffer[ 8 ] = { 0 };
  Command( CMD_PART_INFO, 0, 0, Buffer, sizeof( Buffer ));

  UartPrintf( "%s Si%02x%02x (%s)\n", Name,
    Buffer[ 1 ], Buffer[ 2 ],
    (Buffer[ 0 ] == 0x11 ) ? "B1" : "C2" );

  return (( Buffer[ 1 ] << 8 ) | Buffer[ 2 ] );
}

void TSi4463::Enable23mHzOnGpio0()
{
  uint8_t const Params[ 1 ] = { 0x40 };
  SetProperties( PROPERTY_GLOBAL_CLK_CFG, Params, sizeof( Params ));

  uint8_t const Data[] = { 7, 0, 0, 0, 0, 0, 0 };
  Command( CMD_GPIO_PIN_CFG, Data, sizeof( Data ), nullptr, 0 );
}

// 4ms
int16_t TSi4463::GetTemperature()
{
  uint8_t Data[ 8 ];
  uint8_t const Cmd[ 1 ] = { 0x10 };

  Command( CMD_GET_ADC_READING, Cmd, sizeof( Cmd ), Data, sizeof( Data ));

  uint16_t const Temp = (Data[4] << 8) | Data[5];
  return ((800 + Data[6]) / 4096.0) * Temp - ((Data[7] / 2) + 256);
}

// 4ms
uint16_t TSi4463::GetVoltage()
{
  uint8_t Data[ 8 ];
  uint8_t const Cmd[ 1 ] = { 0x08 };

  Command( CMD_GET_ADC_READING, Cmd, sizeof( Cmd ), Data, sizeof( Data ));

  uint16_t const Temp = (Data[2] << 8) | Data[3];
  return 30 * Temp / 1280;
}

void TSi4463::LoadRadioConfig( uint8_t const DataRate )
{
  uint8_t Buffer[ 8 ];
  Command( CMD_PART_INFO, 0, 0, Buffer, sizeof( Buffer ));
  uint8_t const* const Data = ( Buffer[ 0 ] == 0x11 ) ? RADIO_CONFIG_B1[ DataRate ] : RADIO_CONFIG_C2[ DataRate ];

  for( auto Index = 0; Data[ Index ]; Index += ( Data[ Index ] + 1 ))
  {
	Command( Data[ Index + 1 ], &Data[ Index + 2 ], Data[ Index ] - 1, nullptr, 0 );
  }
}

void TSi4463::Command( uint8_t const Cmd, uint8_t const *const TxBuf, uint8_t const TxLen, uint8_t *const RxBuf, uint8_t const RxLen )
{
  SEL_L();
  Spi.Write( Cmd );
  if( TxBuf && TxLen )
  {
    Spi.Write( TxBuf, TxLen );
  }
  SEL_L();
  SEL_H();

  bool Done = false;
  for( auto Count = 0; ( Done == false ) && ( Count < CTS_RETRIES ); Count++ )
  {
    SEL_L();

    Spi.Write( CMD_READ_BUF );
    if( Spi.Read() == REPLY_CTS )
    {
      if( RxBuf && RxLen )
      {
        Spi.Read( RxBuf, RxLen );
      }
      Done = true;
    }
    SEL_L();
    SEL_H();
  }
}

void TSi4463::SetProperties( uint16_t const Property, uint8_t const *const Values, uint8_t const Count )
{
  uint8_t Buffer[ 15 ];

  Buffer[ 0 ] = Property >> 8;
  Buffer[ 1 ] = Count;
  Buffer[ 2 ] = Property;
  for( auto Index = 0; Index < Count; Index++ )
  {
	  Buffer[ Index + 3 ] = Values[ Index ];
  }

  Command( CMD_SET_PROPERTY, Buffer, Count + 3, nullptr, 0 );
}

void TSi4463::GetProperties( uint16_t const Property, uint8_t *const Values, uint8_t const Count )
{
  uint8_t Buffer[ 3 ];

  Buffer[ 0 ] = Property >> 8;
  Buffer[ 2 ] = Count;
  Buffer[ 2 ] = Property;

  Command( CMD_GET_PROPERTY, Buffer, sizeof( Buffer ), Values, Count );
}
