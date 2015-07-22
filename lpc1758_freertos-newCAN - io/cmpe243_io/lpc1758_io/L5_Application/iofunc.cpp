/*
 * io_func.cpp
 *
 *  Created on: 03-Oct-2014
 *      Author: Tej
 */
#include "iofunc.hpp"
#include "controller_io.hpp"

#include <stdio.h>
#include <string.h>
#include "io.hpp"
#include "utilities.h"
#include "uart2.hpp"
#include "lpc_pwm.hpp"
#include "eint.h"
#include "adc0.h"
#include <string.h>
#include"semphr.h"
//#include "tasks.hpp"
//#include "soft_timer.hpp"
#include "lpc_rit.h"

/*--------GLCD commands----------*/
//GLCD commands and replies
#define GLCD_ACK       0x06
#define GLCD_NAK       0x15

//COMMAND AND PARAMETERS TABLE
#define READ_OBJ        0x00
#define WRITE_OBJ       0x01
#define WRITE_STR       0x02
#define WRITE_STRU      0x03
#define WRITE_CONTRAST  0x04
#define REPORT_OBJ      0x05
#define REPORT_EVENT    0x07

//VISI-Genie object constants
#define GLCD_OBJ_DIPSW         0
#define GLCD_OBJ_KNOB          1
#define GLCD_OBJ_ROCKERSW      2
#define GLCD_OBJ_ROTARYSW      3
#define GLCD_OBJ_SLIDER        4
#define GLCD_OBJ_TRACKBAR      5
#define GLCD_OBJ_WINBUTTON     6
#define GLCD_OBJ_ANGULAR_METER 7
#define GLCD_OBJ_COOL_GAUGE    8
#define GLCD_OBJ_CUSTOM_DIGITS 9
#define GLCD_OBJ_FORM          10
#define GLCD_OBJ_GAUGE         11
#define GLCD_OBJ_IMAGE         12
#define GLCD_OBJ_KEYBOARD      13
#define GLCD_OBJ_LED           14
#define GLCD_OBJ_LED_DIGITS    15
#define GLCD_OBJ_METER         16
#define GLCD_OBJ_STRINGS       17
#define GLCD_OBJ_THERMOMETER   18
#define GLCD_OBJ_USER_LED      19
#define GLCD_OBJ_VIDEO         20
#define GLCD_OBJ_STATIC_TEXT   21
#define GLCD_OBJ_SOUND         22
#define GLCD_OBJ_TIMER         23
#define GLCD_OBJ_SPECTRUM      24
#define GLCD_OBJ_SCOPE         25
#define GLCD_OBJ_TANK          26
#define GLCD_OBJ_USERIMAGES    27
#define GLCD_OBJ_PINOUTPUT     28
#define GLCD_OBJ_PININPUT      29
#define GLCD_OBJ_4DBUTTON      30
#define GLCD_OBJ_ANIBUTTON     31
#define GLCD_OBJ_COLORPICKER   32
#define GLCD_OBJ_USERBUTTON    33
/*--------GLCD commands----------*/
#define MAX_SEN_VAL 12
#define DEAD_TIME_SEC 3
#define MAX_DEAD_TIME DEAD_TIME_SEC*2
#define CAN_MAX_SPEED 12500.00 //10kbps = 12500Bps

static const uint32_t power_on_uart2 = (1 << 24);
static const uint32_t enb_DLAB = (1 << 7);
//Uart2 &com = Uart2::getInstance();


PWM head(PWM::pwm1, 100);           //PWM for head lights
PWM tail(PWM::pwm2, 100);           //PWM for tail lights
uint8_t hl_mode;
bool hl_mode_change=false;
bool brake =false;
bool flicker=false;
uint8_t flick_cnt=0;
bool dest_reached=false;

uint16_t total_can=0;
uint16_t total_can_old=0;
uint8_t can_per_sec=0;
float can_util=0;
uint8_t time_cnt =0;
uint8_t sec=0;      //seconds

bool glcd_on = true;    //GLCD on/off variable
bool glcd_chg = false;  //GLCD control changed
bool touch = false;

bool resume_pause = false;  //Car resume/pause flag true=Car Running false=Car stopped
bool ctrl_changed = false;  //Car control changed flag

bool touch_hl=false;    //Touch screen headlight button flag
bool touch_mode=false;  //Touch screen mode button flag

