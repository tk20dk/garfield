#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <array>
#include <cstdarg>
#include "stm32f0xx_hal.h"
#include "usbd_cdc_if.h"


void UartPrintf( char const *const Format, ... );
char const* UsbGetLine();
char const* UartGetLine();

inline void SetPin( GPIO_TypeDef *const Port, uint32_t const Pin )
{
  Port->BSRR = Pin;
}
inline void ResetPin( GPIO_TypeDef *const Port, uint32_t const Pin )
{
  Port->BSRR = ( Pin << 16U );
}
inline bool ReadPin( GPIO_TypeDef *const Port, uint32_t const Pin )
{
  return (( Port->IDR & Pin ) != 0U );
}

#endif /* SYSTEM_H_ */
