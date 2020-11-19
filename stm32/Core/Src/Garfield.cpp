#include <Garfield.h>


extern "C" SPI_HandleTypeDef hspi1;

TSpi Spi( hspi1 );
static TGarfield Garfield;


void TGarfield::Setup()
{
  UartPrintf( "Garfield gateway\n" );

  NvDataInit();
  CalDataInit();

  HAL_GPIO_WritePin( LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_SET );
  HAL_GPIO_WritePin( LED2_GREEN_GPIO_Port, LED2_GREEN_Pin, GPIO_PIN_SET );

  if( Radio433.Init() )
  {
    Radio433.SetDataRate( NvData.Nv433.DataRate );
    Radio433.SetChannel( NvData.Nv433.Channel );
    Radio433.SetTxPower( NvData.Nv433.TxPower );
    Radio433.SetOscTune( CalData.Cal433.OscTune );
    Radio433.SetMyAddress( CalData.Cal433.MyAddr );
    Radio433.Receive();
    UartPrintf( "433 Voltage %u\n", Radio433.GetVoltage() );
    UartPrintf( "433 Temperature %d\n", Radio433.GetTemperature() );
//    Radio433.Enable23mHzOnGpio0();
  }
  else
  {
    Hmi1Error();
    UartPrintf( "Radio433 not found\n" );
  }

  if( Radio868.Init() )
  {
    Radio868.SetDataRate( NvData.Nv868.DataRate );
    Radio868.SetChannel( NvData.Nv868.Channel );
    Radio868.SetTxPower( NvData.Nv868.TxPower );
    Radio868.SetOscTune( CalData.Cal868.OscTune );
    Radio868.SetMyAddress( CalData.Cal868.MyAddr );
    Radio868.Receive();
    UartPrintf( "868 Voltage %u\n", Radio868.GetVoltage() );
    UartPrintf( "868 Temperature %d\n", Radio868.GetTemperature() );
//    Radio868.Enable23mHzOnGpio0();
  }
  else
  {
    Hmi2Error();
    UartPrintf( "Radio868 not found\n" );
  }
}

void TGarfield::Loop()
{
  HmiLoop();
  UsbLoop();
  UartLoop();

  if( Radio433IsrFlag )
  {
    Radio433IsrFlag = false;
    Radio433Interrupt();
  }

  if( Radio868IsrFlag )
  {
    Radio868IsrFlag = false;
    Radio868Interrupt();
  }
}

void TGarfield::Radio433Interrupt()
{
  auto const IntStatus1 = Radio433.Interrupt();

  if( IntStatus1 & TRadio::INT1_PACKET_SENT )
  {
    Hmi1Blue( 100 );
    Radio433.Receive();
  }

  if( IntStatus1 & TRadio::INT1_PACKET_RECEIVED )
  {
    auto const Length = Radio433.ReadRxPacket( Data433.Buffer, sizeof( Data433 ));
    if( Length )
    {
      HandleRxPacket( Radio433, Data433, false );
    }
    else
    {
      Radio433.Receive();
    }
  }
}

void TGarfield::Radio868Interrupt()
{
  auto const IntStatus1 = Radio868.Interrupt();

  if( IntStatus1 & TRadio::INT1_PACKET_SENT )
  {
    Hmi2Blue( 100 );
    Radio868.Receive();
  }

  if( IntStatus1 & TRadio::INT1_PACKET_RECEIVED )
  {
    auto const Length = Radio868.ReadRxPacket( Data868.Buffer, sizeof( Data868 ));
    if( Length )
    {
   	  HandleRxPacket( Radio868, Data868, true );
    }
    else
    {
      Radio868.Receive();
    }
  }
}

