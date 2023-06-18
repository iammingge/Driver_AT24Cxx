/**
 * Copyright (c) 2023 iammingge
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file    AT24Cxx.h
 * @brief   AT24Cxx family driver header file
 * @author  iammingge
 *
 *      DATE             NAME                      DESCRIPTION
 *
 *   01-12-2023        iammingge                Initial Version 1.0
 *   18-06-2023        iammingge                1. Support software i2c to drive device
 *											    2. Support hardware i2c to drive device
**/
#ifndef __AT24CXX_H
#define __AT24CXX_H
#include "bus_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AT24CXX_GLOBALS
#define AT24CXX_EXT
#else
#define AT24CXX_EXT extern
#endif

/**
 * @brief AT24Cxx I2C bus mode
 * 0 : software
 * 1 : hardware
 */
#define	AT24Cxx_I2C_MODE			1

/**
 * @brief Erase maximum length at one time
 */
#define AT24Cxx_MAX_ERASE_SIZE		10

/**
 * @brief Once compare size in Readback Write
 */
#define AT24Cxx_MAX_COMPARE_SIZE    10

/**
 * @brief Self-timed Write cycle (5ms max)
 * The delay function in the stm32 HAL library function is inaccurate and may be
 * affected by other peripheral library functions with timeout function. (HAL_Delay)
 */
#define AT24CXX_WCYCLEMS            do { \
                                        /*------ User add 5ms delay ------*/ \
                                        uint32_t time = 80000; \
                                        do {} while (time--); \
                                        /*--------------------------------*/ \
                                    } while (0);

/**
 * @brief AT24Cxx Type
 */
typedef enum
{
    AT24C01 = 0x01,
    AT24C02 = 0x02,
    AT24C04 = 0x03,
    AT24C08 = 0x04,
    AT24C16 = 0x05,
    AT24C32 = 0x06,
    AT24C64 = 0x07,
    AT24C128 = 0x08,
    AT24C256 = 0x09,
    AT24C512 = 0x0A,
    AT24CM01 = 0x0B,
    AT24CM02 = 0x0C
} AT24Cxx_CHIP;

/**
 * @brief AT24Cxx Device Info
 */
typedef struct
{
    AT24Cxx_CHIP type;
    union
    {
        uint8_t byte;				/* bit0 : 1 read, 0 write */
        struct
        {
            uint8_t : 1;
            uint8_t bit0 : 1;
            uint8_t bit1 : 1;
            uint8_t bit2 : 1;
            uint8_t : 4;
        } wordaddr;
        struct
        {
            uint8_t : 1;
            uint8_t bit : 3;
            uint8_t : 4;
        } hardaddr;
        struct
        {
            uint8_t : 4;
            uint8_t bit : 4;	  /* 1 0 1 0 */
        } devtype;
    } i2caddr;
    uint16_t pagesize;
} AT24Cxx_INFO_t;

/**
 * @brief AT24Cxx Device Port
 */
typedef struct
{
#if AT24Cxx_I2C_MODE == 0
    sw_i2c_t *bus;
#else
    hw_i2c_t *bus;
#endif
} AT24Cxx_PORT_t;

/**
 * @brief AT24Cxx Device Struct
 */
typedef struct
{
    AT24Cxx_INFO_t info;
    AT24Cxx_PORT_t port;
} at24cxx_t;

/**
 * @brief AT24Cxx Basic Function
 */
void AT24Cxx_config(at24cxx_t *dev, AT24Cxx_CHIP type, uint8_t devaddr, uint8_t haraddr);  /* AT24Cxx Mount device */
uint8_t AT24Cxx_Write(at24cxx_t *dev, uint32_t saddr, uint8_t *data, uint32_t size);       /* AT24Cxx Write data */
uint8_t AT24Cxx_Read(at24cxx_t *dev, uint32_t saddr, uint8_t *data, uint32_t size);        /* AT24Cxx Read  data */
uint8_t AT24Cxx_Erase(at24cxx_t *dev, uint32_t saddr, uint8_t fdata, uint32_t size);       /* AT24Cxx Erase data */

/**
 * @brief AT24Cxx Application Function
 */
uint8_t AT24Cxx_Readback_Write(at24cxx_t *dev, uint16_t addr, uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif










