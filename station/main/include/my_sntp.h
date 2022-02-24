#ifndef __MY_SNTP_H__
#define __MY_SNTP_H__

#include "main.h"

extern time_t now;
void sntp_sync_time(struct timeval *tv);
void time_sync_notification_cb(struct timeval *tv);

void get_nettime_init(void);
void read_time(void);
#endif