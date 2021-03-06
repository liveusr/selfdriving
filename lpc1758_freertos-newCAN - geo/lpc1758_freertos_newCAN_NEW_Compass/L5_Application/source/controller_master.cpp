/*
 * controller_master.cpp
 *
 *  Created on: Nov 6, 2014
 *      Author: MANish
 */

#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"
#include "utilities.h"

#include "controller_master.hpp"

#ifdef ENABLE_MASTER // enabled from can_common.hpp

#define HEARTBEAT_MISS_THRESHOLD 3

#define POWERUP_FLAG_MASK   ((1 << CONTROLLER_MOTOR) | (1 << CONTROLLER_SENSOR) | (1 << CONTROLLER_GEO) | (1 << CONTROLLER_IO) | (1 << CONTROLLER_BT_ANDROID))
#define HEARTBEAT_FLAG_MASK ((1 << CONTROLLER_MOTOR) | (1 << CONTROLLER_SENSOR) | (1 << CONTROLLER_GEO) | (1 << CONTROLLER_IO) | (1 << CONTROLLER_BT_ANDROID))

static can_msg_id_t filter_list[] = {
        {CONTROLLER_IO,             CONTROLLER_MASTER,      MSG_RESET},

        {CONTROLLER_MOTOR,          CONTROLLER_MASTER,      MSG_POWERUP_SYN_ACK},
        {CONTROLLER_SENSOR,         CONTROLLER_MASTER,      MSG_POWERUP_SYN_ACK},
        {CONTROLLER_GEO,            CONTROLLER_MASTER,      MSG_POWERUP_SYN_ACK},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_MASTER,      MSG_POWERUP_SYN_ACK},
        {CONTROLLER_IO,             CONTROLLER_MASTER,      MSG_POWERUP_SYN_ACK},

        {CONTROLLER_MOTOR,          CONTROLLER_MASTER,      MSG_HEARTBEAT_ACK},
        {CONTROLLER_SENSOR,         CONTROLLER_MASTER,      MSG_HEARTBEAT_ACK},
        {CONTROLLER_GEO,            CONTROLLER_MASTER,      MSG_HEARTBEAT_ACK},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_MASTER,      MSG_HEARTBEAT_ACK},
        {CONTROLLER_IO,             CONTROLLER_MASTER,      MSG_HEARTBEAT_ACK},

        {CONTROLLER_SENSOR,         CONTROLLER_ALL,         MSG_DIST_SENSOR_DATA},
        {CONTROLLER_GEO,            CONTROLLER_ALL,         MSG_GEO_HEADING_DATA},
        {CONTROLLER_MOTOR,          CONTROLLER_ALL,         MSG_SPEED_ENCODER_DATA},

        {CONTROLLER_BT_ANDROID,     CONTROLLER_ALL,         MSG_CAR_PAUSE},
        {CONTROLLER_IO,             CONTROLLER_ALL,         MSG_CAR_PAUSE},

        {CONTROLLER_BT_ANDROID,     CONTROLLER_ALL,         MSG_CAR_RESUME},
        {CONTROLLER_IO,             CONTROLLER_ALL,         MSG_CAR_RESUME},

        {CONTROLLER_BT_ANDROID,     CONTROLLER_ALL,         MSG_DRIVE_MODE},
        {CONTROLLER_IO,             CONTROLLER_ALL,         MSG_DRIVE_MODE},

        //{CONTROLLER_MASTER,         CONTROLLER_MASTER,      MSG_DUMMY},
};

can_controller controller(CONTROLLER_MASTER, filter_list, sizeof(filter_list) / sizeof(can_msg_id_t));
static QueueHandle_t msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(msg_t));

static uint8_t powerup_flag;
static uint8_t heartbeat_ack_flag;
static uint8_t heartbeat_miss_count;
static uint8_t heartbeat_count;

dist_sensor_data_t dist_sensor;
geo_heading_data_t geo_heading;
speed_encoder_data_t speed_encoder;
drive_mode_data_t drive_mode;

/* TODO: populate this variable to send periodic data */
speed_dir_data_t periodic_speed_dir = {SPEED_STOP, TURN_STRAIGHT, DIR_FWD};

class heartbeatTask : public scheduler_task
{
    public:
        heartbeatTask(uint8_t priority) : scheduler_task("heartbeatTask", 1024, priority)
        {
            setRunDuration(1009); // in milliseconds
        }

