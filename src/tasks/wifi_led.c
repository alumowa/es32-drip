
#include "wifi_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "network.h"

#define LED 2

uint16_t blinkInterval = 1000;

void blinkTask(void * pvParameters)
{
    /* GPIO initialize */
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    int level = 0;

    while(true)
    {
        level = !level;
        gpio_set_level(LED, level);
        vTaskDelay(blinkInterval / portTICK_PERIOD_MS);
    }
}

void ledTask(void * pvParameters){

    while(true){

        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
        * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_network_event_group,
                STA_NETWORK_GOT_IP | STA_NETWORK_DISCONNECT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY);

        if (bits & STA_NETWORK_GOT_IP) {
            blinkInterval = 200;
        } else if(bits & STA_NETWORK_DISCONNECT) {
            blinkInterval = 1000;
        }
    }
}

void task_led_init()
{

    xTaskCreate(ledTask, "LEDTASK", 2048, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(blinkTask, "LEDBLINK", 2048, NULL, tskIDLE_PRIORITY, NULL);
}