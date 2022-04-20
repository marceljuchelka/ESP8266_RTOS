//-----------------------------------------------------------------------------------------------------------
// *** Obs≥uga wyúwietlaczy alfanumerycznych zgodnych z HD44780 ***
//
// - Sterowanie: tryb 4-bitowy
// - Dowolne przypisanie kaødego sygna≥u sterujπcego do dowolnego pinu mikrokontrolera
// - Praca z pinem RW pod≥πczonym do GND lub do mikrokontrolera (sprawdzanie BusyFLAG - szybkie operacje LCD)
//
//	Biblioteka ver: 1.0
//
// Pliki 			: lcd44780.c , lcd44780.h
// Mikrokontrolery 	: Atmel AVR
// Kompilator 		: avr-gcc
// èrÛd≥o 			: http://www.atnel.pl
// Data 			: 2019-04-23
// Autor 			: Miros≥aw Kardaú
//----------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mk_lcd44780.h"
#include "sdkconfig.h"



// makrodefinicje operacji na sygna≥ach sterujπcych RS,RW oraz E

#if !USE_I2C
	#define SET_RS 	PORT(LCD_RSPORT) |= (1<<LCD_RS) 				// stan wysoki na linii RS
	#define CLR_RS 	PORT(LCD_RSPORT) &= ~(1<<LCD_RS) 				// stan niski na linii RS

	#define SET_RW 	PORT(LCD_RWPORT) |= (1<<LCD_RW) 				// stan wysoki na RW - odczyt z LCD
	#define CLR_RW 	PORT(LCD_RWPORT) &= ~(1<<LCD_RW) 				// stan niski na RW - zapis do LCD

	#define SET_E 	PORT(LCD_EPORT) |= (1<<LCD_E) 					// stan wysoki na linii E
	#define CLR_E 	PORT(LCD_EPORT) &= ~(1<<LCD_E) 				// stan niski na linii E
#else
	#define SET_RS 	mpxLCD |= (1<<LCD_RS); 	SEND_I2C	// stan wysoki na linii RS
	#define CLR_RS 	mpxLCD &= ~(1<<LCD_RS); SEND_I2C	// stan niski na linii RS

	#define SET_RW 	mpxLCD |= (1<<LCD_RW); 	SEND_I2C	// stan wysoki na RW - odczyt z LCD
	#define CLR_RW 	mpxLCD &= ~(1<<LCD_RW); SEND_I2C	// stan niski na RW - zapis do LCD

	#define SET_E 	mpxLCD |= (1<<LCD_E); 	SEND_I2C	// stan wysoki na linii E
	#define CLR_E 	mpxLCD &= ~(1<<LCD_E); 	SEND_I2C	// stan niski na linii E
#endif


#if USE_RW
uint8_t check_BF(void);			// deklaracja funkcji wewnÍtrznej
#endif


#if USE_I2C == 1
//Deklaracja zmiennej na potrzeby obs≥ugi multipleksera
static uint8_t	mpxLCD;
#endif

//********************* FUNKCJE WEWN TRZNE *********************

//----------------------------------------------------------------------------------------
//
//		 Ustawienie wszystkich 4 linii danych jako WYjúcia
//
//----------------------------------------------------------------------------------------
static inline void data_dir_out(void)
{
	#if !USE_I2C
		DDR(LCD_D7PORT)	|= (1<<LCD_D7);
		DDR(LCD_D6PORT)	|= (1<<LCD_D6);
		DDR(LCD_D5PORT)	|= (1<<LCD_D5);
		DDR(LCD_D4PORT)	|= (1<<LCD_D4);
	#else
		//Zerowanie zmiennych danych (D4..D7)
		mpxLCD	&= ~(1<<LCD_D7);
		mpxLCD	&= ~(1<<LCD_D6);
		mpxLCD	&= ~(1<<LCD_D5);
		mpxLCD	&= ~(1<<LCD_D4);
		SEND_I2C;
	#endif
}

