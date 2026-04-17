#include "dfplayer.h"
#include "usart_dri.h"

// G?i thu vi?n RTOS d? dùng hàm Delay
#include "FreeRTOS.h"
#include "task.h"

static void DF_SendCmd(uint8_t cmd, uint16_t param) {
    uint8_t buffer[10] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0, 0, 0, 0, 0xEF};
    uint16_t sum;
    
    buffer[3] = cmd;
    buffer[5] = (uint8_t)(param >> 8);
    buffer[6] = (uint8_t)(param & 0xFF); // Thêm & 0xFF cho an toàn tuy?t d?i
    
    // Tính Checksum
    sum = 0 - (buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5] + buffer[6]);
    buffer[7] = (uint8_t)(sum >> 8);
    buffer[8] = (uint8_t)(sum & 0xFF);
    
    // G?i 10 byte qua UART
    for(int i = 0; i < 10; i++) {
        USART1_SendChar(buffer[i]);
    }
    
    // [C?I TI?N ÐÁNG GIÁ NH?T]: B?t bu?c Delay 30ms sau M?I l?nh g?i di
    // Giúp DFPlayer có th?i gian "tiêu hoá" l?nh, ch?ng r?t l?nh 100%.
    vTaskDelay(pdMS_TO_TICKS(30)); 
}

void DF_Config(void) {
    USART1_Config();
}

void DF_PlayFolder(uint8_t folder, uint8_t file) {
    DF_SendCmd(0x0F, (folder << 8) | file);
}

void DF_SetVolume(uint8_t volume) {
    DF_SendCmd(0x06, (volume > 30) ? 30 : volume);
}

void DF_Reset(void) {
    DF_SendCmd(0x0C, 0);
    vTaskDelay(pdMS_TO_TICKS(1500)); // Reset c?n ngh? lâu hon bình thu?ng (1.5s) d? m?ch kh?i d?ng l?i
}

void DF_Pause(void) {
    DF_SendCmd(0x0E, 0);
}

// L?nh 0x0D: Playback (Resume)
void DF_Play(void) {
    DF_SendCmd(0x0D, 0);
}

// B? sung: Qua bài g?c c?a module
void DF_Next(void) {
    DF_SendCmd(0x01, 0);
}

// B? sung: Lùi bài g?c c?a module
void DF_Prev(void) {
    DF_SendCmd(0x02, 0);
}