drive_mode_data_t io_drive_mode;
speed_encoder_data_t io_speed;
//speed_dir_data_t io_speed_dir_data;
geo_heading_data_t io_geo_head;
geo_location_data_t io_geo_loc;
checkpoint_data_t io_checkpoint_data;
dist_sensor_data_t io_snr_data;
other_sensor_data_t io_other_snr_data;

/*powerup_syn_ack_data_t mot_ver;
powerup_syn_ack_data_t sen_ver;
powerup_syn_ack_data_t geo_ver;
powerup_syn_ack_data_t bt_ver;

powerup_ack_data_t mas_ver;*/

heartbeat_ack_data_t mot_can;
heartbeat_ack_data_t mas_can;
heartbeat_ack_data_t sen_can;
heartbeat_ack_data_t geo_can;
heartbeat_ack_data_t bt_can;

module_stat_time_t stat_time;
data_rcv_t data_rcv;

GLCD :: GLCD(uint8_t priority) : scheduler_task("GLCD", 4096, priority)     //2048
{
    GLCD_init();
    io_drive_mode.mode=MODE_MAP;
    data_rcv.drive_mode=true;
    data_rcv.versions=true;
    data_rcv.headlights=true;
    setRunDuration(100); //run after every 100ms
}

bool GLCD:: run(void *p)
{
    display();
    touch_screen();
    return true;
}

void GLCD:: tx_data(char data_tx)
{
    LPC_UART2->THR = data_tx; //write data to be sent in into THR reg
    while (!(LPC_UART2->LSR & (1 << 6))); //wait till TEMT bit in LSR reg is set i.e tx buf is empty
}

void GLCD:: GLCD_init(void)
{
    unsigned int clock;
    clock = sys_get_cpu_clock();
    //printf("Clock freq:%d", clock);
    LPC_SC->PCONP |= power_on_uart2;
        LPC_SC->PCLKSEL1 &= ~(3 << 16);
        LPC_SC->PCLKSEL1 |= (1 << 16);
        LPC_PINCON->PINSEL4 &= ~(0xF << 16);
        LPC_PINCON->PINSEL4 |= (0xA << 16);
        LPC_UART2->LCR = enb_DLAB;
        LPC_UART2->DLM = 1; //1;
        LPC_UART2->DLL = (clock) / (16 * 9600); //56;//(clock) /(16*9600);  // baud = clock / (16*(256*DLM+DLL) )
        //LPC_UART2->DLM = 0; //1;
        //LPC_UART2->DLL = (clock) / (16 * 115200); //56;//(clock) /(16*9600);  // baud = clock / (16*(256*DLM+DLL) )

        LPC_UART2->LCR = 3;
        LPC_UART2->FCR = ((1 << 0) | (1 << 6)); // enable fifo mode

        LPC_UART2->IER = 1; //enable RDA interrupt
        NVIC_EnableIRQ(UART2_IRQn); //NOTE: singleton class in uart2.cpp is commented.

        rit_enable(timer,500);  // Run RIT for 500ms
}

/*Function to send value for particular object to GLCD*/
void GLCD::GLCD_write(char cmd, char obj_id, char obj_index, char data_msb,
        char data_lsb)
{
    static char checksum;
    checksum = cmd ^ obj_id ^ obj_index ^ data_msb ^ data_lsb;
    tx_data(cmd);
    tx_data(obj_id);
    tx_data(obj_index);
    tx_data(data_msb);
    tx_data(data_lsb);
    tx_data(checksum);
}

/*Function to send string to GLCD*/
void GLCD::GLCD_write_str(char cmd, char str_index, char *str)
{
    static char checksum;
    uint8_t len;
    len = strlen(str);
    tx_data(cmd);
    tx_data(str_index);
        checksum = (cmd ^ str_index ^ len);
        tx_data(len);
        for (int i = 0; str[i] != '\0'; i++)
        {
            tx_data(str[i]); //Send every character from string
            checksum ^= str[i];
        }
        tx_data(checksum);
}

