/*
 * controller_bt_android.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"

#include "controller_bt_android.hpp"

#ifdef ENABLE_BT_ANDROID // enabled from can_common.hpp

static can_msg_id_t filter_list[] = {
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_RESET},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_POWERUP_SYN},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_POWERUP_ACK},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_HEARTBEAT},

        {CONTROLLER_SENSOR,         CONTROLLER_ALL,             MSG_DIST_SENSOR_DATA},
        {CONTROLLER_SENSOR,         CONTROLLER_ALL,             MSG_OTHER_SENSOR_DATA},

        {CONTROLLER_GEO,            CONTROLLER_ALL,             MSG_GEO_HEADING_DATA},
        {CONTROLLER_GEO,            CONTROLLER_ALL,             MSG_GEO_LOCATION_DATA},
        {CONTROLLER_GEO,            CONTROLLER_BT_ANDROID,      MSG_CHECKPOINT_REQUEST},

        {CONTROLLER_MOTOR,          CONTROLLER_ALL,             MSG_SPEED_ENCODER_DATA},

        {CONTROLLER_IO,             CONTROLLER_ALL,             MSG_CAR_RESUME},
        {CONTROLLER_IO,             CONTROLLER_ALL,             MSG_CAR_PAUSE},
        {CONTROLLER_IO,             CONTROLLER_ALL,             MSG_DRIVE_MODE},

        {CONTROLLER_MOTOR,          CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},
        {CONTROLLER_SENSOR,         CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},
        {CONTROLLER_GEO,            CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},
        {CONTROLLER_IO,             CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},

        {CONTROLLER_BT_ANDROID,     CONTROLLER_BT_ANDROID,      MSG_DUMMY},
};

can_controller controller(CONTROLLER_BT_ANDROID, filter_list, sizeof(filter_list) / sizeof(can_msg_id_t));
static QueueHandle_t msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(msg_t));

static uint8_t heartbeat_count;

dist_sensor_data_t dist_sensor;
other_sensor_data_t other_sensor;
geo_heading_data_t geo_heading;
geo_location_data_t geo_location;
checkpoint_request_data_t checkpoint_request;
speed_encoder_data_t speed_encoder;
drive_mode_data_t drive_mode;

/* TODO: populate this variable to send periodic data */
checkpoint_data_t periodic_checkpoint_data;
uint8_t is_first_checkpoint;

class periodicCheckpointTask : public scheduler_task
{
    public:
        periodicCheckpointTask(uint8_t priority) : scheduler_task("periodicCheckpointTask", 1024, priority)
        {
            setRunDuration(811); // in milliseconds
        }

        bool run(void *p)
        {
            if(!is_first_checkpoint) {
                /* NOTE: this task runs only to send 1st checkpoint */
                //scheduler_task::getTaskPtrByName("periodicCheckpointTask")->suspend();
                return true;
            }

            controller.can_send(CONTROLLER_GEO, MSG_CHECKPOINT_DATA, (uint8_t *) &periodic_checkpoint_data, sizeof(periodic_checkpoint_data));

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
                    drive_mode.mode = 1 + (drive_mode.mode % 6);
                    controller.can_send(CONTROLLER_ALL, MSG_DRIVE_MODE, (uint8_t *) &drive_mode, sizeof(drive_mode));
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
    scheduler_add_task(new periodicCheckpointTask(PRIORITY_HIGH));

    /* setup debug variables */
    scheduler_task *dt = new debugTask(PRIORITY_MEDIUM);
    scheduler_add_task(dt);
    ((debugTask *)dt)->setInt(&geo_location.dist_to_next_checkpoint, 0);
    ((debugTask *)dt)->setInt(&geo_location.dist_to_next_checkpoint, 1);
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

        case MSG_HEARTBEAT_ACK:
            handle_heartbeat_ack(msg);
            break;

        case MSG_DIST_SENSOR_DATA:
            handle_dist_sensor_data(msg);
            break;

        case MSG_OTHER_SENSOR_DATA:
            handle_other_sensor_data(msg);
            break;

        case MSG_GEO_HEADING_DATA:
            handle_geo_heading_data(msg);
            break;

        case MSG_GEO_LOCATION_DATA:
            handle_geo_location_data(msg);
            break;

        case MSG_CHECKPOINT_REQUEST:
            handle_checkpoint_request(msg);
            break;

        case MSG_SPEED_ENCODER_DATA:
            handle_speed_encoder_data(msg);
            break;

        case MSG_CAR_RESUME:
            handle_car_resume(msg);
            break;

        case MSG_CAR_PAUSE:
            handle_car_pause(msg);
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

    vTaskDelay(CONTROLLER_BT_ANDROID * 10);
    controller.can_send(CONTROLLER_MASTER, MSG_HEARTBEAT_ACK, (uint8_t *) &data, sizeof(data));
    heartbeat_count = (heartbeat_count + 1) % 100;

    return true;
}

bool canRxProcessTask :: handle_heartbeat_ack(msg_t msg)
{
    heartbeat_ack_data_t data;

    data = msg.heartbeat_ack_data;

    return true;
}

bool canRxProcessTask :: handle_dist_sensor_data(msg_t msg)
{
    dist_sensor = msg.dist_sensor_data;

    /* TODO: display this data on screen */
    //printf("L %2d M %2d R %2d B %2d\n", data.left, data.middle, data.right, data.back);

    return true;
}

bool canRxProcessTask :: handle_other_sensor_data(msg_t msg)
{
    other_sensor = msg.other_sensor_data;

    /* TODO: display this data on screen */
    //printf("B %d L %d \n", data.battery, data.light);

    return true;
}

bool canRxProcessTask :: handle_geo_heading_data(msg_t msg)
{
    geo_heading = msg.geo_heading_data;

    /* TODO: display this data on screen */
    //printf("V %2d CA %2d DA %2d LT %f LG %f FD %3d CD %3d\n", data.is_valid, data.current_angle, data.desired_angle, data.latitude, data.longitude, data.dist_to_next_checkpoint, data.dist_to_final_destination);

    return true;
}

bool canRxProcessTask :: handle_geo_location_data(msg_t msg)
{
    geo_location = msg.geo_location_data;

    /* TODO: display this data on screen */
    //printf("V %2d CA %2d DA %2d LT %f LG %f FD %3d CD %3d\n", data.is_valid, data.current_angle, data.desired_angle, data.latitude, data.longitude, data.dist_to_next_checkpoint, data.dist_to_final_destination);

    return true;
}

bool canRxProcessTask :: handle_checkpoint_request(msg_t msg)
{
    checkpoint_request = msg.checkpoint_request_data;

    /* TODO: return requested checkpoint, and suspend periodic task */

    return true;
}

bool canRxProcessTask :: handle_speed_encoder_data(msg_t msg)
{
    speed_encoder = msg.speed_encoder_data;

    /* TODO: display this data on screen */
    //printf("S %2d\n", data.speed);

    return true;
}

bool canRxProcessTask :: handle_car_resume(msg_t msg)
{
    /* TODO: change start/stop toggle button on LCD */

    return true;
}

bool canRxProcessTask :: handle_car_pause(msg_t msg)
{
    /* TODO: change start/stop toggle button on LCD */

    return true;
}

bool canRxProcessTask :: handle_drive_mode(msg_t msg)
{
    drive_mode = msg.drive_mode_data;

    /* TODO: display current drive mode on screen */
    //printf("M %d\n", data.mode);

    return true;
}

#endif /* ENABLE_BT_ANDROID */
