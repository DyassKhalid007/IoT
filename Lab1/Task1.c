
#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h> 

PROCESS(blink_process, "Blink LED process");
AUTOSTART_PROCESSES(&blink_process);


PROCESS_THREAD(blink_process, ev, data)
{
	//Order:Green,Red,Blue

	PROCESS_BEGIN();
	leds_mask_t red_mask = 0x10; 
	leds_mask_t blue_mask = 0x40;
	leds_mask_t green_mask = 0x20;
	while(1)
	{
	    
		leds_on(green_mask);
		clock_delay(10000);
		leds_off(green_mask);
		leds_on(green_mask|red_mask);
		clock_delay(10000);
		leds_off(green_mask|red_mask);
		leds_on(green_mask|red_mask|blue_mask);
		clock_delay(10000);
		leds_off(green_mask|red_mask|blue_mask);
		clock_delay(10000);

	}
	
	PROCESS_END();
}

