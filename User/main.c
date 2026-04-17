/* ==============================================================================================
 * PROJECT: MP3 PLAYER & AUDIO VISUALIZER (SOUNDPOD)
 * MCU: STM32F103C8T6 (Bluepill)
 * OS: FreeRTOS
 * 
 * 
 * 1. TFT ILI9225
 *    - RST  (Reset)        -> PB0
 *    - RS   (Data/Command) -> PB1
 *    - CS   (Chip Select)  -> PB8
 *    - SCK  (Clock SPI)    -> PB13 
 *    - SDA  (MOSI SPI)     -> PB15 
 *    - VCC / GND           -> 3.3V / GND
 * 
 * 2. DFPLAYER MINI
 *    - RX                  -> TX STM32 
 *    - TX                  -> RX STM32 
 *    - DAC_R 				      -> [10k Ohm] -> PA5 
 *    - SPK1 / SPK2         -> LOA
 *    - VCC / GND           -> 5V / GND 
 * 
 * 3. WS2812B (8 BÓNG)
 *    - DIN  (Data In)      -> [220 Ohm] -> PA6 (STM32 PWM - TIM3_CH1_DMA)
 *    - VCC / GND           -> 5V / GND
 * 
 * 4. BUTTONS  
 *    - NEXT            		-> PA2
 *    - PREV 			          -> PA0
 *    - PLAY/PAUSE      		-> PA1
 *    - Nút VOL+ / VOL-     -> PA4 / PA3
 * 
 */
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

#include "dfplayer.h"
#include "button.h"     
#include "ili9225.h" 
#include "ili9225_fonts.h"

#define TOTAL_SONGS     3
#define DEFAULT_VOLUME  5 
#define MAX_VOLUME      30


#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_YELLOW    0xFFE0
#define COLOR_DARKGREY  0x7BEF

const char* songNames[TOTAL_SONGS] = {
    "1. 50 nam ve sau",    
    "2. Co minh va ta",
    "3. Quen nguoi da qua yeu"
};

typedef struct {
    uint8_t currentSong;
    uint8_t totalSongs;
    uint8_t isPlaying;
    uint8_t volume;
} DisplayData_t;


#define NUM_LEDS 8
#define WS2812_0 28   // Duty cycle cho bit 0 (~30%)
#define WS2812_1 61   // Duty cycle cho bit 1 (~68%)


uint16_t pwmData[(NUM_LEDS * 24) + 50]; 


TaskHandle_t xAudioTaskHandle = NULL;
TaskHandle_t xButtonTaskHandle = NULL;
TaskHandle_t xOledTaskHandle = NULL;
TaskHandle_t xVisualizerTaskHandle = NULL;

QueueHandle_t xAudioQueue;
QueueHandle_t xDisplayQueue;


void vTaskAudio(void *pvParameters);
void vTaskButton(void *pvParameters);
void vTaskLed(void *pvParameters);
void vTaskOled(void *pvParameters);
void vTaskVisualizer(void *pvParameters);

void Visualizer_Hardware_Init(void);
uint16_t ADC_Read_Value(void);
void WS2812_Init(void);
void WS2812_SetColor(int led_index, uint8_t r, uint8_t g, uint8_t b);
void WS2812_Update(void);


