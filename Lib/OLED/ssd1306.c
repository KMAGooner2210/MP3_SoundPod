

#include"ssd1306.h"
#include"i2c.h"
#define SSD1306_WRITECOMMAND(command)      SSD1306_I2C_Write(SSD1306_I2C_ADDR, 0x00, (command))
#define SSD1306_WRITEDATA(data)            SSD1306_I2C_Write(SSD1306_I2C_ADDR, 0x40, (data))
#define ABS(x)   ((x) > 0 ? (x) : -(x))

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];


typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;

static SSD1306_t SSD1306;

#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26	// Cuon phai
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27 // Cuon trai
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 // Cuon nghieng sang phai + doc
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A // Cuon nghieng sang trai + doc
#define SSD1306_DEACTIVATE_SCROLL                    0x2E // Dung cuon
#define SSD1306_ACTIVATE_SCROLL                      0x2F // Bat dau cuon
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3 // Thiet lap vung cuon

#define SSD1306_NORMALDISPLAY       0xA6		// nen den chu trang
#define SSD1306_INVERTDISPLAY       0xA7		// nen trang chu den


/**
 * @brief Kich hoat cuon ngang sang phai cho cac hang duoc chi dinh 
	
 * @details Ham nay thiet lap cuon ngang sang phai cho mot vung cu the tren man hinh,tu hang bat dau den hang ket thuc
	
 * @param start_row Hang bat dau de cuon (0 den 7)
 
 * @param end_row   Hang ket thuc de cuon 0 den 7)
 
 * @return Khong co gia tri tra ve
 */
 
void SSD1306_ScrollRight(uint8_t start_row, uint8_t end_row)
{
  SSD1306_WRITECOMMAND (SSD1306_RIGHT_HORIZONTAL_SCROLL);  
  SSD1306_WRITECOMMAND (0x00);  
  SSD1306_WRITECOMMAND(start_row);  
  SSD1306_WRITECOMMAND(0X00);  
  SSD1306_WRITECOMMAND(end_row);  
  SSD1306_WRITECOMMAND(0X00);
  SSD1306_WRITECOMMAND(0XFF);
  SSD1306_WRITECOMMAND (SSD1306_ACTIVATE_SCROLL); 
}

/**
 * @brief Kich hoat cuon ngang sang trau cho cac hang duoc chi dinh

 * @details Ham nay thiet lap cuon ngang sang trai cho mot vung cu the cua man hinh, tu hang bat dau den hang ket thuc

 * @param start_row Hang bat dau de cuon (0 den 7)

 * @param end_row   Hang ket thuc de cuon 0 den 7)

 * @return Khong co gia tri tra ve
 */

void SSD1306_ScrollLeft(uint8_t start_row, uint8_t end_row)
{
  SSD1306_WRITECOMMAND (SSD1306_LEFT_HORIZONTAL_SCROLL);  
  SSD1306_WRITECOMMAND (0x00);  
  SSD1306_WRITECOMMAND(start_row);  
  SSD1306_WRITECOMMAND(0X00);  
  SSD1306_WRITECOMMAND(end_row);  
  SSD1306_WRITECOMMAND(0X00);
  SSD1306_WRITECOMMAND(0XFF);
  SSD1306_WRITECOMMAND (SSD1306_ACTIVATE_SCROLL); 
}

/**
 * @brief Kich hoat cuon cheo sang phai (doc va ngang) cho cac hang duoc chi dinh

 * @details Ham nay thiet lap cuon cheo (vua ngang sang phai vua doc) 

 * @param start_row Hang bat dau de cuon (0 den 7)

 * @param end_row   Hang ket thuc de cuon 0 den 7)

 * @return Khong co gia tri tra ve
 */

