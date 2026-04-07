/*
 * mqtt_client.h
 *
 *  Created on: 31. 3. 2026
 *      Author: Student
 */

#ifndef INC_MQTT_CLIENT_H_
#define INC_MQTT_CLIENT_H_

#include "lwip/netif.h"

void mqtt_app_init(struct netif *netif);
void mqtt_app_start(void);
void mqtt_app_stop(void);
void mqtt_app_process(void);

uint8_t mqtt_app_is_connected(void);
uint8_t mqtt_app_is_subscribed(void);

err_t mqtt_app_publish(const char *topic, const void *payload, u16_t len);

uint8_t mqtt_app_rx_ready(void);
const char *mqtt_app_rx_topic(void);
const uint8_t *mqtt_app_rx_data(void);
u16_t mqtt_app_rx_len(void);
void mqtt_app_rx_clear(void);

#endif /* INC_MQTT_CLIENT_H_ */
