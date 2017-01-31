# JPD290
FT290's new life as 2m, 23, 13, 9, 6, 3, 1.2cm all mode transceiver 

I got a FT290 which was not quite useful at the time. Tried to sell it, but got one offer of 30 euro, so I decided to give this transceiver a new life as a multi-band, all mode transceiver for use with my transverters from 23cm up to 24GHz.
I only used the HF board, and half of the powersupply board (I cut it actually in half) for the + and - 6.8V.
The original 4bit Yeasu processor is replaced with an Atmel ATMEGA328. 
It drives the LCD display (4bit mode), reads the band and mode switch, the rotary controller and an VFO A/B flipswitch. On the back of the housing 6 BNC's are mounted for each transvert, and the band switch  drives a relay board to choose the right in/output BNC. Also, this band switch powers a RF Relay to switch the antenna (dish 90cm) to the right transverter. 

Questions or pictures: mail me! I will publish an article about this rebuild shortly.

Attention: in the fuse settings the default /8 is switched off, so the controller runs on 8MHz. 