void SSD1306_Scrolldiagright(uint8_t start_row, uint8_t end_row)
{
  SSD1306_WRITECOMMAND(SSD1306_SET_VERTICAL_SCROLL_AREA);  
  SSD1306_WRITECOMMAND (0x00);  
  SSD1306_WRITECOMMAND(SSD1306_HEIGHT);

  SSD1306_WRITECOMMAND(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  SSD1306_WRITECOMMAND (0x00);
  SSD1306_WRITECOMMAND(start_row);
  SSD1306_WRITECOMMAND(0X00);
  SSD1306_WRITECOMMAND(end_row);
  SSD1306_WRITECOMMAND (0x01);
  SSD1306_WRITECOMMAND (SSD1306_ACTIVATE_SCROLL);
}

/**
 * @brief Kich hoat cuon cheo sang trai (doc va ngang) cho cac hang duoc chi dinh

 * @details Ham nay thiet lap cuon cheo (vua ngang sang trai vua doc) 

 * @param start_row Hang bat dau de cuon (0 den 7)

 * @param end_row   Hang ket thuc de cuon 0 den 7)

 * @return None
 */

void SSD1306_Scrolldiagleft(uint8_t start_row, uint8_t end_row)
{
  SSD1306_WRITECOMMAND(SSD1306_SET_VERTICAL_SCROLL_AREA);  
  SSD1306_WRITECOMMAND (0x00);  
  SSD1306_WRITECOMMAND(SSD1306_HEIGHT);

  SSD1306_WRITECOMMAND(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  SSD1306_WRITECOMMAND (0x00);
  SSD1306_WRITECOMMAND(start_row);
  SSD1306_WRITECOMMAND(0X00);
  SSD1306_WRITECOMMAND(end_row);
  SSD1306_WRITECOMMAND (0x01);
  SSD1306_WRITECOMMAND (SSD1306_ACTIVATE_SCROLL);
}

/**
 * @brief Tat chuc nang cuon tren man hinh

 * @details Ham nay gui lenh de dung moi hoat dong cuon tren man hinh

 * @return None
 */

void SSD1306_Stopscroll(void)
{
	SSD1306_WRITECOMMAND(SSD1306_DEACTIVATE_SCROLL);
}


/**
 * @brief Dat che do hien thi dao nguoc hoac binh thuong

 * @details Neu tham so i khac 0, man se hien thi o che do dao nguoc (trang thanh den va nguoc lai)
						Neu i bang 0, man hinh tro ve che do binh thuong
			
 * @param i 	neu khac 0, dat che do dao nguoc; neu bang 0, dat che do binh thuong

 * @return Khong co gia tri tra ve
 */

void SSD1306_InvertDisplay (int i)
{
  if (i) SSD1306_WRITECOMMAND (SSD1306_INVERTDISPLAY);

  else SSD1306_WRITECOMMAND (SSD1306_NORMALDISPLAY);

}


/**
 * @brief 	Ve mot bitmap len man hinh tai vi tri duoc chi dinh

 * @details Ham nay ve mot hinh anh bitmap len man hinh tai toa do (x,y)
		        kich thuoc va mau sac co dinh

 * @param   x					Toa do x cua diem bat dau (goc ben trai) cua bitmap

 * @param   y					Toa do y cua diem bat dau (goc ben phai) cua bitmap

 * @param   bitmap		Con tro den mang du lieu bitmap

 * @param   w					Chieu rong cua bitmap (tinh bang pixel)

 * @param   h   			Chieu cao cua bitmap (tinh bang pixel)

 * @param   color 		Mau cua pixel 

 * @return Khong co gia tri tra ve
 */
 
void SSD1306_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color)
{

    int16_t byteWidth = (w + 7) / 8; 
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++)
    {
        for(int16_t i=0; i<w; i++)
        {
            if(i & 7)
            {
               byte <<= 1;
            }
            else
            {
               byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
            if(byte & 0x80) SSD1306_DrawPixel(x+i, y, color);
        }
    }
}

/**
 * @brief 	Khoi tao va cau hinh man hinh SSD1306 qua I2C
 * @details Ham nay thuc hien cac buoc khoi tao man hinh theo trinh tu chuan datasheet
 * @return  1 neu thanh cong, 0 neu that bai
 */
uint8_t SSD1306_Config(void) {
    
    /* Khoi tao I2C */
    SSD1306_I2C_Config();

    /* Kiem tra thiet bi */
    if (SSD1306_IsDeviceReady() != 0) { 
        return 0;
    }

    /* Doi thiet bi san sang */
    uint32_t p = 2500;
    while(p>0)
        p--;

    /**==========--------------------------==========**/
    
    /**---- B1: BAT DAU KHOI TAO - TAT MAN HINH ----**/
    SSD1306_WRITECOMMAND(0xAE);     // Display OFF 
    
    
    /**---- B2: CAU HINH DIA CHI BO NHO ----**/
    SSD1306_WRITECOMMAND(0x20);     // Set Memory Addressing Mode
    SSD1306_WRITECOMMAND(0x00);     // Horizontal Addressing Mode (Hoac 0x01: Vertical, 0x02: Page)
    
    
    /**---- B3: CAU HINH VI TRI BAT DAU ----**/
    SSD1306_WRITECOMMAND(0x21);     // Set Column Address
    SSD1306_WRITECOMMAND(0x00);     // Column start address = 0
    SSD1306_WRITECOMMAND(0x7F);     // Column end address = 127
    
    SSD1306_WRITECOMMAND(0x22);     // Set Page Address 
    SSD1306_WRITECOMMAND(0x00);     // Page start address = 0
    SSD1306_WRITECOMMAND(0x07);     // Page end address = 7 
    
    
    /**---- B4: CAU HINH PHAN CUNG CO BAN ----**/
    SSD1306_WRITECOMMAND(0x40);     // Set Display Start Line = 0
    SSD1306_WRITECOMMAND(0xA1);     // Set Segment Re-map (0xA0: normal, 0xA1: remap)
    SSD1306_WRITECOMMAND(0xC8);     // Set COM Output Scan Direction (0xC0: normal, 0xC8: remap)
    
    
    /**---- B5: CAU HINH PHAN CUNG NANG CAO ----**/
    SSD1306_WRITECOMMAND(0xA8);     // Set Multiplex Ratio
    SSD1306_WRITECOMMAND(0x3F);     // Ratio = 64 (MUX = 64-1 = 63) cho 128x64
    
    SSD1306_WRITECOMMAND(0xDA);     // Set COM Pins Hardware Configuration
    SSD1306_WRITECOMMAND(0x12);     // Sequential, disable COM left/right remap (cho 128x64)
    
    
    /**---- B6: CAU HINH DIEN AP & TAN SO ----**/
    SSD1306_WRITECOMMAND(0x81);     // Set Contrast Control
    SSD1306_WRITECOMMAND(0x7F);     // Contrast value = 127 
    
    SSD1306_WRITECOMMAND(0xD5);     // Set Display Clock Divide Ratio/Oscillator Frequency
    SSD1306_WRITECOMMAND(0x80);     // Ratio = 1, Frequency = 8 
    
    SSD1306_WRITECOMMAND(0xD9);     // Set Pre-charge Period
    SSD1306_WRITECOMMAND(0x22);     // Phase1 = 2, Phase2 = 2
    
    SSD1306_WRITECOMMAND(0xDB);     // Set VCOMH Deselect Level
    SSD1306_WRITECOMMAND(0x20);     // VCOMH = 0.77 x VCC
    
    
    /**---- B7: KICH HOAT BO NGUON ----**/
    SSD1306_WRITECOMMAND(0x8D);     // Charge Pump Setting
    SSD1306_WRITECOMMAND(0x14);     // Enable Charge Pump
    
    
    /**---- B8: CAU HINH HIEN THI CUOI CUNG ----**/
    SSD1306_WRITECOMMAND(0xA4);     // Entire Display ON (resume to RAM content)
    SSD1306_WRITECOMMAND(0xA6);     // Set Normal Display (0xA7: Inverse)
    
    SSD1306_WRITECOMMAND(0xD3);     // Set Display Offset
    SSD1306_WRITECOMMAND(0x00);     // No offset
    
    SSD1306_WRITECOMMAND(0x2E);     // Deactivate Scrolling (thay the SSD1306_DEACTIVATE_SCROLL)
    
    
    /**---- B9: KET THUC KHOI TAO - BAT MAN HINH ----**/
    SSD1306_WRITECOMMAND(0xAF);     // Display ON


    /**========== KHOI TAO DU LIEU ==========**/
    
    /* Xoa man hinh */
    SSD1306_Fill(SSD1306_COLOR_BLACK);

    /* Cap nhat man hinh */
    SSD1306_UpdateScreen();

    /* Thiet lap gia tri mac dinh */
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;
    return 1;
}


/**
 * @brief 	Cap nhat noi dung buffer len SSD1306

 * @details Ham nay gui noi dung tu SSD1306_Buffer den man hinh hien thi
						chia thanh 8 page de truyen du lieu

 * @return  None
 */

void SSD1306_UpdateScreen(void) {
	uint8_t m;

	for (m = 0; m < 8; m++) {
		SSD1306_WRITECOMMAND(0xB0 + m);
		SSD1306_WRITECOMMAND(0x00);
		SSD1306_WRITECOMMAND(0x10);

		/* Write multi data */
		SSD1306_I2C_WriteMulti(SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
	}
}


/**
 * @brief 	Dao nguoc mau sac cua toan bo noi dung trong bo dem man hinh

 * @details Ham nay dao nguoc trang thai mau (trang thanh den va nguoc lai) trong bo dem
						va cap nhat trang thai dao nguoc

 * @return  None
 */

void SSD1306_ToggleInvert(void) {
	uint16_t i;

	/* Dao nguoc trang thai */
	SSD1306.Inverted = !SSD1306.Inverted;

	/* Dao nguoc bo nho */
	for (i = 0; i < sizeof(SSD1306_Buffer); i++) {
		SSD1306_Buffer[i] = ~SSD1306_Buffer[i];
	}
}


/**
 * @brief 	Dien toan bo man hinh bang mot mau duoc chi dinh

 * @details Ham nay dien toan bo bo dem bang gia tri mau (trang hoac den)
						de hien thi tren man hinh

 * @param   color  Mau de dien

 * @return  None
 */

void SSD1306_Fill(SSD1306_COLOR_t color) {
	/* Thiet lap bo nho */
	memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}


/**
 * @brief 	Ve mot pixel tai toa do duoc chi dinh tren man hinh

 * @details Ham nay dat mau cho 1 pixel cu the trong bo dem, 
            kiem tra xem toa do co hop le khong va ap dung trang thai dao nguoc neu can
						
 * @param   x  Toa do x cua pixel ( ÐK: < SSD1306_WIDTH)

 * @param   y  Toa do y cua pixel ( ÐK: < SSD1306_HEIGHT)

 * @color   color   Mau cua pixel

 * @return  None
 */

void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) {
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Loi neu toa do khong hop le */
		return;
	}

	/* Kiem tra neu pixel bi dao nguoc */
	if (SSD1306.Inverted) {
		color = (SSD1306_COLOR_t)!color;
	}

	/* Thiet lap mau */
	if (color == SSD1306_COLOR_WHITE) {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}


/**
 * @brief 	Thiet lap con tro viet tai toa do duoc chi dinh tren man hinh

 * @details Ham nay dat vi tri con tro de ve ky tu hoac hinh anh tiep theo
						
 * @param   x  Toa do x cua con tro ( ÐK: < SSD1306_WIDTH)

 * @param   y  Toa do y cua con tro ( ÐK: < SSD1306_HEIGHT)

 * @return  None
 */


void SSD1306_GotoXY(uint16_t x, uint16_t y) {
	/* Thiet lap con tro ghi */
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}


/**
 * @brief 	Ve mot ky tu tai vi tri con tro hien tai tren man hinh 

 * @details Ham nay ve mot ky tu ASCII su dung font chu duoc chi dinh
						bat dau tu vi tri con tro hien tai
						
 * @param   ch  Ky tu can ve (ASCII, tu 32 tro len)

 * @param   Font  Con tro den cau truc phong chu (FontDef_t) chua du lieu phong

 * @param   color Mau cua ky tu 

 * @return  Tra ve ky tu da viet neu thanh cong , 0 neu co loi (vuot qua size man)
 */

char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color) {
	
	//i: lap de duyet qua hang
	//j: lap de duyet qua cot
	//b: luu tru bitmap 
	
	
	uint32_t i, b, j;

	/* Kiem tra khong gian trong tren man hinh*/
	if (
		SSD1306_WIDTH <= (SSD1306.CurrentX + Font->FontWidth) ||
		SSD1306_HEIGHT <= (SSD1306.CurrentY + Font->FontHeight)
	) {
		/* Loi neu khong du khong gian */
		return 0;
	}

	/* Duyet qua phong chu */
	
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t) color);
			} else {
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}


	SSD1306.CurrentX += Font->FontWidth;
	return ch;
}


