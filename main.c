#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"

#define   KP        20
#define   KD        5
#define   KI        10

#define   ADC_VREF  1096
#define   AVG_SMPL  128
#define   ADC_GAIN  34
#define   TIP_COEF  60

#define   TEMP_LOW  30
#define   TEMP_HIGH 480

volatile uint8_t  CLK_L=0,CLK=0,DAT=0;                     //encoder
volatile int      encoder=0,push=0;                        //encoder
volatile uint16_t adc_temp=0,adc_vin=0,num=0,long_press=0; //adc-enc
volatile int set_temp=0,tov0=0;
volatile uint32_t adc_sum=0,adc_avg=0,ad_smp=0,tip_temp=0;
int pwm=400;

int main(void){

CLKPR=0x80;CLKPR=0x00;
DDRB   |= (1 << DDB1);
TCCR1A  =0x00;
TCCR1B  =0x00;
TCCR1A |= (1 << COM1A1)|(1<<COM1A0);
TCCR1A |= (1 << WGM11);
TCCR1B |= (1 << WGM12)|(1 << WGM13);
TCCR1B |= (1 << CS10);
ICR1    = 401;
OCR1A   = 400;


TCCR0A=0x00;
TCCR0B=0x05;
TIMSK0=(1<<TOV0);


ADMUX|=(1<<REFS0)|(1<<REFS1);
ADMUX|=(1<<MUX0)|(1<<MUX1)|(1<<MUX2);
ADCSRA|=(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
ADCSRA|=(1<<ADIE)|(1<<ADEN)|(1<<ADSC);
sei();

lcd_init();
lcd_backlight(80);
lcd_contrast(230);

DDRD &=~((1<<2)|(1<<3)|(1<<7));
PCICR  |=(1<<PCIE2);
PCMSK2 |=(1<<PCINT18)|(1<<PCINT19)|(1<<PCINT23);
sei();

while(1){
		   
		   tip_temp=ADC_VREF;
		   tip_temp*=1000;
		   tip_temp/=1024;
		   tip_temp*=adc_avg;
		   tip_temp/=ADC_GAIN;
		   tip_temp/=TIP_COEF;
		   
		   
		   set_temp=encoder;
		   if     (set_temp<TEMP_LOW ){set_temp=TEMP_LOW; }
		   else if(set_temp>TEMP_HIGH){set_temp=TEMP_HIGH;}
		   int error=(set_temp-tip_temp);
		   pwm=(error*KP);
		   if(pwm<0){pwm=1;}
		   else if(pwm>400){pwm=400;}
		   OCR1A=pwm;
		   
		   
		   lcd_blank();
		   Set_temp(1,1,set_temp);
		   Iron_temp(10,1,tip_temp);
		   PWM_out(10,2,pwm);
		   lcd_print();
		   _delay_ms(10);
		   }

}


ISR(PCINT2_vect){
uint8_t TEMP=(PIND & 0b10001100)>>2;
CLK=(TEMP & 0b00000001);
DAT=(TEMP & 0b00100000)>>5;
if(CLK!=CLK_L){if(DAT!=CLK){encoder++;}else{encoder--;}}
if(TEMP & 0b00001000){if(tov0>7800){long_press=1;}}else{push++;TCNT0=0;tov0=0;long_press=0;}
CLK_L=CLK;
}

ISR(ADC_vect){
adc_temp=ADCW;
adc_sum+=adc_temp;
ad_smp++;
if(ad_smp>AVG_SMPL){adc_avg=adc_sum/AVG_SMPL;adc_sum=0;ad_smp=0;}
ADCSRA|=(1<<ADSC);
}

ISR(TIMER0_OVF_vect){
tov0++;
}