int main(void){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    SystemInit();
    

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);
    

    DF_Config();
    Button_Init();
    

    ILI9225_GPIO_t ili_pins;
    ili_pins.rst_port = GPIOB; ili_pins.rst_pin = GPIO_Pin_0; 
    ili_pins.rs_port  = GPIOB; ili_pins.rs_pin  = GPIO_Pin_1; 
    ili_pins.cs_port  = GPIOB; ili_pins.cs_pin  = GPIO_Pin_8; 
    ili_pins.led_port = GPIOB; ili_pins.led_pin = GPIO_Pin_9; 
    
    for(volatile uint32_t i=0; i<3000000; i++); 
    ILI9225_Init(&ili_pins);

    
    Visualizer_Hardware_Init(); 
    WS2812_Init();
    
   
    xAudioQueue = xQueueCreate(10, sizeof(uint8_t));
    xDisplayQueue = xQueueCreate(5, sizeof(DisplayData_t));
    
    
    xTaskCreate(vTaskAudio, "Audio", 128, NULL, 4, &xAudioTaskHandle);
    xTaskCreate(vTaskButton, "Button", 128, NULL, 3, &xButtonTaskHandle);
    xTaskCreate(vTaskOled, "Oled", 256, NULL, 2, &xOledTaskHandle); 
    xTaskCreate(vTaskLed, "Led", 64, NULL, 1, NULL);
    xTaskCreate(vTaskVisualizer, "Visual", 256, NULL, 3, &xVisualizerTaskHandle);
    
 
    vTaskStartScheduler();
    
    while(1);
}


void Visualizer_Hardware_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

   
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_55Cycles5);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

void WS2812_Init(void) {
  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

   
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

  
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 90 - 1; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; // Duty cycle m?c d?nh
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_Cmd(TIM3, ENABLE);


    for(int i = 0; i < ((NUM_LEDS * 24) + 50); i++) {
        pwmData[i] = 0;
    }
}


uint16_t ADC_Read_Value(void) {
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC1);
}

void WS2812_SetColor(int led_index, uint8_t r, uint8_t g, uint8_t b) {
    if(led_index >= NUM_LEDS) return;
    

    uint32_t color = (g << 16) | (r << 8) | b;
    int start_index = led_index * 24;
    
    for(int i = 23; i >= 0; i--) {
        if(color & (1 << i)) {
            pwmData[start_index + (23 - i)] = WS2812_1;
        } else {
            pwmData[start_index + (23 - i)] = WS2812_0;
        }
    }
}

void WS2812_Update(void) {
    
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel6); 
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&TIM3->CCR1;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pwmData;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = (NUM_LEDS * 24) + 50;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
    DMA_Cmd(DMA1_Channel6, ENABLE);
}


