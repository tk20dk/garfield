#ifndef GARFIELD_H_
#define GARFIELD_H_

#define SI4432_no
#define SI4463

#include <system.h>
#include "nvdata.h"
#include <message.h>

#ifdef SI4432
#include <si4432.h>
#endif

#ifdef SI4463
  #include <si4463.h>
#endif


class TGarfield
{
  template< int32_t const Size >
  class TAverage
  {
  public:
    TAverage() :
      Sum( 0 ),
      Count( 0 ),
      Index( 0 ),
      Data()
    {
      Reset();
    }
    ~TAverage()
    {
    }

    void Reset()
    {
      Sum = 0;
      Count = 0;
      Index = 0;
      Data.fill( 0 );
    }

    TAverage& operator<<( int32_t const Value )
    {
      Sum -= Data[ Index ];
      Sum += Value;
      Data[ Index ] = Value;
      ++Index %= Size;
      if( Count < Size )
      {
        Count++;
      }
      return *this;
    }

    operator int32_t() const
    {
      return GetAverage();
    }

    int32_t GetAverage() const
    {
      return Count ? Sum / Count : 0;
    }

  private:
    int32_t Sum;
    int32_t Count;
    int32_t Index;
    std::array< int32_t, Size > Data;
  };

public:
  TGarfield();
  ~TGarfield();

  void Setup();
  void Loop();
  void HAL_GPIO_EXTI_Callback( uint16_t const GPIO_Pin );

private:
  void HmiError();
  void Hmi1Error();
  void Hmi2Error();
  void Hmi1Blue( uint32_t const Interval = 0 );
  void Hmi2Blue( uint32_t const Interval = 0 );
  void HmiLoop();
  void UsbLoop();
  void UartLoop();
  void HandleRxPacket( TRadio &Radio, TData const &Data, bool const Retransmit );
  void Radio433Interrupt();
  void Radio868Interrupt();

  void Tune( THeader &Header, bool const Hello, int32_t &TuneInterval, int32_t &TuneOffset );
  void SyncHandler( char const* const Args );
  void HelloHandler( char const* const Args );

  void SendAck( TRadio &Radio, TData const &Data, bool const Retransmit );
  void SendConfig( TRadio &Radio, TData const &Data, bool const Retransmit );

  void SendUsb( TSensorTVHI const &Sensor, int32_t const RssiGateway, int32_t const RssiSensor, int32_t const FreqOffset );
  void SendUart( TSensorTVHI const &Sensor, int32_t const RssiGateway, int32_t const RssiSensor, int32_t const FreqOffset );

  void SkipSpace( char const* &Line );
public:
  void LogHandler( char const* const Line );
  void HelpHandler( char const* const Line );
  void TuneHandler( char const* const Line );
  void ConfigHandler( char const* const Line );

private:
  bool LogMode;
  bool Radio433IsrFlag;
  bool Radio868IsrFlag;
  TData Data433;
  TData Data868;
  TRadio Radio433;
  TRadio Radio868;
  uint32_t TimeRx;
  uint32_t TickSync;
  uint32_t TimeoutHmiBlue1;
  uint32_t TimeoutHmiBlue2;
  std::array< float, 32 > Scale;
  std::array< uint32_t, 22 > LastRx;
  std::array< TAverage< 20 >, 22 > AverageOffset;
  std::array< TAverage< 20 >, 22 > AverageInterval;
};


#endif // GARFIELD_H_