/*Function to send int to string object of GLCD*/
void GLCD::GLCD_write_int(char cmd, char str_index, int val)
{
    static char checksum;
    uint8_t len;
    char str[5];
    sprintf(str,"%d",val);
    len = strlen(str);
    tx_data(cmd);
    tx_data(str_index);
        checksum = (cmd ^ str_index ^ len);
        tx_data(len);
        for (int i = 0; str[i] != '\0'; i++)
        {
            tx_data(str[i]); //Send every character from string
            checksum ^= str[i];
        }
        tx_data(checksum);
}
/*Function to send float to string object of GLCD*/
void GLCD::GLCD_write_float(char cmd, char str_index, float val, uint8_t precision)
{
    static char checksum;
    uint8_t len;
    char str[12];
    sprintf(str,"%.*f",precision,val);
    len = strlen(str);
    tx_data(cmd);
    tx_data(str_index);
        checksum = (cmd ^ str_index ^ len);
        tx_data(len);
        for (int i = 0; str[i] != '\0'; i++)
        {
            tx_data(str[i]); //Send every character from string
            checksum ^= str[i];
        }
        tx_data(checksum);
}


/*Function to set contrast of GLCD*/
void GLCD::GLCD_contrast(char val)
{
    static char checksum;
    checksum = WRITE_CONTRAST ^ val;
    tx_data(WRITE_CONTRAST);
    tx_data(val);
    tx_data(checksum);
}

