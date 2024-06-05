/*Created by Abhay Dutta, 3rd Jun, 2024 */

/* This file contains useful functions for SIM7600 

*/

#include "sim7600.h"

const int uart_port0 = UART_NUM_0;
const int uart_port2 = UART_NUM_2;

char ATcommand[220];
float latitude;
float longitude;

uint8_t rx_buffer[200] = {0};
uint8_t temp_rx_buffer[200] = {0};
uint8_t ATisOK = 0;
uint8_t count = 0;
uint8_t retryCount = 0;
uint8_t gps_count = 0;
uint8_t downloadOK = 0;
uint8_t battFlag = 0;
uint8_t gps_flag = 0;
uint8_t gps_count1 = 0;
uint8_t gps_count2 = 0;
uint8_t post_failed = 0;
uint8_t rxbuff[256];
uint8_t signalStrength;
uint8_t cchStrength;
uint8_t download_count = 0;
int cchFlag;
float battVoltage;
float initial_latitude;
float initial_longitude;
float final_latitude;
float final_longitude;
float distance;


//xxxxxxxxxxxxxxxxxxxxxxxxxxx Application Data xxxxxxxxxxxxxxxxxxxxxxxxxxx//
//Please fill up According to your application need before using or flashing.
char data_posting_Url[] = "xxxxxxxxxxxxx put your end-point URL here xxxxxxxxxxxxx";
char apiKey[] = "xxxxxxxxxxxxx put your end-point Api Key here xxxxxxxxxxxxx";  
char serverURL[] = "xxxxxxxxxxxxx put your Server Root URL here xxxxxxxxxxxxx"; //eg : "www.thingspeak.com"

char pNumberCode[] = "xxxxx"; //eg: "*901#" , use the USSD code that your cellular network uses to check phone number!
char balanceCode[] = "xxxxx"; //eg: "*903#" , use the USSD code that your cellular network uses to check cellular Balance!

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

void uart_read(void *buf, uint32_t length, TickType_t ticks_to_wait);

//=======================SIM7600 Power Up sequence=========================//
void sim7600_powerup()
{
  uart_write_bytes(uart_port0, "POWERING UP\n", strlen("POWERING UP\n"));
  gpio_set_level(SIM_POWER_PIN, 1);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  gpio_set_level(SIM_POWER_PIN, 0);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  uart_write_bytes(uart_port0, "LEAVING POWERING UP\n", strlen("LEAVING POWERING UP\n"));
}


