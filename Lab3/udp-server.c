#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"

#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uiplib.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;
typedef struct 
{
  int hum;
  int light;
  int temp; 
  int count;
  
  
}packet;

typedef struct 
{
	double light_avg;
	double temp_avg;
	double hum_avg;

}aveage;

#define SIZE 3
static aveage avg[SIZE];
#define mytype uip_ipaddr_t


static mytype ptrArray[SIZE];
static int current = 0;


void setArrayAvg()
{
	static int i = 0;
	for(i=0;i<SIZE;i++)
	{
		avg[i].light_avg = 0;
		avg[i].temp_avg = 0;
		avg[i].hum_avg = 0;
	}
}
void setIndexArrayAvg(int i)
{
	avg[i].light_avg=0;
	avg[i].temp_avg=0;
	avg[i].hum_avg=0;
}

void calculateAvg(int i)
{
	avg[i].light_avg/=10;
	avg[i].temp_avg/=10;
	avg[i].hum_avg/=10;

	LOG_INFO("Light avg=%u",(unsigned int)avg[i].light_avg);
	LOG_INFO_("\n");
	LOG_INFO("Temp avg=%u",(unsigned int)avg[i].temp_avg);
	LOG_INFO_("\n");
	LOG_INFO("Hum avg=%u",(unsigned int)avg[i].hum_avg);
	LOG_INFO_("\n");
}


static bool searchMyTypeArray(mytype p)
{
	// if(current==-1)
	// {
	// 	LOG_INFO("Returning gibberish");
	// 	return false;
	// }
	static int i=0;
	// LOG_INFO("aLREADY PRESENT");
	for(i=0;i<SIZE;i++)
	{

		// LOG_INFO_6ADDR(ptrArray[i]);
		if(uip_ipaddr_cmp(&ptrArray[i],&p))
		{
			
			return true;
		}
	}
	

	return false;
}

static int searchIndexMyTypeArray(mytype p)
{
	static int i=0;
	for(i=0;i<SIZE;i++)
	{
		// LOG_INFO("Comparing:");
		// LOG_INFO_6ADDR(ptrArray[i]);
		// LOG_INFO_("\n");
		// LOG_INFO_6ADDR(p);
		if(uip_ipaddr_cmp(&ptrArray[i],&p))
		{
			return i;
		}
	}

	return -1;

}


void insertPtr(mytype p)
{
	ptrArray[current++] = p;
}

static bool checkPacketCount(packet p)
{
	if(p.count%10==0)
	{
		return true;
	}
	return false;
}
void debugMyData(packet p);

void debugAvg(int i)
{
	LOG_INFO("Light sum=%i",(int)avg[i].light_avg);
	LOG_INFO_("\n");
	LOG_INFO("Temp sum=%i",(int)avg[i].temp_avg);
	LOG_INFO_("\n");
	LOG_INFO("Hum sum=%i",(int)avg[i].hum_avg);
	LOG_INFO_("\n");
}
void addToAvg(packet p,int i)
{


	// debugMyData(p);
	avg[i].light_avg+=p.light;
	avg[i].hum_avg+=p.hum;
	avg[i].temp_avg+=p.temp;
	// debugAvg(i);
}

void debugMyData(packet p)
{
	LOG_INFO("Temp=%i",(int)p.temp);
	LOG_INFO_("\n");
	LOG_INFO("Hum=%i",(int)p.hum);
	LOG_INFO_("\n");
	LOG_INFO("Light=%i",(int)p.light);
	LOG_INFO_("\n");
}


PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

	static void
	receive_packet(struct simple_udp_connection *c,
		 const uip_ipaddr_t *sender_addr,
		 uint16_t sender_port,
		 const uip_ipaddr_t *receiver_addr,
		 uint16_t receiver_port,
		 const uint8_t *data,
		 uint16_t datalen)

	{

		static packet p;
		p = *(packet *) data;
		



	  LOG_INFO("Received packet from:");
	  LOG_INFO_6ADDR(sender_addr);
	  LOG_INFO_("\n");
	  

	  
	  
	  static int result=0;
	  static uip_ipaddr_t dp;
	  dp = *sender_addr;
	
	  

	  result = searchMyTypeArray(dp);
	  LOG_INFO("Packet count:%i",p.count);
	  LOG_INFO_("\n");
	  // LOG_INFO("p.light=%i",p.light);
	  
	  if(!result)
	  {
	  	// LOG_INFO("SHSHIFKHFSJSFH");
	  	insertPtr(dp);
	  }
	  static int index=0; 
	  index = searchIndexMyTypeArray(dp);
	  addToAvg(p,index);
	  if(checkPacketCount(p))
	  {
	  	calculateAvg(index);
	  	setIndexArrayAvg(index);
	  }


	  


	}



PROCESS_THREAD(udp_server_process, ev, data)
{
	// setMyTypeArray();
 setArrayAvg();
  PROCESS_BEGIN();

  //initializing my ptr array
  

  /* Initialize DAG root */

  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, receive_packet);

  PROCESS_END();
}

