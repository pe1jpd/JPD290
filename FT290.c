/*
 * ombouw FT290 transceiver
 * 
 * #	change								date		by
 * ------------------------------------------------------------
 * 1.0	initial version						05-09-16	pe1jpd
 *
 *
 *
 *   
	 1	 2	 3	 4	 5	 6	 7	 8	 9	10	11	12	13	14
	PC6	PD0	PD1	PD2	PD3	PD4	VCC	GND	PB6	PB7	PD5	PD6	PD7	PB0
				ROT1ROT2MOD0		BND1BND2MOD1MOD2MOD3 E
									D6	D7

	PWM		RS	D3	D4
		AB			BND0			CLK	DATALE	
	PB1	PB2	PB3	PB4	PB5	VCC	REF	GND	PC0	PC1	PC2	PC3	PC4	PC5
	15	16	17	18	19	20	21	22	23	24	25	26	27	28

*/

#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "ft290.h"
#include <util/delay.h>

void lcdInit();
void lcdChar(char c);
void lcdStr(char *s);
void lcdCursor(int col, int row);

long int bandFreq[] = {115200000, 217600000, 325600000, 561600000, 1022400000, 2390400000, 0, 28800000};

int fast;							// flag for fast tuning
int timer;
int enc=0;							// encoder pulsecount
long tick;

ISR(INT0_vect)
{
	if (PIND & (1<<ROT2)) enc--;
}

ISR(INT1_vect)
{
	if (PIND & (1<<ROT1)) enc++;
}

// Interrupt service routine Timer 1
ISR(TIMER1_OVF_vect) 
{
} 

// Interrupt service routine Timer 2
ISR(TIMER2_OVF_vect) 
{
	TCNT2 = 10; //131;						// 125pulses at 8MHz/256=250Hz
	tick++;								// tick=~1ms
} 

void initInterrupts(void)
{
	// setup ratoray interrups
	EIMSK |= (1<<INT0) | (1<<INT1);		// enable INT0/1
	EICRA |= (1<<ISC01) | (1<<ISC11);	// on change

	// Setup Timer 1 with PWM
	TCCR1A = 0x00;						// Normal Mode 
	TCCR1B |= (1<<CS10);				// div/1 clock
	TCCR1A |= (1<<COM1A1) | (1<<WGM10);	// fast PWM  8 bit
	TCCR1B |= (1<<WGM12);
	TIMSK1 |= (1<<TOIE1);  				// Timer1 overflow interrupt 

    // set up Timer 2 
	TCCR2A  = 0x00;						// Normal operation
	TCCR2B |= (1<<CS22)|(1<<CS21);		// prescaler = 256
	TCNT2   = 131;
	TIMSK2 |= (1<<TOIE2);

	// enable interrupts
	sei();
}

long readFreq(int band)
{
	long freq = 14500000;

	uint32_t *address = (uint32_t *)((band<<2)+8);
	if (band>=0 && band<16) {
		freq = eeprom_read_dword(address);
		if (freq<14350000 || freq>14800000 || freq%5!=0) {
			freq = 14500000;
		}
	}
	return freq;
}


void writeFreq(int band, long freq)
{
	uint32_t *address;

	if (band>=0 && band<16) {
		address = (uint32_t *)((band<<2)+8);
		eeprom_write_dword(address, (long)freq);
	}
}

long int hex2bcd(long int n)
{
	unsigned long x, r;
	int c;

	r=0;
	x=100;

	do {
		c = 0;
		for (;;) {
			n-=x;
			if (n<0) break;
			c++;
		}
		n+=x;
		r<<=4;
		r+=c;
		x=x/10;
	} while (x!=0);

	return r;
}

void setPLL(long n)
{
	int i;

	n &= 0xfff;

    for (i=0; i<20; i++) {
        if (n&0x80000)
            sbi(PORTC, DATA);
        else
            cbi(PORTC, DATA);
        sbi(PORTC, CLK);
		_delay_us(1);
        cbi(PORTC, CLK);
        n <<= 1;
    }
    sbi(PORTC, LE);
    _delay_us(1);
    cbi(PORTC, LE);
}

