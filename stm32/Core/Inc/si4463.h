#ifndef SI4463_H_
#define SI4463_H_

#include "system.h"
#include "spi.h"


class TSi4463
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
    DataRateCW,
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

private:
  static uint8_t const CMD_NOP                  = 0x00;
  static uint8_t const CMD_PART_INFO            = 0x01;
  static uint8_t const CMD_POWER_UP             = 0x02;
  static uint8_t const CMD_PATCH_IMAGE          = 0x04;
  static uint8_t const CMD_FUNC_INFO            = 0x10;
  static uint8_t const CMD_SET_PROPERTY         = 0x11;
  static uint8_t const CMD_GET_PROPERTY         = 0x12;
  static uint8_t const CMD_GPIO_PIN_CFG         = 0x13;
  static uint8_t const CMD_GET_ADC_READING      = 0x14;
  static uint8_t const CMD_FIFO_INFO            = 0x15;
  static uint8_t const CMD_PACKET_INFO          = 0x16;
  static uint8_t const CMD_IRCAL                = 0x17;
  static uint8_t const CMD_PROTOCOL_CFG         = 0x18;
  static uint8_t const CMD_GET_INT_STATUS       = 0x20;
  static uint8_t const CMD_GET_PH_STATUS        = 0x21;
  static uint8_t const CMD_GET_MODEM_STATUS     = 0x22;
  static uint8_t const CMD_GET_CHIP_STATUS      = 0x23;
  static uint8_t const CMD_START_TX             = 0x31;
  static uint8_t const CMD_START_RX             = 0x32;
  static uint8_t const CMD_REQUEST_DEVICE_STATE = 0x33;
  static uint8_t const CMD_CHANGE_STATE         = 0x34;
  static uint8_t const CMD_RX_HOP               = 0x36;
  static uint8_t const CMD_READ_BUF             = 0x44;
  static uint8_t const CMD_FAST_RESPONSE_A      = 0x50;
  static uint8_t const CMD_FAST_RESPONSE_B      = 0x51;
  static uint8_t const CMD_FAST_RESPONSE_C      = 0x53;
  static uint8_t const CMD_FAST_RESPONSE_D      = 0x57;
  static uint8_t const CMD_TX_FIFO_WRITE        = 0x66;
  static uint8_t const CMD_RX_FIFO_READ         = 0x77;

  static uint8_t const REPLY_CTS                = 0xff;
  static auto    const CTS_RETRIES              = 2500;

  static uint8_t const INT_STATUS_CHIP_INT_STATUS               = 0x04;
  static uint8_t const INT_STATUS_MODEM_INT_STATUS              = 0x02;
  static uint8_t const INT_STATUS_PH_INT_STATUS                 = 0x01;

  static uint8_t const INT_STATUS_FILTER_MATCH                  = 0x80;
  static uint8_t const INT_STATUS_FILTER_MISS                   = 0x40;
  static uint8_t const INT_STATUS_PACKET_SENT                   = 0x20;
  static uint8_t const INT_STATUS_PACKET_RX                     = 0x10;
  static uint8_t const INT_STATUS_CRC_ERROR                     = 0x08;
  static uint8_t const INT_STATUS_TX_FIFO_ALMOST_EMPTY          = 0x02;
  static uint8_t const INT_STATUS_RX_FIFO_ALMOST_FULL           = 0x01;

  static uint8_t const INT_STATUS_INVALID_SYNC                  = 0x20;
  static uint8_t const INT_STATUS_RSSI_JUMP                     = 0x10;
  static uint8_t const INT_STATUS_RSSI                          = 0x08;
  static uint8_t const INT_STATUS_INVALID_PREAMBLE              = 0x04;
  static uint8_t const INT_STATUS_PREAMBLE_DETECT               = 0x02;
  static uint8_t const INT_STATUS_SYNC_DETECT                   = 0x01;
  static uint8_t const INT_STATUS_CAL                           = 0x40;
  static uint8_t const INT_STATUS_FIFO_UNDERFLOW_OVERFLOW_ERROR = 0x20;
  static uint8_t const INT_STATUS_STATE_CHANGE                  = 0x10;
  static uint8_t const INT_STATUS_CMD_ERROR                     = 0x08;
  static uint8_t const INT_STATUS_CHIP_READY                    = 0x04;
  static uint8_t const INT_STATUS_LOW_BATT                      = 0x02;
  static uint8_t const INT_STATUS_WUT                           = 0x01;

  static uint8_t const CONDITION_RX_START_IMMEDIATE    = 0x00;

  static uint8_t const DEVICE_STATE_NO_CHANGE          = 0x00;
  static uint8_t const DEVICE_STATE_SLEEP              = 0x01;
  static uint8_t const DEVICE_STATE_SPI_ACTIVE         = 0x02;
  static uint8_t const DEVICE_STATE_READY              = 0x03;
  static uint8_t const DEVICE_STATE_ALSO_READY         = 0x04;
  static uint8_t const DEVICE_STATE_TUNE_TX            = 0x05;
  static uint8_t const DEVICE_STATE_TUNE_RX            = 0x06;
  static uint8_t const DEVICE_STATE_TX                 = 0x07;
  static uint8_t const DEVICE_STATE_RX                 = 0x08;

  static uint8_t const CONDITION_TX_COMPLETE_STATE     = 0xf0;
  static uint8_t const CONDITION_RETRANSMIT_NO         = 0x00;
  static uint8_t const CONDITION_RETRANSMIT_YES        = 0x04;
  static uint8_t const CONDITION_START_IMMEDIATE       = 0x00;
  static uint8_t const CONDITION_START_AFTER_WUT       = 0x01;

  static uint16_t const PROPERTY_GLOBAL_XO_TUNE         = 0x0000;
  static uint16_t const PROPERTY_GLOBAL_CLK_CFG         = 0x0001;
  static uint16_t const PROPERTY_GLOBAL_LOW_BATT_THRESH = 0x0002;
  static uint16_t const PROPERTY_GLOBAL_CONFIG          = 0x0003;
  static uint16_t const PROPERTY_GLOBAL_WUT_CONFIG      = 0x0004;
  static uint16_t const PROPERTY_GLOBAL_WUT_M_15_8      = 0x0005;
  static uint16_t const PROPERTY_GLOBAL_WUT_M_7_0       = 0x0006;
  static uint16_t const PROPERTY_GLOBAL_WUT_R           = 0x0007;
  static uint16_t const PROPERTY_GLOBAL_WUT_LDC         = 0x0008;
  static uint16_t const PROPERTY_PKT_FIELD_3_LENGTH_7_0 = 0x1216;
  static uint16_t const PROPERTY_MATCH_VALUE_1          = 0x3000;
  static uint16_t const PROPERTY_MATCH_MASK_1           = 0x3001;
  static uint16_t const PROPERTY_MATCH_CTRL_1           = 0x3002;
  static uint16_t const PROPERTY_MATCH_VALUE_2          = 0x3003;
  static uint16_t const PROPERTY_MATCH_MASK_2           = 0x3004;
  static uint16_t const PROPERTY_MATCH_CTRL_2           = 0x3005;
  static uint16_t const PROPERTY_MATCH_VALUE_3          = 0x3006;
  static uint16_t const PROPERTY_MATCH_MASK_3           = 0x3007;
  static uint16_t const PROPERTY_MATCH_CTRL_3           = 0x3008;
  static uint16_t const PROPERTY_MATCH_VALUE_4          = 0x3009;
  static uint16_t const PROPERTY_MATCH_MASK_4           = 0x300a;
  static uint16_t const PROPERTY_MATCH_CTRL_4           = 0x300b;

  static uint8_t const SYNC_LENGTH    = 4;
  static uint8_t const HEADER_LENGTH  = 4;

  static uint8_t const RxMask[ HEADER_LENGTH ];
  static uint8_t const RxHeader[ HEADER_LENGTH ];
  static uint8_t const SyncWord[ SYNC_LENGTH ];

