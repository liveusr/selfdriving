/*
 * controller_dummy.cpp
 *
 *  Created on: Nov 6, 2014
 *      Author: MANish
 */
#include "file_logger.h"
#include "io.hpp"
#include "stdio.h"
#include "lpc_sys.h"
#include "string.h"

#include "controller_dummy.hpp"

#ifdef ENABLE_DUMMY

#define DUMMY_MASTER 1

#if DUMMY_MASTER

static can_msg_id_t filter_list[] = {
        {CONTROLLER_DUMMY,      CONTROLLER_MASTER,   MSG_POWERUP_SYN_ACK},
        {CONTROLLER_DUMMY,      CONTROLLER_MASTER,   MSG_DUMMY},
};

can_controller controller(CONTROLLER_MASTER, filter_list, sizeof(filter_list) / sizeof(can_msg_id_t));
static QueueHandle_t msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(msg_t));
static uint8_t powerup_flag;

dummy_data_t periodic_data;

class periodicTask : public scheduler_task
{
    public:
        periodicTask(uint8_t priority) : scheduler_task("periodicTask", 1024, priority)
        {
            periodic_data.c = 0; // ON/OFF
            periodic_data.i = 1; // LED1

            setRunDuration(500); // in milliseconds
        }

        bool run(void *p)
        {
            periodic_data.c = !periodic_data.c;
            controller.can_send(CONTROLLER_MASTER, MSG_DUMMY, (uint8_t *) &periodic_data, sizeof(periodic_data));

            return true;
        }
};

void controllerInit(void)
{
    // start any other tasks of module here, like periodic task
    //scheduler_add_task(new periodicTask(PRIORITY_MEDIUM));
}

canRxBufferTask :: canRxBufferTask(uint8_t priority) : scheduler_task("canRxBufferTask", 1024, priority)
{
    /* Nothing to init */
}

bool canRxBufferTask :: run(void *p)
{
    msg_t msg;

    msg.len = sizeof(msg.data);
    controller.can_recv(&msg.src, &msg.msg_num, msg.data, &msg.len);

    if(msg.msg_num == MSG_POWERUP_SYN_ACK) {
        powerup_ack_data_t data;

        powerup_flag = 1;
        controller.can_send(CONTROLLER_ALL, MSG_POWERUP_ACK, (uint8_t *) &data, sizeof(data));

        return true;
    }
    printf("rcvd %d bytes\n", msg.len);
    if(xQueueSend(msg_queue, &msg, 0) == errQUEUE_FULL) {
        LE.on(1);
        LOG_ERROR("msg_queue full.\n");
    }

    return true;
}

canRxProcessTask :: canRxProcessTask(uint8_t priority) : scheduler_task("canRxProcessTask", 1024, priority)
{
    /* Nothing to init */
}

bool canRxProcessTask :: run(void *p)
{
    msg_t msg;

    while(!powerup_flag) {
        controller.can_send(CONTROLLER_ALL, MSG_POWERUP_SYN, NULL, 0);
        vTaskDelayMs(1000);
    }

    if(xQueueReceive(msg_queue, &msg, portMAX_DELAY) == pdFALSE) {
        return true;
    }

    switch(msg.msg_num) {
        case MSG_RESET:
            handle_reset(msg);
            break;

        case MSG_DUMMY:
            handle_dummy(msg);
            break;

        default:
            /* do nothing */
            break;
    }

    return true;
}

bool canRxProcessTask :: handle_reset(msg_t msg)
{
    printf("Rebooting System\n");
    LOG_FLUSH();
    vTaskDelayMs(2000);
    sys_reboot();

    return true;
}

bool canRxProcessTask :: handle_dummy(msg_t msg)
{
    dummy_data_t data;

    memcpy(&data, msg.data, sizeof(dummy_data_t));

    printf("%d %d [%d]\n", data.i, data.c, msg.len);
    LE.set(data.i, data.c);

    return true;
}

#else

