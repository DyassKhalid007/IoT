#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> 
#include "dev/leds.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

//change this for sensing interval

static int SENSING_INTERVAL = 2;

#define SEND_INTERVAL (SENSING_INTERVAL * CLOCK_SECOND)




PROCESS(nullnet_example_process, "Post lab sink node");
AUTOSTART_PROCESSES(&nullnet_example_process);




typedef struct 
{
  int hum;
  int light;
  int temp; 
  int count;
  linkaddr_t addr;
}packet;

typedef struct 
{
  double hum_avg;
  double light_avg;
  double temp_avg;
}average;
static int current = 0;



#define SIZE 99 
static linkaddr_t array[SIZE];
average avgArray[SIZE];
static int pnum[SIZE];


  
void initializer()
{
  static int j =0;
  for(j=0;j<SIZE;j++)
  {
    avgArray[j].hum_avg = 0;
    avgArray[j].light_avg = 0;
    avgArray[j].temp_avg = 0;
    
  }

}










void insert(packet p)
{
  if(current!=SIZE-1)
  {

    array[current] = p.addr;
    avgArray[current].light_avg+=p.light;
    avgArray[current].temp_avg+=p.temp;
    avgArray[current].hum_avg+=p.hum;
    pnum[current]= p.count;


    current = current+1;


  }
  else
  {
    current = 0;
    array[current] = p.addr;
    avgArray[current].light_avg+=p.light;
    avgArray[current].temp_avg+=p.temp;
    avgArray[current].hum_avg+=p.hum;
    pnum[current]=p.count;

    current = current+1;
  }
}
int search(packet p)
{
  int i=0;
  for(i=0;i<current;i++)
  {
    // LOG_INFO("eNTEJDHFHFHFH");
    // LOG_INFO("%i",i);
    // LOG_INFO_LLADDR(&array[i]);
    // LOG_INFO_("\n");
    // LOG_INFO_LLADDR(&p.addr);
    if(linkaddr_cmp(&array[i],&p.addr))
    {

      if(p.count>pnum[i])
      {
        avgArray[i].light_avg+=p.light;
        avgArray[i].temp_avg+=p.temp;
        avgArray[i].hum_avg+=p.hum;
        pnum[i] = p.count;
        
      }
      else
      {
        return -2;
      }

      

      return i;
    }
  }
  return -1;
}

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(packet)) {
    

    packet p;
    p = *(packet *)data;
    
    LOG_INFO("Received %u from ", p.count);
    // LOG_INFO("Light val=%i",p.light);
    LOG_INFO_("\n");
    LOG_INFO_LLADDR(&p.addr);
    LOG_INFO_("\n");



    int result = search(p);
    // LOG_INFO("Result=%i",result);
    if(p.count%10==0&&result!=-1&&result!=-2)
    {
      LOG_INFO("pnum[result]=%i",pnum[result]);
      LOG_INFO("P.COUNT=%i",p.count);

      LOG_INFO("10 Pacekts arrived");
     avgArray[result].light_avg/= 10;
      avgArray[result].temp_avg/= 10;
      avgArray[result].hum_avg/=10;
      LOG_INFO("Average for packets generated at:");
      LOG_INFO_LLADDR(&array[result]);
      LOG_INFO_("\n");
      LOG_INFO("light avg:%u",(unsigned int)avgArray[result].light_avg);
      LOG_INFO_("\n");
      LOG_INFO("temp avg:%u",(unsigned int)avgArray[result].temp_avg);
      LOG_INFO_("\n");
      LOG_INFO("hum avg:%u",(unsigned int)avgArray[result].hum_avg);

      if(avgArray[result].light_avg>100||avgArray[result].temp_avg>24||avgArray[result].hum_avg>70)
         {
          leds_on(0x10);
         }
         else
         {
          leds_off(0x10);
         }

      avgArray[result].light_avg=0;
      avgArray[result].temp_avg=0;
      avgArray[result].hum_avg=0;
     
    }


    if(result==-1)
    {
      // LOG_INFO("Entered");
      insert(p);

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
  initializer();
  
    
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