public:
  TSi4463( char const* const Name, GPIO_TypeDef *const SDN_Port, uint16_t const SDN_Pin, GPIO_TypeDef *const SEL_Port, uint16_t const SEL_Pin );

  bool Init();
  void Reset();
  void Standby();
  void Receive();
  void Transmit( void const *const Data, uint8_t const Length );
  void Transmit0();
  void SetMode( uint8_t const Mode );
  void SetChannel( uint8_t const Channel );
  void SetDataRate( uint8_t const DataRate );
  void ClearRxFifo();
  void ClearTxFifo();
  uint8_t Interrupt();
  uint8_t ReadRxPacket( uint8_t *const Buffer, uint8_t const MaxLen );
  int16_t GetAfcFreqOffset();
  int16_t GetTemperature();
  uint16_t GetVoltage();

  void SetTxPower( uint8_t const TxPower );
  void SetOscTune( uint8_t const Tune );
  void SetMyAddress( uint8_t const Address );
  void Enable23mHzOnGpio0();

  uint8_t GetSyncRssi() const
  {
    return SyncRssi;
  }

private:
  uint16_t GetDeviceType();
  void Command( uint8_t const Cmd, uint8_t const *const TxBuf, uint8_t const TxLen, uint8_t *const RxBuf, uint8_t const RxLen );
  void SetProperties( uint16_t const Property, uint8_t const *const Values, uint8_t const Count );
  void GetProperties( uint16_t const Property, uint8_t *const Values, uint8_t const Count );
  void LoadRadioConfig( uint8_t const DataRate );

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

private:
  uint8_t Channel;
  uint8_t SyncRssi;
  int16_t AfcFreqOffset;
  uint16_t const PinSDN;
  uint16_t const PinSEL;
  char const* const Name;
  GPIO_TypeDef *const PortSDN;
  GPIO_TypeDef *const PortSEL;
};
typedef TSi4463 TRadio;

#endif // SI4463_H_
