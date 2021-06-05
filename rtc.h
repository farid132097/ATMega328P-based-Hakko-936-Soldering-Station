#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t __sec=0,__mnt=0,__hr=0,__day=0,__mnth=0,__yr=0;
volatile uint8_t __day_table[13]={29,31,28,31,30,31,30,31,31,30,31,30,31},__index=0;
volatile uint32_t RTC_TR=0,RTC_DR=0;

void EEPROM_write(uint16_t EE_address, unsigned char EE_data){
while(EECR & (1<<EEPE));
EEAR=EE_address;
EEDR=EE_data;
EECR=(1<<EEMPE);
EECR|=(1<<EEPE);
}

unsigned char EEPROM_read(uint16_t EE_address){
while(EECR & (1<<EEPE));
EEAR=EE_address;
EECR|=(1<<EERE);
return EEDR;
}

void RTC_save_time(void){
EEPROM_write(970,__hr);
EEPROM_write(971,__mnt);
EEPROM_write(972,__sec+12);
EEPROM_write(973,__day);
EEPROM_write(974,__mnth);
EEPROM_write(975,__yr);
}

void RTC_init(uint32_t TIME, uint32_t DATE,uint8_t erase_mem){
if(erase_mem==1){

	RTC_TR=TIME;
    RTC_DR=DATE;
    __hr  =(TIME/10000);
    __mnt =(TIME/100)%100;
    __sec =(TIME%100);
    __day =(DATE/10000);
    __mnth=(DATE/100)%100;
    __yr  =(DATE%100);
	
	RTC_save_time();
}

else{
  __hr  =EEPROM_read(970);
  __mnt =EEPROM_read(971);
  __sec =EEPROM_read(972);
  __day =EEPROM_read(973);
  __mnth=EEPROM_read(974);
  __yr  =EEPROM_read(975);
}
RTC_TR=__hr;
RTC_TR=(RTC_TR<<16)|(__mnt<<8)|__sec;
RTC_DR=__day;
RTC_DR=(RTC_DR<<16)|(__mnth<<8)|__yr;

ASSR  |=(1<<AS2);
TCCR2B|=(1<<CS22)|(1<<CS20);
TIMSK2|=(1<<TOIE2);
sei();
}


ISR(TIMER2_OVF_vect){
__sec++;
if(__sec>59)
   {
    __sec=0;__mnt++;
      if(__mnt>59)
	   {
          __mnt=0;__hr++;
	      if(__hr>23)
		   {
	         __hr=0;__day++;
			 if((__yr%4==0)&&(__mnth==2))
			   {
	             __index=0;
				}
	         else{
			     __index=__mnth;
				 }
             if(__day>__day_table[__index])
			   {
	             __day=1;__mnth++;
	             if(__mnth>12)
				  {
	               __yr++;
		          }
			    }
			 RTC_DR = __day;
			 RTC_DR = (RTC_DR<<16);
			 RTC_DR|= (__mnth<<8) |__yr;
		    }
	    }
	   RTC_TR = __hr;
	   RTC_TR = (RTC_TR<<16)|(__mnt<<8);
   }
RTC_TR=(RTC_TR & 0xFFFF00)|__sec;
}