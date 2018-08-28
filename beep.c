/*
*  Title: Windows X
*  By: Darko Lekic (lekic.d7@gmail.com)
*  By: Nikola Pejic (peja96bl@gmail.com)
*  Description: Beep
*/

#include <system.h>
#include <stdint.h>

// To play standard audio, the CPU needs to be interrupted 44100 times every second

/* 0x61 --> 
* Each PIT channel also has a "gate input" pin which can be used to control whether the input signal (the 1.19MHz one) gets to the channel or not. 
* For PIT channels 0 and 1, the associated gate input pin is not connected to anything. The PIT channel 2 gate is controlled by IO port 0x61, bit 0.
* The output of PIT channel 2 is connected to the PC speaker, so the frequency of the output determines the frequency of the sound produced by the speaker. 
* This is the only channel where the gate input can be controlled by software (via bit 0 of I/O port 0x61), 
* and the only channel where its output (a high or low voltage) can be read by software (via bit 5 of I/O port 0x61).
*/

/* The Programmable Interval Timer - PIT chip uses the following I/O ports:
* I/O port      Usage
* 0x40			Channel 0 data port (read/write)
* 0x41			Channel 1 data port (read/write)
* 0x42			Channel 2 data port (read/write)
* 0x43			Mode/Command register (write only, a read is ignored)
*/

/* 0xB6 = 10110110 -->
* bits 7 and 6 => 10 = Channel 2
* bits 5 and 4 => 11 Access mode: lobyte/hibyte
* bits 3 and 2 and 1 => 110 = Mode 2 (rate generator, same as 010b)
* bit 0 => BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
*/

// Play sound using built in speaker
static void play_sound(uint32_t nFrequence) 
{
	uint32_t Div;
	uint8_t tmp;
	
	// Set the PIT to the desired frequency
	Div = 1193180 / nFrequence;
 	outportb(0x43, 0xb6);

 	outportb(0x42, (uint8_t) (Div) );
 	outportb(0x42, (uint8_t) (Div >> 8));
 
    // And play the sound using the PC speaker
 	tmp = inportb(0x61);
  	if (tmp != (tmp | 3)) 
	{
		outportb(0x61, tmp | 3);
 	}
}
 
// Make it shut up
void nosound() 
{
	uint8_t tmp = inportb(0x61) & 0xFC;
	outportb(0x61, tmp);
}
 
// Make a beep
void beep() 
{
	for(int i = 0; i<10000; i++)
		play_sound(1000);
 	// timer_wait(100);
 	nosound();
	// set_PIT_2(old_frequency);
}