//----------------------------------------------------------------------------------------
//
//		 Ustawienie wszystkich 4 linii danych jako WEjúcia
//
//----------------------------------------------------------------------------------------
#if USE_RW
static inline void data_dir_in(void)
{
	#if !USE_I2C
		DDR(LCD_D7PORT)	&= ~(1<<LCD_D7);
		DDR(LCD_D6PORT)	&= ~(1<<LCD_D6);
		DDR(LCD_D5PORT)	&= ~(1<<LCD_D5);
		DDR(LCD_D4PORT)	&= ~(1<<LCD_D4);
	#else
		// PCF8574 wymaga ustawienia stanu wysokiego dla wejúÊ
		mpxLCD |= (1<<LCD_D7);
		mpxLCD |= (1<<LCD_D6);
		mpxLCD |= (1<<LCD_D5);
		mpxLCD |= (1<<LCD_D4);
		SEND_I2C;
    #endif
}
#endif

//----------------------------------------------------------------------------------------
//
//		 Wys≥anie po≥Ûwki bajtu do LCD (D4..D7)
//
//----------------------------------------------------------------------------------------
static inline void lcd_sendHalf(uint8_t data)
{
	#if !USE_I2C

	if (data&(1<<0)) PORT(LCD_D4PORT) |= (1<<LCD_D4); else PORT(LCD_D4PORT) &= ~(1<<LCD_D4);
	if (data&(1<<1)) PORT(LCD_D5PORT) |= (1<<LCD_D5); else PORT(LCD_D5PORT) &= ~(1<<LCD_D5);
	if (data&(1<<2)) PORT(LCD_D6PORT) |= (1<<LCD_D6); else PORT(LCD_D6PORT) &= ~(1<<LCD_D6);
	if (data&(1<<3)) PORT(LCD_D7PORT) |= (1<<LCD_D7); else PORT(LCD_D7PORT) &= ~(1<<LCD_D7);

	#else
		if (data&(1<<0)) mpxLCD |= (1<<LCD_D4); else mpxLCD &= ~(1<<LCD_D4);
		if (data&(1<<1)) mpxLCD |= (1<<LCD_D5); else mpxLCD &= ~(1<<LCD_D5);
		if (data&(1<<2)) mpxLCD |= (1<<LCD_D6); else mpxLCD &= ~(1<<LCD_D6);
		if (data&(1<<3)) mpxLCD |= (1<<LCD_D7); else mpxLCD &= ~(1<<LCD_D7);
		SEND_I2C;
	#endif
}

#if USE_RW == 1
//----------------------------------------------------------------------------------------
//
//		 Odczyt po≥Ûwki bajtu z LCD (D4..D7)
//
//----------------------------------------------------------------------------------------
static inline uint8_t lcd_readHalf(void) {

	uint8_t result=0;

	#if USE_I2C

		uint8_t res=0;

		res = RECEIVE_I2C;
		// WAØNA ZMIANA - by mirekk36
		if(res&(1<<LCD_D4)) result |= (1<<0);
		if(res&(1<<LCD_D5)) result |= (1<<1);
		if(res&(1<<LCD_D6)) result |= (1<<2);
		if(res&(1<<LCD_D7)) result |= (1<<3);

	#else

		if(PIN(LCD_D4PORT)&(1<<LCD_D4)) result |= (1<<0);
		if(PIN(LCD_D5PORT)&(1<<LCD_D5)) result |= (1<<1);
		if(PIN(LCD_D6PORT)&(1<<LCD_D6)) result |= (1<<2);
		if(PIN(LCD_D7PORT)&(1<<LCD_D7)) result |= (1<<3);

	#endif

	return result;
}
#endif

//----------------------------------------------------------------------------------------
//
//		 Zapis bajtu do wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
void _lcd_write_byte(unsigned char _data) {

	// Ustawienie pinÛw portu LCD D4..D7 jako wyjúcia
	data_dir_out();

	#if USE_RW == 1
		CLR_RW;
	#endif

	SET_E;
	lcd_sendHalf(_data >> 4);			// wys≥anie starszej czÍúci bajtu danych D7..D4
	CLR_E;

	SET_E;
	lcd_sendHalf(_data);				// wys≥anie m≥odszej czÍúci bajtu danych D3..D0
	CLR_E;

	#if USE_RW == 1

		while( (check_BF() & (1<<7)) );

	#else
		_delay_us(120);
	#endif

}

