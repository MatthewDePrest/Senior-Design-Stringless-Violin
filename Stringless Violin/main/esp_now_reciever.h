#ifndef ESP_NOW_RECIEVER_H
#define ESP_NOW_RECIEVER_H

#include "main.h"

void esp_now_receiver_init(void);
void update_local_imu(void);
ImuPacket* get_remote_imu(void);
ImuPacket* get_local_imu(void);

#endif