        bool run(void *p)
        {
            heartbeat_data_t data;

            if((powerup_flag & POWERUP_FLAG_MASK) != POWERUP_FLAG_MASK) {
                /* if all boards are not up, then don't send any msg */
                return true;
            }

            if((heartbeat_ack_flag & HEARTBEAT_FLAG_MASK) != HEARTBEAT_FLAG_MASK) {
                if(!(heartbeat_ack_flag & CONTROLLER_MOTOR)) {
                    printf("[%d] %s not responding\n", heartbeat_miss_count, controller_id_str[CONTROLLER_MOTOR]);
                }
                if(!(heartbeat_ack_flag & CONTROLLER_SENSOR)) {
                    printf("[%d] %s not responding\n", heartbeat_miss_count, controller_id_str[CONTROLLER_SENSOR]);
                }
                if(!(heartbeat_ack_flag & CONTROLLER_GEO)) {
                    printf("[%d] %s not responding\n", heartbeat_miss_count, controller_id_str[CONTROLLER_GEO]);
                }
                if(!(heartbeat_ack_flag & CONTROLLER_IO)) {
                    printf("[%d] %s not responding\n", heartbeat_miss_count, controller_id_str[CONTROLLER_IO]);
                }
                if(!(heartbeat_ack_flag & CONTROLLER_BT_ANDROID)) {
                    printf("[%d] %s not responding\n", heartbeat_miss_count, controller_id_str[CONTROLLER_BT_ANDROID]);
                }

                heartbeat_miss_count++;
                if(heartbeat_miss_count > HEARTBEAT_MISS_THRESHOLD) {
                    /* if heartbeat from any controller is missed, reset master, this will reset all boards */
                    printf("heartbeat missed..\n");
                    printf("resetting the car\n");

                    vTaskDelay(1000);
                    controller.processReset();
                }
            }
            else {
                /* reset heartbeat_flag, this will be set in handle_heartbeat_ack() */
                heartbeat_ack_flag = 0;
                heartbeat_miss_count = 0;
            }

            data.rx_count = controller.get_rx_count();
            data.rx_bytes = controller.get_rx_bytes();
            data.tx_count = controller.get_tx_count();
            data.tx_bytes = controller.get_tx_bytes();

            controller.can_send(CONTROLLER_ALL, MSG_HEARTBEAT, (uint8_t *)&data, sizeof(heartbeat_data_t));
            heartbeat_count = (heartbeat_count + 1) % 100;

            return true;
        }
};

class speedDirTask : public scheduler_task
{
    public:
        speedDirTask(uint8_t priority) : scheduler_task("speedDirTask", 1024, priority)
        {
            setRunDuration(41); // in milliseconds
        }

