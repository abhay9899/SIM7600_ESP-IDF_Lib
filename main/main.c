/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "esp_intr_alloc.h"
#include "string.h"
#include "sim7600.h"

void gpio_init()
{
    gpio_reset_pin(SIM_POWER_PIN);
    gpio_set_direction(SIM_POWER_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(RTS_PIN_SIM);
    gpio_set_direction(RTS_PIN_SIM, GPIO_MODE_OUTPUT);
    gpio_set_level(RTS_PIN_SIM, 1);
}

void function_init()
{
	gpio_init();
	uart_init();
	sim7600_init();
	sim7600_reset();

	ssl_init();
  	gps_init();
}

void app_main(void)
{
	function_init();

	while(1)
	{
		printf("This is just an Example!");
		vTaskDelay(2000 / portTICK_PERIOD_MS);
    	gps_location();
	}
}