void TGarfield::HandleRxPacket( TRadio &Radio, TData const &Data, bool const Retransmit )
{
  int32_t const FreqOffset = Radio.GetAfcFreqOffset();
  int32_t const RssiSensor = Data.Header.Rssi;
  int32_t const RssiGateway = Radio.GetSyncRssi();

  SendAck( Radio, Data, Retransmit );

  if(( Data.Header.MsgId == MsgIdHello ) || ( Data.Header.MsgId == MsgIdSensorTVHI ))
  {
    if( Data.Header.SrcAddr < 50 )
    {
      SendUsb( Data.SensorTVHI, RssiGateway, RssiSensor, FreqOffset );
    }
    if( LogMode == true )
    {
      SendUart( Data.SensorTVHI, RssiGateway, RssiSensor, FreqOffset );
    }
  }
}

void TGarfield::SendAck( TRadio &Radio, TData const &Data, bool const Retransmit )
{
  union
  {
    TAck Ack;
    TAckTune AckTune;
  };
  Ack.Header.DestAddr = Data.Header.SrcAddr;
  Ack.Header.SrcAddr  = Data.Header.DestAddr;
  Ack.Header.Raw      = 0;

  int32_t TuneOffset = 0;
  int32_t TuneInterval = 0;
  if( Retransmit == false )
  {
    Tune( Ack.Header, Data.Header.MsgId == MsgIdHello, TuneInterval, TuneOffset );
  }

  if(( TuneOffset == 0 ) && ( TuneInterval == 0 ))
  {
    Ack.Header.MsgId = MsgIdAck;
    Radio.Transmit( &Ack, sizeof( Ack ));
  }
  else
  {
    AckTune.Header.MsgId = MsgIdAckTune;
    AckTune.Offset       = TuneOffset;
    AckTune.Interval     = TuneInterval;
    Radio.Transmit( &AckTune, sizeof( AckTune ));
  }
}

void TGarfield::Tune( THeader &Header, bool const Hello, int32_t &TuneInterval, int32_t &TuneOffset )
{
  auto const UnitId = Header.DestAddr;
  auto &TimeLastRx = LastRx[ UnitId ];

  if( Hello )
  {
    UartPrintf( "Hello(%u)\n", UnitId );
    TimeLastRx = TimeRx;
    Scale[ UnitId ] = 1.0f;
    return;
  }

  int32_t Diff = TimeRx - TimeLastRx - 60000;
  TimeLastRx = TimeRx;


  // Compensate for lost packages
  while( Diff > 40000 )
  {
    Diff -= 60000;
  }

  if( -200 > Diff || Diff > 200 )
  {
    if( Scale[ UnitId ] == 1.0f )
    {
      Scale[ UnitId ] = 60000.0f / ( 60000.0f + float( Diff ));
    }

    TuneInterval = -Diff * Scale[ UnitId ];
    UartPrintf( "AckTune(%u) Diff:%ld Scale:%d\n", UnitId, Diff, int(Scale[ UnitId ] * 10000) );
    return;
  }

  if( Diff > 1000 )
    Header.Interval = -3;
  else if( Diff < -1000 )
    Header.Interval = 3;
  else if( Diff > 100 )
    Header.Interval = -2;
  else if( Diff < -100 )
    Header.Interval = 2;

  TimeRx -= 500 * UnitId;
  int32_t Offset = (( TimeRx - TickSync ) % 60000 ) - 30000;

  if( Header.Interval == 0 )
  {
    if( -200 > Offset || Offset > 200 )
    {
      TuneOffset = -Offset * Scale[ UnitId ];
      TimeLastRx -= Offset;
      UartPrintf( "AckTune(%u) Offset:%ld\n", UnitId, Offset );
      return;
    }

    if( Offset > 1000 )
    {
      Header.Offset = -3;
      TimeLastRx -= 1000;
    }
    else if( Offset < -1000 )
    {
      Header.Offset = 3;
      TimeLastRx += 1000;
    }
    else if( Offset > 100 )
    {
      Header.Offset = -2;
      TimeLastRx -= 100;
    }
    else if( Offset < -100 )
    {
      Header.Offset = 2;
      TimeLastRx += 100;
    }
    else if( Offset > 10 )
    {
      Header.Offset = -1;
      TimeLastRx -= 10;
    }
    else if( Offset < -10 )
    {
      Header.Offset = 1;
      TimeLastRx += 10;
    }
  }

  auto &AvgOffset = AverageOffset[ UnitId ];
  auto &AvgInterval = AverageInterval[ UnitId ];
  AvgOffset << Offset;
  AvgInterval << Diff;

#if 1
  UartPrintf( "Tune(%u) %2ld %4ld %4ld %2ld %4ld %4ld\n",
    UnitId,
    Header.Interval,
    Diff,
    (int32_t)AvgInterval,
    Header.Offset,
    Offset,
    (int32_t)AvgOffset );
#else
  if( UnitId == 20 )
  {
    for( auto Index = 10; Index < 21; Index++ )
    {
      UartPrintf( "%5ld%5ld", (int32_t)AverageInterval[ Index ], (int32_t)AverageOffset[ Index ]);
    }
    UartPrintf( "\n" );
  }
#endif
}