//=======================SIM7600 Power Down Sequence=========================//
void sim7600_powerdown()
{
  ATisOK = 0;
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  uart_write_bytes(uart_port0, "POWERING DOWN\n", strlen("POWERING DOWN\n"));
  sprintf(ATcommand, "AT+CPOF\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  while (!ATisOK)
  {
    // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
    if (strstr((char *)rx_buffer, "OK"))
    {
      ATisOK = 1;
      uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
    }
    vTaskDelay(1);
  }
  ATisOK = 0;
  // uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, "LEAVING POWERING DOWN\n", strlen("LEAVING POWERING DOWN\n"));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
}

//=======================Main function to initialize SIM7600=========================//
//This function wakes up SIM module if its not powered ON
//Call this fuction for SIM7600 before anything else
//make sure to assign all the information and data to be used like URL, Api Key, Mobile Number for SMS, USSD Codes, Tx, Rx Pins in .C and .H file!
void sim7600_init() 
{
  uart_write_bytes(uart_port0, "VERIFYING SIM MODULE STATUS\n", strlen("VERIFYING SIM MODULE STATUS\n"));
  while (!ATisOK)
  {
    sprintf(ATcommand, "AT\r\n");
    uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
    // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
    uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
    if (strstr((char *)rx_buffer, "OK"))
    {
      ATisOK = 1;
      count = 0;
      uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
    }
    else
    {
      uart_write_bytes(uart_port0, "COUNTING\n", strlen("COUNTING\n"));
      count++;
      if (count == 7 || count == 30)
      {
        uart_write_bytes(uart_port0, "POWERING UP\n", strlen("POWERING UP\n"));
        sim7600_powerup();
      }
    }
    vTaskDelay(1);
  }
  uart_write_bytes(0, "VERIFIED\n", strlen("VERIFIED\n"));
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  ATisOK = 0;
  count = 0;

  sprintf(ATcommand, "AT+CTZU=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  vTaskDelay(50/ portTICK_PERIOD_MS);
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  sprintf(ATcommand, "AT+CTZR=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  vTaskDelay(50 / portTICK_PERIOD_MS);
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  // delete all sms from sim module storage //
  sprintf(ATcommand, "AT+CMGD=1,4\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  vTaskDelay(50 / portTICK_PERIOD_MS);
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
}


//=======================Checks the LTE Signal Strength and Use the information accordingly in your application=========================//
void signal_strength_check()
{
  sprintf(ATcommand, "AT+CSQ\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 2000 / portTICK_PERIOD_MS);
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  if (strstr((char *)rx_buffer, "+CSQ"))
  {
    if (strstr((char *)rx_buffer, "ERROR"))
    {
      printf("ERROR in reading signal strength for now\n");
    }

    else if (strstr((char *)rx_buffer, "CLOSED") || strstr((char *)rx_buffer, "CCHOPEN") || strstr((char *)rx_buffer, "CMGS"))
    {
      printf("CCH PEER CLOSED CASE!\n");
      char *data1[5] = {0};
      int i1 = 0;
      char delim1[] = " ";
      char *ptr1 = strtok((char *)rx_buffer, delim1);
      while (ptr1 != NULL)
      {
        data1[i1] = ptr1;
        ptr1 = strtok(NULL, delim1);
        i1++;
      }

      char *data2[5] = {0};
      int i2 = 0;
      char delim2[] = ",";
      char *ptr2 = strtok((char *)data1[2], delim2);
      while (ptr2 != NULL)
      {
        data2[i2] = ptr2;
        ptr2 = strtok(NULL, delim2);
        i2++;
      }
      printf("RSSI Signal Strength:\n");
      uart_write_bytes(uart_port0, (uint8_t *)data2[0], strlen((char *)data2[0]));
      uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
      signalStrength = atoi(data2[0]);
    }

    else
    {
      char *data1[5] = {0};
      int i1 = 0;
      char delim1[] = " ";
      char *ptr1 = strtok((char *)rx_buffer, delim1);
      while (ptr1 != NULL)
      {
        data1[i1] = ptr1;
        ptr1 = strtok(NULL, delim1);
        i1++;
      }

      char *data2[5] = {0};
      int i2 = 0;
      char delim2[] = ",";
      char *ptr2 = strtok((char *)data1[1], delim2);
      while (ptr2 != NULL)
      {
        data2[i2] = ptr2;
        ptr2 = strtok(NULL, delim2);
        i2++;
      }
      printf("RSSI Signal Strength:\n");
      uart_write_bytes(uart_port0, (uint8_t *)data2[0], strlen((char *)data2[0]));
      uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
      signalStrength = atoi(data2[0]);
    }
  }
  else
  {
    signalStrength = 0;
  }
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
}


//=======================Function to Send SMS to a number=========================//
//put the mobile number to which you want to text and do include country code using +! in the first argument
//put your message in the sencond argument of the API
//eg: sms_task("+0165483983782", "Hello there! This sms is from SIM7600 Module!");
void sms_task(char *mobileNumber, char *message)
{
  sprintf(ATcommand, "AT+CMGF=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  sprintf(ATcommand, "AT+CMGS=\"%s\"\r\n", mobileNumber);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  sprintf(ATcommand, "%s%c", message, 0x1a);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  uart_write_bytes(uart_port0, "SMS SENT\n", strlen("SMS SENT\n"));
}


//=======================calculates distance between two coordinates=========================//
void calc_distance(float lat2, float lat1, float lon2, float lon1) // (initial latitude, destination latitude, initial longitude, destination longitude)
{
  // Haversine formula
  float a = (sin((lat2 - lat1) * C) / 2) * (sin((lat2 - lat1) * C) / 2) + cos(lat1 * C) * cos(lat2 * C) * ((sin((lon2 - lon1) * C)) / 2) * ((sin((lon2 - lon1) * C)) / 2);
  distance = (2 * 6371 * atan2(sqrt(a), sqrt(1 - a)));
  printf("Calculated distance: %f Km\n", distance);
}


//=======================Function for Retriving GPS Location=========================//
void gps_location()
{
  printf("inside GPS Location check!\n");
  int flag = 0;
  sprintf(ATcommand, "AT+CGPSINFO\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  memset(ATcommand, 0, sizeof(ATcommand));
  if (strstr((char *)rx_buffer, "SMS") || strstr((char *)rx_buffer, "CCH_PEER_CLOSED"))
  {
    flag = 1;
  }
  if (strlen((char *)rx_buffer) > 80 && flag != 1) // only parse coordinates if the gps has received the coordinates and use accordingly
  {
    printf("GPS Signal Available!\n");
    gps_count = 0;
    char *data1[30] = {0};
    int i1 = 0;
    char delim1[] = " ";
    char *ptr1 = strtok((char *)rx_buffer, delim1);
    while (ptr1 != NULL)
    {
      data1[i1] = ptr1;
      ptr1 = strtok(NULL, delim1);
      i1++;
    }
    uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));

    char *data2[30] = {0};
    int i2 = 0;
    char delim2[] = ",";
    char *ptr2 = strtok((char *)data1[1], delim2);
    while (ptr2 != NULL)
    {
      data2[i2] = ptr2;
      ptr2 = strtok(NULL, delim2);
      i2++;
    }

    float lat1;
    lat1 = atof(data2[0]) / 100;

    float lon1;
    lon1 = atof(data2[2]) / 100;

    printf("converted lat1: %f,  long1: %f\n", lat1, lon1);
    uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));

    latitude = (int)lat1 + (((lat1 - (int)lat1) * 100) / 60); // final latitude and longitude are saved in Latitude and Longitude global float variables
    longitude = (int)lon1 + (((lon1 - (int)lon1) * 100) / 60);

    printf("Latitide: %f,  longitude: %f\n", latitude, longitude);
    uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  }
  else
  {
    gps_count++;
    if (gps_count == 120) // value of count to re-initialize gps after a while if no gps signal
    {
      gps_init();
      gps_count = 0;
    }
    printf("No GPS Signal Yet!\n");
  }
  memset(rx_buffer, 0, sizeof(rx_buffer));
  vTaskDelay(1);
}


//=======================SMS Initialization Function=========================//
//Call before using sms_task() function for sending SMS
void sms_init() 
{
  sprintf(ATcommand, "AT+CSMP=17,167,0,0\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);
  sprintf(ATcommand, "AT+CMGF=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  uart_write_bytes(uart_port0, "SMS INITIALIZED\n", strlen("SMS INITIALIZED\n"));
}


//=======================GPS initialization function =========================//
//Call this before using gps_location() or gps_location_post() funtion 
void gps_init() 
{
  sprintf(ATcommand, "AT+CGPS=0\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CGPS=1,1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);
  uart_write_bytes(uart_port0, "GPS INITIALIZED\n", strlen("GPS INITIALIZED\n"));
}


//=======================Function for Retriving Real Time and Date for your time zone!=========================//
//Use the time and date for any application you like!
void get_time() 
{
  retry:
  uart_write_bytes(uart_port0, "INSIDE GET TIME\n", strlen("INSIDE GET TIME\n"));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);
  sprintf(ATcommand, "AT+CCLK?\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  if(strstr((char *)rx_buffer, "+CCHOPEN"))
  {
    goto retry;
    memset(rx_buffer, 0, sizeof(rx_buffer));
  }
  char *_data1[5] = {0};
  int _i1 = 0;
  char _delim1[] = "\"";
  char *_ptr1 = strtok((char *)rx_buffer, _delim1);
  while (_ptr1 != NULL)
  {
    _data1[_i1] = _ptr1;
    _ptr1 = strtok(NULL, _delim1);
    _i1++;
  }

  uart_write_bytes(uart_port0, (uint8_t *)_data1[1], strlen((char *)_data1[1]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));

  uart_write_bytes(uart_port0, "PARSING DATE\n", strlen("PARSING DATE\n"));
  char *_data2[5] = {0};
  int _i2 = 0;
  char _delim2[] = ",";
  char *_ptr2 = strtok((char *)_data1[1], _delim2);
  while (_ptr2 != NULL)
  {
    _data2[_i2] = _ptr2;
    _ptr2 = strtok(NULL, _delim2);
    _i2++;
  }
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  uart_write_bytes(uart_port0, (uint8_t *)_data2[0], strlen((char *)_data2[0]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  uart_write_bytes(uart_port0, (uint8_t *)_data2[1], strlen((char *)_data2[1]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));

  char *_data3[5] = {0};
  int _i3 = 0;
  char _delim3[] = "/";
  char *_ptr3 = strtok((char *)_data2[0], _delim3);
  while (_ptr3 != NULL)
  {
    _data3[_i3] = _ptr3;
    _ptr3 = strtok(NULL, _delim3);
    _i3++;
  }
  printf("Date: \n");
  uart_write_bytes(uart_port0, (uint8_t *)_data3[0], strlen((char *)_data3[0]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  uart_write_bytes(uart_port0, (uint8_t *)_data3[1], strlen((char *)_data3[1]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  uart_write_bytes(uart_port0, (uint8_t *)_data3[2], strlen((char *)_data3[2]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));

  uart_write_bytes(uart_port0, "PARSING TIME\n", strlen("PARSING TIME\n"));
  char *_data4[5] = {0};
  int _i4 = 0;
  char _delim4[] = ":";
  char *_ptr4 = strtok((char *)_data2[1], _delim4);
  while (_ptr4 != NULL)
  {
    _data4[_i4] = _ptr4;
    _ptr4 = strtok(NULL, _delim4);
    _i4++;
  }
  printf("Time: \n");
  uart_write_bytes(uart_port0, (uint8_t *)_data4[0], strlen((char *)_data4[0]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  uart_write_bytes(uart_port0, (uint8_t *)_data4[1], strlen((char *)_data4[1]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
  uart_write_bytes(uart_port0, (uint8_t *)_data4[2], strlen((char *)_data4[1]));
  uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));

  hour = atoi(_data4[0]); // hour
  minute = atoi(_data4[1]); // minute
  second = atoi(_data4[2]); // second
  year = atoi(strcat("20", _data3[0])); // year
  month = atoi(_data3[1]); // month
  day = atoi(_data3[2]); // day

  memset(ATcommand, 0, sizeof(ATcommand));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  vTaskDelay(10 / portTICK_PERIOD_MS);
}


//=======================Function for Checking Phone Number, please make sure to assign USSD code in pNumberCode before using=========================//
void pNumber_check()
{
  sprintf(ATcommand, "AT+CREG?\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+COPS?\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CUSD=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CUSD=1,\"%s\"\r\n", pNumberCode);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 500 / portTICK_PERIOD_MS); //
  memset(rx_buffer, 0, sizeof(rx_buffer));
  uart_read(rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
}


//=======================Function for checking Callular Balance, please make sure to assign USSD code in balanceCode before using=========================//
void balance_check()
{
  sprintf(ATcommand, "AT+CREG?\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+COPS?\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CUSD=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CUSD=1,\"%s\"\r\n", balanceCode);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 500 / portTICK_PERIOD_MS); 
  memset(rx_buffer, 0, sizeof(rx_buffer));
  uart_read(rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS); 

  if (strstr((char *)rx_buffer, "ERROR") || strstr((char *)rx_buffer, "unavailable"))
  {
    printf("NETWORK PROBLEM, TRY LATER\n");
  }

  else
  {
    char *_data1[5] = {0};
    int _i1 = 0;
    char _delim1[] = ".";
    char *_ptr1 = strtok((char *)rx_buffer, _delim1);
    while (_ptr1 != NULL)
    {
      _data1[_i1] = _ptr1;
      _ptr1 = strtok(NULL, _delim1);
      _i1++;
    }
    char balance[5];
    sprintf(balance, "%s.%s", _data1[1], _data1[2]);
    g_balance = atof(balance);
    printf("Balance: %0.2f", g_balance);
    printf("\n");
  }
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
}


//=======================Activating LTE service and then initializing ssl configurations=========================//
//call only after initializing SIM7600 using sim7600_init()
void ssl_init() 
{
  sprintf(ATcommand, "AT+CNMP=2\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CSSLCFG=\"sslversion\",0,4\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CSSLCFG=\"authmode\",0,0\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CCHSET=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CCHSTART\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS);
  // uart_read(rx_buffer, BUF_SIZE, 500 / portTICK_PERIOD_MS); //
  // memset(rx_buffer, 0, sizeof(rx_buffer));
  // uart_read(rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  if (strstr((char *)rx_buffer, "+CCHSTART"))
  {
    printf("inside cch parse task !\n");
    char *data1[5] = {0};
    int i1 = 0;
    char delim1[] = " ";
    char *ptr1 = strtok((char *)rx_buffer, delim1);
    while (ptr1 != NULL)
    {
      data1[i1] = ptr1;
      ptr1 = strtok(NULL, delim1);
      i1++;
    }
    printf("CCHSTART VALUE: ");
    uart_write_bytes(uart_port0, (uint8_t *)data1[1], strlen((char *)data1[1]));
    uart_write_bytes(uart_port0, (uint8_t *)"\n", strlen("\n"));
    cchStrength = atoi(data1[1]);

    if (cchStrength == 0)
    {
      cchFlag = 0;
    }
    else
    {
      cchFlag = 1;
    }
  }
  else
  {
    printf("CCH VALUE ERROR!\n");
    cchFlag = 1;
  }
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(50 / portTICK_PERIOD_MS);

  sprintf(ATcommand, "AT+CCHOPEN=0,\"%s\",443,2\r\n", serverURL);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(5000 / portTICK_PERIOD_MS);
}


//=======================function to reset the SIM7600 module in case of power and settings recycle=========================//
//it also initializes the LTE 
void sim7600_reset()
{
  ATisOK = 0;
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  sprintf(ATcommand, "AT+CRESET\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  while (!ATisOK)
  {
    sprintf(ATcommand, "AT\r\n");
    uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
    // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
    uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
    if (strstr((char *)rx_buffer, "OK"))
    {
      ATisOK = 1;
      count = 0;
      uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
    }
  }
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+CMGD=1,4\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS);
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  vTaskDelay(50 / portTICK_PERIOD_MS);
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  ssl_init();
}


//=======================Function for Posting data , please assign data_posting_Url and apiKey before using this =========================//
//JSON data should be in JSON Format eg. {\"data1\":value1,\"data2\":value2} if your Backend api requires JSON format data posting!
void http_data_post(char *jsonData) 
{
  retry:

  downloadOK = 0;
  sprintf(ATcommand, "AT+HTTPTERM\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+HTTPINIT\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+HTTPPARA=\"URL\",\"%s\"\r\n", data_posting_Url);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+HTTPPARA=\"USERDATA\",\"key:%s\"\r\n", apiKey);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 3000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "%s%c", jsonData, 0x0A);
  uint8_t len = strlen(ATcommand);
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+HTTPDATA=%d,10000\r\n", len);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "%s%c", jsonData, 0x0A);
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, "INSIDE WHILE\n", strlen("INSIDE WHILE\n"));
  download_count = 0;
  while (!downloadOK)
  {
    download_count++;
    uart_read(rx_buffer, BUF_SIZE, 500 / portTICK_PERIOD_MS); //
    uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
    if (strstr((char *)rx_buffer, "OK") || download_count >= 200)
    {
      downloadOK = 1;
      download_count = 0;
    }
  }
  download_count = 0;
  uart_write_bytes(uart_port0, "OUTSIDE WHILE\n", strlen("OUTSIDE WHILE\n"));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+HTTPACTION=1\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 500 / portTICK_PERIOD_MS); //
  memset(rx_buffer, 0, sizeof(rx_buffer));
  uart_read(rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));

  //Successful Data post response = 200
  //No balance or Connection = 302
  //Bad request or wrong data format = 400
  if (strstr((char *)rx_buffer, "200")) 
  {
    uart_write_bytes(uart_port0, "SUCCESSFULLY POSTED\n", strlen("SUCCESSFULLY POSTED\n"));

    memset(rx_buffer, 0, sizeof(rx_buffer));
    memset(ATcommand, 0, sizeof(ATcommand));
    sprintf(ATcommand, "AT+HTTPREAD=0,200\r\n");
    uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
    uart_read(rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS); //
    uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
    uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
    // memset(rx_buffer, 0, sizeof(rx_buffer));
    memset(ATcommand, 0, sizeof(ATcommand));

    // checking the backend response
    sprintf((char *)temp_rx_buffer, "%s", rx_buffer);
  }

  //retry sequence in case of no network or post failed condition just in case
  else
  {
    if (retryCount < 1)
    {
      retryCount++;
      uart_write_bytes(uart_port0, "POSTING FAILED, RETRY\n", strlen("POSTING FAILED, RETRY\n"));
      goto retry;
    }
    else
    {
      uart_write_bytes(uart_port0, "RETRY FAILED, RESUMING TASK\n", strlen("RETRY FAILED, RESUMING TASK\n"));
      retryCount = 0;
    }
    memset(rx_buffer, 0, sizeof(rx_buffer));
    memset(ATcommand, 0, sizeof(ATcommand));
    sprintf(ATcommand, "AT+HTTPREAD=0,200\r\n");
    uart_write_bytes(uart_port2, ATcommand, strlen((char*)ATcommand));
    uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS);
    uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
    uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));
    memset(ATcommand, 0, sizeof(ATcommand));
  }

  // sprintf(ATcommand, "AT+HTTPREAD=0,200\r\n");
  // uart_write_bytes(uart_port2, ATcommand, strlen((char*)ATcommand));
  // uart_read_bytes(uart_port2, rx_buffer, BUF_SIZE, 5000 / portTICK_PERIOD_MS);
  // uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  // uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  // memset(rx_buffer, 0, sizeof(rx_buffer));
  // memset(ATcommand, 0, sizeof(ATcommand));

  sprintf(ATcommand, "AT+HTTPTERM\r\n");
  uart_write_bytes(uart_port2, ATcommand, strlen((char *)ATcommand));
  uart_read(rx_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS); //
  uart_write_bytes(uart_port0, ATcommand, strlen((char *)ATcommand));
  uart_write_bytes(uart_port0, rx_buffer, strlen((char *)rx_buffer));
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(ATcommand, 0, sizeof(ATcommand));
  retryCount = 0;

  vTaskDelay(1);
}

//=======================coustom uart read api for better timeout solution=========================//
void uart_read(void *buf, uint32_t length, TickType_t ticks_to_wait)
{
    uint8_t whileFlag = 0;
    uint8_t len = 0;
    portTickType ticks_start = xTaskGetTickCount();
    while (!whileFlag)
    {
        len = uart_read_bytes(uart_port2, buf, length, 200 / portTICK_PERIOD_MS); // edit uart_port in this line for required uart port
        if (len > 0)
        {
            whileFlag = 1;
            ticks_start = 0;
        }
        TickType_t ticks_end = xTaskGetTickCount();
        if (ticks_end - ticks_start >= ticks_to_wait)
        {
            whileFlag = 1;
            ticks_start = 0;
            ticks_end = 0;
        }
    }
    uart_flush(uart_port2);
}


//======================function to initialize UART for SIM7600==========================//
//call before calling sim7600_init()
void uart_init() 
{
    uart_config_t uart_config0 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_config_t uart_config2 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(uart_port0, &uart_config0);
    uart_param_config(uart_port2, &uart_config2);

    uart_set_pin(uart_port0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_pin(uart_port2, UART_2_TX, UART_2_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(uart_port0, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_driver_install(uart_port2, BUF_SIZE * 2, 0, 0, NULL, 0);
}