static can_msg_id_t filter_list[] = {
        {CONTROLLER_IO,         CONTROLLER_ALL,     MSG_RESET},
        {CONTROLLER_MASTER,     CONTROLLER_ALL,     MSG_POWERUP_SYN},
        {CONTROLLER_MASTER,     CONTROLLER_ALL,     MSG_POWERUP_ACK},
        {CONTROLLER_DUMMY,      CONTROLLER_DUMMY,   MSG_DUMMY},
};

can_controller controller(CONTROLLER_DUMMY, filter_list, sizeof(filter_list) / sizeof(can_msg_id_t));
static QueueHandle_t msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(msg_t));

dummy_data_t periodic_data;

class periodicTask : public scheduler_task
{
    private:
        uint16_t i;

    public:
        periodicTask(uint8_t priority) : scheduler_task("periodicTask", 1024, priority)
        {
            periodic_data.c = 0; // ON/OFF
            periodic_data.i = 1; // LED1
            i = 0;

            setRunDuration(20); // in milliseconds
        }

        bool run(void *p)
        {
            periodic_data.c = ((++i % 50) > 45);
            controller.can_send(CONTROLLER_MASTER, MSG_DUMMY, (uint8_t *) &periodic_data, sizeof(periodic_data));
            printf("%d %d [%d]\n", periodic_data.i, periodic_data.c, sizeof(periodic_data));

            return true;
        }
};

void controllerInit(void)
{
    controller_id_t src;
    msg_id_t        msg_num;
    uint8_t         data[MAX_DATA_LEN];
    uint16_t        len;

    LE.on(4); // LED4 will be ON until can communication is setup
    while(1) {
        len = sizeof(data);
        controller.can_recv(&src, &msg_num, data, &len);
        if(msg_num == MSG_POWERUP_SYN) {
            data[0] = CONTROLLER_VERSION;
            controller.can_send(src, MSG_POWERUP_SYN_ACK, data, 1);
        }
        else if(msg_num == MSG_POWERUP_ACK) {
            /* set date time received in "data" */
            LE.off(4); // turn OFF LED4 after can communication is setup
            break;
        }
        else {
            /* this should not get executed */
            LE.on(1);
            LOG_ERROR("Unknown msg %s from %s received.\n", message_id_str[msg_num], controller_id_str[src]);
        }
    }

    // start any other tasks of module here, like periodic task
    scheduler_add_task(new periodicTask(PRIORITY_MEDIUM));
}

canRxBufferTask :: canRxBufferTask(uint8_t priority) : scheduler_task("canRxBufferTask", 1024, priority)
{
    /* Nothing to init */
}

bool canRxBufferTask :: run(void *p)
{
    msg_t msg;

    msg.len = sizeof(msg.data);
    controller.can_recv(&msg.src, &msg.msg_num, msg.data, &msg.len);
    if(xQueueSend(msg_queue, &msg, 0) == errQUEUE_FULL) {
        LE.on(1);
        LOG_ERROR("msg_queue full.\n");
    }

    return true;
}

canRxProcessTask :: canRxProcessTask(uint8_t priority) : scheduler_task("canRxProcessTask", 1024, priority)
{
    /* Nothing to init */
}

bool canRxProcessTask :: run(void *p)
{
    msg_t msg;

    if(xQueueReceive(msg_queue, &msg, portMAX_DELAY) == pdFALSE) {
        return true;
    }

    switch(msg.msg_num) {
        case MSG_RESET:
            handle_reset(msg);
            break;

        case MSG_DUMMY:
            handle_dummy(msg);
            break;

        default:
            /* do nothing */
            break;
    }

    return true;
}

bool canRxProcessTask :: handle_reset(msg_t msg)
{
    printf("Rebooting System\n");
    LOG_FLUSH();
    vTaskDelayMs(2000);
    sys_reboot();

    return true;
}

bool canRxProcessTask :: handle_dummy(msg_t msg)
{
    dummy_data_t data;

    memcpy(&data, msg.data, sizeof(dummy_data_t));

    LE.set(data.i, data.c);

    return true;
}

#endif
#endif
