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
 * @brief   AT24Cxx family driver source file
 * @author  iammingge
 *
 *      DATE             NAME                      DESCRIPTION
 *
 *   01-12-2023        iammingge                Initial Version 1.0
 *   18-06-2023        iammingge                1. Support software i2c to drive device
 *											    2. Support hardware i2c to drive device
**/

#include "AT24Cxx.h"
#include <string.h>

/* -----------------------------------------------------------------------------------------------------------------------------
   |                                                        EEPROM                                                             |
   -----------------------------------------------------------------------------------------------------------------------------
   |   Type   |  Page  |             Byte               |       Bit      |        Device Address  (1/0) |     Word Address     |
   | AT24CM02 |  1024  | 1024 * 256 (0x00000 - 0x3FFFF) | 1024 * 256 * 8 | 0b 1010  X    A17  A16  R/W  | A0 - A17 (two bytes) |
   | AT24CM01 |   512  |  512 * 256 (0x00000 - 0x1FFFF) |  512 * 256 * 8 | 0b 1010  X    X    A16  R/W  | A0 - A16 (two bytes) |
   | AT24C512 |   512  |  512 * 128 (0x00000 - 0x0FFFF) |  512 * 128 * 8 | 0b 1010  X    X    X    R/W  | A0 - A15 (two bytes) |
   | AT24C256 |   512  |  512 *  64 (0x00000 - 0x07FFF) |  512 *  64 * 8 | 0b 1010  X    X    X    R/W  | A0 - A14 (two bytes) |
   | AT24C128 |   256  |  256 *  64 (0x00000 - 0x03FFF) |  256 *  64 * 8 | 0b 1010  X    X    X    R/W  | A0 - A13 (two bytes) |
   | AT24C64  |   256  |  256 *  32 (0x00000 - 0x01FFF) |  256 *  32 * 8 | 0b 1010  X    X    X    R/W  | A0 - A12 (two bytes) |
   | AT24C32  |   128  |  128 *  32 (0x00000 - 0x00FFF) |  128 *  32 * 8 | 0b 1010  X    X    X    R/W  | A0 - A11 (two bytes) |
   | AT24C16  |   128  |  128 *  16 (0x00000 - 0x007FF) |  128 *  16 * 8 | 0b 1010  A10  A9   A8   R/W  | A0 - A10 (one byte)  |
   | AT24C08  |    64  |   64 *  16 (0x00000 - 0x003FF) |   64 *  16 * 8 | 0b 1010  X    A9   A8   R/W  | A0 - A9  (one byte)  |
   | AT24C04  |    32  |   32 *  16 (0x00000 - 0x001FF) |   32 *  16 * 8 | 0b 1010  X    X    A8   R/W  | A0 - A8  (one byte)  |
   | AT24C02  |    32  |   32 *   8 (0x00000 - 0x000FF) |   32 *   8 * 8 | 0b 1010  X    X    X    R/W  | A0 - A7  (one byte)  |
   | AT24C01  |    16  |   16 *   8 (0x00000 - 0x0007F) |   16 *   8 * 8 | 0b 1010  X    X    X    R/W  | A0 - A6  (one byte)  |
   -----------------------------------------------------------------------------------------------------------------------------
**/
/* Tool Function */
#define rbit(val, x)        (((val) & (1<<(x)))>>(x))	    /* Read  1 bit */
#define min(a, b) 	        (((a) < (b)) ? (a) : (b))       /* Take the minimum value */    
#define max(a, b)           (((a) > (b)) ? (a) : (b))       /* Take the maximum value */
/*------------------------------------------------------*/
/*                AT24Cxx Basic Function                */
/*------------------------------------------------------*/
/**
 * @brief  AT24Cxx get page One-time write size
 * @param  {at24cxx_t} *dev : device structure pointer
 * @return {uint16_t}       : one page size
 * @note   none
 */
