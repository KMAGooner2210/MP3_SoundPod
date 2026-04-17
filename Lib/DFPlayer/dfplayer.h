#ifndef __DFPLAYER_H
#define __DFPLAYER_H

#include "stm32f10x.h"

void DF_Config(void);
void DF_PlayFolder(uint8_t folder, uint8_t file);
void DF_SetVolume(uint8_t volume);
void DF_Reset(void);
void DF_Pause(void);
void DF_Play(void);


void DF_Next(void);
void DF_Prev(void);

#endif 