/*
 * Praca_dyplomowa.c
 *
 * Created: 15.10.2020 20:51:52
 * Author : Robert P
 */ 
   #define F_CPU 16000000UL //deklaracja zegara

   #include <avr/io.h> //podstawowa biblioteka
   #include <stdbool.h>// do zmiennej typu BOOL
   #include <util/delay.h> //biblioteka z zegarem
   #include <avr/interrupt.h> //biblioteka do przerwañ
   
    #define dioda_informujaca (1<<PB1)
    #define kierunkowskazy (1<<PB3)
    #define syrena_alarmowa (1<<PB4)
    #define dioda_mikrokontrolera (1<<PB5)
     
    #define sygnal_sterujacy (0<<PD3)
    #define sygnal_przycisku (1<<PB7)
    
	
   void UART_deklaracja(unsigned int BAUD ,bool RXD,bool TXD)
   {
	   // ustawienie prêdkoœci
	   unsigned int ubrr;
	   ubrr=(((F_CPU / (BAUD * 16UL))) - 1);
	   UBRR0H = (unsigned char)(ubrr>>8);
	   UBRR0L = (unsigned char)ubrr;
	   // ustawienie bitów parzystoœci
	   UCSR0C |= 1<<UPM01;// 1 bit parzystoœci
	   // ustawienie iloœci bitów danych
	   UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
	   // ustawienie bitów stopu
	   // domyœlnie jest 1
	   
	   if(RXD)
	   {
		   UCSR0B |= 1<<RXEN0;
	   }
	   if(TXD)
	   {
		   UCSR0B |= 1<<TXEN0;
	   }
   }
   void UART_wysylanie( unsigned char data)
   {
	   while(!(UCSR0A & (1<<UDRE0)));
	   UDR0 = data;
   }
   
   void UART_wysylanie_String(char* StringPtr){
	   
	   while(*StringPtr != 0x00)
	   {
		   UART_wysylanie(*StringPtr);
		   StringPtr++;
	   }
   }
   
   bool UART_aktywacja()
   {
	   return (UCSR0A & (1<<RXC0));
   }
   
   unsigned char UART_zwrocenie()
   {
	   return UDR0;
   }
   
   int main(void)
   {
	  
	   DDRD|=(1<<0);
	   PORTD|= (1<<0);
	   
	   //ISR INT0
	   EICRA |= (1 << ISC00);    // Ustawienie wyzwalacza na opcje 00
	   EIMSK |= (1 << INT0);     // Za³¹czenie przerwania int0
	   
	    DDRB|=(dioda_mikrokontrolera);
	    DDRB|=(dioda_informujaca);
	    DDRB|=(syrena_alarmowa);
	    DDRB&=~(sygnal_sterujacy);
	    DDRB&=~(sygnal_przycisku);

	   UART_deklaracja(9600,true,true);
	   while(1)
	   {
		   POWROT:
		   if(UART_aktywacja())
		   {
			   unsigned char byte=UART_zwrocenie();
			   
			   if(byte == 'A')
			   {
				   UART_wysylanie_String("Alarm uzbrojony \n");
				   while(1)
				   {
					   unsigned char byte=UART_zwrocenie(); //potrzbne odczytanie aby ci¹gle sprawdza³ stan bufora w pêtli
					   
					   if (byte == 'C')
					   {
						   UART_wysylanie_String("Alarm rozbrojony \n");
						   goto POWROT;	   
					   }
					   sei();
					   
					   PORTB|=(dioda_mikrokontrolera); //Oznaczeie uzbrojenia alarmu (w³¹cz diode)
					   PORTB|=(dioda_informujaca);
					   _delay_ms(1000);
					   
					   PORTB&=~(dioda_mikrokontrolera); //Oznaczeie uzbrojenia alarmu (wy³¹cz diode)
					   PORTB&=~(dioda_informujaca);
					   _delay_ms(1000);
					   
					   cli();      
					 } 				
			   }
		   }  
	   }
   }
   
   //Za³¹czenie Alarmu
   ISR(INT0_vect) //Reaguje na stan logiczny pin D3
   {
	   int i;
	   PORTB|=(syrena_alarmowa);
	   
	   DDRD &=~ (1 << DDD4);     // wyczyœæ pin PD4
	   // PD4 is now an input
	   PORTD |= (1 << PORTD4);   // w³¹cz pull-up
	   // PD4 jest teraz wejœciem
	   TCCR0B |= (1<< CS02) | (1 << CS01) | (1 << CS00);
	   // w³¹czenie preslekera na 6, zegar na zboczê rosn¹ce
	   
	   for (i=0; i<=10 ;i++) 
	   {
		  
		   PORTB|=(kierunkowskazy);
		   _delay_ms(1000);

		   PORTB&=~(kierunkowskazy);
		   _delay_ms(1000);
		   
			if (UDR0 =='B')
			{
				goto POWROT2;
			}

		  
		   if ((TIFR0 & (1 << TOV0))|!(sygnal_przycisku & PINB))
		   {
			   for (i=0; i<=100 ;i++) 
			   {
				   
				   
				   PORTB|=(kierunkowskazy);
				   _delay_ms(100);
				   
				   PORTB&=~(kierunkowskazy);
				   _delay_ms(100);
				   
				   if (UDR0=='B')
				   {
					   goto POWROT2;
				   }
			   }
		   }
	   }
	   POWROT2:
	   DDRB&=~(dioda_informujaca);
	   PORTB&=~(syrena_alarmowa);
	   TIFR0 |= (1 << TOV0); //reset licznika
	 }