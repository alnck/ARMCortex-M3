#ifndef _OLED_H
#define _OLED_H

enum {
  OLED_CLRPIXEL,
  OLED_SETPIXEL,
  OLED_INVPIXEL
};

void OLED_SetPage(uint8_t page);
void OLED_SetSegment(uint8_t segment);
void OLED_Start(int bRotate);
void OLED_SetPixel(int x, int y, int c);

#endif