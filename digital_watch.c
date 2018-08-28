/*
*  Title: Windows X
*  By: Darko Lekic (lekic.d7@gmail.com)
*  By: Nikola Pejic (peja96bl@gmail.com)
*  Description: Digital Watch
*/

#include "system.h"

#define CURRENT_YEAR 2018
int century_register = 0x00; // Set by ACPI table parsing code if possible

// Initial - global variable
unsigned char save_second = 0;
unsigned char save_minute = 0;
unsigned char save_hour = 0;

unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

/*
* Register  Contents
* 0x00      Seconds
* 0x02      Minutes
* 0x04      Hours
* 0x06      Weekday
* 0x07      Day of Month
* 0x08      Month
* 0x09      Year
* 0x32      Century (maybe)
* 0x0A      Status Register A
* 0x0B      Status Register B
*/

enum {
	cmos_address = 0x70,
    cmos_data    = 0x71
};

int get_update_in_progress_flag() 
{
	outportb(cmos_address, 0x0A); // 0x0A --> Status Register A 
	return (inportb(cmos_data) & 0x80);
}

unsigned char get_RTC_register(int reg) 
{
	outportb(cmos_address, reg);
	return inportb(cmos_data);
}

/* 
* Function which reads the values from RTC - Real Time Clock
*/
void read_rtc()
{
	unsigned char century;
	unsigned char last_second;
	unsigned char last_minute;
	unsigned char last_hour;
	unsigned char last_day;
	unsigned char last_month;
	unsigned char last_year;
	unsigned char last_century;
	unsigned char registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    // to avoid getting inconsistent values due to RTC updates
	
	// Make sure an update isn't in progress
    while(get_update_in_progress_flag()); 
    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);
    if(century_register != 0) 
	{
		century = get_RTC_register(century_register);
    }

    do {
		last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;
		
		// Make sure an update isn't in progress
        while (get_update_in_progress_flag()); 
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
        if(century_register != 0)
		{
			century = get_RTC_register(century_register);
        }
	} while((last_second != second) || (last_minute != minute) || (last_hour != hour) || (last_day != day) || (last_month != month) || (last_year != year) || (last_century != century));
			 
	registerB = get_RTC_register(0x0B);

    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04)) 
	{
		second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if(century_register != 0) 
		{
			century = (century & 0x0F) + ((century / 16) * 10);
        }
	}

    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(registerB & 0x02) && (hour & 0x80)) 
	{
		hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year
    if(century_register != 0) 
	{
		year += century * 100;
    } else {
		year += (CURRENT_YEAR / 100) * 100;
        if(year < CURRENT_YEAR) year += 100;
	}
}

/*
* Function which print different between two pauses on screen
* Print in format hh:mm:ss
*/
void print_diferent()
{
	int total_second = hour * 3600 + minute * 60 + second;
	int total_second_temp = save_hour * 3600 + save_minute * 60 + save_second;
	total_sum += (total_second - total_second_temp);

	// Recalculate partial seconds, minutes and hours from total number of seconds
	int save_second_pause = total_sum % 60;
	int save_minute_pause = (total_sum % 3600) / 60;
	int save_hour_pause = total_sum / 3600;

	print_values(save_hour_pause, save_minute_pause, save_second_pause);
}

/*
* Function which print time on screen 
* Print in format hh:mm:ss
*/
void print_values(unsigned char hour, unsigned char minute, unsigned char second)
{
	// Print Hours
	int hoursFirst = hour / 10;
	int hourSecond = hour % 10;
	putch(hoursFirst + 0x30);
	putch(hourSecond + 0x30);
	putch(58);

	// Print Minutes
	int minuteFirst = minute / 10;
	int minuteSecond = minute % 10;
	putch(minuteFirst + 0x30);
	putch(minuteSecond + 0x30);
	putch(58);

	// Print Seconds
	int secondFirst = second / 10;
	int secondSecond = second % 10;
	putch(secondFirst + 0x30);
	putch(secondSecond + 0x30);
	puts("\n");
}

// Fully reset stopwatch, set all global parameters on zero
void reset_time()
{
	save_second = 0;
	save_minute = 0;
	save_hour = 0;
	total_sum = 0;
	check_pause = 0;
}

/*
* Function which execute when 's' is pressed on keyboard or left click on mouse or left touchpad click
* Simulate the "start function" on real stopwatch
*/
void press_start_digital_watch()
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

/*
* Function which execute when 'p' is pressed on keyboard or right click on mouse or right touchpad click
* Simulate the "pause function" on real stopwatch
*/
void press_pause_digital_watch()
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
	else {
		puts("Start: \n\tTrenutno vrijeme: ");
		beep();
		print_values(hour + 0x02, minute, second);
		save_second = second;
		save_minute = minute;
		save_hour = hour;
		check_pause = 1;
	}
}

/*
* Function which execute when 'r' is pressed on keyboard or scroll is clicked on mouse
* Simulate the "reset function" on real stopwatch
*/
void press_reset_digital_watch()
{
	cls();
	puts("\t\t\t\tStopwatch\n");
	reset_time();
}
