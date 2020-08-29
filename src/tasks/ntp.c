#include "ntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sntp.h"
#include "network.h"

static const char* TAG = "NTP";

//TODO: Set event bits for refreshing schedules, etc
void time_sync_notification_cb(struct timeval *tv) {

    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "System time updated: %s", strftime_buf);
}

void ntp_task() {

  while(true){

      /**
       * Update time when wifi reconnects
       *
       * The running lwIp sntp service will automatically update the time
       * periodically as configured by the CONFIG_LWIP_SNTP_UPDATE_DELAY
       * interval. But we also want to sync the time when we are connected.
       * calling sntp_stop() and sntp_init() again will force the update
       * without having to re-init all the settings.
       **/

      EventBits_t bits = xEventGroupWaitBits(s_network_event_group,
              STA_NETWORK_GOT_IP,
              pdTRUE,
              pdFALSE,
              portMAX_DELAY);

      if (bits & STA_NETWORK_GOT_IP) {
        sntp_stop();
        sntp_init();
      }
  }
}

void task_ntp_init() {

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();

    // TODO: Fetch this from NVS
    setenv("TZ", "PST8PDT,M3.2.0,M11.1.0", 1);
    tzset();

    xTaskCreate(ntp_task, "NTPTASK", 2048, NULL, tskIDLE_PRIORITY, NULL);
}

