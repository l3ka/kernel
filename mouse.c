/*
*  Title: Windows X
*  By: Darko Lekic (lekic.d7@gmail.com)
*  By: Nikola Pejic (peja96bl@gmail.com)
*  Description: Mouse
*/

#include "system.h"
#include "stdint.h"

const uint8_t DATA_PORT = 0x60;
const uint8_t STATUS_PORT = 0x64;

unsigned char mouse_cycle = 0;
unsigned char previous_flags = 0;
unsigned char mouse_byte[3];

// In case we implement moving cursor
unsigned char mouse_x = 0;
unsigned char mouse_y = 0; 

// Mouse functions
void mouse_handler(struct regs *r)
{
	mouse_byte[mouse_cycle++] = inportb(DATA_PORT);

	if(mouse_cycle == 1) // was 0
	{
		unsigned char flags = mouse_byte[0];

		// If bit 3 is not set, drop the packet
		if((flags & (1 << 3)) == 0)
		{
			mouse_cycle = 0;
	    		return;
	}
	}
	if(mouse_cycle == 3)
	{
		mouse_cycle = 0;
		unsigned char flags = mouse_byte[0];
		if(flags & ((1 << 6) | (1 << 7)))
		{
	    		return;
		}
		if((flags & (1 << 0)) && !(previous_flags & (1 << 0)))
		{
			press_start_digital_watch();
		}
		if((flags & (1 << 1)) && !(previous_flags & (1 << 1)))
		{
			press_pause_digital_watch();
		}
		if((flags & (1 << 2)) && !(previous_flags & (1 << 2)))
		{
			press_reset_digital_watch();
		}
		previous_flags = flags;
	}

	/* // In case we implement moving cursor 
	switch(mouse_cycle)
	{
		case 0:
			mouse_byte[0] = inportb(DATA_PORT);
			mouse_cycle++;
			break;
		case 1:
			mouse_byte[1] = inportb(DATA_PORT);
			mouse_cycle++;
			break;
		case 2:
			mouse_byte[2] = inportb(DATA_PORT);
			mouse_x = mouse_byte[1];
			mouse_y = mouse_byte[2];
			mouse_cycle = 0;
			break;
	}
	*/
}

inline void mouse_wait(unsigned char type)
{
	unsigned int time_out = 100000;
	if(type == 0)
	{
		while(time_out--) // for Data
		{
			if((inportb(STATUS_PORT) & 1) == 1)
	    		{
				return;
	    		}
		}
		return;
	}
	else {
		while(time_out--) // for Signal
		{
	    		if((inportb(STATUS_PORT) & 2) == 0)
	    		{
				return;
	    		}
		}
		return;
	}
}

inline void mouse_write(unsigned char write)
{
	// Wait to be able to send a command
	mouse_wait(1);

	// Tell the mouse we are sending a command
	outportb(STATUS_PORT, 0xD4); // 0xD4 --> Write next byte to second PS/2 port input buffer (only if 2 PS/2 ports supported)(sends next byte to the second PS/2 port)

	// Wait for the final part
	mouse_wait(1);

	// Finally write
	outportb(DATA_PORT, write);
}

unsigned char mouse_read()
{
	// Get's response from mouse
	mouse_wait(0); 
	return inportb(DATA_PORT);
}

void mouse_install()
{
	unsigned char status;

	// Enable the auxiliary mouse device
	mouse_wait(1);
	outportb(STATUS_PORT, 0xA8); // 0xA8 --> Enable second PS/2 port (only if 2 PS/2 ports supported)	

	// Enable the interrupts
	mouse_wait(1);
	outportb(STATUS_PORT, 0x20); // 0x20 --> Read "byte 0" from internal RAM	
	mouse_wait(0);
	status = (inportb(DATA_PORT) | 2);
	mouse_wait(1);
	outportb(STATUS_PORT, DATA_PORT);
	mouse_wait(1);
	outportb(DATA_PORT, status);

	// Tell the mouse to use default settings
	mouse_write(0xF6); // 0xF6 --> Disable Data Reporting
	mouse_read(); // Acknowledge

	// Enable the mouse
	mouse_write(0xF4); // 0xF4 --> Enable Data Reporting
	mouse_read(); // Acknowledge

	// Setup the mouse handler
	irq_install_handler(12, mouse_handler);
}