/*check switch press and display on GLCD*/
void GLCD::display(void)
{
    if(glcd_chg==true)
    {
        glcd_chg=false;
        if(glcd_on==true)
            GLCD_contrast(15);  //GLCD back light on
        else
            GLCD_contrast(0);  //GLCD back light off
    }

    //DATA FOR FORM 0 - HOME SCREEN
    if(data_rcv.speed==true)
    {
        data_rcv.speed=false;
        if(io_speed.speed>=10)  io_speed.speed=10; //Speedometer range is from 0 to 10
        if(io_speed.speed<=0)   io_speed.speed=0;
        GLCD_write(WRITE_OBJ,GLCD_OBJ_COOL_GAUGE,0,0, io_speed.speed);
        GLCD_write_int(WRITE_STR,20,io_speed.speed);
    }


    if(data_rcv.sensors==true)
    {
        data_rcv.sensors=false;
        if(io_snr_data.left>=MAX_SEN_VAL)
            io_snr_data.left=MAX_SEN_VAL;
        else
        {
            if(io_snr_data.left<=0)
                io_snr_data.left=0;
        }
        GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,1,0, MAX_SEN_VAL-io_snr_data.left);
        GLCD_write_int(WRITE_STR,16,io_snr_data.left);

        if(io_snr_data.middle>=MAX_SEN_VAL)
            io_snr_data.middle=MAX_SEN_VAL;
        else
        {
            if(io_snr_data.middle<=0)
                io_snr_data.middle=0;
        }
        GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,2,0, MAX_SEN_VAL-io_snr_data.middle);
        GLCD_write_int(WRITE_STR,15,io_snr_data.middle);

        if(io_snr_data.right>=MAX_SEN_VAL)
            io_snr_data.right=MAX_SEN_VAL;
        else
        {
            if(io_snr_data.right<=0)
                io_snr_data.right=0;
        }
        GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,3,0, MAX_SEN_VAL-io_snr_data.right);
        GLCD_write_int(WRITE_STR,14,io_snr_data.right);

        if(io_snr_data.back>=MAX_SEN_VAL)
            io_snr_data.back=MAX_SEN_VAL;
        else
        {
                if(io_snr_data.back<=0)
                    io_snr_data.back=0;
        }
        GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,4,0, MAX_SEN_VAL-io_snr_data.back);
        GLCD_write_int(WRITE_STR,17,io_snr_data.back);
    }

    if(data_rcv.other_sensors==true)
    {
        data_rcv.other_sensors=false;
        if(io_other_snr_data.light>=100)
            io_other_snr_data.light=100;
        else
        {
            if(io_other_snr_data.light<=0)
                io_other_snr_data.light=0;
        }
        GLCD_write_int(WRITE_STR,23,io_other_snr_data.light);

        if(io_other_snr_data.battery>=100)
            io_other_snr_data.battery=100;
        else
        {
            if(io_other_snr_data.battery<=0)
                io_other_snr_data.battery=0;
        }
        GLCD_write_int(WRITE_STR,24,io_other_snr_data.battery);
        GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,0,0, io_other_snr_data.battery);
    }


    //DATA FOR FORM 1 - MODULE STATUS

    if(data_rcv.status==true)
    {
        data_rcv.status=false;
        if(stat_time.motor>=DEAD_TIME_SEC)
        {
            stat_time.motor=MAX_DEAD_TIME;
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,0,0,0);  //RED = MODULE DEAD
        }
        else
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,0,0,1);  //GREEN = MODULE OK

        if(stat_time.master>=DEAD_TIME_SEC)
        {
            stat_time.master=MAX_DEAD_TIME;
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,1,0,0);
        }
        else
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,1,0,1);

        if(stat_time.sensor>=DEAD_TIME_SEC)
        {
            stat_time.sensor=MAX_DEAD_TIME;
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,2,0,0);
        }
        else
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,2,0,1);

        if(stat_time.geo>=DEAD_TIME_SEC)
        {
            stat_time.geo=MAX_DEAD_TIME;
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,3,0,0);
        }
        else
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,3,0,1);

        if(stat_time.bt>=DEAD_TIME_SEC)
        {
            stat_time.bt=MAX_DEAD_TIME;
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,4,0,0);
        }
        else
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,4,0,1);

        if(stat_time.io>=DEAD_TIME_SEC)
        {
            stat_time.io=MAX_DEAD_TIME;
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,5,0,0);
        }
        else
            GLCD_write(WRITE_OBJ,GLCD_OBJ_USER_LED,5,0,1);


        GLCD_write_int(WRITE_STR, 2, mot_can.tx_count);     //Display all module's CAN TX and RX count
        GLCD_write_int(WRITE_STR, 8, mot_can.rx_count);
        GLCD_write_int(WRITE_STR, 3, mas_can.tx_count);
        GLCD_write_int(WRITE_STR, 9, mas_can.rx_count);
        GLCD_write_int(WRITE_STR, 4, sen_can.tx_count);
        GLCD_write_int(WRITE_STR, 10, sen_can.rx_count);
        GLCD_write_int(WRITE_STR, 5, geo_can.tx_count);
        GLCD_write_int(WRITE_STR, 11, geo_can.rx_count);
        GLCD_write_int(WRITE_STR, 6, bt_can.tx_count);
        GLCD_write_int(WRITE_STR, 12, bt_can.rx_count);
        GLCD_write_int(WRITE_STR, 7, controller.get_tx_count());
        GLCD_write_int(WRITE_STR, 13, controller.get_rx_count());

        GLCD_write_float(WRITE_STR, 38,can_util,2);
        //GLCD_write_int(WRITE_STR, 38, can_per_sec);
    }

    if(data_rcv.versions==true)
    {
        data_rcv.versions=false;
        /*GLCD_write_int(WRITE_STR, 29, mot_ver.version);
        GLCD_write_int(WRITE_STR, 30, mas_ver.version);
        GLCD_write_int(WRITE_STR, 31, sen_ver.version);
        GLCD_write_int(WRITE_STR, 32, geo_ver.version);
        GLCD_write_int(WRITE_STR, 33, bt_ver.version);
        GLCD_write_int(WRITE_STR, 34, CONTROLLER_VERSION);*/
        GLCD_write_str(WRITE_STR, 29, "2.1");
        GLCD_write_str(WRITE_STR, 30, "2.1");
        GLCD_write_str(WRITE_STR, 31, "2.1");
        GLCD_write_str(WRITE_STR, 32, "2.1");
        GLCD_write_str(WRITE_STR, 33, "2.1");
        GLCD_write_str(WRITE_STR, 34, "2.1");
    }


    if(ctrl_changed==true)
    {
        ctrl_changed=false;
        if(resume_pause==true)
            GLCD_write_str(WRITE_STR, 22, "RESUME");
        else
            GLCD_write_str(WRITE_STR, 22, "PAUSE ");
    }

    if(data_rcv.drive_mode==true)
    {
        data_rcv.drive_mode=false;
        switch(io_drive_mode.mode)
        {
            case MODE_FREE_RUN:
                GLCD_write_str(WRITE_STR,37,"FREE RUN");
                GLCD_write_str(WRITE_STR,35,"FREE RUN");    //FOR FORM 4 SETTIGNS SCREEN
                break;
            case MODE_HOME:
                GLCD_write_str(WRITE_STR,37,"HOME    ");
                GLCD_write_str(WRITE_STR,35,"HOME    ");    //FOR FORM 4 SETTIGNS SCREEN
                break;
            case MODE_MAP:
                GLCD_write_str(WRITE_STR,37,"MAP    ");
                GLCD_write_str(WRITE_STR,35,"MAP    ");     //FOR FORM 4 SETTIGNS SCREEN
                break;
            default:
                GLCD_write_str(WRITE_STR,37,"MAP    ");
                GLCD_write_str(WRITE_STR,35,"MAP    ");     //FOR FORM 4 SETTIGNS SCREEN
        }
    }
    if(data_rcv.headlights==true)
    {
        data_rcv.headlights=false;
        switch(hl_mode)
        {
            case AUTO:
                GLCD_write_str(WRITE_STR,21,"AUTO");
                GLCD_write_str(WRITE_STR,36,"AUTO");     //FOR FORM 4 SETTIGNS SCREEN
                break;
            case ON:
                GLCD_write_str(WRITE_STR,21,"ON  ");
                GLCD_write_str(WRITE_STR,36,"ON  ");    //FOR FORM 4 SETTIGNS SCREEN
                break;
            case OFF:
                GLCD_write_str(WRITE_STR,21,"OFF ");
                GLCD_write_str(WRITE_STR,36,"OFF ");    //FOR FORM 4 SETTIGNS SCREEN
                break;
            default:
                GLCD_write_str(WRITE_STR,21,"OFF  ");
                GLCD_write_str(WRITE_STR,36,"OFF  ");    //FOR FORM 4 SETTIGNS SCREEN
                break;
        }
    }

    if(data_rcv.geo_head_data==true)
    {
        data_rcv.geo_head_data=false;
        if(io_geo_head.destination_reached==true)
            dest_reached=true;
        else
            dest_reached=false;
        GLCD_write_int(WRITE_STR,19,io_geo_head.current_angle);  //NOT DEPENDENT ON GPS FIX
        if(io_geo_head.is_valid==true)                           //DISPLAY PARAMATERS ONLY IF GPS IS FIXED
        {
            //GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,5,0,((io_checkpoint_data.total_distance-io_geo_loc.dist_to_final_destination)/io_checkpoint_data.total_distance)*100);
            //GLCD_write_int(WRITE_STR,1,io_geo_loc.dist_to_final_destination);
            //GLCD_write_int(WRITE_STR,27,io_geo_loc.longitude);
            //GLCD_write_int(WRITE_STR,28,io_geo_loc.latitude);
            GLCD_write_int(WRITE_STR,18,io_geo_head.desired_angle);
        }
        else
        {
            //GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,5,0,0);
            //GLCD_write_str(WRITE_STR,1,"GPS NF");
            //GLCD_write_str(WRITE_STR,27,"GPS NF");
            //GLCD_write_str(WRITE_STR,28,"GPS NF");
            GLCD_write_str(WRITE_STR,18,"GPS NF");
        }

    }
    if(data_rcv.geo_loc_data==true)
    {
        data_rcv.geo_loc_data=false;
        if(io_geo_loc.is_valid==true)
        {
            GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,5,0,((io_checkpoint_data.total_distance-io_geo_loc.dist_to_final_destination)/io_checkpoint_data.total_distance)*100);
            GLCD_write_float(WRITE_STR,1,io_geo_loc.dist_to_final_destination,3);
            GLCD_write_float(WRITE_STR,27,io_geo_loc.longitude,6);
            GLCD_write_float(WRITE_STR,28,io_geo_loc.latitude,6);
        }
        else
        {
            GLCD_write(WRITE_OBJ,GLCD_OBJ_GAUGE,5,0,0);
            GLCD_write_str(WRITE_STR,1,"GPS NF");
            GLCD_write_str(WRITE_STR,27,"GPS NF");
            GLCD_write_str(WRITE_STR,28,"GPS NF");
        }
    }

    GLCD_write_int(WRITE_STR,26,(int)AS.getX());    //Accelerometer output
    GLCD_write_int(WRITE_STR,25,(int)AS.getY());
}

