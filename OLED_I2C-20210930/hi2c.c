#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "hi2c.h"

#define HI2C_TIMEOUT    1000

static I2C_TypeDef     *_i2c[] = {
  I2C1,
  I2C2,
};

enum { FALSE, TRUE };

// Hardware I2C çevresel birimi baþlatma fonksiyonu
void HI2C_Init(int nI2C, int bRate)
{
  I2C_InitTypeDef i2cInit;
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  if (nI2C == I2C_1) {
    // 2) SCL ve SDA I/O uçlarý yapýlandýrýlacak
    IO_Init(IOP_I2C1_SCL, IO_MODE_ALTERNATE_OD);
    IO_Init(IOP_I2C1_SDA, IO_MODE_ALTERNATE_OD);

    // 1) I2C çevresel birim clock aktif olmalý
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  }
  else if (nI2C == I2C_2) {
    // 2) SCL ve SDA I/O uçlarý yapýlandýrýlacak
    IO_Init(IOP_I2C2_SCL, IO_MODE_ALTERNATE_OD);
    IO_Init(IOP_I2C2_SDA, IO_MODE_ALTERNATE_OD);
    
    // 1) I2C çevresel birim clock aktif olmalý
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  }
  
  // 3) I2C baþlangýç parametreleri yapýlandýrýlacak
  i2cInit.I2C_Ack = I2C_Ack_Enable; // ??
  i2cInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  i2cInit.I2C_ClockSpeed = bRate;
  i2cInit.I2C_DutyCycle = I2C_DutyCycle_2;
  i2cInit.I2C_Mode = I2C_Mode_I2C;
  i2cInit.I2C_OwnAddress1 = 0x55;
  
  I2C_Init(pI2C, &i2cInit);
  
  // 4) Çevresel birim aktif (on)
  I2C_Cmd(pI2C, ENABLE);
}

static int HI2C_Event(int nI2C, int event)
{
  unsigned long tmOut = HI2C_TIMEOUT;
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  do {
    if (I2C_CheckEvent(pI2C, event) == SUCCESS)
      break;
  } while (--tmOut);
  
  return (tmOut != 0);
}

// Stop koþulu oluþturur
void HI2C_Stop(int nI2C)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  I2C_GenerateSTOP(pI2C, ENABLE);
}

// Start koþulu oluþturur, ardýndan konreol byte'ý gönderir
// Slave'in ACK cevabýna geri döner
// Geri dönüþ: TRUE, FALSE
int HI2C_Start(int nI2C, unsigned char ctl)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  I2C_GenerateSTART(pI2C, ENABLE);      // Start condition
  
  if (!HI2C_Event(nI2C, I2C_EVENT_MASTER_MODE_SELECT)) {
    HI2C_Stop(nI2C);
    return FALSE;
  }
  
  I2C_SendData(pI2C, ctl);     // !!
  
  return HI2C_Event(nI2C, ctl & 1 ? 
    I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :
    I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
}
                      
// I2C bus üzerinden 8-bit veri gönderir
// Geri dönüþ: Slave acknowledge TRUE: baþarýlý
int HI2C_Send(int nI2C, unsigned char val)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];

  I2C_SendData(pI2C, val);
  
  return HI2C_Event(nI2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
}

int HI2C_Recv(int nI2C, unsigned char *pVal)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  if (!HI2C_Event(nI2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
    return FALSE;
  
  *pVal = I2C_ReceiveData(pI2C);
  return TRUE;
}

// Slave'e ack gönderme veya yapýlandýrmasý
// ack: ACK biti lojik deðeri 0: olumlu
void HI2C_Acknowledge(int nI2C, int ack)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];

  I2C_AcknowledgeConfig(pI2C, ack ? DISABLE : ENABLE);
}

////////////////////////////////////////////////////////////////////////////////
// GENERIC I2C FUNCTIONS

// I2C Generic Write
// I2C üzerinden yazma cümlesi ile veri gönderir
// nI2C: I2C çevresel no
// devAdr: Slave device hardware 7-bit address (high 7-bit)
// buf: Gönderilecek verinin baþlangýç adresi
// len: Gönderilecek veri uzunluðu (kontrol byte hariç)
int HI2C_Write(int nI2C, unsigned char devAdr, const void *buf, int len)
{
  const unsigned char *ptr = (const unsigned char *)buf;
  
  devAdr &= 0xFE;
  
  if (!HI2C_Start(nI2C, devAdr)) {
    HI2C_Stop(nI2C);
    return FALSE;
  }
  
  while (len) {
    if (!HI2C_Send(nI2C, *ptr++)) 
      break;

    --len;    
  }
    
  HI2C_Stop(nI2C);
  return (len == 0);    
}

int HI2C_Read()
{
  return TRUE;
}



