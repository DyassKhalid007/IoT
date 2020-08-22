
#include "contiki.h"
#include "dev/light-sensor.h"
#include <stdio.h>
#include "dev/leds.h"
#include "sht11-sensor.h"


//to be used in case oldest reading is to be replaced

// #define SIZE 10
// struct queue
// {
// 	int array[SIZE];
// 	int rear;
// 	int front;
// };

// void initialize(struct queue *q)
// {
// 	q->rear = 0;
// 	q->front = 0;
// 	// printf("Enter initialize\n");
// }



// void enqueue(struct queue*q,int d)
// {
// 	// printf("Entered enqueue\n");

// 	if(q->rear!=SIZE)
// 	{
// 		q->array[q->rear] = d;
// 		q->rear++;
// 		// printf("Added value %i\n",d);
// 	}
// }

// void dequeue(struct queue*q)
// {
// 	if(q->front!=q->rear)
// 	{
// 		int i =0;
// 		for(i=0;i<q->rear-1;i++)
// 		{
// 			q->array[i] = q->array[i+1];
// 		}
// 		q->rear = q->rear-1;
// 	}

// }

// void printq(struct queue*q)
// {
// 	int i =0;
// 	for(i=0;i<q->rear;i++)
// 	{
// 		printf("%i\n",q->array[i]);
// 	}
// }

// double average(struct queue*q)
// {
// 	int i = 0;
// 	double sum = 0;
// 	for(i=0;i<q->rear;i++)
// 	{
// 		sum+=q->array[i];
// 	}
// 	return sum/SIZE;
// }



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
	return (val*0.4071);
  // return 0.625*1e4*val;
  //return val*0.4521;
}
PROCESS(light_sensing_process,"Light Sensor Example");
AUTOSTART_PROCESSES(&light_sensing_process);

PROCESS_THREAD(light_sensing_process,ev,data)
{ 
      static double light_val;
      static double temp_val;
      static double hum_val;
      static double light_avg = 0;
      static double temp_avg = 0;
      static double hum_avg = 0;
      static int count = 0;
      static struct etimer et;
      const int SENSING_INTERVAL = 10;
      // static int decimal;
      // static double fraction;


      PROCESS_BEGIN();

      while(1)
      {

      		
      	   //activating sensors and waiting for the event
      	   SENSORS_ACTIVATE(light_sensor);
      	   SENSORS_ACTIVATE(sht11_sensor);
      	   etimer_set(&et, CLOCK_SECOND*SENSING_INTERVAL);
      	   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));


      	   //counter to be used for counting 10 readings
      	   count = count+1;
    	   
    	   //getting sensor data and converting the output into real world value using datasheet
           light_val = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
           if((int)light_val<0)
           {
            light_val = 0;
           }
           else
           {
            light_val = converLight(light_val);
           }
           

           temp_val = sht11_sensor.value(SHT11_SENSOR_TEMP);

           if((int)temp_val<0)
           {
            temp_val = 0;
            hum_val = 0;//becuase it depends on temperature 
           }
           else
           {
            temp_val = convertTemp(temp_val);
            hum_val = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
            if((int)hum_val<0)
            {
              hum_val = 0;
            }
            else
            {
              hum_val = convertHumid(hum_val,temp_val);
            }
            
           }
           // decimal = temp_val;
           // fraction = (temp_val-decimal)*100;
           // printf("Light=%02i ",(int)light_val);
           
          

           //calculatin sums
           // temp_val = 25;
           // hum_val = 0;
           // light_avg = 0;

           light_avg+=light_val;
           temp_avg+=temp_val;
           hum_avg+=hum_val;



       
           
	   if(count%10==0)
	   {

	   	 //calculating averages
	   	 light_avg = light_avg/10.0;
	   	 temp_avg = temp_avg/10.0;
	   	 hum_avg = hum_avg/10.0;


	   	 //some printing shiz for debugging 
	   	   printf("%i\n",count );
	   	   printf("Light Avg=%02u \n",(unsigned int)light_avg);
      	 printf("Temp Avg=%02u \n",(unsigned int)temp_avg);
      	 printf("Humidity Avg=%02u \n",(unsigned int)hum_avg);


      	 //checking the condition and then raising alarm based on that 

      	 if(light_avg>100||temp_avg>24||hum_avg>70)
      	 {
      	 	leds_on(0x10);
      	 }
      	 else
      	 {
      	 	leds_off(0x10);
      	 }


      	 //setting zeroes 
      	 count = 0;
      	 light_avg = 0;
      	 temp_avg = 0;
      	 hum_avg = 0;

	   }

	   //deactivating sensors and resetting the timer
	   SENSORS_DEACTIVATE(light_sensor);
       SENSORS_DEACTIVATE(sht11_sensor);
	   etimer_reset(&et);

      } 
    
      PROCESS_END();
}