void vTaskVisualizer(void *pvParameters) {
    uint16_t sample;
    uint16_t max_val, min_val;
    uint16_t current_amplitude;
    int16_t display_amplitude = 0; 

    
    for(int i = 0; i < NUM_LEDS; i++) WS2812_SetColor(i, 0, 0, 0);
    WS2812_Update();

    for(;;) {
        max_val = 0;
        min_val = 4095;

        
        for(int i = 0; i < 400; i++) {
            sample = ADC_Read_Value();
            if(sample > max_val) max_val = sample;
            if(sample < min_val) min_val = sample;
        }
        
     
        current_amplitude = max_val - min_val;

        
        if (current_amplitude > display_amplitude) {
            display_amplitude = current_amplitude; 
        } else {
            display_amplitude -= 40; 
            if (display_amplitude < 0) display_amplitude = 0;
        }

       
        int leds_to_light = (display_amplitude * NUM_LEDS) / 1200; 
        if(leds_to_light > NUM_LEDS) leds_to_light = NUM_LEDS;

        
        for(int i = 0; i < NUM_LEDS; i++) {
            if(i < leds_to_light) {
                if(i <= 3)      WS2812_SetColor(i, 0, 50, 0);    
                else if(i <= 5) WS2812_SetColor(i, 50, 50, 0);   
                else            WS2812_SetColor(i, 50, 0, 0);    
            } else {
                WS2812_SetColor(i, 0, 0, 0);
            }
        }
        
      
        WS2812_Update();
        
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}



void vTaskAudio(void *pvParameters){
    uint8_t currentSong = 1;
    uint8_t currentVolume = DEFAULT_VOLUME;
    uint8_t cmd;
    uint8_t logicIsPlaying = 1;
    DisplayData_t dispData;
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    DF_SetVolume(currentVolume);
    vTaskDelay(pdMS_TO_TICKS(100));
    DF_PlayFolder(1, currentSong);
    
    dispData.currentSong = currentSong;
    dispData.totalSongs = TOTAL_SONGS;
    dispData.isPlaying = logicIsPlaying;
    dispData.volume = currentVolume;
    xQueueSend(xDisplayQueue, &dispData, 0);
    
    for(;;){
        if(xQueueReceive(xAudioQueue, &cmd, portMAX_DELAY) == pdPASS){
            uint8_t stateChanged = 0;
            switch(cmd){
                case CMD_NEXT:
                    currentSong++;
                    if(currentSong > TOTAL_SONGS) currentSong = 1;
                    DF_PlayFolder(1, currentSong);
                    logicIsPlaying = 1; stateChanged = 1; break;
                case CMD_PREV:
                    currentSong--;
                    if (currentSong < 1) currentSong = TOTAL_SONGS;
                    DF_PlayFolder(1, currentSong);
                    logicIsPlaying = 1; stateChanged = 1; break;
                case CMD_PP:
                    if(logicIsPlaying){ DF_Pause(); logicIsPlaying = 0;} 
                    else { DF_Play(); logicIsPlaying = 1;}
                    stateChanged = 1; break;
                case CMD_VOL_UP:
                    if(currentVolume <= (MAX_VOLUME - 3)){ currentVolume += 3; DF_SetVolume(currentVolume); stateChanged = 1;}
                    break;
                case CMD_VOL_DOWN:
                    if(currentVolume >= 3){ currentVolume -= 3; DF_SetVolume(currentVolume); stateChanged = 1;}
                    break;
            }
            if(stateChanged){
                dispData.currentSong = currentSong;
                dispData.isPlaying = logicIsPlaying;
                dispData.volume = currentVolume;
                xQueueSend(xDisplayQueue, &dispData, pdMS_TO_TICKS(10));
            }
        }
    }
}

void vTaskOled(void *pvParameters){
    DisplayData_t rcvData;
    char buffer[32];
    uint8_t firstDraw = 1; 
    
    const uint8_t* currentFont = Terminal11x16; 
    uint8_t f_w = currentFont[0]; 
    uint8_t f_h = currentFont[1]; 
    
    uint16_t scrW = ILI9225_GetWidth();
    uint16_t scrH = ILI9225_GetHeight();
    

    ILI9225_FillScreen(COLOR_BLACK);
    ILI9225_DrawRectangle(2, 2, scrW-2, scrH-2, COLOR_CYAN);
    char* bootTxt = "SOUNDPOD";
    ILI9225_DrawString(scrW/2 - (strlen(bootTxt)*f_w)/2, scrH/2 - 20, bootTxt, COLOR_YELLOW, COLOR_BLACK, currentFont);
    uint16_t loadY = scrH/2 + 10;
    ILI9225_DrawRectangle(scrW/2 - 52, loadY, scrW/2 + 52, loadY + 10, COLOR_WHITE);
    for(int i = 0; i <= 100; i += 20){
        ILI9225_FillRectangle(scrW/2 - 50, loadY + 2, scrW/2 - 50 + i, loadY + 8, COLOR_GREEN);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    vTaskDelay(pdMS_TO_TICKS(300));
    
    for(;;){
        if(xQueueReceive(xDisplayQueue, &rcvData, portMAX_DELAY) == pdPASS){
            if(firstDraw){
                ILI9225_FillScreen(COLOR_BLACK); 
                ILI9225_FillRectangle(0, 0, scrW, 25, COLOR_CYAN);
                char* title = "MP3 PLAYER";
                ILI9225_DrawString(scrW/2 - (strlen(title)*f_w)/2, 5, title, COLOR_BLACK, COLOR_CYAN, currentFont);
                
                ILI9225_DrawRectangle(2, 30, scrW-2, 110, COLOR_DARKGREY);
                char* npTxt = "- Now Playing -";
                ILI9225_DrawString(scrW/2 - (strlen(npTxt)*f_w)/2, 38, npTxt, COLOR_YELLOW, COLOR_BLACK, currentFont);
                
                ILI9225_DrawRectangle(2, 116, scrW-2, scrH-2, COLOR_DARKGREY);
                firstDraw = 0; 
            }
            
            ILI9225_FillRectangle(4, 58, scrW-4, 108, COLOR_BLACK); 
            char* sName = (char*)songNames[rcvData.currentSong - 1];
            int len = strlen(sName);
            if(len <= 14){
                ILI9225_DrawString(scrW/2 - (len*f_w)/2, 74, sName, COLOR_WHITE, COLOR_BLACK, currentFont);
            } else {
                int splitIndex = 14; 
                for(int i = 14; i > 5; i--){ if(sName[i] == ' '){ splitIndex = i; break;}}
                char line1[20] = {0}; char line2[20] = {0};
                strncpy(line1, sName, splitIndex); 
                int startL2 = (sName[splitIndex] == ' ') ? splitIndex + 1 : splitIndex;
                strncpy(line2, sName + startL2, 15); 
                int startX = scrW/2 - (strlen(line1)*f_w)/2; 
                ILI9225_DrawString(startX, 62, line1, COLOR_WHITE, COLOR_BLACK, currentFont);
                int indentX = startX + (3 * f_w); 
                ILI9225_DrawString(indentX, 84, line2, COLOR_WHITE, COLOR_BLACK, currentFont);
            }
            
            ILI9225_FillRectangle(5, 126, scrW-5, 126 + f_h, COLOR_BLACK);
            if(rcvData.isPlaying){
                char* pTxt = "> PLAYING <";
                ILI9225_DrawString(scrW/2 - (strlen(pTxt)*f_w)/2, 126, pTxt, COLOR_GREEN, COLOR_BLACK, currentFont); 
            } else {
                char* pTxt = "|| PAUSED ||";
                ILI9225_DrawString(scrW/2 - (strlen(pTxt)*f_w)/2, 126, pTxt, COLOR_RED, COLOR_BLACK, currentFont); 
            }
            
            ILI9225_FillRectangle(5, 156, scrW-5, 156 + f_h, COLOR_BLACK);
            sprintf(buffer, "Track: %02d/%02d", rcvData.currentSong, rcvData.totalSongs);
            ILI9225_DrawString(scrW/2 - (strlen(buffer)*f_w)/2, 156, buffer, COLOR_MAGENTA, COLOR_BLACK, currentFont);
            
            sprintf(buffer, "Vol: %02d", rcvData.volume);
            ILI9225_FillRectangle(5, 188, 5 + (strlen("Vol: 00")*f_w), 188 + f_h, COLOR_BLACK); 
            ILI9225_DrawString(10, 188, buffer, COLOR_WHITE, COLOR_BLACK, currentFont);
            
            uint16_t barX = 10 + (strlen(buffer) * f_w) + 5; 
            uint16_t barMaxW = scrW - barX - 10;             
            uint16_t barW = (rcvData.volume * barMaxW) / MAX_VOLUME; 
            ILI9225_FillRectangle(barX, 191, barX + barMaxW, 191 + 10, COLOR_DARKGREY); 
            if(rcvData.volume > 0){
                ILI9225_FillRectangle(barX, 191, barX + barW, 191 + 10, COLOR_CYAN);    
            }
        }
    }
}

void vTaskButton(void *pvParameters){
    uint8_t btnStatus;
    uint8_t lastState = CMD_NONE;
    for(;;){
        btnStatus = Button_Scan(); 
        if(btnStatus != CMD_NONE){
            if(btnStatus != lastState){
                xQueueSend(xAudioQueue, &btnStatus, pdMS_TO_TICKS(10));
                lastState = btnStatus;
            }
        }else{
            lastState = CMD_NONE;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vTaskLed(void *pvParameters){
    for(;;){
        GPIOC->ODR ^= GPIO_Pin_13;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}