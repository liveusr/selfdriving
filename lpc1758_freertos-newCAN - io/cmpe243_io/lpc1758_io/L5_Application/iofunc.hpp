/*
 * iofunc.h
 *
 *  Created on: 03-Oct-2014
 *      Author: Tej
 */

#ifndef IOFUNC_H_
#define IOFUNC_H_

#include "scheduler_task.hpp"
#include "lpc_pwm.hpp"
#include "controller_io.hpp"
#include "file_logger.h"

extern bool resume_pause;
extern bool ctrl_changed;

extern drive_mode_data_t io_drive_mode;
extern speed_encoder_data_t io_speed;
//extern speed_dir_data_t io_speed_dir_data;
extern geo_heading_data_t io_geo_head;
extern geo_location_data_t io_geo_loc;
extern checkpoint_data_t io_checkpoint_data;
extern dist_sensor_data_t io_snr_data;
extern other_sensor_data_t io_other_snr_data;

/*extern powerup_syn_ack_data_t mot_ver;
extern powerup_syn_ack_data_t sen_ver;
extern powerup_syn_ack_data_t geo_ver;
extern powerup_syn_ack_data_t bt_ver;

extern powerup_ack_data_t mas_ver;*/

extern heartbeat_ack_data_t mot_can;
extern heartbeat_ack_data_t mas_can;   // Master not sending RX TX counts
extern heartbeat_ack_data_t sen_can;
extern heartbeat_ack_data_t geo_can;
extern heartbeat_ack_data_t bt_can;

extern module_stat_time_t stat_time;
extern data_rcv_t data_rcv;

void UART_int();
void car_con();
void home_dst();
void reset_all();
void glcd_con();
void timer();

/*task to display information on GLCD*/
class GLCD : public scheduler_task
{
    public:
        GLCD(uint8_t priority);
        void GLCD_init(void);
        void tx_data(char data_tx);
        void GLCD_write(char cmd, char obj_id, char obj_index, char data_msb,
                char data_lsb);
        void GLCD_write_str(char cmd, char str_index, char *str);
        void GLCD_write_int(char cmd, char str_index, int val);
        void GLCD_write_float(char cmd, char str_index, float val, uint8_t precision);
        void GLCD_contrast(char val);
        void display();
        void touch_screen();
        bool run(void *p);

};

class LIGHT: public scheduler_task
{
    public:
        LIGHT(uint8_t priority);
        void light_init();
        void chk_light();
        bool run(void *p);
};

/*class BAT : public scheduler_task
{
    public:
        BAT(uint8_t priority);
        void bat_init();
        void chk_bat();
        bool run(void *p);
};*/

class SWITCH: public scheduler_task
{
    public:
            SWITCH(uint8_t priority);
            void sw_init();
            bool run(void *p);
};


#endif /* IOFUNC_H_ */