/**
 * @brief 	Ve mot chuoi ky tu tai vi tri con tro hien tai tren man hinh

 * @details Ham nay ve tung ky tu trong chuoi su dung font duoc chi dinh
						bat dau tu vi tri con tro hien tai
						
 * @param   str  Con tro den chuoi ky tu can ve (ket thuc bang null)

 * @param   Font  Con tro den cau truc phong chu (FontDef_t) 

 * @param   color Mau cua chuoi ky tu 

 * @return  Tra ve 0 neu thanh cong, hoac ky tu gay loi neu that bai (vuot qua kich thuoc man hinh)
 */

char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	/* Ve tung ky tu */
	while (*str) {
		
		if (SSD1306_Putc(*str, Font, color) != *str) {
			return *str;
		}

		str++;
	}

	return *str;
}

/**
 * @brief 	Ve mot duong thang tren man hinh

 * @details Ham nay su dung thuat toan Bresenham de ve duong thang tu diem (x0, y0) den
						(x1, y1) voi mau duoc chi dinh
						
 * @param   x0  Toa do x cua diem bat dau

 * @param   y0  Toa do y cua diem bat dau

 * @param   x1  Toa do x cua diem ket thuc

 * @param   y1  Toa do y cua diem ket thuc

 * @param   c  Mau cua duong thang

 * @return  None
 */

