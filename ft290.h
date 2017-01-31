#define F_CPU 		8000000UL

/*
 * ATTENTION! F_CPU must be 8000000UL
 */
#define version		"1.1"

#define sbi(x,y)	x |= _BV(y) //set bit - using bitwise OR operator 
#define cbi(x,y)	x &= ~(_BV(y)) //clear bit - using bitwise AND operator
#define tbi(x,y)	x ^= _BV(y) //toggle bit - using bitwise XOR operator
#define is_high(x,y) (x & _BV(y) == _BV(y)) //check if the y'th bit of register 'x' is high ... test if its AND with 1 is 1

#define FALSE		0
#define TRUE		!FALSE

#define SHORT		1
#define LONG		2

#define FM			1
#define LSB			2
#define USB			4
#define CW			8

// PORTD
#define UP			PD0			// in + pullup
#define	DN			PD1			// in + pullup
#define ROT1		PD2			// in
#define ROT2		PD3			// in
#define MOD0		PD4			// in
#define	MOD1		PD5			// in
#define	MOD2		PD6			// in
#define	MOD3		PD7			// in

// PORTC
#define LE	     	PC2			// PLL
#define DATA    	PC1			// PLL
#define CLK     	PC0			// PLL

// PORTB
#define	LCD_D7		PB7			// LCD out & band in 
#define	LCD_D6		PB6			// LCD out & band in
#define	LCD_D5		PB5			// LCD out & band in 
#define	LCD_D4		PB4			// LCD out
#define	LCD_RS		PB3			// out
#define VFOAB		PB2			// in
#define DAC			PB1			// out 50Hz steps
#define	LCD_E		PB0			// LCD E & band switch en



