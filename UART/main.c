#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "uart.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlar� ba�lang��
  Sys_IoInit();
  
  // LED ba�lang��
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Konsol ba�lang��
  Sys_ConsoleInit();
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk ge�i� saati
  clock_t t1;           // G�ncel saat de�eri
  
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

void Task_Print(void)
{
  static unsigned count;
  
  printf("SAYI:%10u\n", ++count);
  UART_printf("Count: %10lu\r", count);
}

int main()
{
  // Ba�lang�� yap�land�rmalar�
  init();
    
  printf("Hello, world!\n");
  UART_puts("\n\n\nMerhaba dunya!\n");

  /*
  printf("\rBirinci SATIR");
  printf("\nikinci SATIR");
  printf("\f");
  printf("\r1. SATIR");
  printf("\n2. SATIR");
  */
  
  // G�rev �evrimi (Task Loop)
  // Co-Operative Multitasking (Yard�mla�mal� �oklu g�rev) 
  while (1)
  {
    Task_LED();
    Task_Print();

    // Echo test
    /*
    {
      unsigned char c;

      if (UART_DataReady(g_conUART)) {
        c = UART_Recv(g_conUART);
        UART_Send(g_conUART, c);
      }
    }
    */
    
    {
      unsigned char c;

      if (UART_DataReady(g_conUART)) {
        UART_putch('\n');
        do {
          c = UART_Recv(g_conUART);
          UART_putch(c);
        } while (c != '\r');

        UART_putch('\n');
      }
    }
  }
  
  //return 0;
}


