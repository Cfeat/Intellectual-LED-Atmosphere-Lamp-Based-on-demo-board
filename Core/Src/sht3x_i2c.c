
//-- Includes ------------------------------------------------------------------
#include "sht3x_i2c.h"
#include "main.h"
#include "tim.h"
//-- Defines -------------------------------------------------------------------

void I2C_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	/*Configure GPIO pins :  PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void I2C_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	/*Configure GPIO pins :  PB14 */
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

u8t I2C_SDA_READ(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);
}

void I2C_SCL_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	/*Configure GPIO pins :  PB13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

u8t I2C_SCL_READ(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
}

//==============================================================================
void I2c_Init(void)                       /* -- adapt the init for your uC -- */
{

}

//==============================================================================
void I2c_StartCondition(void)
{
//==============================================================================
	SDA_OUT();     //sda线输出
	IIC_SDA_H;	
	Delay_us(1);	
	IIC_SCL_H;
	Delay_us(1);

	IIC_SDA_L;     //START:when CLK is high,DATA change form high to low 
	Delay_us(10);  // hold time start condition (t_HD;STA)
	IIC_SCL_L;
	Delay_us(10);	
}

//==============================================================================
void I2c_StopCondition(void)
{
//==============================================================================
	SDA_OUT();//sda线输出
	IIC_SCL_L;
	IIC_SDA_L;//STOP:when CLK is high DATA change form low to high
	Delay_us(4);
	IIC_SCL_H; 
	IIC_SDA_H;//发送I2C总线结束信号
	Delay_us(4);
}

//==============================================================================
etError I2c_WriteByte(u8t txByte)
{
//==============================================================================
	etError error = NO_ERROR;
	u8t mask;
	
	SDA_OUT(); 	    
	IIC_SCL_L;
	
	for(mask = 0x80; mask > 0; mask >>= 1)// shift bit for masking (8 times)
	{
		if((mask & txByte) == 0) 
			IIC_SDA_L; // masking txByte, write bit to SDA-Line
		else 
			IIC_SDA_H;
		
		Delay_us(1);	 // data set-up time (t_SU;DAT)
		IIC_SCL_H; // generate clock pulse on SCL
		Delay_us(5);	 // SCL high time (t_HIGH)
		IIC_SCL_L;
		Delay_us(1);	 // data hold time(t_HD;DAT)
	}
	
	SDA_IN();    //SDA设置为输入  // release SDA-line
	IIC_SCL_H;   // clk #9 for ack
	
	Delay_us(1); // data set-up time (t_SU;DAT)
	if(READ_SDA) 
		error = ACK_ERROR; // check ack from i2c slave
	IIC_SCL_L;
	Delay_us(20); // wait to see byte package on scope
	return error; // return error code
}

//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_L;
	Delay_us(2);
	IIC_SCL_H;
	Delay_us(2);
	IIC_SCL_L;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_H;
	Delay_us(2);
	IIC_SCL_H;
	Delay_us(2);
	IIC_SCL_L;
}
//==============================================================================
etError I2c_ReadByte(u8t *rxByte, etI2cAck ack, u8t timeout)
{
//==============================================================================
	etError error = NO_ERROR;
	u8t mask;
  *rxByte = 0x00;
	
	SDA_IN();//SDA设置为输入 // release SDA-line
	for(mask = 0x80; mask > 0; mask >>= 1) // shift bit for masking (8 times)
	{
		IIC_SCL_H; // start clock on SCL-line
		Delay_us(1); // clock set-up time (t_SU;CLK)
		error = I2c_WaitWhileClockStreching(timeout);// wait while clock streching
		Delay_us(3); // SCL high time (t_HIGH)
		if(READ_SDA) 
			*rxByte |= mask; // read bit
		IIC_SCL_L;
		Delay_us(1); // data hold time(t_HD;DAT)
	}
	if(ack == ACK) 
	{
		SDA_OUT(); 	 
		IIC_SDA_L; // send acknowledge if necessary
	}
	else
	{
		SDA_OUT(); 	 
		IIC_SDA_H; 
	}	
	Delay_us(1); // data set-up time (t_SU;DAT)
	IIC_SCL_H;  // clk #9 for ack
	Delay_us(5); // SCL high time (t_HIGH)
	IIC_SCL_L; 
//	SDA_OUT(); 	 
//	IIC_SDA=1; // release SDA-line
	Delay_us(20);  // wait to see byte package on scope
  return error; // return with no errorSample Code for SHT3x	
}

//==============================================================================
etError I2c_WaitWhileClockStreching(u8t timeout)
{
//==============================================================================
	etError error = NO_ERROR;
	while(READ_SCL== 0)
	{
		if(timeout-- == 0) return TIMEOUT_ERROR;
		Delay_us(1000);
	}
	return error;
}

//==============================================================================
etError I2c_GeneralCallReset(void)
{
//==============================================================================
    etError error;

    I2c_StartCondition();
    error = I2c_WriteByte(0x00);
    if(error == NO_ERROR) 
		error = I2c_WriteByte(0x06);

    return error;
}

void Delay_us(u32t nbrOfUs) /* -- adapt this delay for your uC -- */
{
//	u32t i, j;
//	for(j=0; j<500; j++)// 450 ~500
//	for(i = 0; i < nbrOfUs; i++)
//	{
//		__NOP(); // nop's may be added or removed for timing adjustment
//	}
	KE1_Delay_us(nbrOfUs);
}
