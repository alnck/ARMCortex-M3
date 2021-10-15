#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "keyboard.h"
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
  
  // Console baþlangýç
  Sys_ConsoleInit();
  
  // Matrix keyboard baþlangýç
  KB_Init();
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
  
  // y = A * sin(2 * pi * x);
  for (x = 0; x < 128; ++x) {
    fx = 32 + 30.0 * sin(2 * pi * (x / 50.0));
    y = 63 - (int)(fx + 0.5);
    OLED_SetPixel(x, y, OLED_SETPIXEL);
  }
}

void Task_Print(void)
{
  static unsigned count;
  int row, col;
  
  OLED_GetCursor(&row, &col);
  OLED_SetCursor(0, 0);
  printf("SAYI:%10u", ++count);
  OLED_SetCursor(row, col);
}

void Task_DisplayClock(void)
{
  static clock_t t0, t1;
  int row, col;
  
  t1 = clock();
  
  if (t1 != t0) {
    OLED_GetCursor(&row, &col);
    OLED_SetCursor(0, 0);
    printf("Clock:%10u\r", t1);
    OLED_SetCursor(row, col);
    t0 = t1;
  }
}

void OLED_Test(void)
{
  int c;
  
  OLED_Start(1);
    
  OLED_SetPixel(0, 0, OLED_SETPIXEL);
  OLED_SetPixel(0, 5, OLED_SETPIXEL);
  OLED_SetPixel(127, 63, OLED_SETPIXEL);
  OLED_SetPixel(61, 19, OLED_SETPIXEL);
  OLED_SetPixel(61, 19, OLED_INVPIXEL);
  OLED_SetPixel(61, 19, OLED_INVPIXEL);
  OLED_SetPixel(0, 5, OLED_CLRPIXEL);

  OLED_SetPage(2);
  OLED_SetSegment(26);
  
  OLED_Data(0x7E);
  OLED_Data(0x11);
  OLED_Data(0x11);
  OLED_Data(0x11);
  OLED_Data(0x7E);
  
  c = OLED_GetPixel(0, 0);
  c = OLED_GetPixel(0, 1);
  
  OLED_Line(37, 11, 93, 55, OLED_SETPIXEL);
  OLED_Line(127, 0, 0, 63, OLED_SETPIXEL);
  
  OLED_Circle(61, 30, 15, OLED_SETPIXEL);
  OLED_Circle(41, 17, 6, OLED_SETPIXEL);
  
  DrawSin();
  
  OLED_PixelData(5, 40, OLED_SETPIXEL);
  OLED_PixelData(6, 39, OLED_SETPIXEL);
  OLED_PixelData(7, 38, OLED_SETPIXEL);
  OLED_PixelData(8, 37, OLED_SETPIXEL);
  OLED_UpdateDisplay();

  /*
  for (c = 0; c < NPGS * NSEG; ++c)
    _DspRam[c] = 0xAA;
  
  OLED_UpdateDisplay();  
  */
  
  OLED_Scroll(1);
  OLED_Scroll(2);
  OLED_Scroll(1);

  OLED_SetFont(FNT_LARGE);
  OLED_SetCursor(3, 13);
  OLED_PutChar('D');
  OLED_PutChar('e');
  OLED_PutChar('n');
  OLED_PutChar('e');
  OLED_PutChar('m');
  OLED_PutChar('e');
}  
  
void DisplayUniqueID(void)
{
  const uint32_t *pID = (const uint32_t *)0x1FFFF7E8;
  
  OLED_SetCursor(1, 0);
  printf("Uniq_ID:%08X", pID[2]);
  printf("%08X", pID[1]);
  printf("%08X\n", pID[0]);
}

int main()
{
  
  // Baþlangýç yapýlandýrmalarý
  init();

  //OLED_Test();
  
  OLED_SetFont(FNT_LARGE);
  /*
  printf("Hello, world!\n");
  OLED_SetFont(FNT_SMALL);
  OLED_SetCursor(2, 0);
  printf("Merhaba dünya!\n");
  
  printf("ABCÇDEFGÐHIÝJKLMNOÖPRSÞTUÜVYZ\n");
  printf("abcçdefgðhýijklmnoöprsþtuüvyz\n");
  printf("012345678901234567890123456\n");
  OLED_SetFont(FNT_LARGE);
  */
  
  DisplayUniqueID();
  
  printf("Waiting...\r");
  DelayMs(10000);
  printf("          \r");
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    Task_Print();
    //Task_DisplayClock();
    
    //printf("g_Key: %2d\r", g_Key);
    if (_kbhit()) {
      printf("%c", _getch());
    }
  }
  
  //return 0;
}


