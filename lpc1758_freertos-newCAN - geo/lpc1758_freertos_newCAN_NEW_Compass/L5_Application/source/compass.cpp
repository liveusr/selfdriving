/*
 * compass.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Ishan
 */

#include "compass.hpp"
#include "io.hpp"
#include "utilities.h"
extern geo_heading_data_t periodic_geo_heading_data;
float current_angle=0;

Compass::Compass(uint8_t priority) :
   scheduler_task("Compass", 4 * 512, priority)
{
    I2C2 &i2c = I2C2 :: getInstance();
    i2c.init(100);
    setRunDuration(50);
}


bool Compass::run(void *p)
    {
        current_angle=get_Compass_HeadingDegree();
        periodic_geo_heading_data.current_angle= get_Zone(get_Compass_HeadingDegree());
        return true;
    }

uint8_t Compass::get_Zone(float Heading)
{
    return (uint8_t)(Heading/18.0);
}


float Compass::get_Compass_HeadingDegree(void)
{
    I2C2 &i2c = I2C2 :: getInstance();
    uint8_t buf[2];
    int heading;
    float headingDegrees;

    i2c.writeReg(0x00, 0x00, 'A');
    delay_ms(10);
    i2c.readRegisters(0x00, 0x00, (char *) buf, 2);
    heading = ((buf[0] << 8) + buf[1]);
    delay_ms(10);
    headingDegrees=(float)heading/10.0;
    headingDegrees = (headingDegrees-70 < 0)? (headingDegrees-70+360): (headingDegrees-70);

    if(headingDegrees>=360)
        headingDegrees=0;
    //printf("%f\n", headingDegrees);
    return headingDegrees;
}


