#ifndef _NETWORK_H
#define _NETWORK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

//Network Events
#define STA_NETWORK_INIT        BIT0
#define STA_NETWORK_GOT_IP      BIT1
#define STA_NETWORK_DISCONNECT  BIT2

extern EventGroupHandle_t s_network_event_group;

void wifi_connect(char* ssid, char* password);

#endif