void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c) {
	int16_t dx, dy, sx, sy, err, e2, i, tmp;

	/* Kiem tra tran */
	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}
	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}
	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}
	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}

	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Duong thang dung */
		for (i = y0; i <= y1; i++) {
			SSD1306_DrawPixel(x0, i, c);
		}
		return;
	}

	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Duong ngang  */
		for (i = x0; i <= x1; i++) {
			SSD1306_DrawPixel(i, y0, c);
		}

		return;
	}

	while (1) {
		SSD1306_DrawPixel(x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}


/**
 * @brief 	Ve mot hinh chu nhat rong tren man hinh

 * @details Ham nay ve vien cua mot hinh chu nhat bang cach ve
						4 duong thang bao quanh
						
 * @param   x  Toa do x cua goc tren ben trai

 * @param   y  Toa do y cua goc tren ben trai

 * @param   w  Chieu rong cua hinh chu nhat

 * @param   h Chieu cao cua hinh chu nhat

 * @param   c  Mau cua hinh chu nhat

 * @return  None
 */

void SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {

	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		return;
	}


	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}

	/* Ve 4 duong */
	SSD1306_DrawLine(x, y, x + w, y, c);         /* Duong tren */
	SSD1306_DrawLine(x, y + h, x + w, y + h, c); /* Duong duoi */
	SSD1306_DrawLine(x, y, x, y + h, c);         /* Duong trai */
	SSD1306_DrawLine(x + w, y, x + w, y + h, c); /* Duong phai */
}


