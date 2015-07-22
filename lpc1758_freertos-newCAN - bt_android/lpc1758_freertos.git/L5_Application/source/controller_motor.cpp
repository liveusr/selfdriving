/*
 * controller_motor.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"

#include "controller_motor.hpp"

#ifdef ENABLE_MOTOR // enabled from can_common.hpp

static can_msg_id_t filter_list[] = {
        {CONTROLLER_MASTER,         CONTROLLER_ALL,         MSG_RESET},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,         MSG_POWERUP_SYN},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,         MSG_POWERUP_ACK},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,         MSG_HEARTBEAT},

        {CONTROLLER_MASTER,         CONTROLLER_MOTOR,       MSG_SPEED_DIR_COMMAND},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_ALL,         MSG_DRIVE_MODE},
        {CONTROLLER_IO,             CONTROLLER_ALL,         MSG_DRIVE_MODE},

        {CONTROLLER_MOTOR,          CONTROLLER_MOTOR,       MSG_DUMMY},
};

can_controller controller(CONTROLLER_MOTOR, filter_list, sizeof(filter_list) / sizeof(can_msg_id_t));
static QueueHandle_t msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(msg_t));

static uint8_t heartbeat_count;

speed_dir_data_t speed_dir_cmd;
drive_mode_data_t drive_mode;

/* TODO: populate this variable to send periodic data */
speed_encoder_data_t periodic_encoder_data;

class periodicEncoderTask : public scheduler_task
{
    public:
        periodicEncoderTask(uint8_t priority) : scheduler_task("periodicEncoderTask", 1024, priority)
        {
            setRunDuration(131); // in milliseconds
        }

        bool run(void *p)
        {
            controller.can_send(CONTROLLER_ALL, MSG_SPEED_ENCODER_DATA, (uint8_t *) &periodic_encoder_data, sizeof(periodic_encoder_data));

            return true;
        }
};

class debugTask : public scheduler_task
{
    private:
        static const uint8_t MAX_DEBUG_DATA = 6;
        uint8_t curr_index;

    public:
        struct {
                uint8_t *left_val;
                uint8_t *right_val;
                uint8_t *int_val;
                uint8_t *hex_val;
        } debug_data[MAX_DEBUG_DATA];

        debugTask(uint8_t priority) : scheduler_task("debugTask", 1024, priority)
        {
            int8_t i = 0;
            for(i = 0; i < MAX_DEBUG_DATA; i++) {
                debug_data[i].left_val = NULL;
                debug_data[i].right_val = NULL;
                debug_data[i].int_val = NULL;
                debug_data[i].hex_val = NULL;
            }

            curr_index = 0;

            setRunDuration(100); // in milliseconds
        }

        bool run(void *p)
        {
            if(curr_index == MAX_DEBUG_DATA) {
                LD.setLeftDigit('-');
                LD.setRightDigit('-');
            }
            else {
                if(debug_data[curr_index].hex_val) {
                    uint8_t c;
                    c = (*debug_data[curr_index].hex_val >> 4) & 0x0F;
                    LD.setLeftDigit((c < 10)? c: (c + 20));
                    c = *debug_data[curr_index].hex_val & 0x0F;
                    LD.setRightDigit((c < 10)? c: (c + 20));
                }
                else if(debug_data[curr_index].int_val) {
                    LD.setNumber(*debug_data[curr_index].int_val);
                }
                else if(debug_data[curr_index].left_val && debug_data[curr_index].right_val) {
                    LD.setLeftDigit(*debug_data[curr_index].left_val);
                    LD.setRightDigit(*debug_data[curr_index].right_val);
                }
                else {
                    curr_index = ((curr_index + 1) % (MAX_DEBUG_DATA + 1));
                }
            }

            switch(SW.getSwitchValues() & 0x0F) {
                case 0x01:
                    /* switch 1 pressed */
                    curr_index = ((curr_index + 1) % (MAX_DEBUG_DATA + 1));
                    vTaskDelay(500);
                    break;

                case 0x02:
                    /* switch 2 pressed */
                    periodic_encoder_data.speed = (periodic_encoder_data.speed + 1) % 20;
                    vTaskDelay(500);
                    break;

                case 0x04:
                    /* switch 3 pressed */
                    break;

                case 0x08:
                    /* switch 4 pressed */
                    break;
            }

            return true;
        }

