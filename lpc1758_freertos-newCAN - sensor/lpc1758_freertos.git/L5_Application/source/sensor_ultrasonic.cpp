/*  Ping))) 3 pin sensor

 *  by Rutwik (Preet's suggestion)       Date: 11/5/2014
 *
 *
 */

#include <stdio.h>
#include "utilities.h"
#include "uart0_min.h"
#include "eint.h"
#include "sensor_ultrasonic.hpp"
#include "tasks.hpp"
#include "io.hpp"
#include "adc0.h"
#include "semphr.h"
#include "queue.h"
#include "uart0.hpp"
#include "controller_sensor.hpp"
#include "math.h"

#ifdef ENABLE_SENSOR // enabled from can_common.hpp

bool edge_center = 1, edge_right = 0, edge_left = 0;      //1: flag for pulse status

int reading =0,distance,light_percent;
float voltage,bat_volt,light_reading,bat_reading;

int systime_center = 0, systime_right = 0, systime_left = 0;    //variable for storing system_get_uptime
int x = 0, y = 0, z = 0;
uint8_t ultraDist[5];
QueueHandle_t queueL = 0;
QueueHandle_t queueR = 0;
QueueHandle_t queueC = 0;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;


extern dist_sensor_data_t  periodic_dist_data;
extern other_sensor_data_t periodic_other_data;

//*********************** Function to filter the glitches

int calc_mean_val_left(int num)
{
#ifdef MEAN_QUEUE_LEN
#error "MEAN_QUEUE_LEN already defined!"
#else
#define MEAN_QUEUE_LEN 10
#endif

   static int queue[MEAN_QUEUE_LEN];
   static int sorted_queue[MEAN_QUEUE_LEN];
   static int head = 0;
   static int tail = (MEAN_QUEUE_LEN - 1);

   int i;
   int temp;

   int removed_data, removed_flag;
   int insert_data, insert_flag;

   removed_data = queue[head];
   head = ((head + 1) % MEAN_QUEUE_LEN);
   tail = ((tail + 1) % MEAN_QUEUE_LEN);
   queue[tail] = num;

   removed_flag = 0;
   insert_data = num;
   insert_flag = 0;
   for(i = 0; i < (MEAN_QUEUE_LEN - 1); i++) {
      if(removed_data == sorted_queue[i] || removed_flag) {
         sorted_queue[i] = sorted_queue[i + 1];
         removed_flag = 1;
      }

      if(insert_data < sorted_queue[i] || insert_flag) {
         temp = insert_data;
         insert_data = sorted_queue[i];
         sorted_queue[i] = temp;
         insert_flag = 1;
      }
   }
   sorted_queue[i] = insert_data;

   return sorted_queue[MEAN_QUEUE_LEN / 2];

#undef MEAN_QUEUE_LEN
}

int calc_mean_val_right(int num)
{
#ifdef MEAN_QUEUE_LEN
#error "MEAN_QUEUE_LEN already defined!"
#else
#define MEAN_QUEUE_LEN 10
#endif

   static int queue[MEAN_QUEUE_LEN];
   static int sorted_queue[MEAN_QUEUE_LEN];
   static int head = 0;
   static int tail = (MEAN_QUEUE_LEN - 1);

   int i;
   int temp;

   int removed_data, removed_flag;
   int insert_data, insert_flag;

   removed_data = queue[head];
   head = ((head + 1) % MEAN_QUEUE_LEN);
   tail = ((tail + 1) % MEAN_QUEUE_LEN);
   queue[tail] = num;

   removed_flag = 0;
   insert_data = num;
   insert_flag = 0;
   for(i = 0; i < (MEAN_QUEUE_LEN - 1); i++) {
      if(removed_data == sorted_queue[i] || removed_flag) {
         sorted_queue[i] = sorted_queue[i + 1];
         removed_flag = 1;
      }

      if(insert_data < sorted_queue[i] || insert_flag) {
         temp = insert_data;
         insert_data = sorted_queue[i];
         sorted_queue[i] = temp;
         insert_flag = 1;
      }
   }
   sorted_queue[i] = insert_data;

   return sorted_queue[MEAN_QUEUE_LEN / 2];

#undef MEAN_QUEUE_LEN
}