static uint16_t AT24Cxx_GetPageWriteSize(at24cxx_t *dev)
{
    if (dev->info.type == AT24C01 || \
        dev->info.type == AT24C02)
    {
        return 0x08;  /* One time write <= 8 byte */
    }
    else if (dev->info.type == AT24C04 || \
        dev->info.type == AT24C08 || \
        dev->info.type == AT24C16)
    {
        return 0x10;  /* One time write <= 16 byte */
    }
    else if (dev->info.type == AT24C32 || \
        dev->info.type == AT24C64)
    {
        return 0x20;  /* One time write <= 32 byte */
    }
    else if (dev->info.type == AT24C128 || \
        dev->info.type == AT24C256)
    {
        return 0x40;
    }
    else if (dev->info.type == AT24C512)
    {
        return 0x80;
    }
    else if (dev->info.type == AT24CM01 || \
        dev->info.type == AT24CM02)
    {
        return 0x100;
    }

    return 0x08;    /* Default maximum number of bytes written at once */
}
/**
 * @brief  AT24Cxx Init
 * @param  {at24cxx_t} *dev    : device structure pointer
 * @param  {AT24Cxx_CHIP} type : AT24Cxx series chip model
 * @param  {uint8_t} devaddr   : device address
 * @param  {uint8_t} haraddr   : hardware address
 * @return none
 * @note   none
 */
void AT24Cxx_config(at24cxx_t *dev, AT24Cxx_CHIP type, uint8_t devaddr, uint8_t haraddr)
{
    dev->info.type = type;
    dev->info.i2caddr.devtype.bit = devaddr;
    dev->info.i2caddr.hardaddr.bit = haraddr;
    dev->info.pagesize = AT24Cxx_GetPageWriteSize(dev);

#if AT24Cxx_I2C_MODE == 0

    /* Software reset */
    swi2c_reset(dev->port.bus);

#endif
}
/**
 * @brief  AT24Cxx set data word address
 * @param  {at24cxx_t} *dev    : device structure pointer
 * @param  {uint32_t} addr     : word address
 * @param  {uint8_t} *addrsize : word address size
 * @return none
 * @note   none
 */
static void AT24Cxx_SetWordAddress(at24cxx_t *dev, uint32_t addr, uint8_t *addrsize)
{
    if (dev->info.type >= AT24C32)
    {
        switch (dev->info.type)
        {
            case AT24CM01: {
                dev->info.i2caddr.wordaddr.bit0 = rbit(addr, 16);
                break;}
            case AT24CM02: {
                dev->info.i2caddr.wordaddr.bit0 = rbit(addr, 16);
                dev->info.i2caddr.wordaddr.bit1 = rbit(addr, 17);
                break;}
            default: break;
        }

        *addrsize = 2;
    }
    else
    {
        switch (dev->info.type)
        {
            case AT24C04: {
                dev->info.i2caddr.wordaddr.bit0 = rbit(addr, 8);
                break;}
            case AT24C08: {
                dev->info.i2caddr.wordaddr.bit0 = rbit(addr, 8);
                dev->info.i2caddr.wordaddr.bit1 = rbit(addr, 9);
                break;}
            case AT24C16: {
                dev->info.i2caddr.wordaddr.bit0 = rbit(addr, 8);
                dev->info.i2caddr.wordaddr.bit1 = rbit(addr, 9);
                dev->info.i2caddr.wordaddr.bit2 = rbit(addr, 10);
                break;}
            default: break;
        }

        *addrsize = 1;
    }
}
/**
 * @brief  AT24Cxx read memory data
 * @param  {at24cxx_t} *dev : device structure pointer
 * @param  {uint32_t} saddr : start address
 * @param  {uint8_t} *data  : read data pointer
 * @param  {uint32_t} size  : read data size (See the "Byte" column in the 30 line list above)
 * @return {uint8_t}        : 0 --- success
 *                            1 --- error
 * @note   none
 */
uint8_t AT24Cxx_Read(at24cxx_t *dev, uint32_t saddr, uint8_t *data, uint32_t size)
{
    uint8_t memaddr_size = 1;
    uint8_t rsp = 0;

#if AT24Cxx_I2C_MODE == 0

    uint32_t i = 0;

    /* Set data word address */
    AT24Cxx_SetWordAddress(dev, saddr, &memaddr_size);

    /*--------------------------------------------------*/
    /* IIC start */
    swi2c_strt(dev->port.bus);

    /* IIC send i2c address and at24cxx address */
    if (memaddr_size == 1)
    {
        rsp |= swi2c_waddr(dev->port.bus, dev->info.i2caddr.byte);
        rsp |= swi2c_wbyte(dev->port.bus, LSB_16(saddr));
    }
    else
    {
        rsp |= swi2c_waddr(dev->port.bus, dev->info.i2caddr.byte);
        rsp |= swi2c_wbyte(dev->port.bus, MSB_16(saddr));
        rsp |= swi2c_wbyte(dev->port.bus, LSB_16(saddr));
    }
    /*--------------------------------------------------*/

    /*--------------------------------------------------*/
    /* IIC start */
    swi2c_strt(dev->port.bus);

    /* IIC send i2c address */
    rsp |= swi2c_raddr(dev->port.bus, dev->info.i2caddr.byte);

    /* IIC send read data to memory */
    for (i = 0; i < size - 1; i++)
    {
        data[i] = swi2c_rbyte(dev->port.bus, ACK);
    }
    data[i] = swi2c_rbyte(dev->port.bus, NACK);

    /* IIC stop */
    swi2c_stop(dev->port.bus);
    /*--------------------------------------------------*/

#else

    /* Set data word address */
    AT24Cxx_SetWordAddress(dev, saddr, &memaddr_size);

    /* Read data */
    rsp = dev->port.bus->rmem(dev->info.i2caddr.byte, saddr, memaddr_size, data, size);

#endif

    return rsp;
}
/**
 * @brief  AT24Cxx write memory data
 * @param  {at24cxx_t} *dev : device structure pointer
 * @param  {uint32_t} saddr : start address
 * @param  {uint8_t} *data  : write data pointer
 * @param  {uint32_t} size  : write data size (See the "Byte" column in the 30 line list above)
 * @return {uint8_t}        : 0 --- success
 *                            1 --- error
 * @note   none
 */