#if USE_RW == 1
//----------------------------------------------------------------------------------------
//
//		 Odczyt bajtu z wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
uint8_t _lcd_read_byte(void) {

	uint8_t result=0;
	data_dir_in();

	SET_RW;

	SET_E;

	result = (lcd_readHalf() << 4);		// odczyt starszej czÍúci bajtu z LCD D7..D4

	CLR_E;

	SET_E;
	result |= lcd_readHalf();			// odczyt m≥odszej czÍúci bajtu z LCD D3..D0
	CLR_E;

	return result;
}
#endif


#if USE_RW == 1
//----------------------------------------------------------------------------------------
//
//		 Sprawdzenie stanu Busy Flag (ZajÍtoúci wyúwietlacza)
//
//----------------------------------------------------------------------------------------
uint8_t check_BF(void) {

	CLR_RS;

	return _lcd_read_byte();
}
#endif


//----------------------------------------------------------------------------------------
//
//		 Zapis komendy do wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
void lcd_write_cmd(uint8_t cmd) {

	CLR_RS;

	_lcd_write_byte(cmd);
}

//----------------------------------------------------------------------------------------
//
//		 Zapis danych do wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
void lcd_write_data(uint8_t data) {

	SET_RS;
	_lcd_write_byte(data);
}




//**************************  FUNKCJE PRZEZNACZONE TAKØE DLA INNYCH MODU£”W  ******************


//----------------------------------------------------------------------------------------
//
//		 Wys≥anie pojedynczego znaku do wyúwietlacza LCD w postaci argumentu
//
//		 8 w≥asnych znakÛw zdefiniowanych w CGRAM
//		 wysy≥amy za pomocπ kodÛw 0x80 do 0x87 zamiast 0x00 do 0x07
//
//----------------------------------------------------------------------------------------
void lcd_char(char c) {

	lcd_write_data( ( c>=0x80 && c<=0x87 ) ? (c & 0x07) : c);
}


//----------------------------------------------------------------------------------------
//
//		 Wys≥anie stringa do wyúwietlacza LCD z pamiÍci RAM
//
//----------------------------------------------------------------------------------------
void lcd_str(char * str) {

	register char znak;
	while ( (znak=*(str++)) ) lcd_char( znak );

}

void lcd_str_al( uint8_t y, uint8_t x, char * str, uint8_t align ) {

	if( _center == align ) x = x/2 - strlen(str)/2+1;
	else if( _right == align ) x = x - strlen(str)+1;

	lcd_locate( y, x );

	register char znak;
	while ( (znak=*(str++)) ) lcd_char( znak );

}










//----------------------------------------------------------------------------------------
//
//		 Wyúwietla liczbÍ dziesiÍtnπ na wyúwietlaczu LCD
//
//----------------------------------------------------------------------------------------
void lcd_int( int32_t val ) {

	char bufor[17];
	lcd_str( itoa(val, bufor, 10) );
}

void lcd_int_al( uint8_t y, uint8_t x, int32_t val, uint8_t align ) {

	char bufor[17];
	lcd_str_al( y, x, itoa(val, bufor, 10), align );
}

void lcd_long( uint32_t val ) {

	char bufor[17];
	lcd_str( itoa(val, bufor, 10) );
}

void lcd_long_al( uint8_t y, uint8_t x, uint32_t val, uint8_t align ) {

	char bufor[17];
	lcd_str_al( y, x, itoa(val, bufor, 10), align );
}

void lcd_float(uint8_t y,uint8_t x,float val, uint8_t len,uint8_t align){
//	char buf[17];
//	sprintf(%5.2f,val);
//	dtostrf(val,3,len,buf);
//	lcd_str_al(y,x,buf,align);
}

