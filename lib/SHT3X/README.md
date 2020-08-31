# SHT3X

Lib to interface with Sensirion SHT3X temp + hum sensors.

## Usage


### One shot mode
```
#include "sht3x.h"

sht3x_t sht3x;

void tempTask(){

    while(true){

        sht3x_reading_t reading = {0};
        sht3x_oneshot(&sht3x, &reading, SHT3X_CS_MEDREP);

        if(reading.valid){
            printf("Temp (%fC %fF)   Hum: %f%%\n", reading.temp_c, reading.temp_f, reading.hum);
        }else{
            //Crc check failed
        }


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

### Periodic mode

Configure periodic mode by calling `sht3x_set_periodic`, passing a `sht3x` and a `sht3x_periodic_t`.
Once configured, measurements can be read out using `sht3x_fetch_periodic` in a similar process
as the one shot mode.
