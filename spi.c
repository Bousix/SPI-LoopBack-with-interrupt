#include "spi.h"

uint8_t spiTxBuff[SPI_TX_MAX] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};
uint8_t spiRxBuff[SPI_RX_MAX];
static volatile int spiRxCount= 0;
volatile unsigned int flag = 1;
/* configure pins used by SPI1
         * PA4 = NSS
	 * PA5 = SCK
	 * PA6 = MISO
	 * PA7 = MOSI
*/
void init_SPI1(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStruct;
  SPI_InitTypeDef SPI_InitStruct; 
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5|GPIO_Pin_4;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	
  // connect SPI1 pins to SPI alternate function
  //GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1);
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
  //Set chip select high 
  GPIOA->BSRRL |= GPIO_Pin_4; // set PE4 high
	
  // enable peripheral clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
  /* configure SPI1 in Mode 0 
   * CPOL = 0 --> clock is low when idle
   * CPHA = 0 --> data is sampled at the first edge
   */
  SPI_StructInit(&SPI_InitStruct); // set default settings 
  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        // clock is low when idle
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft ; // set the NSS management to internal and pull internal NSS high
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // SPI frequency is APB2 frequency / 4
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted MSB first
  SPI_Init(SPI1, &SPI_InitStruct); 
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable SPI1*/
  SPI_Cmd(SPI1, ENABLE);
  return;
}

void SPI1_Write(uint8_t *txBuff,int length,tSPI_Callback fct)
{
  static int i =0;
  
  SPI_I2S_ITConfig(SPI1,SPI_I2S_IT_RXNE,ENABLE);
  if (txBuff)
  {
    for (i=0;txBuff[i]; i++)
    {
      while (!flag);
      while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);
      SPI_I2S_SendData(SPI1,txBuff[i]);
      flag--;
    }
    if (fct)
    {
      //callback called in the end of the transert
      fct();
    }
  }
}

void SPI1_IRQHandler()
{
  spiRxBuff[spiRxCount] = SPI_I2S_ReceiveData(SPI1);
  spiRxCount++;
  flag++;
}
