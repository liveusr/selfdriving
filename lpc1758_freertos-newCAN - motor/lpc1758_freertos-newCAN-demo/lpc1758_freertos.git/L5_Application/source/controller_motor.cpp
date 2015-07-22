/*
 * controller_motor.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"
#include "encoder.hpp"

#include "controller_motor.hpp"

#ifdef ENABLE_MOTOR // enabled from can_common.hpp

float curr_pwm;
float base_speed = 0;
float current_speed = 0;

#define left            (float)10.50
#define slight_left     (float)13.53
#define half_left       (float)12.35
#define straight        (float)14.70
#define slight_right    (float)16.00
#define half_right      (float)17.37
#define right           (float)20.03

#define normal_stop     (float)12.81
#define emergency_stop  (float)10.0
#define dc_forward      (float)15.90
#define dc_reverse      (float)14.0
#define base_slow       (float)15.7
#define base_normal     (float)16.3
#define base_turbo      (float)16.9

#define speedinc_delay         10
#define inc_factor      (float)0.2

volatile bool rev_flag = 0;
volatile bool speed_flag = 0;
volatile bool startup = 0;
int disp_speed = 0, disp_dir = 0;
int duty_cycle_left, duty_cycle_right;
#define initSeq         (float)15.0


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
                float   *float_val;
        } debug_data[MAX_DEBUG_DATA];

        debugTask(uint8_t priority) : scheduler_task("debugTask", 1024, priority)
        {
            int8_t i = 0;
            for(i = 0; i < MAX_DEBUG_DATA; i++) {
                debug_data[i].left_val = NULL;
                debug_data[i].right_val = NULL;
                debug_data[i].int_val = NULL;
                debug_data[i].hex_val = NULL;
                debug_data[i].float_val = NULL;
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
                else if(debug_data[curr_index].float_val) {
                    uint8_t temp = *debug_data[curr_index].float_val * 10;
                    LD.setLeftDigit(temp / 10);
                    LD.setRightDigit(temp % 10);
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
                    current_speed = current_speed + 0.1f;
                    vTaskDelay(500);
                    break;

                case 0x04:
                    /* switch 3 pressed */
                    current_speed = 15.8f;
                    vTaskDelay(500);
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

        void setFloat(void *val, uint8_t index)
        {
            debug_data[index].float_val = (float *) val;
        }
};

