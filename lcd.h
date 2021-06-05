#include <avr/io.h>
#include <util/delay.h>

#define LCD_RS_DDR  DDRC
#define LCD_E_DDR   DDRC
#define LCD_D4_DDR  DDRC
#define LCD_D5_DDR  DDRC
#define LCD_D6_DDR  DDRC
#define LCD_D7_DDR  DDRC

#define LCD_V0_DDR  DDRD
#define LCD_LED_DDR DDRD

#define LCD_RS_PORT PORTC
#define LCD_E_PORT  PORTC
#define LCD_D4_PORT PORTC
#define LCD_D5_PORT PORTC
#define LCD_D6_PORT PORTC
#define LCD_D7_PORT PORTC


#define LCD_RS      0
#define LCD_E       1
#define LCD_D4      2
#define LCD_D5      3
#define LCD_D6      4
#define LCD_D7      5
#define LCD_V0_PWM  5
#define LCD_LED_PWM 6

#define degree      0
#define tip         1
#define pulse       2
#define arrow       3


uint8_t lcd_char[32];
uint8_t c_char[32]={
                      0b01100,
					  0b10010,
					  0b10010,
					  0b01100,
					  0b00000,
					  0b00000,
					  0b00000,
					  0b00000,
					  
					  0b00100,
					  0b01100,
					  0b01100,
					  0b01100,
					  0b01100,
					  0b11110,
					  0b11110,
					  0b00000,
					  
					  0b01110,
					  0b01010,
					  0b01010,
					  0b01010,
					  0b01010,
					  0b01010,
					  0b11011,
					  0b00000,
					  
					  0b01000,
					  0b01100,
					  0b01110,
					  0b01111,
					  0b01110,
					  0b01100,
					  0b01000,
					  0b00000
					  };

void ports_init(void){
LCD_RS_DDR|=(1<<LCD_RS);
LCD_E_DDR |=(1<<LCD_E);
LCD_D4_DDR|=(1<<LCD_D4);
LCD_D5_DDR|=(1<<LCD_D5);
LCD_D6_DDR|=(1<<LCD_D6);
LCD_D7_DDR|=(1<<LCD_D7);
LCD_RS_PORT&=~(1<<LCD_RS);
LCD_E_PORT &=~(1<<LCD_E);
}

void cmd_4_bit(uint8_t cmd){
if(cmd & 0x01){LCD_D4_PORT|=(1<<LCD_D4);}else{LCD_D4_PORT&=~(1<<LCD_D4);}
if(cmd & 0x02){LCD_D5_PORT|=(1<<LCD_D5);}else{LCD_D5_PORT&=~(1<<LCD_D5);}
if(cmd & 0x04){LCD_D6_PORT|=(1<<LCD_D6);}else{LCD_D6_PORT&=~(1<<LCD_D6);}
if(cmd & 0x08){LCD_D7_PORT|=(1<<LCD_D7);}else{LCD_D7_PORT&=~(1<<LCD_D7);}
LCD_E_PORT |=(1<<LCD_E);
LCD_E_PORT &=~(1<<LCD_E);
_delay_ms(2);
}

void lcd_send(uint8_t mode, uint8_t cmd){
if(mode==1){LCD_RS_PORT|=(1<<LCD_RS);}else{LCD_RS_PORT&=~(1<<LCD_RS);}
cmd_4_bit(cmd>>4);
cmd_4_bit(cmd);
}

void lcd_pos(uint8_t x,uint8_t y){
if     (y==1){lcd_send(0,0x80+0x00+x-1);}
else if(y==2){lcd_send(0,0x80+0x40+x-1);}
}

void lcd_custom_char(void){
lcd_send(0,0x40);
for(uint8_t i=0;i<32;i++){lcd_send(1,c_char[i]);}
}

void lcd_cc(uint8_t x, uint8_t y, uint8_t cc){
lcd_char[x-1+(y-1)*16]=cc;
}

void lcd_backlight(uint8_t bcklt){
OCR0A=bcklt;
}

void lcd_contrast(uint8_t cntrst){
OCR0B=cntrst;
}

void lcd_init(void){
LCD_V0_DDR |=(1<<LCD_V0_PWM);
LCD_LED_DDR|=(1<<LCD_LED_PWM);
TCCR0A  =(1<<COM0B1)|(1<<COM0A1)|(1<<WGM00)|(1<<WGM01);
TCCR0B  =(1<<CS01);
OCR0B   =10;
OCR0A   =10;
_delay_ms(50);
ports_init();
_delay_ms(500);
cmd_4_bit(0x03);
_delay_ms(150);
cmd_4_bit(0x03);
_delay_ms(50);
cmd_4_bit(0x03);
cmd_4_bit(0x02);
lcd_send(0,0x28);
lcd_send(0,0x01);
_delay_ms(50);
lcd_send(0,0x02);
lcd_send(0,0x08);
lcd_send(0,0x0C);
lcd_send(0,0x06);
lcd_send(0,0x10);
lcd_send(0,0x80);
_delay_ms(500);
lcd_custom_char();
lcd_send(0,0x01);
}

void lcd_blank(void){
for(uint8_t i=0;i<32;i++){lcd_char[i]=' ';}
}

void lcd_print_number(uint8_t x,uint8_t y,uint32_t number){
uint8_t  num[4],dgt=0;
uint32_t n=number;
if(n!=0){
  while(n!=0){n/=10;dgt++;}
  for(int i=(dgt-1);i>-1;i--){num[i]=(number%10)+48;number/=10;}
  for(uint8_t i=0;i<dgt;i++){lcd_char[i+x-1+(y-1)*16]=num[i];}
  }
else{lcd_char[x-1+(y-1)*16]=48;}
}

void Iron_temp(uint8_t x, uint8_t y, uint16_t temp){
lcd_cc(x,y,1);
lcd_print_number(x+2,y,temp);
if(temp>99){lcd_cc(x+5,y,0);lcd_cc(x+6,y,'C');}
else if(temp>9){lcd_cc(x+4,y,0);lcd_cc(x+5,y,'C');}
else{lcd_cc(x+3,y,0);lcd_cc(x+4,y,'C');}
}

void Set_temp(uint8_t x, uint8_t y, uint16_t temp){
lcd_cc(x,y,3);
lcd_print_number(x+2,y,temp);
if(temp>99){lcd_cc(x+5,y,0);lcd_cc(x+6,y,'C');}
else if(temp>9){lcd_cc(x+4,y,0);lcd_cc(x+5,y,'C');}
else{lcd_cc(x+3,y,0);lcd_cc(x+4,y,'C');}
}

void PWM_out(uint8_t x, uint8_t y, uint16_t temp){
lcd_cc(x,y,2);
lcd_print_number(x+2,y,temp);
}

void lcd_print(void){
lcd_pos(1,1);
for(uint8_t i=0;i<16;i++){lcd_send(1,lcd_char[i]);}
lcd_pos(1,2);
for(uint8_t i=0;i<16;i++){lcd_send(1,lcd_char[i+16]);}
}