uint8_t AT24Cxx_Write(at24cxx_t *dev, uint32_t saddr, uint8_t *data, uint32_t size)
{
    uint32_t i = 0;
    uint32_t RemainSize = size;
    uint32_t EndAddr = saddr + size;
    uint16_t CurPageSize = 0;
    uint8_t memaddr_size = 1;
    uint8_t rsp = 0;

#if AT24Cxx_I2C_MODE == 0

    uint16_t j = 0;

    for (i = saddr; i < EndAddr; i += size)
    {
        /* Get the remaining size of the current page */
        CurPageSize = dev->info.pagesize - (i % dev->info.pagesize);

        /* Current write size, Update remaining size */
        size = min(RemainSize, CurPageSize);
        RemainSize -= size;

        /* Set data word address */
        AT24Cxx_SetWordAddress(dev, i, &memaddr_size);

        /*--------------------------------------------------*/
        /* IIC start */
        swi2c_strt(dev->port.bus);

        /* IIC send i2c address and at24cxx address */
        if (memaddr_size == 1)
        {
            rsp |= swi2c_waddr(dev->port.bus, dev->info.i2caddr.byte);
            rsp |= swi2c_wbyte(dev->port.bus, LSB_16(i));
        }
        else
        {
            rsp |= swi2c_waddr(dev->port.bus, dev->info.i2caddr.byte);
            rsp |= swi2c_wbyte(dev->port.bus, MSB_16(i));
            rsp |= swi2c_wbyte(dev->port.bus, LSB_16(i));
        }

        /*  IIC send write data to memory */
        for (j = 0; j < size; j++)
        {
            rsp |= swi2c_wbyte(dev->port.bus, *(data++));
        }

        /* IIC stop */
        swi2c_stop(dev->port.bus);
        /*--------------------------------------------------*/

        /* Self-timed Write cycle */
        AT24CXX_WCYCLEMS;
    }

#else

    for (i = saddr; i < EndAddr; i += size)
    {
        /* Get the remaining size of the current page */
        CurPageSize = dev->info.pagesize - (i % dev->info.pagesize);

        /* Current write size, Update remaining size */
        size = min(RemainSize, CurPageSize);
        RemainSize -= size;

        /* Set data word address */
        AT24Cxx_SetWordAddress(dev, i, &memaddr_size);

        /* Write data */
        rsp = dev->port.bus->wmem(dev->info.i2caddr.byte, i, memaddr_size, data, size);
        data += size;

        /* Self-timed Write cycle */
        AT24CXX_WCYCLEMS;
    }

#endif

    return rsp;
}
/**
 * @brief  AT24Cxx erase memory data
 * @param  {at24cxx_t} *dev : device structure pointer
 * @param  {uint32_t} saddr : start address
 * @param  {uint8_t} fdata  : filling data (0x00 - 0xFF)
 * @param  {uint32_t} size  : erase data size (See the "Byte" column in the 30 line list above)
 * @return {uint8_t}        : 0 --- success
 *                            1 --- error
 * @note   none
 */
