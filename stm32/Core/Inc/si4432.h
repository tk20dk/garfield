#ifndef SI4432_H__
#define SI4432_H__

#include "system.h"
#include "spi.h"


class TSi4432
{
public:
  static uint32_t constexpr INT1_CRC_ERROR        = 0x01U; // CRC error in received packet
  static uint32_t constexpr INT1_PACKET_RECEIVED  = 0x02U; // Packet received interrupt
  static uint32_t constexpr INT1_PACKET_SENT      = 0x04U; // Packet sent interrupt

  static uint32_t constexpr INT2_POWER_ON_RESET   = 0x01U; // Power On Reset
  static uint32_t constexpr INT2_CHIP_READY       = 0x02U; // Chip Ready
  static uint32_t constexpr INT2_WAKEUP_TIMER     = 0x08U; // Wake-up Timer
  static uint32_t constexpr INT2_RSSI_LEVEL       = 0x10U; // RSSI Level
  static uint32_t constexpr INT2_INVALID_PREAMBLE = 0x20U; // Invalid Preamble
  static uint32_t constexpr INT2_VALID_PREAMBLE   = 0x40U; // Valid Preamble
  static uint32_t constexpr INT2_SYNC_WORD        = 0x80U; // Sync Word

  enum EDataRate
  {
    DataRate1kbps25kHz,
    DataRate2kbps25kHz,
    DataRate4kbps25kHz,
    DataRate8kbps25kHz,
    DataRate16kbps25kHz,
    DataRate32kbps32kHz,
    DataRate64kbps32kHz,
    DataRate128kbps64kHz,
    DataRate256kbps128kHz,
    DataRateNoOf
  };

  enum ETxPower
  {
    TxPower1dBm,
    TxPower2dBm,
    TxPower5dBm,
    TxPower8dBm,
    TxPower11dBm,
    TxPower14dBm,
    TxPower17dBm,
    TxPower20dBm,
    TxPowerNoOf
  };

  struct TStatus
  {
    TStatus( uint8_t const *const Data ) :
      Mode1( Data[ 5 ] ),
      Mode2( Data[ 6 ] ),
      DevStatus( Data[ 0 ] ),
      IntStatus1( Data[ 1 ] ),
      IntStatus2( Data[ 2 ] ),
      IntEnable1( Data[ 3 ] ),
      IntEnable2( Data[ 4 ] )
    {
    }

    uint8_t Mode1;
    uint8_t Mode2;
    uint8_t DevStatus;
    uint8_t IntStatus1;
    uint8_t IntStatus2;
    uint8_t IntEnable1;
    uint8_t IntEnable2;
  };

private:
  static uint32_t const PWRSTATE_STANDBY = 0x00U; // define standby mode
  static uint32_t const PWRSTATE_READY   = 0x01U; // define ready mode
  static uint32_t const PWRSTATE_RX      = 0x05U; // define Rx mode
  static uint32_t const PWRSTATE_TX      = 0x09U; // define Tx mode
  static uint32_t const PWRSTATE_MASK    = PWRSTATE_READY | PWRSTATE_RX | PWRSTATE_TX;

  static uint32_t const TX_HDR     = 0x3a;
  static uint32_t const RX_HDR     = 0x47;
  static uint32_t const TX_PKT_LEN = 0x3e;
  static uint32_t const RX_PKT_LEN = 0x4b;
  static uint32_t const FIFO       = 0x7f;

  static uint8_t const SYNC_LENGTH    = 4;
  static uint8_t const HEADER_LENGTH  = 4;
  static uint8_t const SyncWord[ SYNC_LENGTH ];

public:
  TSi4432( char const* const Name, GPIO_TypeDef *const SDN_Port, uint16_t const SDN_Pin, GPIO_TypeDef *const SEL_Port, uint16_t const SEL_Pin );

  bool Init();
  void Idle();
  void Reset();
  void Standby();
  void Receive();
  void Transmit( void const *const Data, uint8_t const Length );
  void SetMode( uint8_t const Mode );
  void SetChannel( uint8_t const Channel );
  void SetDataRate( uint8_t const DataRate );
  void SetMyAddress( uint8_t const Address );
  void ClearRxFifo();
  void ClearTxFifo();
  TStatus ReadStatus();
  uint8_t Interrupt();
  uint8_t ReadRxPacket( uint8_t *const Buffer, uint8_t const MaxLen );
  int32_t GetAfcFreqOffset();
  int16_t GetTemperature();
  uint16_t GetVoltage();

  void SetTxPower( uint8_t const TxPower )
  {
    WriteRegister( 0x6d, static_cast< uint8_t >( TxPower ));
  }
  void SetOscTune( uint8_t const OscLoad )
  {
    WriteRegister( 0x09, OscLoad );
  }
  uint8_t ReadRssi()
  {
    return ReadRegister( 0x26 );
  }
  uint8_t GetSyncRssi()
  {
    return SyncRssi;
  }

private:
  void EnableInterrupt( uint8_t const Mask1, uint8_t const Mask2 = 0U );
  void DisableInterrupt( uint8_t const Mask1, uint8_t const Mask2 = 0U );
  void SetClearRegister( uint8_t const Register, uint8_t const Set, uint8_t const Clear );
  uint8_t ReadRegister( uint8_t const Register );
  void WriteRegister( uint8_t const Register, uint8_t const Data );
  void ReadRegister( uint8_t const Register, uint8_t *const Buffer, uint8_t const Length );
  void WriteRegister( uint8_t const Register, uint8_t const *const Buffer, uint8_t const Length );

  void SDN_H()
  {
    SetPin( PortSDN, PinSDN );
  }
  void SDN_L()
  {
    ResetPin( PortSDN, PinSDN );
  }
  void SEL_H()
  {
    SetPin( PortSEL, PinSEL );
  }
  void SEL_L()
  {
    ResetPin( PortSEL, PinSEL );
  }
  void TX0_RX0()
  {
    WriteRegister( 0x0e, 0x00 ); // Antenna state for ready mode, standby mode, power down mode
  }
  void TX1_RX0()
  {
    WriteRegister( 0x0e, 0x01 ); // Antenna state for Tx
  }
  void TX0_RX1()
  {
    WriteRegister( 0x0e, 0x02 ); // Antenna state for Rx
  }

private:
  uint8_t SyncRssi;
  uint16_t const PinSDN;
  uint16_t const PinSEL;
  char const* const Name;
  GPIO_TypeDef *const PortSDN;
  GPIO_TypeDef *const PortSEL;
};
typedef TSi4432 TRadio;

#endif // SI4432_H__
