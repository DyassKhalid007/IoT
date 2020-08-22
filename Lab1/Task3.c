#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include "dev/leds.h"



PROCESS(timer_process,"Timer Process");
AUTOSTART_PROCESSES(&timer_process);

PROCESS_THREAD(timer_process,ev,data)
{ 
      static struct etimer et;
      static int array[] = {0x10,0x20,0x40};
      static int r = 0;
      PROCESS_BEGIN();

      
      while(1)
      {
         r = rand()%3;
         if(r<0) //if remainder is <0 simply adding the dividend corrects it as per euclids algorithm
         {
            r = r+3;
         }

         etimer_set(&et, CLOCK_SECOND*1);
         PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

         printf("Timer Expired!!!\n"); //check that 1 second has passed down
         leds_toggle(array[r]);

         etimer_reset(&et);

      }
    
      PROCESS_END();
}