int calc_mean_val_center(int num)
{
#ifdef MEAN_QUEUE_LEN
#error "MEAN_QUEUE_LEN already defined!"
#else
#define MEAN_QUEUE_LEN 10
#endif

   static int queue[MEAN_QUEUE_LEN];
   static int sorted_queue[MEAN_QUEUE_LEN];
   static int head = 0;
   static int tail = (MEAN_QUEUE_LEN - 1);

   int i;
   int temp;

   int removed_data, removed_flag;
   int insert_data, insert_flag;

   removed_data = queue[head];
   head = ((head + 1) % MEAN_QUEUE_LEN);
   tail = ((tail + 1) % MEAN_QUEUE_LEN);
   queue[tail] = num;

   removed_flag = 0;
   insert_data = num;
   insert_flag = 0;
   for(i = 0; i < (MEAN_QUEUE_LEN - 1); i++) {
      if(removed_data == sorted_queue[i] || removed_flag) {
         sorted_queue[i] = sorted_queue[i + 1];
         removed_flag = 1;
      }

      if(insert_data < sorted_queue[i] || insert_flag) {
         temp = insert_data;
         insert_data = sorted_queue[i];
         sorted_queue[i] = temp;
         insert_flag = 1;
      }
   }
   sorted_queue[i] = insert_data;

   return sorted_queue[MEAN_QUEUE_LEN / 2];

#undef MEAN_QUEUE_LEN
}


int calc_mean_val_back(int num)
{
#ifdef MEAN_QUEUE_LEN
#error "MEAN_QUEUE_LEN already defined!"
#else
#define MEAN_QUEUE_LEN 10
#endif

   static int queue[MEAN_QUEUE_LEN];
   static int sorted_queue[MEAN_QUEUE_LEN];
   static int head = 0;
   static int tail = (MEAN_QUEUE_LEN - 1);

   int i;
   int temp;

   int removed_data, removed_flag;
   int insert_data, insert_flag;

   removed_data = queue[head];
   head = ((head + 1) % MEAN_QUEUE_LEN);
   tail = ((tail + 1) % MEAN_QUEUE_LEN);
   queue[tail] = num;

   removed_flag = 0;
   insert_data = num;
   insert_flag = 0;
   for(i = 0; i < (MEAN_QUEUE_LEN - 1); i++) {
      if(removed_data == sorted_queue[i] || removed_flag) {
         sorted_queue[i] = sorted_queue[i + 1];
         removed_flag = 1;
      }

      if(insert_data < sorted_queue[i] || insert_flag) {
         temp = insert_data;
         insert_data = sorted_queue[i];
         sorted_queue[i] = temp;
         insert_flag = 1;
      }
   }
   sorted_queue[i] = insert_data;

   return sorted_queue[MEAN_QUEUE_LEN / 2];

#undef MEAN_QUEUE_LEN
}
//**********************Trigger and Echo Functions (including queue receive)
void trigger_left (void)
{
    LPC_GPIO2->FIODIR |= echo_pin_left;                //configure as output for next trigger pulse
    LPC_GPIO2->FIOCLR = echo_pin_left;
    LPC_GPIO2->FIOSET = echo_pin_left;
    delay_us(5);  // ideally only 2-5 us pulse required
    LPC_GPIO2->FIOCLR = echo_pin_left;
    //delay_us(700);
    systime_left = (int)sys_get_uptime_us();
    edge_left = 1;
    LPC_GPIO2->FIODIR &= ~echo_pin_left;     //configure as input
}

int wait_for_Lsensor_data()
{
    int value;
    LPC_GPIO2->FIODIR &= ~echo_pin_left;     //configure as input
    if (!xQueueReceive(queueL, &value, 30)) {
        value = 0;
    }

    edge_left = 1;
    return value;
}