        void setHex(void *val, uint8_t index)
        {
            debug_data[index].hex_val = (uint8_t *) val;
        }

        void setInt(void *val, uint8_t index)
        {
            debug_data[index].int_val = (uint8_t *) val;
        }

        void setLeftRight(void *valLeft, void *valRight, uint8_t index)
        {
            debug_data[index].left_val = (uint8_t *) valLeft;
            debug_data[index].right_val = (uint8_t *) valRight;
        }
};

void controllerInit(void)
{
    /* clear error flag */
    SET_ERROR(ERROR_NO_ERROR);

    controller.processBootSequence(CONTROLLER_VERSION);

    /* TODO: start any other tasks of module here, like periodic task */
    scheduler_add_task(new periodicEncoderTask(PRIORITY_HIGH));

    /* setup debug variables */
    scheduler_task *dt = new debugTask(PRIORITY_MEDIUM);
    scheduler_add_task(dt);
    ((debugTask *)dt)->setInt(&periodic_encoder_data.speed, 0);
    ((debugTask *)dt)->setLeftRight(&speed_dir_cmd.turn, &speed_dir_cmd.speed, 1);
    ((debugTask *)dt)->setInt(&drive_mode.mode, 2);
    ((debugTask *)dt)->setInt(&heartbeat_count, 4);
    ((debugTask *)dt)->setHex(&error_flag, 5);
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

    if(msg.msg_num == MSG_RESET) {
        controller.processReset();
    }

    if(xQueueSend(msg_queue, &msg, 0) == errQUEUE_FULL) {
        SET_ERROR(ERROR_MSG_QUEUE_FULL);
        LOG_ERROR("msg_queue full\n");
        printf("msg_queue full\n");
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

    if((error_flag & ERROR_MSG_QUEUE_FULL) == ERROR_MSG_QUEUE_FULL) {
        RESET_ERROR(ERROR_MSG_QUEUE_FULL);
    }

    switch(msg.msg_num) {
        case MSG_HEARTBEAT:
            handle_heartbeat(msg);
            break;

        case MSG_SPEED_DIR_COMMAND:
            handle_speed_dir_command(msg);
            break;

        case MSG_DRIVE_MODE:
            handle_drive_mode(msg);
            break;

        default:
            /* do nothing */
            break;
    }

    return true;
}

bool canRxProcessTask :: handle_heartbeat(msg_t msg)
{
    heartbeat_data_t data;

    data.rx_count = controller.get_rx_count();
    data.rx_bytes = controller.get_rx_bytes();
    data.tx_count = controller.get_tx_count();
    data.tx_bytes = controller.get_tx_bytes();

    vTaskDelay(CONTROLLER_MOTOR * 10);
    controller.can_send(CONTROLLER_MASTER, MSG_HEARTBEAT_ACK, (uint8_t *) &data, sizeof(data));
    heartbeat_count = (heartbeat_count + 1) % 100;

    return true;
}

bool canRxProcessTask :: handle_speed_dir_command(msg_t msg)
{
    speed_dir_cmd = msg.speed_dir_data;

    /* TODO: process received left, right, fwd, rev, etc. commands stored in data */
    //printf("S %d T %d D %d\n", data.speed, data.turn, data.direction);

    return true;
}

bool canRxProcessTask :: handle_drive_mode(msg_t msg)
{
    drive_mode = msg.drive_mode_data;

    /* TODO: process only if received drive mode is MODE_SLOW/MODE_NORMAL/MODE_TURBO to change base drive speed */
    //printf("M %d\n", data.mode);

    return true;
}

#endif /* ENABLE_MOTOR */