/**
 * @brief 	Ve mot hinh chu nhat kin tren man hinh

 * @details Ham nay ve mot hinh chu nhat kin bang cach ve nhieu duong ngang
						song song de lap day vung duoc chi dinh
						
 * @param   x  Toa do x cua goc tren ben trai

 * @param   y  Toa do y cua goc tren ben trai

 * @param   w  Chieu rong cua hinh chu nhat

 * @param   h Chieu cao cua hinh chu nhat

 * @param   c  Mau cua hinh chu nhat

 * @return  None
 */

void SSD1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	uint8_t i;


	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		return;
	}


	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}

	/* Ve cac duong*/
	for (i = 0; i <= h; i++) {
		/* Ve duong ngang */
		SSD1306_DrawLine(x, y + i, x + w, y + i, c);
	}
}


/**
 * @brief 	Ve mot hinh tam giac rong tren man hinh

 * @details Ham nay ve vien cua mot hinh tam giac bang cach ve 
						3 duong thang noi 3 dinh duoc chi dinh
						
 * @param   x1  Toa do x cua dinh thu nhat

 * @param   y1  Toa do y cua dinh thu nhat

 * @param   x2  Toa do x cua dinh thu hai

 * @param   y2  Toa do y cua dinh thu hai

 * @param   x3  Toa do x cua dinh thu ba

 * @param   y3  Toa do y cua dinh thu ba

 * @param   color  Mau cua hinh tam giac

 * @return  None
 */
 
