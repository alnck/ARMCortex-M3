#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk geçiþ saati
  clock_t t1;           // Güncel saat deðeri
  
  t1 = clock();
  
  switch (state) {
  case I_LED_OFF:
      t0 = t1;      
      IO_Write(IOP_LED, 1);     // LED off
      state = S_LED_OFF;
      //break;    
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 0);     // LED On
    state = S_LED_ON;
    //break;    
  case S_LED_ON:
    if (t1 >= t0 + CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
  }  
}

void DrawSin(void)
{
  int x, y;
  float fx, pi = 3.14159265359;
  
  for (x = 0; x < 128; ++x) {
    fx = 32 + 30.0 * sin(2 * pi * (x / 50.0));
    y = 63 - (int)(fx + 0.5);
    OLED_SetPixel(x, y, OLED_SETPIXEL);
  }
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();

  OLED_Start(1);
  
  OLED_SetPixel(0, 0, OLED_SETPIXEL);
  OLED_SetPixel(0, 5, OLED_SETPIXEL);
  OLED_SetPixel(127, 63, OLED_SETPIXEL);
  OLED_SetPixel(61, 19, OLED_SETPIXEL);
  OLED_SetPixel(61, 19, OLED_INVPIXEL);
  OLED_SetPixel(61, 19, OLED_INVPIXEL);
  OLED_SetPixel(0, 0, OLED_CLRPIXEL);

  OLED_SetPage(2);
  OLED_SetSegment(26);
  
  OLED_Data(0x7E);
  OLED_Data(0x11);
  OLED_Data(0x11);
  OLED_Data(0x11);
  OLED_Data(0x7E);
  
  DrawSin();
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
  }
  
  //return 0;
}