void controllerInit(void)
{
    /* clear error flag */
    SET_ERROR(ERROR_NO_ERROR);

    controller.processBootSequence(CONTROLLER_VERSION);

    /* TODO: start any other tasks of module here, like periodic task */
    scheduler_add_task(new periodicEncoderTask(PRIORITY_HIGH));
    scheduler_add_task(new encoderTask());
    //scheduler_add_task(new speedTask());

    /* setup debug variables */
    scheduler_task *dt = new debugTask(PRIORITY_MEDIUM);
    scheduler_add_task(dt);
    ((debugTask *)dt)->setInt(&periodic_encoder_data.speed, 0);
    ((debugTask *)dt)->setFloat(&curr_pwm, 1);
    ((debugTask *)dt)->setLeftRight(&speed_dir_cmd.turn, &speed_dir_cmd.speed, 2);
    ((debugTask *)dt)->setLeftRight(&speed_dir_cmd.direction, &drive_mode.mode, 3);
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
    motorInit();
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

    //speed_dir_data_t speed_dir_cmd;
    //speed_dir_cmd = msg.speed_dir_data;

    disp_speed = speed_dir_cmd.speed;
    disp_dir = speed_dir_cmd.direction;

    /* TODO: process received left, right, fwd, rev, etc. commands stored in data */
    //printf("S %d T %d D %d\n", data.speed, data.turn, data.direction);

    if (speed_dir_cmd.direction == DIR_FWD)    //when master is sending reverse condition we have to ignore this switch case
    {
        periodic_encoder_data.brake = 0;
        switch(speed_dir_cmd.speed)
        {
            case SPEED_SLOW:
                current_speed = base_speed;
                for(int i=0;i<3;i++)
                {
                    pwm_dcmotor(current_speed);
                    current_speed = current_speed + inc_factor;
                    delay_ms(speedinc_delay);
                }
                break;
            case SPEED_MEDIUM:
                current_speed = base_speed;
                for(int i=0;i<3;i++)
                {
                    pwm_dcmotor(current_speed);
                    current_speed = current_speed + inc_factor;
                    delay_ms(speedinc_delay);
                }
                break;
            case SPEED_FAST:
                current_speed = base_speed;
                for(int i=0;i<3;i++)
                {
                    pwm_dcmotor(current_speed);
                    current_speed = current_speed + inc_factor;
                    delay_ms(speedinc_delay);
                }
                break;
            case SPEED_STOP:
                pwm_dcmotor(normal_stop);
                break;
            case SPEED_EMERGENCY_STOP:
                pwm_dcmotor(emergency_stop);
                break;
            default:
                pwm_dcmotor(emergency_stop);
                break;
        }
    }
    switch(speed_dir_cmd.turn)
    {
        case TURN_STRAIGHT:
            pwm_servomotor(straight);
            break;
        case TURN_FULL_LEFT:
            pwm_servomotor(left);
            break;
        case TURN_SLIGHT_LEFT:
            pwm_servomotor(slight_left);
            break;
        case TURN_HALF_LEFT:
            pwm_servomotor(half_left);
            break;
        case TURN_FULL_RIGHT:
            pwm_servomotor(right);
            break;
        case TURN_SLIGHT_RIGHT:
            pwm_servomotor(slight_right);
            break;
        case TURN_HALF_RIGHT:
            pwm_servomotor(half_right);
            break;
        default:
            pwm_servomotor(straight);
            break;
    }
    if ((speed_dir_cmd.speed == SPEED_EMERGENCY_STOP) || (speed_dir_cmd.speed == SPEED_STOP))
    {

        periodic_encoder_data.brake = 1;
        if (startup == 0)
        {
            printf("Brake\n");
            pwm_dcmotor(initSeq);
            delay_ms(10);
            pwm_dcmotor(dc_forward);
            startup = 1;
            speed_flag = 1;
        }
        else if (speed_flag == 0 && rev_flag == 1)
        {
            pwm_dcmotor(initSeq);
            rev_flag = 0;
        }
        else if (speed_flag == 0 && rev_flag == 0)
        {
            pwm_dcmotor(emergency_stop);
            rev_flag = 0;
        }
        else
        {
            pwm_dcmotor(initSeq);
            rev_flag = 0;
        }
    }
    else
    {
        switch(speed_dir_cmd.direction)
        {
            case DIR_FWD:
                pwm_dcmotor(dc_forward);
                printf("Forward\n");
                rev_flag = 0;
                speed_flag = 0;
                periodic_encoder_data.brake = 0;
                periodic_encoder_data.dir = speed_dir_cmd.direction;
                break;
            case DIR_REV:
                if(rev_flag == 0)
                {
                    reverseInit();
                    rev_flag = 1;
                }
                printf("Reverse\n");
                pwm_dcmotor(dc_reverse);
                speed_flag = 1;
                periodic_encoder_data.brake = 0;
                periodic_encoder_data.dir = speed_dir_cmd.direction;
                break;
            default:
                pwm_dcmotor(normal_stop);
                break;
        }
    }
    return true;
}

bool canRxProcessTask :: handle_drive_mode(msg_t msg)
{
    drive_mode = msg.drive_mode_data;
    drive_mode_data_t data;

    /* TODO: process only if received drive mode is MODE_SLOW/MODE_NORMAL/MODE_TURBO to change base drive speed */
    //printf("M %d\n", data.mode);

    if (data.mode == MODE_SLOW)
            base_speed = base_slow;
    else if (data.mode == MODE_NORMAL)
            base_speed = base_normal;
    else if (data.mode == MODE_TURBO)
            base_speed = base_turbo;

    return true;
}

#endif /* ENABLE_MOTOR */