        bool run(void *p)
        {
            if((powerup_flag & POWERUP_FLAG_MASK) != POWERUP_FLAG_MASK) {
                /* if all boards are not up, then don't send any msg */
                return true;
            }

            controller.can_send(CONTROLLER_MOTOR, MSG_SPEED_DIR_COMMAND, (uint8_t *)&periodic_speed_dir, sizeof(speed_dir_data_t));

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
                    periodic_speed_dir.speed = 1 + (periodic_speed_dir.speed % 4);
                    vTaskDelay(500);
                    break;

                case 0x04:
                    /* switch 3 pressed */
                    periodic_speed_dir.turn = 1 + (periodic_speed_dir.turn % 7);
                    vTaskDelay(500);
                    break;

                case 0x08:
                    /* switch 4 pressed */
                    periodic_speed_dir.direction = 1 + (periodic_speed_dir.direction % 2);
                    vTaskDelay(500);
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
#if 0
    printf( "%2d  powerup_syn_ack_data_t    \n",  sizeof(powerup_syn_ack_data_t    ));
    printf( "%2d  powerup_ack_data_t        \n",  sizeof(powerup_ack_data_t        ));
    printf( "%2d  heartbeat_data_t          \n",  sizeof(heartbeat_data_t          ));
    printf( "%2d  heartbeat_ack_data_t      \n",  sizeof(heartbeat_ack_data_t      ));
    printf( "%2d  dist_sensor_data_t        \n",  sizeof(dist_sensor_data_t        ));
    printf( "%2d  other_sensor_data_t       \n",  sizeof(other_sensor_data_t       ));
    printf( "%2d  geo_heading__data_t       \n",  sizeof(geo_heading_data_t        ));
    printf( "%2d  geo_location_data_t       \n",  sizeof(geo_location_data_t       ));
    printf( "%2d  speed_encoder_data_t      \n",  sizeof(speed_encoder_data_t      ));
    printf( "%2d  checkpoint_data_t         \n",  sizeof(checkpoint_data_t         ));
    printf( "%2d  checkpoint_request_data_t \n",  sizeof(checkpoint_request_data_t ));
    printf( "%2d  drive_mode_data_t         \n",  sizeof(drive_mode_data_t         ));
    printf( "%2d  motor_direction_t         \n",  sizeof(motor_direction_t         ));
    printf( "%2d  speed_dir_data_t          \n",  sizeof(speed_dir_data_t          ));
    printf( "%2d  error_data_t              \n",  sizeof(error_data_t              ));
    printf( "%2d  dummy_data_t              \n",  sizeof(dummy_data_t              ));
    getchar();
#endif
    /* clear error flag */
    SET_ERROR(ERROR_NO_ERROR);

    /* reset all boards when master resets */
    controller.can_send(CONTROLLER_ALL, MSG_RESET, NULL, 0);

    // start any other tasks of module here, like periodic task
    scheduler_add_task(new heartbeatTask(PRIORITY_HIGH));
    scheduler_add_task(new speedDirTask(PRIORITY_HIGH));

    /* setup debug variables */
    scheduler_task *dt = new debugTask(PRIORITY_MEDIUM);
    scheduler_add_task(dt);
    ((debugTask *)dt)->setLeftRight(&periodic_speed_dir.turn, &periodic_speed_dir.speed, 0);
    ((debugTask *)dt)->setLeftRight(&dist_sensor.left, &dist_sensor.right, 1);
    ((debugTask *)dt)->setLeftRight(&geo_heading.current_angle, &geo_heading.desired_angle, 2);
    ((debugTask *)dt)->setLeftRight(&dist_sensor.middle, &speed_encoder.speed, 3);
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

    if((powerup_flag & POWERUP_FLAG_MASK) != POWERUP_FLAG_MASK) {
        if(msg.msg_num == MSG_POWERUP_SYN_ACK) {
            powerup_flag |= (1 << msg.src);
            printf("syn_ack received form %s\n", controller_id_str[msg.src]);

            /* wait until all the controllers are up */
            if((powerup_flag & POWERUP_FLAG_MASK) == POWERUP_FLAG_MASK) {
                powerup_ack_data_t data;

                // set date and time to send to other controllers in this variable
                data.version = CONTROLLER_VERSION;

                printf("sending ack\n");
                controller.can_send(CONTROLLER_ALL, MSG_POWERUP_ACK, (uint8_t *)&data, sizeof(data));
                LE.off(4); // turn OFF LED4 after can communication is setup
            }
        }

        /* don't buffer any msg until all boards are up */
        return true;
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

    /* don't process any msg until all boards are up */
    while((powerup_flag & POWERUP_FLAG_MASK) != POWERUP_FLAG_MASK) {
        LE.on(4); // LED4 will be ON until can communication is setup

        /* all controllers are not up yet, send POWERUP_SYN every 1 second */
        printf("sending syn\n");
        controller.can_send(CONTROLLER_ALL, MSG_POWERUP_SYN, NULL, 0);
        vTaskDelayMs(1000);
    }

    if(xQueueReceive(msg_queue, &msg, portMAX_DELAY) == pdFALSE) {
        /* no message received, do nothing */
        return true;
    }

    if((error_flag & ERROR_MSG_QUEUE_FULL) == ERROR_MSG_QUEUE_FULL) {
        RESET_ERROR(ERROR_MSG_QUEUE_FULL);
    }

    switch(msg.msg_num) {
        case MSG_HEARTBEAT_ACK:
            handle_heartbeat_ack(msg);
            break;

        case MSG_DIST_SENSOR_DATA:
            handle_dist_sensor_data(msg);
            break;

        case MSG_GEO_HEADING_DATA:
            handle_geo_heading_data(msg);
            break;

        case MSG_SPEED_ENCODER_DATA:
            handle_speed_encoder_data(msg);
            break;

        case MSG_CAR_PAUSE:
            handle_car_pause_data(msg);
            break;

        case MSG_CAR_RESUME:
            handle_car_resume_data(msg);
            break;

        case MSG_DRIVE_MODE:
            handle_drive_mode_data(msg);
            break;

        default:
            /* do nothing */
            break;
    }

    return true;
}

bool canRxProcessTask :: handle_heartbeat_ack(msg_t msg)
{
    /* TODO: check if all modules have ack'ed */
    heartbeat_ack_flag |= (1 << msg.src);

    return true;
}

bool canRxProcessTask :: handle_dist_sensor_data(msg_t msg)
{
    dist_sensor = msg.dist_sensor_data;

    //printf("L %2d M %2d R %2d B %2d\n", data.left, data.middle, data.right, data.back);

    return true;
}

bool canRxProcessTask :: handle_geo_heading_data(msg_t msg)
{
    geo_heading = msg.geo_heading_data;

    //printf("V %2d CA %2d DA %2d LT %f LG %f FD %3d CD %3d\n", data.is_valid, data.current_angle, data.desired_angle, data.latitude, data.longitude, data.dist_to_next_checkpoint, data.dist_to_final_destination);

    return true;
}

bool canRxProcessTask :: handle_speed_encoder_data(msg_t msg)
{
    speed_encoder = msg.speed_encoder_data;

    //printf("S %2d\n", data.speed);

    return true;
}

bool canRxProcessTask :: handle_car_pause_data(msg_t msg)
{
    /* TODO: send stop signal to motor module */

    return true;
}

bool canRxProcessTask :: handle_car_resume_data(msg_t msg)
{
    /* TODO: resume algo and send signal to motor module accordingly */

    return true;
}

bool canRxProcessTask :: handle_drive_mode_data(msg_t msg)
{
    drive_mode = msg.drive_mode_data;

    //printf("DM %2d\n", data.mode);

    return true;
}

#endif /* ENABLE_MASTER */
