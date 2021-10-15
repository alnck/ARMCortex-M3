#include <stdio.h>
#include <stdint.h>

#include "stm32f10x_spi.h"
#include "system.h"
#include "io.h"

// SOFTWARE SPI

////////////////////////////////////////////////////////////////////////////////

#define SPI_PORT        SPI1

// HARDWARE SPI

// Donanýmsal SPI çev. birimini baþlatýr
void HSPI_Start(void)
{
  SPI_InitTypeDef spiInit;
  
  // 1) SPI clock aktif olacak
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  
  // 2) SPI pinleri yapýlandýrýlacak
  IO_Init(IOP_SPI_SCK, IO_MODE_ALTERNATE);
  IO_Init(IOP_SPI_MOSI, IO_MODE_ALTERNATE);
  IO_Init(IOP_SPI_MISO, IO_MODE_INPUT);
  
  // 3) SPI Init structure parametreleri yapýlandýrýlacak
  spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  // SPI mode 0
  spiInit.SPI_CPOL = SPI_CPOL_Low;
  spiInit.SPI_CPHA = SPI_CPHA_1Edge;
  spiInit.SPI_DataSize = SPI_DataSize_8b;
  spiInit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInit.SPI_Mode = SPI_Mode_Master;
  spiInit.SPI_NSS = SPI_NSS_Soft;
  
  SPI_Init(SPI_PORT, &spiInit);
  
  // 4) SPI çevresel birimini baþlat
  SPI_Cmd(SPI_PORT, ENABLE);
}

// SPI birimi üzerinden 8-bit veri gönderir ve alýr
// val: gönderilen deðer
// return: alýnan deðer
uint8_t HSPI_Data(uint8_t val)
{
  // SPI Transmit buffer boþ mu?
  while (!SPI_I2S_GetFlagStatus(SPI_PORT, SPI_I2S_FLAG_TXE)) ;
  
  SPI_I2S_SendData(SPI_PORT, val);
  
  while (!SPI_I2S_GetFlagStatus(SPI_PORT, SPI_I2S_FLAG_RXNE)) ;
  
  val = SPI_I2S_ReceiveData(SPI_PORT);
  
  return val;
}

