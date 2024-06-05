// #include "gatts_table_creat_demo.h"

#ifndef SIM7600_H_
#define SIM7600_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/timer.h"
#include "esp_timer.h"

#define C 0.017453292519943295
#define BUF_SIZE (1024)

#define UART_2_RX 38 //Put the Rx2 GPIO Pin Number here
#define UART_2_TX 37 //Put the Tx2 GPIO Pin Number here

#define SIM_POWER_PIN 1 //Put SIM7600 Power Pin, if used in your project to control SIM7600 Power
#define RTS_PIN_SIM 36 //Put SIM7600 Reset Pin, if used in your project to reset SIM7600 Module

int day, month, year, second, minute, hour;
float g_balance;

void uart_init();
void sim7600_powerup();
void sim7600_powerdown();
void gps_init();
void get_time();
void sms_task();
void balance_check();
void gps_location();
void calc_distance(float lat2, float lat1, float lon2, float lon1);
void sim7600_init();
void ssl_init();
void sms_init();
void sim7600_reset();
void pNumber_check();
void signal_strength_check();

void http_data_post(char *jsonData);

#endif