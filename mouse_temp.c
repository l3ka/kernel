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
unsigned char mouse_x = 0;
unsigned char mouse_y = 0; 

//Mouse functions
void mouse_handler(struct regs *a)
{
    mouse_byte[mouse_cycle++] = inportb(DATA_PORT);

    if(mouse_cycle == 1){ // was 0
        unsigned char flags = mouse_byte[0];

        // If bit 3 is not set, drop the packet
        if((flags & (1 << 3)) == 0){
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
		read_rtc();
		puts("Stoperica start:\n\tTrenutno vrijeme: ");
		beep();
		print_values(hour + 0x02, minute, second);
		reset_time();
		// Save values initial
		save_second = second;
		save_minute = minute;
		save_hour = hour;
		check_pause = 1;
		}
	if((flags & (1 << 1)) && !(previous_flags & (1 << 1)))
		{
		read_rtc();
		if(check_pause == 1)
		{
			puts("Pauza: \n\tTrenutno vrijeme: ");
			print_values(hour + 0x02, minute, second);
			puts("\tUkupno vrijeme: ");
			print_diferent();
			puts("\n");
			check_pause = 0;
		}
		else 	{	
			puts("Start: \n\tTrenutno vrijeme: ");
			beep();
			print_values(hour + 0x02, minute, second);
			save_second = second;
			save_minute = minute;
			save_hour = hour;
			check_pause = 1;
			}
		}
	if((flags & (1 << 2)) && !(previous_flags & (1 << 2)))
		{
		cls();
		//puts("Reset: \n");
		puts("\t\t\t\tStopwatch\n");
		reset_time();
		}

	previous_flags = flags;
    }

    /*switch(mouse_cycle)
    {
        case 0:
          mouse_byte[0] = inportb(0x60);
          mouse_cycle++;
        break;
        case 1:
          mouse_byte[1] = inportb(0x60);
          mouse_cycle++;
        break;
        case 2:
          mouse_byte[2] = inportb(0x60);
          mouse_x = mouse_byte[1];
          mouse_y = mouse_byte[2];
          mouse_cycle = 0;
        break;
    }*/
}

inline void mouse_wait(unsigned char a_type)
{
    unsigned int time_out = 100000;
    if(a_type == 0)
    {
	while(time_out--) //Data
        {
            if((inportb(0x64) & 1) == 1)
            {
                return;
            }
        }
        return;
    }
    else
    {
        while(time_out--) //Signal
        {
            if((inportb(0x64) & 2) == 0)
            {
                return;
            }
        }
        return;
    }
}

inline void mouse_write(unsigned char write) //unsigned char
{
    //Wait to be able to send a command
    mouse_wait(1);
    //Tell the mouse we are sending a command
    outportb(0x64, 0xD4);
    //Wait for the final part
    mouse_wait(1);
    //Finally write
    outportb(0x60, write);
}

unsigned char mouse_read()
{
    //Get's response from mouse
    mouse_wait(0); 
    return inportb(0x60);
}

void mouse_install()
{
    unsigned char status;

    //Enable the auxiliary mouse device
    mouse_wait(1);
    outportb(0x64, 0xA8);
  
    //Enable the interrupts
    mouse_wait(1);
    outportb(0x64, 0x20);
    mouse_wait(0);
    status = (inportb(0x60) | 2);
    mouse_wait(1);
    outportb(0x64, 0x60);
    mouse_wait(1);
    outportb(0x60, status);
  
    //Tell the mouse to use default settings
    mouse_write(0xF6);
    mouse_read();  //Acknowledge
  
    //Enable the mouse
    mouse_write(0xF4);
    mouse_read();  //Acknowledge

    //Setup the mouse handler
    irq_install_handler(12, mouse_handler);
}
