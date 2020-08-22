#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> 
#include "sht11-sensor.h"
#include "dev/light-sensor.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (8 * CLOCK_SECOND)


static int current = 0;
#define SIZE 99 
static linkaddr_t array[SIZE];

void insert(linkaddr_t p)
{
	if(current!=SIZE-1)
	{
		array[current++] = p;
	}
	else
	{
		current = 0;
		array[current++] = p;
	}
}
bool search(linkaddr_t p)
{
	int i=0;
	for(i=0;i<current;i++)
	{
		if(linkaddr_cmp(&array[i],&p))
		{
			return true;
		}
	}
	return false;
}

typedef struct
{
	int count;
	linkaddr_t addr;
	int hum;
  	int light;
  	int temp; 
	
}packet;

//helper functions 
static double 
convertTemp(double val)
{
  //d1+d2*val
  double d1 = -39.75;
  double d2 = 0.01;
  return d1+d2*val;

}

static  double 
convertHumid(double val,double temp)
{
  //12 bit values for constants 
  double c1 = -4;
  double c2 = 0.0405;
  double c3 = -2.8e-6;
  double rh_linear = ((c1+(c2*val))+(c3*val*val));
  

  double t1 = 0.01;
  double t2 = 0.00008;
  double rh_true = (((temp-25)*(t1+t2*val))+rh_linear);
  return rh_true;
}

static double 
converLight(double val)
{
  return (val*0.4521);

}





	PROCESS(receive_process, "Receive example");
	AUTOSTART_PROCESSES(&receive_process);


	void input_callback(const void *data, uint16_t len,
	  const linkaddr_t *src, const linkaddr_t *dest)
	{
	  if(len == sizeof(packet)) {
	    static packet p;	 
 	  	p = *(packet*)data;

	    // LOG_INFO("Received %u from ", p.count);
	    // LOG_INFO_LLADDR(src);
	    // LOG_INFO_("\n");

	    
	    LOG_INFO("Packet origin is:");
	    LOG_INFO_LLADDR(&p.addr);
	    LOG_INFO_("\n");

	    // LOG_INFO("Packet is currently at:");
	    // LOG_INFO_LLADDR(&linkaddr_node_addr);
	    // LOG_INFO_("\n");

	    // LOG_INFO("Packet came from:");
	    // LOG_INFO_LLADDR(src);
	    // LOG_INFO_("\n");


	    int result = search(p.addr);
	    if(result)
	    {
	    	// LOG_INFO("Packet already forwarded here");
	    }
	    else
	    {
	    	// LOG_INFO("Broadcasting");
	    	insert(p.addr);
	    	nullnet_buf=(void *)&p;
    		nullnet_len = sizeof(p);

	    	NETSTACK_NETWORK.output(NULL);
	    }


	  }
	}

	PROCESS_THREAD(receive_process, ev, data)
	{
	  static struct etimer periodic_timer;
	  static uint8_t count = 0;

	  PROCESS_BEGIN();
	  
	  nullnet_set_input_callback(input_callback);
	  etimer_set(&periodic_timer, SEND_INTERVAL);

	  static packet p;
	  p.count = 0;
	  static int light_val;
	  static int temp_val;
	  static int hum_val;
	  
	 
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending %u to ", count);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");

  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(sht11_sensor);
  //getting sensor data and converting the output into real world value using datasheet
  light_val = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
  light_val = converLight(light_val);
  temp_val = sht11_sensor.value(SHT11_SENSOR_TEMP);
  temp_val = convertTemp(temp_val);
  hum_val = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
  hum_val = convertHumid(hum_val,temp_val);

  p.light = light_val;
  p.temp = temp_val;
  p.hum = hum_val;


    p.count = p.count+1;
    nullnet_buf=(void *)&p;
    nullnet_len = sizeof(p);
    
    // nullnet_buf=&count;
    // nullnet_len = sizeof(count);
    
   
    p.addr = linkaddr_node_addr;
    // array[current++] = linkaddr_node_addr;

    NETSTACK_NETWORK.output(NULL);
    count++;
    current=0;
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
