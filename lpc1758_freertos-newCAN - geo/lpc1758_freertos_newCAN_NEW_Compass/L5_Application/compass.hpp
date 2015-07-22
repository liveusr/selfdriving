/*
 * compass.hpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Ishan
 */

#ifndef COMPASS_HPP_
#define COMPASS_HPP_

#include "i2c2_device.hpp"  // I2C Device base class
#include <math.h>
#include <stdio.h>
#include "scheduler_task.hpp"
#include "semphr.h"
#include "controller_geo.hpp"

//#define I2C_Addr 0x3C

extern float Compass_Degrees;                             // Global variable to store Compass heading
//extern SemaphoreHandle_t Compass_lock;                    // Create semaphore lock to guard Compass heading variable


typedef enum
{
   _0_75HZ  = (0x0<<2),
   _1_5HZ   = (0x1<<2),
   _3HZ     = (0x2<<2),
   _7_5HZ   = (0x3<<2),
   _15HZ    = (0x4<<2),
   _30HZ    = (0x5<<2),
   _75HZ    = (0x6<<2),
   reserved = (0x7<<2)
} Compass_speed;           //this enum declares speeds for taking readings

typedef enum
{
   one_sample     = (0x0<<5),
   two_samples    = (0x1<<5),
   three_samples  = (0x2<<5),
   eight_samples  = (0x3<<5)
} Compass_avg_samples;     //this enum declares number of samples whose average is calculated for one reading

typedef enum
{
   no_bias        = 0x0,
   posetive_bias  = 0x1,
   negative_bias  = 0x2,
   bias_reserved  = 0x3
} Compass_bias;            //this enum declares bias for calibrating compass using internal magnetic compensation

typedef enum
{
   continuous           = 0x0,
   single               = 0x1,
   mesurement_reserved  = 0x3
} Compass_mesurementmode;   //this enum declares mode for taking readings

typedef enum
{
   Config_Reg_A    = 0x0,
   Config_Reg_B    = 0x1,
   Mode_reg        = 0x2,
   Dataout_X_MSB   = 0x3,
   Dataout_X_lSB   = 0x4,
   Dataout_Z_MSB   = 0x5,
   Dataout_Z_lSB   = 0x6,
   Dataout_Y_MSB   = 0x7,
   Dataout_Y_lSB   = 0x8,
   Status_Reg      = 0x9
} Compass_Registers;       //this enum declares registers for HMC5883L compass



class Compass: public scheduler_task
{
    public:
    uint8_t get_Zone(float Heading);
    float get_Compass_HeadingDegree(void);
    Compass(uint8_t priority);
    bool run(void *p);
};

#endif /* COMPASS_HPP_ */