void TGarfield::SyncHandler( char const* const Args )
{
  TickSync = HAL_GetTick();
  UartPrintf( "sync\n" );
}

void TGarfield::HelloHandler( char const* const Args )
{
  UartPrintf( "hello\n" );
  TickSync = 0;
}

void TGarfield::UsbLoop()
{
  struct
  {
    char Name[ 8 ];
    uint32_t Length;
    void (TGarfield::*Func)( char const* );
  } DataHandler[] =
  {
    { "sync", 4, &TGarfield::SyncHandler },
    { "hello ", 6, &TGarfield::HelloHandler }
  };

  char const* Line = UsbGetLine();
  if( Line == nullptr )
  {
    return;
  }

  if( *Line == '\0' )
  {
    return;
  }

  for( auto &Handler : DataHandler )
  {
    if( memcmp( Line, Handler.Name, Handler.Length ) == 0 )
    {
      Line += Handler.Length;
      SkipSpace( Line );
      (this->*Handler.Func)( Line );
      return;
    }
  }

  UartPrintf( "Unknown command: %s\n", Line );
}

void TGarfield::SendUsb( TSensorTVHI const &Sensor, int32_t const RssiGateway, int32_t const RssiSensor, int32_t const FreqOffset )
{
  static char Buffer[ 512 ];

  auto const Length = snprintf( Buffer, sizeof( Buffer ),
    "{\"Id\": %u, \"Voltage\": %u, \"Humidity\": %u, \"Illuminance\": %u, \"Temperature\": %d, \"RssiGateway\": %ld, \"RssiSensor\": %ld, \"OffsetGateway\": %ld, \"OffsetSensor\": %d, \"RetryTimeouts\": %u, \"AckTimeouts\": %u}\n",
    Sensor.Header.SrcAddr,
    Sensor.Voltage,
    Sensor.Humidity,
    Sensor.Illuminance,
    Sensor.Temperature,
    RssiGateway,
    RssiSensor,
    FreqOffset,
    Sensor.FreqOffset,
    Sensor.RetryTimeouts,
    Sensor.AckTimeouts );

  if( Length > 0 )
  {
    CDC_Transmit_FS( reinterpret_cast<uint8_t*>( Buffer ), Length );
  }
}

void TGarfield::SendUart( TSensorTVHI const &Sensor, int32_t const RssiGateway, int32_t const RssiSensor, int32_t const FreqOffset )
{
  UartPrintf( "Unit(%u) T:%d, V:%u, H:%u I:%u Grssi:%ld Srssi:%ld gafc:%ld safc:%d retry:%u ack:%u\n",
    Sensor.Header.SrcAddr,
    Sensor.Temperature,
    Sensor.Voltage,
    Sensor.Humidity,
    Sensor.Illuminance,
    RssiGateway,
    RssiSensor,
    FreqOffset,
    Sensor.FreqOffset,
    Sensor.RetryTimeouts,
    Sensor.AckTimeouts );
}

