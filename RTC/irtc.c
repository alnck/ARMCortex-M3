#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"

#define IRTC_FLAG       0xE3BA
#define RTC_PS  32768

volatile int g_Time = 0;
volatile int g_RtcAlarm = 0;
volatile int g_RtcOW = 0;
volatile int g_RtcChanged = 0;

// RTC Interrupt Handler
// Farkl� 3 kaynaktan ortaya ��kan kesmeleri i�leyecek
void RTC_IRQHandler(void)
{
  // RTC Second
  if (RTC_GetITStatus(RTC_IT_SEC)) { // Get Event Flag bit
    g_RtcChanged = 1;
  
    RTC_ClearITPendingBit(RTC_IT_SEC);    // Event Flag bit = 0
  }
  
  // RTC Alarm
  if (RTC_GetITStatus(RTC_IT_ALR)) { 
    g_RtcAlarm = 1;
    
    RTC_ClearITPendingBit(RTC_IT_ALR);    // Event Flag bit = 0
  }

  // RTC Overflow
  if (RTC_GetITStatus(RTC_IT_OW)) { 
    // Sayac� geni�letme �rne�i
    //++BKP_DR2;      // Asl�nda bu BKP domain'de olmal�
    g_RtcOW = 1;
    
    RTC_ClearITPendingBit(RTC_IT_OW);    // Event Flag bit = 0
  }
}

// Saniye sayac� olarak ref zaman�ndan bu yana 
// ge�en s�reye geri d�ner
uint32_t IRTC_GetTime(void)
{
  return RTC_GetCounter();
}

void IRTC_SetTime(uint32_t tmVal)
{
    // RTC'de s�ren i�lem varsa bekle
    RTC_WaitForLastTask();

    RTC_SetCounter(tmVal);

    // RTC'de s�ren i�lem varsa bekle
    RTC_WaitForLastTask();
}

void IRTC_Init(void)
{
  // PWR ve BKP birimleri saat i�areti
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
  // Backup register'lar�na eri�imi a��yoruz
  PWR_BackupAccessCmd(ENABLE);
  
  if (BKP_ReadBackupRegister(BKP_DR1) != IRTC_FLAG) {
    // Backup domain resetlenmi�
    // RTC ilk kez ba�lat�lacak (BKP domain power on reset)
    BKP_DeInit();
    
    // LSE'yi �al��t�r�yoruz
    RCC_LSEConfig(RCC_LSE_ON);
    
    // LSE �al��ana kadar bekle
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) ;
    
    // RTC clock kayna�� = LSE olsun
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    // RTC clock aktif
    RCC_RTCCLKCmd(ENABLE);
    
    // RTC register'lar�na eri�im i�in senkronlama gerekli
    RTC_WaitForSynchro();
    
    // RTC'de s�ren i�lem varsa bekle
    RTC_WaitForLastTask();
    
    // RTC prescaler ayar� (LSE periyot register'�)
    RTC_SetPrescaler(RTC_PS - 1);
    
    // ��lem tamamland�
    BKP_WriteBackupRegister(BKP_DR1, IRTC_FLAG);
  }
  else {
    // RTC daha �nce ba�lat�lm��
    // RTC register'lar�na eri�im i�in senkronlama gerekli
    RTC_WaitForSynchro();    
  }
}

void IRTC_IntConfig(void)
{
  // RTC second
  // RTC Periodic interrupt 
  // sayac�n her bir art���nda kesme alabiliyoruz
  
  // 1) Peripheral (�evresel)
  // 2) Core (NVIC)
  
  RTC_ClearITPendingBit(RTC_IT_SEC);    // 0) False interrupt �nlemi
  RTC_ITConfig(RTC_IT_SEC, ENABLE);     // 1 (�evresel kesme izni)
  
  RTC_ClearITPendingBit(RTC_IT_ALR);    // 0) False interrupt �nlemi
  RTC_ITConfig(RTC_IT_ALR, ENABLE);     // 1 (�evresel kesme izni)

  RTC_ITConfig(RTC_IT_OW, ENABLE);     // 1 (�evresel kesme izni)
  
  NVIC_SetPriority(RTC_IRQn, 3); // 2A) RTC kesme �nceli�i = 3 (0 en y�ksek �ncelik)
  NVIC_EnableIRQ(RTC_IRQn);
  
  // RTC Alarm
  
  // RTC Overflow
}

void IRTC_SetAlarm(uint32_t alrVal)
{
  RTC_SetAlarm(alrVal - 1);
}


