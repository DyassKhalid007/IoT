
#include "contiki.h"
#include "dev/light-sensor.h"
#include <stdio.h>
#include "dev/leds.h"

PROCESS(light_sensing_process,"Light Sensor Example");
AUTOSTART_PROCESSES(&light_sensing_process);

PROCESS_THREAD(light_sensing_process,ev,data)
{ 
      static int val;
      static int threshold = 200;

      PROCESS_BEGIN();

      while(1)
      {
      	   SENSORS_ACTIVATE(light_sensor);
    	   
           val = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
           // val = -1;

      	   printf("Light=%02i \n",val);
           if(val>threshold)
           {
            leds_off(0x40);
            leds_on(0x10);
            
           }
           else //error check is also included
           {

            leds_off(0x10);
            leds_on(0x40);
           }


    	   SENSORS_DEACTIVATE(light_sensor);
    	   
	   clock_delay(1000);

      } 
    
      PROCESS_END();
}

