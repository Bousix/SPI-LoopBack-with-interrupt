#ifndef SPI_H
#define SPI_H
#include "stm32f4xx_spi.h"
#include "misc.h"

#define SPI_TX_MAX 10
#define SPI_RX_MAX 10

/**
SPI Buffer
***********************************/
extern uint8_t spiTxBuff[SPI_TX_MAX];
extern uint8_t spiRxBuff[SPI_RX_MAX];

#define CHECK_SPI1_IRQ(SPI_I2S_IT,handle) if (SPI_I2S_GetITStatus(SPI1,(SPI_I2S_IT))==SET) goto handle;

typedef void (*tSPI_Callback)(void); 
extern void init_SPI1(void);
extern void SPI1_Write(uint8_t *txBuff,int length,tSPI_Callback fct);
/**
SPI Callback
*******************************/
extern void spi_TxCallback();
extern void spi_RxCallback();

#endif