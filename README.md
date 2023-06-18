# Driver_AT24Cxx

### *Usage*

#### Step 1 ：User add I2C port function

```c
/**
 * @brief Hardware i2c bus port (master mode)
 */
typedef struct
{
    uint8_t(*send)(uint16_t devaddr, uint8_t *pdata, uint32_t size);
    uint8_t(*recv)(uint16_t devaddr, uint8_t *pdata, uint32_t size);	
    uint8_t(*wmem)(uint16_t devaddr, uint16_t memaddr, uint8_t memaddrsize, uint8_t *pdata, uint32_t size);
    uint8_t(*rmem)(uint16_t devaddr, uint16_t memaddr, uint8_t memaddrsize, uint8_t *pdata, uint32_t size);
} hw_i2c_t;

/**
 * @brief Software i2c bus port (master mode)
 */
typedef struct
{
    void (*holdtime)(uint8_t mult);
    void (*sda_mode)(IO_MODE mode);
    void (*scl_mode)(IO_MODE mode);
    void (*set_scl)(uint8_t level);
    void (*set_sda)(uint8_t level);
    uint8_t(*get_sda)(void);
} sw_i2c_t;

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
```

#### Step 2 ：Init AT24Cxx Device （Mounted Devices）

```c
#if AT24Cxx_I2C_MODE == 0
    /* software I2C Bus */
    sw_i2c.set_scl = set_SCL;
    sw_i2c.scl_mode = mode_SCL;
    sw_i2c.set_sda = set_SDA;
    sw_i2c.get_sda = get_SDA;
    sw_i2c.sda_mode = mode_SDA;
    sw_i2c.holdtime = bus_delay;
    swi2c_config(&sw_i2c);
    ext_eeprom.port.bus = &sw_i2c;
#else
    /* hardware I2C Bus */
    hw_i2c.wmem = wmem;
    hw_i2c.rmem = rmem;
    hw_i2c.recv = rdata;
    hw_i2c.send = wdata;
    ext_eeprom.port.bus = &hw_i2c;
#endif
/* Init AT24Cxx Device */
AT24Cxx_config(&ext_eeprom, AT24C02, 0x0A, 0x00);
```

#### Step 3 ：Test code

```c
#define TEST_ADDR	0x0000

uint8_t err = 0;
char buf1[50] = "Hello World !!!";
char buf2[50];

err = AT24Cxx_Write(&ext_eeprom, TEST_ADDR, (uint8_t *)buf1, strlen(buf1));

memset(b, 0x00, sizeof(buf2));
err |= AT24Cxx_Read (&ext_eeprom, TEST_ADDR, (uint8_t *)buf2, strlen(buf1));

err |= AT24Cxx_Erase(&ext_eeprom, TEST_ADDR, 0xFF, strlen(buf1));

memset(b, 0x00, sizeof(buf2));
err |= AT24Cxx_Read (&ext_eeprom, TEST_ADDR, (uint8_t *)buf2, strlen(buf1));
```

