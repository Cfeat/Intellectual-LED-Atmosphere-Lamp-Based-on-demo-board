
#ifndef SHT3X_I2C_HAL_H
#define SHT3X_I2C_HAL_H

//-- Includes ------------------------------------------------------------------
#include "main.h"

//-- Enumerations --------------------------------------------------------------

// I2C acknowledge
typedef enum{
  ACK  = 0,
  NACK = 1,
}etI2cAck;

// Error codes
typedef enum{
  NO_ERROR       = 0x00, // no error
  ACK_ERROR      = 0x01, // no acknowledgment error
  CHECKSUM_ERROR = 0x02, // checksum mismatch error
  TIMEOUT_ERROR  = 0x04, // timeout error
  PARM_ERROR     = 0x80, // parameter out of range error
}etError;

//IO方向
#define SDA_IN()       I2C_SDA_IN()
#define SDA_OUT()      I2C_SDA_OUT()

//IO操作函数	 
#define IIC_SCL_H      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
#define IIC_SCL_L      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
#define IIC_SDA_H      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)
#define IIC_SDA_L      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)

#define READ_SDA       I2C_SDA_READ()  //输入SDA 
#define READ_SCL       I2C_SCL_READ()  //输入SCL 


typedef unsigned char   u8t;      ///< range: 0 .. 255
typedef signed char     i8t;      ///< range: -128 .. +127
                                      
typedef unsigned short  u16t;     ///< range: 0 .. 65535
typedef signed short    i16t;     ///< range: -32768 .. +32767
                                      
typedef unsigned long   u32t;     ///< range: 0 .. 4'294'967'295
typedef signed long     i32t;     ///< range: -2'147'483'648 .. +2'147'483'647
                                      
typedef float           ft;       ///< range: +-1.18E-38 .. +-3.39E+38
typedef double          dt;       ///< range:            .. +-1.79E+308

typedef enum{
	FALSE = 0,
	TRUE = 1
}bt;

//==============================================================================
void I2c_Init(void);
//==============================================================================
// Initializes the ports for I2C interface.
//------------------------------------------------------------------------------

//==============================================================================
void I2c_StartCondition(void);
//==============================================================================
// Writes a start condition on I2C-Bus.
//------------------------------------------------------------------------------
// remark: Timing (delay) may have to be changed for different microcontroller.
//       _____
// SDA:       |_____
//       _______
// SCL:         |___

//==============================================================================
void I2c_StopCondition(void);
//==============================================================================
// Writes a stop condition on I2C-Bus.
//------------------------------------------------------------------------------
// remark: Timing (delay) may have to be changed for different microcontroller.
//              _____
// SDA:   _____|
//            _______
// SCL:   ___|

//==============================================================================
etError I2c_WriteByte(u8t txByte);
//==============================================================================
// Writes a byte to I2C-Bus and checks acknowledge.
//------------------------------------------------------------------------------
// input:  txByte       transmit byte
//
// return: error:       ACK_ERROR = no acknowledgment from sensor
//                      NO_ERROR  = no error
//
// remark: Timing (delay) may have to be changed for different microcontroller.

//==============================================================================
etError I2c_ReadByte(u8t *rxByte, etI2cAck ack, u8t timeout);
//==============================================================================
// Reads a byte on I2C-Bus.
//------------------------------------------------------------------------------
// input:  ack          Acknowledge: ACK or NACK
//
// return: rxByte
//
// remark: Timing (delay) may have to be changed for different microcontroller.

//==============================================================================
etError I2c_WaitWhileClockStreching(u8t timeout);
//==============================================================================

etError I2c_GeneralCallReset(void);

void Delay_us(u32t nbrOfUs);
#endif