void setFrequency(long f, long mf)
{
	long x, n, r;

	f = f - 13119000 - mf;
//	f = f-14200000;		// pll from 2-6MHz
	x = f/1000;
	r = f%1000;
	r /= 5;

	n = hex2bcd(x);

	setPLL(n);
	OCR1A = r;
}

int getRotaryTurn()
{
	int inc, count;

	count=0;

	if (enc!=0) {

		timer = tick; tick=0;

		inc=1;
		if (timer<2)
			inc=10;
		
		if (enc>0) {
			count += inc;
		}
		if (enc<0) {
			count -= inc;
		}
		enc = 0;
	}

	return count;
}

void displayFrequency(int band, int vfo, unsigned long f)
{
	unsigned long r, x = 1000000000;
	unsigned char c;
	int suppress=TRUE;
	int p=0;

	lcdCursor(0,0);

	f += bandFreq[band>>1];

	do {
		r=f/x;
		f-=r*x;
		x/=10;
		if (r!=0)
			suppress=FALSE;
		if (suppress) {
			c = ' ';
		}
		else {
			if (p==5) {
				lcdChar('.');
				p++;
			}
			else if (p==9) {
				lcdChar(',');
				p++;
			}
			c = 0x30+r;
		}
		lcdChar(c);
		p++;
	} while (x!=0);

	lcdChar('0');
	lcdChar(' ');
	lcdChar(' ');

	if (vfo==0) 
		lcdChar('A');
	else
		lcdChar('B');
}

int readBandSwitch()
{
	register uint8_t a, b;

	a = b = PINB;
	b >>= 4;		// bandx2
	b &= 0x0e;
	a >>= 2;		// vfo
	a &= 0x01;
	b |= a;

	b ^= 0xf;		// 0000.bbbv

	return (int)b;
}

int readUpDn()
{
	register int b;

	b = PIND&3;
	return b;
}

int readModeSwitch()
{
	unsigned char b=PIND;

	b >>= 4;
	b &= 0xf;
	return (int)b;
}

int main()
{
	int band, lastBand;
	int vfo;
	int updn, lastUpdn=3;
	int mode, lastMode=-1;
	long freq, lastFreq, mf;
	long int tick2sec;
	int c=0;

	DDRB = 0x0b;

	DDRC = 0xff;
	PORTC = 0x00;

	DDRD = 0x00;
	PORTD = 0x03;		// Up/DN internal pullup

	lcdInit();
	initInterrupts();

	band = readBandSwitch();
	lastBand = band;
	vfo = band&1;
	freq = readFreq(band);
	lastFreq = freq;

	mf = 1081000;
	tick2sec = tick+2000;

	for (;;) {

		// step 50Hz, freq in mult of 10Hz
		freq += 5*c;
		setFrequency(freq, mf);
		displayFrequency(band, vfo, (unsigned long)freq);

		for (;;) {

			// read band/vfo switches
			band = readBandSwitch();
			if (band != lastBand) {
				writeFreq(lastBand, freq);
				freq = readFreq(band);
				lastBand = band;
				vfo = band&1;
				lastFreq = freq;
				break;
			}

			// read mode
			mode = readModeSwitch();
			if (mode != lastMode) {
				lcdCursor(13,1);
				switch (mode) {
					case USB:
						mf = 1080850;
						lcdStr("USB");
						break;
					case LSB:
						mf = 1081150;
						lcdStr("LSB");
						break;
					case CW:
						mf = 1081000;
						lcdStr(" CW");
						break;
					default:
						mf = 1081000;
						lcdStr(" FM");
						break;
				}
				lastMode = mode;
				break;
			}

			// handle encoder 
			c = getRotaryTurn();
			if (c != 0)
				break;

			// handle Up/Dn inputs
			updn = readUpDn();
			if (updn != lastUpdn) {
				lastUpdn = updn;
				if (~updn&1)
					c = 1;
				else if (~updn&2)
					c = -1;
				break;
			}

			if (tick > tick2sec && freq != lastFreq) {
				writeFreq(band, freq);
				tick2sec = tick+2000;
				lastFreq = freq;
			}
		}
	}
}

