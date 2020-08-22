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

typedef struct 
{
	double temp;
	double light;
	double hum;
	int count;
}packet;

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

	// printf("%i\n",rh_true);
	
	return rh_true;
}

static double 
converLight(double val)
{
	return (val*0.4521);
  // return 0.625*1e4*val;
  //return val*0.4521;
}

PROCESS(send_process, "Send example");
AUTOSTART_PROCESSES(&send_process);

PROCESS_THREAD(send_process, ev, data)
{
  static struct etimer periodic_timer;
  static uint8_t count = 0;
  static packet p;


  PROCESS_BEGIN();

  etimer_set(&periodic_timer, SEND_INTERVAL);
  static double light_val;
  static double temp_val;
  static double hum_val;      
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending %u to ", count);
    // LOG_INFO("Sending light=%i\n",light_val);
    // LOG_INFO("Sending temp=%i\n",temp_val);
    // LOG_INFO("Sending hum=%i\n",hum_val);
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

	p.temp = temp_val;
	p.light = light_val;
	p.hum = hum_val;
	p.count = count;

	
	// LOG_INFO("Sending light=%i\n",(int)light_val);
 //    LOG_INFO("Sending temp=%i\n",(int)temp_val);
 //    LOG_INFO("Sending hum=%i\n",(int)hum_val);
 //    LOG_INFO_LLADDR(NULL);
    // LOG_INFO_("\n");



    nullnet_buf= (void*)&p;
    nullnet_len = sizeof(p);


    NETSTACK_NETWORK.output(NULL);
    count++;
    etimer_reset(&periodic_timer);
   
  }

  PROCESS_END();
}
	
	
