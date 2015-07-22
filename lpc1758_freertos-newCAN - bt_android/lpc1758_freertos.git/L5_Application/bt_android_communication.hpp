/*
 * bt_android_communication.hpp
 *
 *  Created on: Dec 11, 2014
 *      Author: MANish
 */

#ifndef BT_ANDROID_COMMUNICATION_HPP_
#define BT_ANDROID_COMMUNICATION_HPP_

/* This header contains the BluetoothSend and BluetoothReceive class declarations.
 * The enum msgName_t is used to identify the incoming message from the android app.
 * For now, the expected messages are start and stop to control the on/off remotely.
 */


#include "scheduler_task.hpp"
#include "soft_timer.hpp"
#include "command_handler.hpp"
#include "wireless.h"
#include "char_dev.hpp"

#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include "uart2.hpp"
#include "io.hpp"

typedef enum{
            stopMsg,
            startMsg,
            testMsg,
            defaultMsg
}msgName_t;

typedef struct {
        float latitude;
        float longitude;
} bt_checkpoint_data_t;

class BluetoothReceiveData : public scheduler_task
{

    public:
        BluetoothReceiveData(uint8_t priority);
        bool run(void *p);

    private:
        unsigned int sendTimeout;
        const char *startMessage, *stopMessage, *testMessage;
        char *receivedData;
        msgName_t msg;
};

class BluetoothSendData
{

    public:
        BluetoothSendData();
        bool bt_send(uint8_t *buf, size_t len);

    private:
        //char * sendBuffer;
        //unsigned int sendTimeout;
//-----for UART2--------------------------
        uint32_t power_on_uart2;
        uint32_t enb_8bit;
        uint32_t enb_DLAB;
        uint32_t dis_DLAB;
        uint32_t fifo_enb;
        uint32_t fifo_dis;
};




#endif /* BT_ANDROID_COMMUNICATION_HPP_ */
