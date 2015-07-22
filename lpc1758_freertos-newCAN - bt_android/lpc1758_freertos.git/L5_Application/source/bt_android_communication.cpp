/*
 * bt_android_communication.cpp
 *
 *  Created on: Dec 11, 2014
 *      Author: MANish
 */

/* This file contains the implementation of the BluetoothTasks and also the new definition for the
 * IRQHandler. Currently all incoming messages from the app have been set to equal length hence there's
 * only a single condition for count > 4.
 * UART-2 registers have been set to interrupt enable and FIFO modes. Previously tried using the UART2
 * API's to send and receive data but there was a problem with the message Queue which got full and stopped
 * transmitting after that. The problem was solved using the THR register directly in a loop to send out
 * the buffer.
 */

#include <stdio.h>

#include "rtc_alarm.h"
#include "rtc.h"
#include "file_logger.h"
#include "io.hpp"
#include "c_tlm_var.h"
#include "uart2.hpp"
#include "semphr.h"
#include "portmacro.h"
#include "bt_android_communication.hpp"
#include "task.h"
#include "stdbool.h"
#include "utilities.h"
#include "string.h"
#include "switches.hpp"
#include "controller_bt_android.hpp"

extern can_controller controller;
extern bt_checkpoint_data_t checkpoint_list[];
extern uint16_t total_checkpoints;
extern uint8_t is_first_checkpoint;
extern uint8_t car_paused;
extern checkpoint_data_t periodic_checkpoint_data;

static char recBuffer[200];
static uint16_t count;

extern "C"
{

void UART2_IRQHandler()
{
    if(LPC_UART2->IIR & 4)  //4 = 0100  [010 : recv data available, 0 : atleast 1 interrupt pending]
    {
        recBuffer[count++] = LPC_UART2->RBR;
    }
}

}

BluetoothSendData::BluetoothSendData()
{
    //sendTimeout = 20;
    //-----for UART2--------------------------
    power_on_uart2 = (1<<24);
    enb_8bit       = (3<<0);
    enb_DLAB       = (1<<7);
    dis_DLAB       = ~(1<<7);
    fifo_enb       = (1<<0);
    fifo_dis       = ~(1<<0);

    unsigned int clock;
    clock = sys_get_cpu_clock();
    printf("CLOCK = %x \n", clock);

    LPC_SC->PCONP       |= power_on_uart2;
    LPC_SC->PCLKSEL1    &= ~(3<<16);
    LPC_SC->PCLKSEL1    |= (1<<16);
    LPC_PINCON->PINSEL4 &= ~(0xF<<16);
    LPC_PINCON->PINSEL4 |= (0xA<<16);
    LPC_UART2->LCR       = enb_DLAB;
    LPC_UART2->DLM       = 0; //1;
    LPC_UART2->DLL       = (clock) / (16 * 115200); //56;//(clock) /(16*9600);  // baud = clock / (16* (256*DLM+DLL) )
    LPC_UART2->LCR       = 3;
    LPC_UART2->FCR       = ((1<<0) | (1<<6));       // enable fifo mode

    LPC_UART2->IER       = 1;
    //LPC_UART2->IER       = 3;//enable RDA interrupt
    NVIC_EnableIRQ(UART2_IRQn);

}

bool BluetoothSendData :: bt_send(unsigned char *buf, size_t len)
{
    /* send data */
    if(!(LPC_UART2->LSR & (1 << 6))) {
        return true;
    }

    for(uint16_t i = 0; i < len; i++) {
        LPC_UART2->THR = buf[i];
        while(!(LPC_UART2->LSR & (1 << 6)));
    }

    return true;
}

BluetoothReceiveData::BluetoothReceiveData(uint8_t priority) :
                            scheduler_task("BR_BluetoothReceive", 2048, priority)
{
    setRunDuration(20); // in milliseconds
}

bool BluetoothReceiveData::run(void *p)
{
    if(count > 0)
    {
        if(recBuffer[0] == 'a')
        {
            printf("Start\n");
            car_paused = 0;
            controller.can_send(CONTROLLER_ALL, MSG_CAR_RESUME, NULL, 0);
        }
        else if(recBuffer[0] == 'b')
        {
            printf("Stop\n");
            car_paused = 1;
            controller.can_send(CONTROLLER_ALL, MSG_CAR_PAUSE, NULL, 0);
        }
        else if(recBuffer[0] == 'g')
        {
            //char *checkpoints[20] = {NULL};
            char checkpoints[20][20] = {0};
            char *token;
            int m=0;
            vTaskDelay(100);
            token = strtok(recBuffer, ",");

            while(token != NULL)
            {
                if(m==0){
                    strcpy(checkpoints[m], token+1);
                    m++;
                }
                else{
                    strcpy(checkpoints[m++], token);
                }
                token = strtok(NULL, ",");
                if(token[0] == '$') {
                    break;
                }
            }


            int n;
            for(n=0; n<m; n++){
                //printf("%s\n", checkpoints[n]);
                if((n % 2) == 0) {
                    //lat save at n/2
                    sscanf(checkpoints[n], "%f", &checkpoint_list[n/2].latitude);
                    printf("[%d]lat: %.6f\n", n/2, checkpoint_list[n/2].latitude);
                }
                else {
                    //long save at n/2
                    sscanf(checkpoints[n], "%f", &checkpoint_list[n/2].longitude);
                    printf("[%d]long : %.6f\n\n", n/2, checkpoint_list[n/2].longitude);
                }
            }

            total_checkpoints = m/2;
            is_first_checkpoint = 1;

//            printf("total_checkpoints %d \n", total_checkpoints);
//            printf("total dist %f\n", checkpoint_list[total_checkpoints].latitude);
            periodic_checkpoint_data.total_distance = checkpoint_list[total_checkpoints].latitude;
        }

        memset(recBuffer, 0, sizeof(recBuffer));
        count = 0;

    }
    return true;
}
