#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

PROCESS(button_process, "Button Process");
AUTOSTART_PROCESSES(&button_process);

PROCESS_THREAD(button_process, ev, data)
{
	PROCESS_BEGIN();
	
	SENSORS_ACTIVATE(button_sensor);
	static int count = 0;
	leds_mask_t blue_mask = 0x40;
	leds_mask_t green_mask = 0x20;
	
	while(1) {
	
	PROCESS_WAIT_EVENT_UNTIL(data == &button_sensor);

	count = count+1;
	printf("Button Press\n");

	if(count%2)
	{
		leds_on(blue_mask|green_mask);
	}
	else
	{
		leds_off(blue_mask|green_mask);
	}

	}
	
	PROCESS_END();
}