void trigger_right (void)
{
    LPC_GPIO2->FIODIR |= echo_pin_right;                //configure as output for next trigger pulse
    LPC_GPIO2->FIOCLR = echo_pin_right;
    LPC_GPIO2->FIOSET = echo_pin_right;
    delay_us(5);  // ideally only 2-5 us pulse required
    LPC_GPIO2->FIOCLR = echo_pin_right;
    //delay_us(700);
    systime_right = (int)sys_get_uptime_us();
    edge_right = 1;
    LPC_GPIO2->FIODIR &= ~echo_pin_right;     //configure as input
}

int wait_for_Rsensor_data()
{
    int value;
    LPC_GPIO2->FIODIR &= ~echo_pin_right;     //configure as input
    if (!xQueueReceive(queueR, &value, 30)) {
        value = 0;
    }

    edge_right = 1;
    return value;
}


void trigger_center (void)
{
    LPC_GPIO2->FIODIR |= echo_pin_center;                //configure as output for next trigger pulse
    LPC_GPIO2->FIOCLR = echo_pin_center;
    LPC_GPIO2->FIOSET = echo_pin_center;

//    LPC_GPIO0->FIOCLR = echo_pin_center;
//    LPC_GPIO0->FIOSET = echo_pin_center;

    delay_us(5);  // ideally only 2-5 us pulse required

    LPC_GPIO2->FIOCLR = echo_pin_center;
    //LPC_GPIO0->FIOCLR = echo_pin_center;

    //delay_us(700);
    systime_center = (int)sys_get_uptime_us();
    edge_center = 1;

    LPC_GPIO2->FIODIR &= ~echo_pin_center;     //configure as input
    //LPC_GPIO0->FIODIR &= ~echo_pin_center;     //configure as input

}

int wait_for_Csensor_data()
{
    int value;
    LPC_GPIO2->FIODIR &= ~echo_pin_center;     //configure as input

   // LPC_GPIO0->FIODIR &= ~echo_pin_center;     //configure as input
    if (!xQueueReceive(queueC, &value, 30)) {
        value = 0;
    }

    edge_center = 1;
    return value;
}

//******************** Init functions for sensors and interrupt (callback functions)
void init_io_left()
{
    LPC_PINCON->PINSEL4 &= ~(3<<2);    //P2.1 as GPIO (for Echo pin)
    LPC_GPIO2->FIODIR |= echo_pin_left;      //configure as output
}

void init_io_center()
{

    LPC_GPIO2->FIODIR |= echo_pin_center;      //configure as output

    //LPC_GPIO0->FIODIR |= echo_pin_center;      //configure as output
}