// konwersja do postaci binarnej liczb max 32-bitowych
// ARG:
// val - liczba do konwersji
// len - iloúÊ znakÛw postaci binarnej z zerami nieznaczπcymi
void lcd_bin_al( uint8_t y, uint8_t x, uint32_t val, uint8_t len, uint8_t align ) {
	char str[len+1];
	memset( str, 0, len+1 );
	for( int8_t i=0, k=len-1; i<len; i++ ) {
		uint32_t a = val >> k;
		if( a & 0x0001 ) str[k]='1'; else str[k]='0';
		k--;
	}
//	strrev( str );
	lcd_str_al( y, x, str, align );
}

void lcd_bin( uint32_t val, uint8_t len ) {
	char str[len+1];
	memset( str, 0, len+1 );
	for( int8_t i=0, k=len-1; i<len; i++ ) {
		uint32_t a = val >> k;
		if( a & 0x0001 ) str[k]='1'; else str[k]='0';
		k--;
	}
//	strrev( str );
	lcd_str( str );
}


//----------------------------------------------------------------------------------------
//
//		 Wyúwietla liczbÍ szestnastkowπ HEX na wyúwietlaczu LCD
//
//----------------------------------------------------------------------------------------
void lcd_hex( int32_t val ) {
	char bufor[17];
	lcd_str( itoa(val, bufor, 16) );
}

void lcd_hex_al( uint8_t y, uint8_t x, int32_t val, uint8_t align ) {
	char bufor[17];
	lcd_str_al( y, x, itoa(val, bufor, 16), align );
}



//----------------------------------------------------------------------------------------
//
//		Definicja w≥asnego znaku na LCD z pamiÍci RAM
//
//		argumenty:
//		nr: 		- kod znaku w pamiÍci CGRAM od 0x80 do 0x87
//		*def_znak:	- wskaünik do tablicy 7 bajtÛw definiujπcych znak
//
//----------------------------------------------------------------------------------------
void lcd_defchar(uint8_t nr, uint8_t *def_znak) {

	register uint8_t i,c;
	lcd_write_cmd( 64+((nr&0x07)*8) );
	for(i=0;i<8;i++) {
		c = *(def_znak++);
		lcd_write_data(c);
	}
}



//----------------------------------------------------------------------------------------
//
//		Ustawienie kursora w pozycji Y-wiersz, X-kolumna
//
// 		Y = od 0 do 3
// 		X = od 0 do n
//
//		funkcja dostosowuje automatycznie adresy DDRAM
//		w zaleønoúci od rodzaju wyúwietlacza (ile posiada wierszy)
//
//----------------------------------------------------------------------------------------
void lcd_locate(uint8_t y, uint8_t x) {

	switch(y) {
		case 0: y = LCD_LINE1; break;

#if (LCD_ROWS>1)
	    case 1: y = LCD_LINE2; break; // adres 1 znaku 2 wiersza
#endif
#if (LCD_ROWS>2)
    	case 2: y = LCD_LINE3; break; // adres 1 znaku 3 wiersza
#endif
#if (LCD_ROWS>3)
    	case 3: y = LCD_LINE4; break; // adres 1 znaku 4 wiersza
#endif
	}

	lcd_write_cmd( (0x80 + y + x) );
}



//----------------------------------------------------------------------------------------
//
//		Kasowanie ekranu wyúwietlacza
//
//----------------------------------------------------------------------------------------
void lcd_cls(void) {

	lcd_write_cmd( LCDC_CLS );

	#if USE_RW == 0
		_delay_ms(4.9);
	#endif
}



//----------------------------------------------------------------------------------------
//
//		PowrÛt kursora na poczπtek
//
//----------------------------------------------------------------------------------------
void lcd_home(void) {

	lcd_write_cmd( LCDC_CLS|LCDC_HOME );

	#if USE_RW == 0
		_delay_ms(4.9);
	#endif
}


//----------------------------------------------------------------------------------------
//
//		W≥πczenie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_cursor_on(void) {

	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON|LCDC_CURSORON);
}