void GLCD:: touch_screen(void)
{
    if(touch_hl==true)
    {
        touch_hl=false;
        switch(hl_mode)
        {
            case AUTO:
                hl_mode=OFF;
                break;
            case ON:
                hl_mode=AUTO;
                break;
            case OFF:
                hl_mode=ON;
                break;
            default:
                hl_mode=OFF;
                break;
        }
        data_rcv.headlights=true;
        hl_mode_change=true;
    }
    if(touch_mode==true)
    {
        touch_mode=false;
        if(io_drive_mode.mode==MODE_FREE_RUN)
            io_drive_mode.mode = MODE_HOME;
        else
            io_drive_mode.mode = MODE_FREE_RUN;
        controller.can_send(CONTROLLER_ALL, MSG_DRIVE_MODE,(uint8_t *) &io_drive_mode, sizeof(io_drive_mode));
        data_rcv.drive_mode=true;
    }
}

void timer()
{
    if (time_cnt++>=2)
    {
        time_cnt=0;
        sec++;
        if(sec>=1)
        {
            stat_time.motor++;
            stat_time.master++;
            stat_time.sensor++;
            stat_time.geo++;
            stat_time.bt++;
            stat_time.io++;

            total_can = mot_can.tx_bytes+mas_can.tx_bytes+sen_can.tx_bytes+geo_can.tx_bytes+
                        bt_can.tx_bytes+controller.get_tx_bytes();
            //GLCD_write_int(WRITE_STR, 38, (total_can-total_can_old));
            can_per_sec=total_can-total_can_old;
            can_util=((float)can_per_sec/CAN_MAX_SPEED)*100.00;
            //printf("\nCAN messages/sec: %d",can_per_sec);
            //printf("\nCAN Utilization: %f",can_util);
            total_can_old=total_can;
        }
        if(sec>=DEAD_TIME_SEC)
        {
            sec=0;
            data_rcv.status=true;
        }
    }

}


