#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "spi.h"

#include "oled.h"

#define NPGS    8
#define NSEG    128

static int      _page, _segment;

static uint8_t  _DspRam[NPGS * NSEG];

// SSD1306 OLED display'e komut gönderir
void OLED_Command(uint8_t cmd)
{
  IO_Write(IOP_OLED_DC, 0);     // komut
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 0);     // CS=0 aktif
#endif  

  SPI_Data(cmd);
  
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 1);     // CS=0 pasif
#endif  
}

// OLED display'e pixel data gönderir
// Her data byte 8 pixele karþýlýk gelir
void OLED_Data(uint8_t data)
{
  IO_Write(IOP_OLED_DC, 1);     // data
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 0);     // CS=0 aktif
#endif 

  SPI_Data(data);
  
  _DspRam[_page * NSEG + _segment] = data;
  
  if (++_segment >= NSEG)
    _segment = 0;
  
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 1);     // CS=0 pasif
#endif  
}

void OLED_SetPage(uint8_t page)
{
  page &= 0x07;
  
  _page = page;
  OLED_Command(0xB0 | page);
}

void OLED_SetSegment(uint8_t segment)
{
  segment &= 0x7F;
  
  _segment = segment;
  
  OLED_Command(segment & 0x0F);
  OLED_Command(0x10 | (segment >> 4));
}

void OLED_FillPage(uint8_t page, uint8_t ch)
{
  int i;
  
  OLED_SetPage(page);
  OLED_SetSegment(0);
  
  for (i = 0; i < NSEG; ++i)
    OLED_Data(ch);
}

void OLED_FillDisplay(uint8_t ch)
{
  int page;
  
  for (page = 0; page < NPGS; ++page)
    OLED_FillPage(page, ch);
}

void OLED_ClearDisplay(void)
{
  OLED_FillDisplay(0);
}

void OLED_Start(int bRotate)
{
  SPI_Start();
  
#ifdef IO_OLED_GND
  IO_Write(IOP_OLED_GND, 0);
  IO_Init(IOP_OLED_GND, IO_MODE_OUTPUT);
#endif  

#ifdef IO_OLED_VDD
  IO_Write(IOP_OLED_VDD, 1);
  IO_Init(IOP_OLED_VDD, IO_MODE_OUTPUT);
#endif  
  
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 1);
  IO_Init(IOP_OLED_CS, IO_MODE_OUTPUT);
#endif  
  
  IO_Init(IOP_OLED_DC, IO_MODE_OUTPUT);
  
#ifdef IO_OLED_RES
  // SSD1306 reset pulse
  IO_Write(IOP_OLED_RES, 0);
  IO_Init(IOP_OLED_RES, IO_MODE_OUTPUT);
  DelayMs(10);
  IO_Write(IOP_OLED_RES, 1);
#endif  
  DelayMs(80);
  
  OLED_Command(0xAE);   // Display off
  
  OLED_Command(0xD4);   
  OLED_Command(0x80);   
  
  OLED_Command(0xD3);   
  OLED_Command(0x00);   
  
  OLED_Command(0x40);   

  OLED_Command(0x8D);   
  OLED_Command(0x14);   
  
  if (bRotate) {
    OLED_Command(0xA0);   
    OLED_Command(0xC0);   
  }
  else {
    OLED_Command(0xA1);   
    OLED_Command(0xC8);   
  }
  
  OLED_Command(0xDA);   
  OLED_Command(0x12);   
  
  OLED_Command(0xDA);   
  OLED_Command(0x12);   

  OLED_Command(0x81);   
  OLED_Command(0xCF);   

  OLED_Command(0xD9);   
  OLED_Command(0xF1);   

  OLED_Command(0xDB);   
  OLED_Command(0x40);   

  OLED_Command(0xA4);   
  OLED_Command(0xA6);   
  OLED_Command(0xAF);   
  
  OLED_ClearDisplay();
}

void OLED_SetPixel(int x, int y, int c)
{
  uint8_t page, bitIdx, val;
  
  x &= 0x7F;
  y &= 0x3F;
  
  page = y >> 3;
  //page = y / 8;
  
  bitIdx = y & 7;
  //bitIdx = y % 8;
  
  val = _DspRam[page * NSEG + x];
  
  switch (c) {
  case OLED_SETPIXEL:
    val |= (1 << bitIdx);
    break;
    
  case OLED_CLRPIXEL:
    val &= ~(1 << bitIdx);
    break;
    
  case OLED_INVPIXEL:
    val ^= (1 << bitIdx);
    break;
  }
  
  OLED_SetPage(page);
  OLED_SetSegment(x);
  OLED_Data(val);
}