uint8_t AT24Cxx_Erase(at24cxx_t *dev, uint32_t saddr, uint8_t fdata, uint32_t size)
{
    uint32_t i = 0;
    uint32_t RemainSize = size;
    uint32_t EndAddr = saddr + size;
    uint16_t CurPageSize = 0;
    uint8_t memaddr_size = 1;
    uint8_t rsp = 0;

#if AT24Cxx_I2C_MODE == 0

    uint16_t j = 0;

    for (i = saddr; i < EndAddr; i += size)
    {
        /* Get the remaining size of the current page */
        CurPageSize = dev->info.pagesize - (i % dev->info.pagesize);

        /* Current write size, Update remaining size */
        size = min(RemainSize, CurPageSize);
        RemainSize -= size;

        /* Set data word address */
        AT24Cxx_SetWordAddress(dev, i, &memaddr_size);

        /*--------------------------------------------------*/
        /* IIC start */
        swi2c_strt(dev->port.bus);

        /* IIC send i2c address and at24cxx address */
        if (memaddr_size == 1)
        {
            rsp |= swi2c_waddr(dev->port.bus, dev->info.i2caddr.byte);
            rsp |= swi2c_wbyte(dev->port.bus, LSB_16(i));
        }
        else
        {
            rsp |= swi2c_waddr(dev->port.bus, dev->info.i2caddr.byte);
            rsp |= swi2c_wbyte(dev->port.bus, MSB_16(i));
            rsp |= swi2c_wbyte(dev->port.bus, LSB_16(i));
        }

        /*  IIC send write data to memory */
        for (j = 0; j < size; j++)
        {
            rsp |= swi2c_wbyte(dev->port.bus, fdata);
        }

        /* IIC stop */
        swi2c_stop(dev->port.bus);
        /*--------------------------------------------------*/

        /* Self-timed Write cycle */
        AT24CXX_WCYCLEMS;
    }

#else

    uint16_t erase_size = max(8, AT24Cxx_MAX_ERASE_SIZE);
	uint8_t fbuf[erase_size];
	
    /* Get the maximum erase size allowed */
    erase_size = min(erase_size, dev->info.pagesize);

    /* Format erase buffer area */
    memset(fbuf, fdata, erase_size);

    for (i = saddr; i < EndAddr; i += size)
    {
        /* Get the remaining size of the current page */
        CurPageSize = erase_size - (i % erase_size);

        /* Current write size, Update remaining size */
        size = min(RemainSize, CurPageSize);
        RemainSize -= size;

        /* Set data word address */
        AT24Cxx_SetWordAddress(dev, i, &memaddr_size);

        /* Write data */
        rsp = dev->port.bus->wmem(dev->info.i2caddr.byte, i, memaddr_size, fbuf, size);

        /* Self-timed Write cycle */
        AT24CXX_WCYCLEMS;
    }

#endif

    return rsp;
}
/*------------------------------------------------------*/
/*             AT24Cxx Application Function             */
/*------------------------------------------------------*/
/**
 * @brief  AT24Cxx Write data and read back for verification
 * @param  {at24cxx_t} *dev : device structure pointer
 * @param  {uint16_t} addr  : start address
 * @param  {uint8_t} *data  : write data pointer
 * @param  {uint16_t} size  : write data size (See the "Byte" column in the 30 line list above)
 * @return {uint8_t}        : 0 --- success
 *                            1 --- error
 * @note   none
 */
uint8_t AT24Cxx_Readback_Write(at24cxx_t *dev, uint16_t addr, uint8_t *data, uint16_t size)
{
    uint8_t i, j;
    uint8_t compare_data[AT24Cxx_MAX_COMPARE_SIZE];
    uint8_t remainder;
    uint16_t quotient;
    uint16_t point = 0;

    /* write Data */
    AT24Cxx_Write(dev, addr, data, size);

    /* calulate quotient and remainder of copies */
    quotient = size / AT24Cxx_MAX_COMPARE_SIZE;
    remainder = size % AT24Cxx_MAX_COMPARE_SIZE;

    /* compare quotient of copies */
    for (i = 0; i < quotient; i++)
    {
        point = i * AT24Cxx_MAX_COMPARE_SIZE;
        AT24Cxx_Read(dev, addr + point, compare_data, AT24Cxx_MAX_COMPARE_SIZE);
        for (j = 0; j < AT24Cxx_MAX_COMPARE_SIZE; j++)
        {
            if (data[point + j] != compare_data[j]) return 1;
        }
    }

    /* compare quotient of copies */
    if (quotient > 0)
    {
        point = i * AT24Cxx_MAX_COMPARE_SIZE;
    }
    AT24Cxx_Read(dev, addr + point, compare_data, remainder);
    for (j = 0; j < remainder; j++)
    {
        if (data[point + j] != compare_data[j]) return 1;
    }

    return 0;
}