//----------------------------------------------------------------------------------------
//
//		Wy≥πczenie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_cursor_off(void) {

	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON);
}


//----------------------------------------------------------------------------------------
//
//		W£πcza miganie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_blink_on(void) {

	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON|LCDC_CURSORON|LCDC_BLINKON);
}

//----------------------------------------------------------------------------------------
//
//		WY≥πcza miganie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_blink_off(void) {

	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON);
}





//----------------------------------------------------------------------------------------
//
//		 ******* INICJALIZACJA WYåWIETLACZA LCD ********
//
//----------------------------------------------------------------------------------------
void lcd_init( void ) {


#if USE_I2C

		// zerowanie bufora danych
		mpxLCD = 0;

		// ustawienie prÍdkoci i2c
//		i2cSetBitrate( I2C_KHZ );

#if USE_BACKLIGHT == 1
		// w≥πczenie podwietlenia
		lcd_LED(1);
#endif

		vTaskDelay(15/portTICK_PERIOD_MS);		//vTaskDelay(15/portTICK_PERIOD_MS);
		mpxLCD &= ~(1<<LCD_E);
		mpxLCD &= ~(1<<LCD_RS);
		#if USE_RW == 1
			mpxLCD &= ~(1<<LCD_RW);
		#endif
		SEND_I2C;

	#else
		// inicjowanie pinÛw portÛw ustalonych do pod≥πczenia z wyúwietlaczem LCD
		// ustawienie wszystkich jako wyjúcia

#if USE_BACKLIGHT == 1
		DDR(LCD_LED_PORT) |= (1<<LCD_LED);
		lcd_LED(0);
#endif

		data_dir_out();
		DDR(LCD_RSPORT) |= (1<<LCD_RS);
		DDR(LCD_EPORT) |= (1<<LCD_E);
		#if USE_RW == 1
			DDR(LCD_RWPORT) |= (1<<LCD_RW);
		#endif


		PORT(LCD_RSPORT) |= (1<<LCD_RS);
		PORT(LCD_EPORT) |= (1<<LCD_E);
		#if USE_RW == 1
			PORT(LCD_RWPORT) |= (1<<LCD_RW);
		#endif

		_delay_ms(15);
		PORT(LCD_EPORT) &= ~(1<<LCD_E);
		PORT(LCD_RSPORT) &= ~(1<<LCD_RS);
		PORT(LCD_RWPORT) &= ~(1<<LCD_RW);
	#endif



	// jeszcze nie moøna uøywaÊ Busy Flag
	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	vTaskDelay(4/portTICK_PERIOD_MS);



	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	os_delay_us(100);



	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	os_delay_us(100);

	SET_E;
	lcd_sendHalf(0x02);// tryb 4-bitowy
	CLR_E;
	os_delay_us(100);



	// juø moøna uøywaÊ Busy Flag
	// tryb 4-bitowy, 2 wiersze, znak 5x7
	lcd_write_cmd( LCDC_FUNC|LCDC_FUNC4B|LCDC_FUNC2L|LCDC_FUNC5x7 );

	// wy≥πczenie kursora
	lcd_write_cmd( LCDC_ONOFF|LCDC_CURSOROFF );
	// w≥πczenie wyúwietlacza
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON );
	// przesuwanie kursora w prawo bez przesuwania zawartoúci ekranu
	lcd_write_cmd( LCDC_ENTRY|LCDC_ENTRYR );



	// kasowanie ekranu
	lcd_cls();

}




void lcd_LED( uint8_t enable ) {

#if USE_BACKLIGHT == 1
	#if USE_I2C
			if( enable ) mpxLCD |= (1<<LCD_LED);
			else mpxLCD &= ~(1<<LCD_LED);

			SEND_I2C;
	#else
			if( enable ) PORT(LCD_LED_PORT) |= (1<<LCD_LED);
			else PORT(LCD_LED_PORT) &= ~(1<<LCD_LED);
	#endif
#endif
}