void init_io_right()
{
    LPC_PINCON->PINSEL4 &= ~(3<<0);    //P2.0 as GPIO (for Echo pin)
    LPC_GPIO2->FIODIR |= echo_pin_right;      //configure as output
}
void initEchoInt_left(eint_intr_t eintType)
{
    eint3_enable_port2(1,eintType,ext_callback_left);  //P2.1 - Echo pin
}
void initEchoInt_right(eint_intr_t eintType)
{
    eint3_enable_port2(0,eintType,ext_callback_right);  //p2.0 - Echo pin
}
void initEchoInt_center(eint_intr_t eintType)
{
    eint3_enable_port2(2,eintType,ext_callback_center);  //P2.2 - Echo pin

   // eint3_enable_port0(26,eintType,ext_callback_center);  //P0.26 - Echo pin
}
void ext_callback_left()
{
    //long yield = 0;
    //systime_left = (int)sys_get_uptime_us() - systime_left; //get the duration of pulse
    x = (int)sys_get_uptime_us() - systime_left; //get the duration of pulse
    //edge_left = 0;                                     //echo pulse width is available
   // LPC_GPIO2->FIODIR |= echo_pin_left;                //configure as output for next trigger pulse
    xQueueSendFromISR(queueL, (void*)&x, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void ext_callback_right()
{
    //long yield = 0;
  //  long task_awake=0;
    xHigherPriorityTaskWoken = pdFALSE;
    y = (int)sys_get_uptime_us() - systime_right; //get the duration of pulse
   // edge_right = 0;                                     //echo pulse width is available
   // LPC_GPIO2->FIODIR |= echo_pin_right;                //configure as output for next trigger pulse

    xQueueSendFromISR(queueR, (void*)&y, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void ext_callback_center()
{
    //long yield = 0;
   // LE.on(3);
  //  long task_awake=0;
    xHigherPriorityTaskWoken = pdFALSE;
    z = (int)sys_get_uptime_us() - systime_center; //get the duration of pulse
   // edge_right = 0;                                     //echo pulse width is available

    //LPC_GPIO2->FIODIR |= echo_pin_center;                //configure as output for next trigger pulse

    //LPC_GPIO0->FIODIR |= echo_pin_center;                //configure as output for next trigger pulse

    xQueueSendFromISR(queueC, (void*)&z, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


sendTrigTask::sendTrigTask() : scheduler_task("trig", 4 * 512, PRIORITY_CRITICAL)
{

}

//********************** Trigger Task init function
bool sendTrigTask::init(void)
{
    init_io_center();
    initEchoInt_center(eint_falling_edge);      //Look rising edge of echo pulse
    setRunDuration(50);
   // init_io_center();
   // initEchoInt_center(eint_falling_edge);      //Look rising edge of echo pulse

    init_io_left();
    initEchoInt_left(eint_falling_edge);      //Look rising edge of echo pulse

    init_io_right();
    initEchoInt_right(eint_falling_edge);      //Look rising edge of echo pulse
    queueL = xQueueCreate(1, sizeof(int));
    queueR = xQueueCreate(1, sizeof(int));
    queueC = xQueueCreate(1, sizeof(int));
    LPC_PINCON->PINSEL3 |= (3 << 28); // ADC-4 is on P1.30, select this as ADC0.4
//    LPC_PINCON->PINSEL3 |= (3 << 30); // ADC-5 is on P1.31, select this as ADC0.5
    //LE.init();
    //LD.init();
   // LD.setNumber(0);

    return true;
}

bool sendTrigTask::run(void *p)
{
//TODO : Sending FLOAT value/ Computing actual distance
    int sensorLData = 0, sensorRData = 0, sensorCData = 0;
    //printf("\nTriggering left\n");

    trigger_left();
    sensorLData = calc_mean_val_left(wait_for_Lsensor_data());
    periodic_dist_data.left = ((sensorLData-750)*13/22500);

//    if(((sensorLData-750)*13/22500)<3)
//        LD.setLeftDigit(1);
   // printf("\nLeft %d %d", ((sensorLData-750)*13/22500), (sensorLData-750)*13/750);
    // printf("\n                           Triggering Right\n");
    trigger_right();
    sensorRData =calc_mean_val_right(wait_for_Rsensor_data());
    periodic_dist_data.right = (sensorRData-750)*13/22500;
//    if((sensorRData-750)*13/22500<3)
//        LD.setRightDigit(1);
   // printf("\n          Right  %d %d", (sensorRData - 750)*13/22500, (sensorRData - 750)*13/750);
    //  printf("                Triggering Center\n");
    trigger_center();
    sensorCData =calc_mean_val_center(wait_for_Csensor_data());
    periodic_dist_data.middle = (sensorCData-750)*13/22500;

   // LD.setNumber(periodic_dist_data.middle);

   // printf("\n                      Center  %d %d", (sensorCData - 750)*13/22500, (sensorCData - 750)*13/750);

    reading = adc0_get_reading(4); // Read current value of ADC-4
    voltage = ((3.3 / 4096)*reading); // Output voltage of sensor connected to adc
    distance = (voltage*1000)/6.4 ; //distance in inches //  6.4mv/inch
    distance=calc_mean_val_back(distance*2.54);
    distance = distance/30;
    periodic_dist_data.back = distance;
    printf("L  %d C   %d R   %d B   %d\n",periodic_dist_data.left, periodic_dist_data.middle, periodic_dist_data.right, periodic_dist_data.back);
//    if(distance<3)
//                LD.setRightDigit(2);
   // printf("\n                                   back: %d",distance);          //used as a front sensor
//    periodic_other_data.light = LS.getPercentValue();
//    periodic_other_data.battery = 0;

    return true;


//    int sensorLData = 0, sensorRData = 0, sensorCData = 0;
//    //printf("\nTriggering left\n");
//    trigger_left();
//    sensorLData = calc_mean_val_left(wait_for_Lsensor_data());
//    periodic_dist_data.left = (sensorLData-750)*13/22500;
//    printf("\nLeft %d %d", (sensorLData-750)*13/22500, (sensorLData-750)*13/750);
//   // printf("\n                           Triggering Right\n");
//    trigger_right();
//    sensorRData =calc_mean_val_right(wait_for_Rsensor_data());
//    periodic_dist_data.right = (sensorRData-750)*13/22500;
//    printf("\n          Right  %d %d", (sensorRData - 750)*13/22500, (sensorRData - 750)*13/750);
//  //  printf("                Triggering Center\n");
//    trigger_center();
//    sensorCData =calc_mean_val_center(wait_for_Csensor_data());
//    periodic_dist_data.middle = (sensorCData-750)*13/22500;
//    printf("\n                      Center  %d %d", (sensorCData - 750)*13/22500, (sensorCData - 750)*13/750);
//    reading = adc0_get_reading(4); // Read current value of ADC-4
//    voltage = ((3.3 / 4096)*reading); // Output voltage of sensor connected to adc
//    distance = (voltage*1000)/6.4 ; //distance in inches //  6.4mv/inch
//    distance=calc_mean_val_back(distance*2.54);
//    distance = distance/30;
//    periodic_dist_data.back = distance;
//    printf("\n                                   back: %d",distance);          //used as a front sensor
//    periodic_other_data.light = LS.getPercentValue();
//    periodic_other_data.battery = 0;
//    return true;
}

otherSensorsTask::otherSensorsTask() : scheduler_task("othr", 4 * 512, PRIORITY_MEDIUM)
{
    setRunDuration(1153);
}

bool otherSensorsTask::init(void)
{
    LPC_PINCON->PINSEL1 |=  (1 << 20); // ADC-3 is on P0.26, select this as ADC0.3
    LPC_PINCON->PINSEL3 |= (3 << 30); // ADC-5 is on P1.31, select this as ADC0.5
    return true;
}

bool otherSensorsTask::run(void *p)
{
    light_reading = adc0_get_reading(3); // Read current value of ADC-3
    light_reading= (light_reading/4096)*100;
    light_percent=int(light_reading);
    printf("Light Sensor : %d percent\n",light_percent);
    periodic_other_data.light = light_percent;

//     periodic_other_data.light = LS.getPercentValue();

//    light_reading = adc0_get_reading(3); // Read current value of ADC-3
//    periodic_other_data.light = reading*100/4096; //converting to percentage

    bat_reading = adc0_get_reading(5); // Read current value of ADC-5

    bat_volt = (bat_reading/4096.0)*3.3;
    printf("bat_volt = %f\n", bat_volt);
    //***********8edited
    //periodic_other_data.battery = bat_volt;
    //printf("Battery Sensor : %d percent\n",periodic_other_data.battery);

    if(bat_volt <= 2.30)
        periodic_other_data.battery =10;
    else if(bat_volt>2.30 && bat_volt <=2.35)
        periodic_other_data.battery =10;
    else if(bat_volt>2.35 && bat_volt <=2.40)
        periodic_other_data.battery =20;
    else if(bat_volt>2.40 && bat_volt <=2.45)
        periodic_other_data.battery =30;
    else if(bat_volt>2.45 && bat_volt <=2.50)
        periodic_other_data.battery =40;
    else if(bat_volt>2.50 && bat_volt <=2.55)
        periodic_other_data.battery =50;
    else if(bat_volt>2.55 && bat_volt <=2.60)
        periodic_other_data.battery =60;
    else if(bat_volt>2.60 && bat_volt <=2.65)
        periodic_other_data.battery =70;
    else if(bat_volt>2.65 && bat_volt <=2.70)
        periodic_other_data.battery =80;
    else if(bat_volt>2.70 && bat_volt <=2.75)
        periodic_other_data.battery =90;
    else if(bat_volt>2.75 && bat_volt <=2.80)
        periodic_other_data.battery =100;
    else
        periodic_other_data.battery =100;
    printf("Battery Sensor : %d percent\n",periodic_other_data.battery);

    return true;
}

#endif /* ENABLE_SENSOR */


