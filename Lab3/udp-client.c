#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sht11-sensor.h"
#include "dev/light-sensor.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL (2 * CLOCK_SECOND)



static struct simple_udp_connection udp_conn;
typedef struct 
{
  int hum;
  int light;
  int temp; 
  int count;
  
  

  
}packet;
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
static double 
convertTemp(double val)
{
  //d1+d2*val
  double d1 = -39.75;
  double d2 = 0.01;
  return d1+d2*val;

}
static int 
getLight()
{
	SENSORS_ACTIVATE(light_sensor);
	int light_val = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
  	light_val = converLight(light_val);
  	SENSORS_DEACTIVATE(light_sensor);

  	// LOG_INFO("Light_val=%i",light_val);
  	return light_val;
}
static int 
getTemp()
{
	SENSORS_ACTIVATE(sht11_sensor);
	int temp_val = sht11_sensor.value(SHT11_SENSOR_TEMP);
  	temp_val = convertTemp(temp_val);
  	// LOG_INFO("Temp_val=%i",temp_val);
  	SENSORS_DEACTIVATE(sht11_sensor);
  	return temp_val;
}
static int 
getHum()
{
	SENSORS_ACTIVATE(sht11_sensor);
	int temp_val = sht11_sensor.value(SHT11_SENSOR_TEMP);
  	temp_val = convertTemp(temp_val);
  	int hum_val = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
  	hum_val = convertHumid(hum_val,temp_val);
  	// LOG_INFO("Hum_val=%i",hum_val);
  	SENSORS_DEACTIVATE(sht11_sensor);
  	return hum_val;

}



PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);

static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count=1;
  // static char str[32];
  uip_ipaddr_t dest_ipaddr;
  uip_ipaddr_t addr;
  uip_gethostaddr(&addr);

  // uip_ipaddr_t src_ipaddr;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);


  //defining my packet cutie
  static packet p;

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      LOG_INFO("Sending request %u to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");


      // LOG_INFO("Request send from:");
      // LOG_INFO_6ADDR(&addr);
      
 	  
     //  LOG_INFO("Sending request from:");
     //  NETSTACK_ROUTING.get_sr_node_ipaddr(&src_ipaddr);
     // LOG_INFO_6ADDR(&src_ipaddr);

      LOG_INFO_("\n");
      // snprintf(str, sizeof(str), "hello %d", count);

      //getting sensor values 
      // int light = getLight();
      // LOG_INFO("%i",light);
      p.light = getLight();
      p.temp = getTemp();
      p.hum = getHum();
      p.count = count;
      LOG_INFO("p.light=%i",p.light);
      LOG_INFO_("\n");
      
      
      simple_udp_sendto(&udp_conn, (void*)&p, sizeof(packet), &dest_ipaddr);
      count++;
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
