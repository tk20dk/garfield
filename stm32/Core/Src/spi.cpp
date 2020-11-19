#include <string.h>
#include "spi.h"


TSpi::TSpi( SPI_HandleTypeDef &hspi ) :
  hspi( hspi )
{
}

void TSpi::Write( uint8_t const Data )
{
  uint8_t TxData = Data;
  auto const Result = HAL_SPI_Transmit( &hspi, &TxData, 1U, SpiTimeout );
  assert_param( Result == HAL_OK );
}

// auto const Result = HAL_SPI_Transmit_DMA( &hspi, TxData, Length );
void TSpi::Write( void const *const Data, uint32_t const Length )
{
  auto TxData = const_cast< uint8_t* >( reinterpret_cast< uint8_t const* >( Data ));
  uint8_t Buffer[ 64 ];

  if(( (int)TxData & 0x00000001 ) && ( Length <= sizeof( Buffer )))
  {
    TxData = Buffer;
    memcpy( TxData, Data, Length );
  }

  auto const Result = HAL_SPI_Transmit( &hspi, TxData, Length, SpiTimeout );
  assert_param( Result == HAL_OK );

  while( HAL_SPI_GetState( &hspi ) == HAL_SPI_STATE_BUSY_TX )
  {
  }
}

uint8_t TSpi::Read()
{
  uint8_t RxData;
  auto const Result = HAL_SPI_Receive( &hspi, &RxData, 1U, SpiTimeout );
  assert_param( Result == HAL_OK );
  return RxData;
}

// auto const Result = HAL_SPI_Receive_DMA( &hspi, Data, Length );
void TSpi::Read( uint8_t *const Data, uint32_t const Length )
{
  auto const Result = HAL_SPI_Receive( &hspi, Data, Length, SpiTimeout );
  assert_param( Result == HAL_OK );

  while( HAL_SPI_GetState( &hspi ) == HAL_SPI_STATE_BUSY_RX )
  {
  }
}

uint8_t TSpi::WriteRead( uint8_t const Data )
{
  uint8_t RxData;
  uint8_t TxData = Data;
  auto const Result = HAL_SPI_TransmitReceive( &hspi, &TxData, &RxData, 1U, SpiTimeout );
  assert_param( Result == HAL_OK );
  return RxData;
}

//  auto const Result = HAL_SPI_TransmitReceive_DMA( &hspi, TxData, RxData, Length );
void TSpi::WriteRead( void const *const Data, uint8_t *const RxData, uint32_t const Length )
{
  auto TxData = const_cast< uint8_t* >( reinterpret_cast< uint8_t const* >( Data ));

  auto const Result = HAL_SPI_TransmitReceive( &hspi, TxData, RxData, Length, SpiTimeout );
  assert_param( Result == HAL_OK );

  while( HAL_SPI_GetState( &hspi ) == HAL_SPI_STATE_BUSY_TX_RX )
  {
  }
}

