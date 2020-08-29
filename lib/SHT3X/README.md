# SHT3X

Lib to interface with Sensirion SHT3X temp + hum sensors.

## TODO:

* CRC8 Support
* Add periodic sampling mode

## Usage

```
#include "sht3x.h"

sht3x_t sht3x;

void tempTask(){

    while(true){

        sht3x_reading_t reading = {0};
        sht3x_readOnce(&sht3x, &reading, SHT3X_CS_MEDREP);

        printf("Temp (%fC %fF)   Hum: %f%%\n", reading.temp_c, reading.temp_f, reading.hum);

        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
}

void app_main()
{
    sht3x.address = 0x45;
    sht3x.i2cport = I2C_NUM_0;

    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    int res = sht3x_initialize(&sht3x, i2c_config);
    if(res != SHT3X_OK){
        printf("FAIL\n");
    }

    xTaskCreate(tempTask, "TempTask", 2048, NULL, 1, NULL);
}
```