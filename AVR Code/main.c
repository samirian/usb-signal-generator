#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "usbdrv.h"

#define F_CPU 16000000L
#include <util/delay.h>

#define USB_DATA_OUT 2
#define USB_DATA_IN 4

//{0x80,0xd1,0xfd,0xee,0xab,0x54,0x11,0x2,0x2e,0x80}
static uchar replyBuf[21];
static volatile uint8_t counter = 0;
static uchar dataReceived = 0, dataLength = 0; // for USB_DATA_IN
static volatile uint8_t signal[50]= {0x80,0x8f,0x9f,0xae,0xbd,
								0xca,0xd7,0xe2,0xeb,0xf3,
								0xf9,0xfd,0xff,0xff,0xfd,
								0xf9,0xf3,0xeb,0xe2,0xd7,
								0xca,0xbd,0xae,0x9f,0x8f,
								0x80,0x70,0x60,0x51,0x42,
								0x35,0x28,0x1d,0x14,0xc,
								0x6,0x2,0x0,0x0,0x2,
								0x6,0xc,0x14,0x1d,0x28,
								0x35,0x42,0x51,0x60,0x70};

ISR(TIMER1_COMPA_vect){
	PORTA = signal[counter];
	counter = (counter + 1)%50;
}

void init_timer1(uint8_t prescaler, uint16_t ocr1a){
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1<<WGM12);
	switch(prescaler){
		case 0:
			break;
		case 1:
			TCCR1B |= (1<<CS10);//CTC no prescaler
			break;
		case 2:
			TCCR1B |= (1<<CS11);//CTC no prescaler
			break;
		case 3:
			TCCR1B |= (1<<CS11) | (1<<CS10);//CTC no prescaler
			break;
		case 4:
			TCCR1B |= (1<<CS12);//CTC no prescaler
			break;
		case 5:
			TCCR1B |= (1<<CS12) | (1<<CS10);//CTC no prescaler
			break;
	}
	uint8_t serg = SREG;
	cli();
	TCNT1 = 0;
	OCR1A = ocr1a;
	SREG = serg;
	TIMSK |= (1<<OCIE1A);
	sei();
}

// this gets called when custom control message is received
USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data; // cast data to correct type
	uint8_t sreg;
	switch(rq->bRequest) { // custom command is in the bRequest field
	    case USB_DATA_OUT: // send data to PC
			sreg = SREG;
			cli();
			replyBuf[0] = (uint8_t)((OCR1A & 0xFF00) >> 8);
			replyBuf[1] = (uint8_t)(OCR1A & 0x00FF);
			replyBuf[2] = 'D';
			SREG = sreg;
        	usbMsgPtr = replyBuf;
	    	return 3;

	    case USB_DATA_IN: // receive data from PC
			dataLength  = (uchar)rq->wLength.word;
	        dataReceived = 0;
			
			if(dataLength  > sizeof(replyBuf)) // limit to buffer size
				dataLength  = sizeof(replyBuf);
			return USB_NO_MSG; // usbFunctionWrite will be called now
    }
    return 0; // should not get here
}
// This gets called when data is sent from PC to the device
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
	uchar i;
	for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++){
		replyBuf[dataReceived] = data[i];
	}
	init_timer1(replyBuf[0], (replyBuf[1] + ( (replyBuf[2]) << 8) )  );
    return (dataReceived == dataLength); // 1 if we received it all, 0 if not
}

int main() {
	uchar i;
	DDRA = 0xff;
    wdt_enable(WDTO_1S); // enable 1s watchdog timer
    init_timer1(3, 0xffff);
    usbInit();
	
    usbDeviceDisconnect(); // enforce re-enumeration
    for(i = 0; i<250; i++) { // wait 500 ms
        wdt_reset(); // keep the watchdog happy
        _delay_ms(2);
    }

    sei(); // Enable interrupts after re-enumeration
    usbDeviceConnect();
    while(1) {
    	sei();
        wdt_reset(); // keep the watchdog happy
        usbPoll();
    }
    return 0;
}