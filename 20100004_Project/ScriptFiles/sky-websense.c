#include "contiki.h"
#include "httpd-simple.h"
#include "dev/sht11/sht11-sensor.h"
#include "dev/light-sensor.h"
#include "dev/leds.h"
#include <stdio.h>



static int pcount = 1;


PROCESS(web_sense_process, "Sense Web Demo");
PROCESS(webserver_nogui_process, "Web server");

PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}

AUTOSTART_PROCESSES(&web_sense_process,&webserver_nogui_process);

#define HISTORY 16
static int temperature[HISTORY];
static int light1[HISTORY];
static int hum1[HISTORY];



static int sensors_pos;

static int
get_light(void)
{
  return 10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7;
}

static int
get_temp(void)
{
  return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
}




static int 
getHum()
{
  
  // int hum_val = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
  // float first = -4+(0.0405*hum_val)+(-2.8e-6*hum_val*hum_val);
  

    

    return 116;
}
static int 
calculateHumAvg()
{

  
    static int i = 0;
    int avg = 0;

    i = sensors_pos;
    do 
    {
        avg+=hum1[i];
        i  = (i+1)%HISTORY;

    }while(i!=sensors_pos);

    return (int)avg/HISTORY;
    
}


// static const char *TOP = "<html><head><title>Contiki Web Sense</title></head><body>\n";
// static const char *BOTTOM = "</body></html>\n";

static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)


//function found on internet
static void
generate_chart(const char *title, const char *unit, int min, int max, int *values)
{
  int i;
  
  ADD("<h1>%s</h1>\n"
      "<img src=\"http://chart.apis.google.com/chart?"
      "cht=lc&chs=400x300&chxt=x,x,y,y&chxp=1,100|3,100&"
      "chxr=2,%d,%d|0,0,30&chds=%d,%d&chxl=1:|Time|3:|%s&chd=t:",
      title, min, max, min, max, unit);
  for(i = 0; i < HISTORY; i++) {
    ADD("%s%d", i > 0 ? "," : "", values[(sensors_pos + i) % HISTORY]);
  }
  ADD("\">");
}
static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  // SEND_STRING(&s->sout, TOP);

  if(strncmp(s->filename, "/index", 6) == 0 
     ) {
    blen = 0;
    ADD(
      "{\"Light\""":%u,\"Temperature\""":%u,\"Humidity\""":%u}",
        get_light(),get_temp(),getHum());
    SEND_STRING(&s->sout, buf);
  }
  else if(strncmp(s->filename, "/0", 2) == 0)
  {
    blen = 0;
    static int val = 0;
    if(pcount>=16)
    { 
      val = calculateHumAvg();

    }
    ADD("<h1>Current readings</h1>\n"
        
        "Humidity:%u<br>"
        "Humidity Average:%u<br>"
        "Pcount:%u<br>",
       
        getHum(),val,pcount);
    SEND_STRING(&s->sout, buf);

  }
  else if(strncmp(s->filename,"/1",2)==0)
  {
    blen=0;
    generate_chart("Temperature", "Celsius", 0, 50, temperature);
    SEND_STRING(&s->sout, buf);

  }
  else if(strncmp(s->filename,"/2",2)==0)
  {
    blen=0;
    generate_chart("Humidity", "%RH", 0, 150, hum1);
    SEND_STRING(&s->sout, buf);

  }
  else if(strncmp(s->filename,"/3",2)==0)
  {
    /* Turn on leds */
    blen=0;
    leds_on(LEDS_RED);
    SEND_STRING(&s->sout, "Turned on leds!");
  } 
  else if(strncmp(s->filename,"/4",2)==0)
  {
    blen = 0;
    leds_off(LEDS_RED);
    
    SEND_STRING(&s->sout,"Turned off leds!");
  }
  // SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}

httpd_simple_script_t httpd_simple_get_script(const char *name)
{
  return send_values;
}


PROCESS_THREAD(web_sense_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  sensors_pos = 0;


  etimer_set(&timer, CLOCK_SECOND * 2);
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

    light1[sensors_pos] = get_light();;
    temperature[sensors_pos] = get_temp();
    hum1[sensors_pos] = getHum();
    

    sensors_pos = (sensors_pos + 1) % HISTORY;
    pcount++;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