void SSD1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color) {
	/* Draw lines */
	SSD1306_DrawLine(x1, y1, x2, y2, color);
	SSD1306_DrawLine(x2, y2, x3, y3, color);
	SSD1306_DrawLine(x3, y3, x1, y1, color);
}

/**
 * @brief 	Ve mot hinh tam giac kin tren man hinh

 * @details Ham nay su dung thuat toan de lap day hinh tam giac bang
						cach ve cac duong thang tu mot canh den dinh doi dien
						
 * @param   x1  Toa do x cua dinh thu nhat

 * @param   y1  Toa do y cua dinh thu nhat

 * @param   x2  Toa do x cua dinh thu hai

 * @param   y2  Toa do y cua dinh thu hai

 * @param   x3  Toa do x cua dinh thu ba

 * @param   y3  Toa do y cua dinh thu ba

 * @param   color  Mau cua hinh tam giac

 * @return  None
 */
 
void SSD1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color) {
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
					yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
					curpixel = 0;

	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		SSD1306_DrawLine(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}


/**
 * @brief 	Ve mot hinh tron rong tren man hinh

 * @details Ham nay su dung thuat toan Bresenham de ve vien hinh tron
						voi tam va ban kinh duoc chi dinh
						
 * @param   x0  Toa do x cua tam hinh tron

 * @param   y0  Toa do y cua tam hinh tron

 * @param   r  	Ban kinh cua hinh tron

 * @param   c   Mau cua hinh trong

 * @return  None
 */

void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawPixel(x0 + x, y0 + y, c);
        SSD1306_DrawPixel(x0 - x, y0 + y, c);
        SSD1306_DrawPixel(x0 + x, y0 - y, c);
        SSD1306_DrawPixel(x0 - x, y0 - y, c);

        SSD1306_DrawPixel(x0 + y, y0 + x, c);
        SSD1306_DrawPixel(x0 - y, y0 + x, c);
        SSD1306_DrawPixel(x0 + y, y0 - x, c);
        SSD1306_DrawPixel(x0 - y, y0 - x, c);
    }
}


/**
 * @brief 	Ve mot hinh tron kin tren man hinh

 * @details Ham nay su dung thuat toan Bresenham de ve hinh trong day
						bang cach ve duong ngang lap day ben trong hinh tron
						
 * @param   x0  Toa do x cua tam hinh tron

 * @param   y0  Toa do y cua tam hinh tron

 * @param   r  	Ban kinh cua hinh tron

 * @param   c   Mau cua hinh trong

 * @return  None
 */

