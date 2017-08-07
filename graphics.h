/*
 * graphics.h
 *
 *  Created on: Aug 11, 2012
 *      Author: Mishel
 */

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "spi.h"

void screen_clear(void);
void screen_black(void);
void drawchar(int x, int y, char c);
void drawstring(int x, int y, char* c);
void invert(void);
void invertxy(unsigned int, unsigned int, unsigned int, unsigned int);
void SetPixel (unsigned int,unsigned int);
void ClearPixel(int , int);
void drawchar10x16(int, int, char);
void drawstring10x16(unsigned int, unsigned int, char *);
#endif /* GRAPHICS_H_ */
