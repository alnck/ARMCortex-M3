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
// Farklý 3 kaynaktan ortaya çýkan kesmeleri iþleyecek
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
    // Sayacý geniþletme örneði
    //++BKP_DR2;      // Aslýnda bu BKP domain'de olmalý
    g_RtcOW = 1;
    
    RTC_ClearITPendingBit(RTC_IT_OW);    // Event Flag bit = 0
  }
}

// Saniye sayacý olarak ref zamanýndan bu yana 
// geçen süreye geri döner
uint32_t IRTC_GetTime(void)
{
  return RTC_GetCounter();
}

void IRTC_SetTime(uint32_t tmVal)
{
    // RTC'de süren iþlem varsa bekle
    RTC_WaitForLastTask();

    RTC_SetCounter(tmVal);

    // RTC'de süren iþlem varsa bekle
    RTC_WaitForLastTask();
}

void IRTC_Init(void)
{
  // PWR ve BKP birimleri saat iþareti
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
  // Backup register'larýna eriþimi açýyoruz
  PWR_BackupAccessCmd(ENABLE);
  
  if (BKP_ReadBackupRegister(BKP_DR1) != IRTC_FLAG) {
    // Backup domain resetlenmiþ
    // RTC ilk kez baþlatýlacak (BKP domain power on reset)
    BKP_DeInit();
    
    // LSE'yi çalýþtýrýyoruz
    RCC_LSEConfig(RCC_LSE_ON);
    
    // LSE çalýþana kadar bekle
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) ;
    
    // RTC clock kaynaðý = LSE olsun
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    // RTC clock aktif
    RCC_RTCCLKCmd(ENABLE);
    
    // RTC register'larýna eriþim için senkronlama gerekli
    RTC_WaitForSynchro();
    
    // RTC'de süren iþlem varsa bekle
    RTC_WaitForLastTask();
    
    // RTC prescaler ayarý (LSE periyot register'ý)
    RTC_SetPrescaler(RTC_PS - 1);
    
    // Ýþlem tamamlandý
    BKP_WriteBackupRegister(BKP_DR1, IRTC_FLAG);
  }
  else {
    // RTC daha önce baþlatýlmýþ
    // RTC register'larýna eriþim için senkronlama gerekli
    RTC_WaitForSynchro();    
  }
}

void IRTC_IntConfig(void)
{
  // RTC second
  // RTC Periodic interrupt 
  // sayacýn her bir artýþýnda kesme alabiliyoruz
  
  // 1) Peripheral (çevresel)
  // 2) Core (NVIC)
  
  RTC_ClearITPendingBit(RTC_IT_SEC);    // 0) False interrupt önlemi
  RTC_ITConfig(RTC_IT_SEC, ENABLE);     // 1 (Çevresel kesme izni)
  
  RTC_ClearITPendingBit(RTC_IT_ALR);    // 0) False interrupt önlemi
  RTC_ITConfig(RTC_IT_ALR, ENABLE);     // 1 (Çevresel kesme izni)

  RTC_ITConfig(RTC_IT_OW, ENABLE);     // 1 (Çevresel kesme izni)
  
  NVIC_SetPriority(RTC_IRQn, 3); // 2A) RTC kesme önceliði = 3 (0 en yüksek öncelik)
  NVIC_EnableIRQ(RTC_IRQn);
  
  // RTC Alarm
  
  // RTC Overflow
}

void IRTC_SetAlarm(uint32_t alrVal)
{
  RTC_SetAlarm(alrVal - 1);
}


