
#ifndef _WIFI_H
#define _WIFI_H

bool wifi_connect();

void wifi_on_connection(void (*callback)(void));

void wifi_on_disconnection(void (*callback)(void));

char *wifi_get_ip();

#endif