void TGarfield::HAL_GPIO_EXTI_Callback( uint16_t const GPIO_Pin )
{
  switch( GPIO_Pin )
  {
    case IRQ_RADIO433_Pin:
    {
      TimeRx = HAL_GetTick();
      Radio433IsrFlag = true;
    }
    break;

    case IRQ_RADIO868_Pin:
    {
      TimeRx = HAL_GetTick();
      Radio868IsrFlag = true;
    }
    break;

    default:
    {
      HmiError();
    }
    break;
  }
}

void TGarfield::HmiLoop()
{
  if( TimeoutHmiBlue1 && ( HAL_GetTick() >= TimeoutHmiBlue1 ))
  {
    TimeoutHmiBlue1 = 0;
    HAL_GPIO_WritePin( LED1_BLUE_GPIO_Port, LED1_BLUE_Pin, GPIO_PIN_RESET );
    HAL_GPIO_WritePin( LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_SET );
  }
  if( TimeoutHmiBlue2 && ( HAL_GetTick() >= TimeoutHmiBlue2 ))
  {
    TimeoutHmiBlue2 = 0;
    HAL_GPIO_WritePin( LED2_BLUE_GPIO_Port, LED2_BLUE_Pin, GPIO_PIN_RESET );
    HAL_GPIO_WritePin( LED2_GREEN_GPIO_Port, LED2_GREEN_Pin, GPIO_PIN_SET );
  }
}

void TGarfield::Hmi1Blue( uint32_t const Interval )
{
  TimeoutHmiBlue1 = HAL_GetTick() + Interval;
  HAL_GPIO_WritePin( LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LED1_BLUE_GPIO_Port, LED1_BLUE_Pin, GPIO_PIN_SET );

}

void TGarfield::Hmi2Blue( uint32_t const Interval )
{
  TimeoutHmiBlue2 = HAL_GetTick() + Interval;
  HAL_GPIO_WritePin( LED2_GREEN_GPIO_Port, LED2_GREEN_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LED2_BLUE_GPIO_Port, LED2_BLUE_Pin, GPIO_PIN_SET );
}

void TGarfield::HmiError()
{
  Hmi1Error();
  Hmi2Error();
}
void TGarfield::Hmi1Error()
{
  HAL_GPIO_WritePin( LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LED1_BLUE_GPIO_Port, LED1_BLUE_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LED1_RED_GPIO_Port, LED1_RED_Pin, GPIO_PIN_SET );
}
void TGarfield::Hmi2Error()
{
  HAL_GPIO_WritePin( LED2_GREEN_GPIO_Port, LED2_GREEN_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LED2_BLUE_GPIO_Port, LED2_BLUE_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LED2_RED_GPIO_Port, LED2_RED_Pin, GPIO_PIN_SET );
}

extern "C" void GarfieldSetup()
{
  Garfield.Setup();
}

extern "C" void GarfieldLoop()
{
  Garfield.Loop();
}

extern "C" void HAL_GPIO_EXTI_Callback( uint16_t const GPIO_Pin )
{
  Garfield.HAL_GPIO_EXTI_Callback( GPIO_Pin );
}

TGarfield::TGarfield() :
  LogMode( false ),
  Radio433IsrFlag( false ),
  Radio868IsrFlag( false ),
  Data433(),
  Data868(),
  Radio433( "433", SDN_RADIO433_GPIO_Port, SDN_RADIO433_Pin, CS_RADIO433_GPIO_Port, CS_RADIO433_Pin ),
  Radio868( "868", SDN_RADIO868_GPIO_Port, SDN_RADIO868_Pin, CS_RADIO868_GPIO_Port, CS_RADIO868_Pin ),
  TimeRx( 0 ),
  TickSync( 0 ),
  TimeoutHmiBlue1( 0 ),
  TimeoutHmiBlue2( 0 ),
  Scale(),
  LastRx(),
  AverageOffset(),
  AverageInterval()
{
  Scale.fill( 1.0f );
  LastRx.fill( 0 );
}

TGarfield::~TGarfield()
{
}