/*************************** LIGHT CLASS FUNCTIONS ****************************/
LIGHT :: LIGHT(uint8_t priority) :scheduler_task("LIGHT", 2048, priority)
{
    light_init();
    setRunDuration(50); //run after every 50mS
}
bool LIGHT :: run(void *p)
{
    chk_light();
    return true;
}
void LIGHT::light_init()
{
    //pwm initialized outside
    //PWM head(PWM::pwm1, 0);           //PWM for head lights
    //PWM tail(PWM::pwm2, 0);           //PWM for tail lights
    hl_mode=OFF;
    data_rcv.headlights=true;
    head.set(0);
    tail.set(0);
}
void LIGHT::chk_light(void)
{
    if(hl_mode_change==true)
    {
        hl_mode_change=false;
        switch(hl_mode)
        {
            case AUTO:
                head.set(100-io_other_snr_data.light);    //Automatic head lights
                tail.set(10);
                break;
            case ON:
                head.set(100);  //Head light On
                tail.set(10);   //Tail light On - DIM
                break;
            case OFF:
                head.set(0);    //Head light Off
                tail.set(0);    //Tail light Off
                break;
            default:
                head.set(0);    //Head light Off
                tail.set(0);    //Tail light Off
                break;
        }
    }
    if(hl_mode==AUTO)
    {
        head.set(100-io_other_snr_data.light);
    }

    if(data_rcv.speed_dir_data==true)
    {
        data_rcv.speed_dir_data=false;
        if(io_speed.brake==SPEED_STOP || io_speed.brake==SPEED_EMERGENCY_STOP )
        {
            //printf("\nI am STOPPED !");
            tail.set(100);
            brake=true;
        }
        else
        {
            brake=false;
        }
        if(io_speed.dir==DIR_REV && brake==false)
        {
            if(flicker==true)   //Tail light blinking
            {
                tail.set(0);
                flicker=false;
            }
            else
            {
                tail.set(100);
                flicker=true;
            }
            //printf("\nI am taking REVERSE !");    //Debugging
        }
        if(io_speed.dir==DIR_FWD && brake==false)
        {
            hl_mode_change=true;                    //Go to default state
            //printf("\nI am going FORWARD !");     //Debugging
        }

    }
    if(data_rcv.chkpt_data==true)
    {
        flick_cnt++;        //Head light blink
        if(flick_cnt%2)
        head.set(100);
        else
        head.set(0);

        if(flick_cnt>=6)
        {
            flick_cnt=0;
            hl_mode_change=true;
            data_rcv.chkpt_data=false;
        }
    }
    if(dest_reached==true)
    {
        if(flicker==true)   //Head and Tail light blinking
        {
            head.set(0);
            tail.set(0);
            flicker=false;
        }
        else
        {
            head.set(100);
            tail.set(100);
            flicker=true;
        }
    }
}
/*************************** LIGHT CLASS FUNCTIONS ****************************/


