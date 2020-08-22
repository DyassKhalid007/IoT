#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> 

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (2 * CLOCK_SECOND)

static linkaddr_t dest_addr1 = {{ 0x02, 0x02, 0x02, 0x00, 0x02, 0x74, 0x12, 0x00 }};
static linkaddr_t dest_addr2 = {{ 0x03, 0x03, 0x03, 0x00, 0x03, 0x74, 0x12, 0x00 }};


PROCESS(nullnet_example_process, "NullNet unicast example");
AUTOSTART_PROCESSES(&nullnet_example_process);


static double hum_val=0;
static double light_val=0;
static double temp_val=0;

static double second_hum_val=0;
static double second_light_val=0;
static double second_temp_val=0;





typedef struct 
{
  int hum;
  int light;
  int temp; 
  int count;
}packet;

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(packet)) {
    

    packet p;
    p = *(packet *)data;
    
    LOG_INFO("Received %u from ", p.count);
    // LOG_INFO("Light val=%i",p.light);
    LOG_INFO_("\n");
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");

    if(linkaddr_cmp(&dest_addr1,src))
    {
      light_val+=p.light;
      temp_val+=p.temp;
      hum_val+=p.hum;
    }
    else if(linkaddr_cmp(&dest_addr2,src))
    {
      second_light_val+=p.light;
      second_temp_val+=p.temp;
      second_hum_val+=p.hum;

    }

    
    if(p.count%10==0&&(linkaddr_cmp(&dest_addr1,src)))
    {
      LOG_INFO("10 Pacekts arrived");
      light_val = light_val/10;
      temp_val = temp_val/10;
      hum_val = hum_val/10;
      LOG_INFO("light avg:%u",(unsigned int)light_val);
      LOG_INFO_("\n");
      LOG_INFO("temp avg:%u",(unsigned int)temp_val);
      LOG_INFO_("\n");
      LOG_INFO("hum avg:%u",(unsigned int)hum_val);

      light_val = 0;
      temp_val = 0;
      hum_val = 0;
    }
    else if(p.count%10==0&&(linkaddr_cmp(&dest_addr2,src)))
    {

      LOG_INFO("10 Pacekts arrived");
      second_light_val = light_val/10;
      second_temp_val = temp_val/10;
      second_hum_val = hum_val/10;
      LOG_INFO("light avg:%u",(unsigned int)second_light_val);
      LOG_INFO_("\n");
      LOG_INFO("temp avg:%u",(unsigned int)second_temp_val);
      LOG_INFO_("\n");
      LOG_INFO("hum avg:%u",(unsigned int)second_hum_val);

      second_light_val = 0;
      second_temp_val = 0;
      second_hum_val = 0;

    }
  }
}

PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  // static unsigned count = 0;

  PROCESS_BEGIN();

  // nullnet_buf = (uint8_t *)&count;
  // nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  
    
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      // LOG_INFO("Sending %u to ", count);
      // LOG_INFO_LLADDR(&dest_addr);
      // LOG_INFO_("\n");


      // // NETSTACK_NETWORK.output(&dest_addr);

      // count++;
      etimer_reset(&periodic_timer);
    }


  PROCESS_END();
}