void SSD1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);
    SSD1306_DrawLine(x0 - r, y0, x0 + r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
        SSD1306_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);

        SSD1306_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
        SSD1306_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}


/**
 * @brief 	Xoa toan bo man hinh

 * @details Ham nay lam day man hinh bang mau den va cap nhat noi dung len man hinh
						
 * @return  None
 */

void SSD1306_Clear (void)
{
	SSD1306_Fill (0);
    SSD1306_UpdateScreen();
}


/**
 * @brief 	Bat toan bo man hinh

 * @details Ham nay gui cac lenh de kich hoat nguon va bat hien thi man hinh
						
 * @return  None
 */

void SSD1306_ON(void) {
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x14);
	SSD1306_WRITECOMMAND(0xAF);
}


/**
 * @brief 	Tat toan bo man hinh

 * @details Ham nay gui cac lenh de tat nguon va hien thi cua man hinh
						
 * @return  None
 */

void SSD1306_OFF(void) {
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x10);
	SSD1306_WRITECOMMAND(0xAE);
}



void SSD1306_I2C_Config() {
	I2C1_Config(); // Call SPL init function
	uint32_t p = 250000;
	while(p>0)
		p--;
	
}

/**
 * @brief 	Kiem tra xem SSD1306 co san sang khong

 * @details Ham nay gui tin hieu START va dia chi thiet bi de kiem tra SSD1306 co phan hoi khong

 * @return  0 neu thiet bi san sang (ACK), 1 neu xay ra timeout
 */

uint8_t SSD1306_IsDeviceReady(void) {
    __IO uint32_t  timeout = 1000;

    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((timeout--) == 0) return 1; // Timeout error
    }

    
    I2C_Send7bitAddress(I2C1, SSD1306_I2C_ADDR, I2C_Direction_Transmitter); // Transmit to check if device ACKs
    timeout = 1000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if ((timeout--) == 0) return 1; // Timeout error
    }

    
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 0; 
}



/**
 * @brief 	Ghi nhieu byte du lieu qua I2C den SSD1306

 * @details Ham nay gui mot chuoi du lieu qua I2C, bat dau bang dia chi thanh ghi
						va tiep theo la cac byte du lieu
						
 * @param   address  Dia chi I2C cua SSD1306

 * @param   reg  		Thanh ghi hoac byte dieu khien de gui du lieu

 * @param   data  	Con tro den mang du lieu can gui

 * @param   count   So luong byte du lieu can gui

 * @return  None
 */

void SSD1306_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
    __IO uint32_t timeout = 1000;

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((timeout--) == 0) return;
    }

    I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter);
    timeout = 1000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if ((timeout--) == 0) return;
    }

    I2C_SendData(I2C1, reg);
    timeout = 1000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING)) {
        if ((timeout--) == 0) return;
    }

    for (uint16_t i = 0; i < count; i++) {
        I2C_SendData(I2C1, data[i]);
        timeout = 1000;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if ((timeout--) == 0) return;
        }
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
}


/**
 * @brief 	Ghi mot byte du lieu qua I2C den SSD1306

 * @details Ham nay gui mot byte du lieu qua I2C, bat dau bang dia chi thanh ghi
						
 * @param   address  Dia chi I2C cua SSD1306

 * @param   reg  		Thanh ghi hoac byte dieu khien de gui du lieu

 * @param   data  	Con tro den mang du lieu can gui

 * @return  None
 */
void SSD1306_I2C_Write(uint8_t address, uint8_t reg, uint8_t data) {
    __IO uint32_t timeout = 1000;

   
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((timeout--) == 0) return;
    }

    
    I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter);
    timeout = 1000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if ((timeout--) == 0) return;
    }

  
    I2C_SendData(I2C1, reg);
    timeout = 1000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING)) {
        if ((timeout--) == 0) return;
    }

   
    I2C_SendData(I2C1, data);
    timeout = 1000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if ((timeout--) == 0) return;
    }

    
    I2C_GenerateSTOP(I2C1, ENABLE);
}