/*************************** SWITCH CLASS FUNCTIONS ****************************/
SWITCH :: SWITCH(uint8_t priority) :scheduler_task("SWITCH", 1024, priority)
{
    sw_init();
    setRunDuration(25);
}
void SWITCH::sw_init(void)
{
    LPC_GPIO2->FIODIR |= (1<<4);
    LPC_GPIO2->FIODIR |= (1<<5);
    LPC_GPIO2->FIODIR |= (1<<6);
    LPC_GPIO2->FIODIR |= (1<<7);
    eint3_enable_port2(4,eint_rising_edge,car_con);
    eint3_enable_port2(5,eint_rising_edge,glcd_con);
    eint3_enable_port2(6,eint_rising_edge,home_dst);
    eint3_enable_port2(7,eint_rising_edge,reset_all);
}
bool SWITCH::run(void *p)
{
    return true;
}

void car_con(void)   // To start / stop the car manually - TESTING
{
    ctrl_changed=true;
    if(resume_pause==true)
    {
        resume_pause=false;
        controller.can_send(CONTROLLER_MASTER, MSG_CAR_PAUSE, NULL, NULL);//SEND CAN MESSAGE
    }
    else
    {
        resume_pause=true;
        controller.can_send(CONTROLLER_MASTER, MSG_CAR_RESUME, NULL, NULL);//SEND CAN MESSAGE
    }
}

/*void headlight(void) // To turn on/off headlights manually
{
    static int cnt=0;
    if(cnt==0)
    {
        cnt=1;
        lt.am=false;
        lt.on=true;
        disp.GLCD_write_str(WRITE_STR,21,"ON");
    }
    else if(cnt==1)
    {
        cnt=2;
        lt.am=false;
        lt.on=false;
        disp.GLCD_write_str(WRITE_STR,21,"OFF");
    }
    else if(cnt==2)
    {
        cnt=0;
        lt.am=true;
        lt.on=true;
        disp.GLCD_write_str(WRITE_STR,21,"AUTO");
    }

    //LE.on(3);
}*/

void glcd_con(void) // To turn on/off GLCD back light
{
    glcd_chg=true;
    if(glcd_on==true)
        glcd_on=false;          //turn GLCD back light off
    else
       glcd_on=true;            //turn GLCD back light on
}
void home_dst(void) // To set mode
{
    /*if(io_drive_mode.mode==MODE_FREE_RUN)
        io_drive_mode.mode = MODE_HOME;
    else
        io_drive_mode.mode = MODE_FREE_RUN;*/
    io_drive_mode.mode=MODE_HOME;
    controller.can_send(CONTROLLER_ALL, MSG_DRIVE_MODE,(uint8_t *) &io_drive_mode, sizeof(io_drive_mode));
    data_rcv.drive_mode=true;
}
void reset_all(void) // To reset all boards
{
    controller.can_send(CONTROLLER_MASTER, MSG_RESET, NULL, NULL);
    //LOG_FLUSH();
    //vTaskDelayMs(2000);
    //sys_reboot();
}
/*************************** SWITCH CLASS FUNCTIONS ****************************/

/*Interrupt handler function to handle data coming from GLCD touch screen*/
extern "C"
{
void UART2_IRQHandler()
{
    if (LPC_UART2->IIR & 4)
    {
    static char rec;
    rec = LPC_UART2->RBR;
    if(rec==REPORT_EVENT)
    {
        rec = LPC_UART2->RBR;
        if(rec==GLCD_OBJ_4DBUTTON)
        {
            rec = LPC_UART2->RBR;
            if(rec==0x00)   //MODE SELECT
                {
                    LE.on(1);
                    printf("\n1 Switch press");
                    touch_mode=true;
                }
            if(rec==0x01)   //Headlights
                {
                    LE.off(1);
                    printf("\n2 Switch press");
                    touch_hl=true;
                }
            rec = LPC_UART2->RBR;
            rec = LPC_UART2->RBR;
            rec = LPC_UART2->RBR;
        }

    }
}
}
}



