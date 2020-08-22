#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "sht11-sensor.h"
#include "dev/light-sensor.h"

#include <string.h>
#include <stdio.h> 

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (2 * CLOCK_SECOND)

static linkaddr_t dest_addr = {{ 0x01, 0x01, 0x01, 0x00, 0x01, 0x74, 0x12, 0x00 }};

typedef struct 
{
  int hum;
  int light;
  int temp; 
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
  return rh_true;
}

static double 
converLight(double val)
{
  return (val*0.4521);

}



PROCESS(nullnet_example_process, "NullNet unicast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  {
  // if(len == sizeof(unsigned)) {
  //   unsigned count;
  //   memcpy(&count, data, sizeof(count));
  //   LOG_INFO("Received %u from ", count);
  //   LOG_INFO_LLADDR(src);
  //   LOG_INFO_("\n");
  }
}

PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static int count = 1;
  static packet p;
  p.count = 0;
  static int light_val;
  static int temp_val;
  static int hum_val;
  PROCESS_BEGIN();

  // nullnet_buf = (uint8_t *)&count;
  // nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
    
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      // LOG_INFO("Sending %u to ", p.count);
      // LOG_INFO_LLADDR(&dest_addr);
      // LOG_INFO_("\n");

      //sensors shiz;
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
  p.count = count;

  nullnet_buf=(void *)&p;
  nullnet_len = sizeof(p);
  
      NETSTACK_NETWORK.output(&dest_addr);
      etimer_reset(&periodic_timer);
      count++;
    }


  PROCESS_END();
}

