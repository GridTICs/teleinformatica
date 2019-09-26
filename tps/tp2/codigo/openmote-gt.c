#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "net/rime/broadcast.h"
#include "net/packetbuf.h"

#include "dev/adxl346.h"
#include "dev/max44009.h"
#include "dev/sht21.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define BROADCAST_CHANNEL   129
/*---------------------------------------------------------------------------*/
PROCESS(openmote_gt_process, "OpenMote-CC2538 gt process");
AUTOSTART_PROCESSES(&openmote_gt_process);
/*---------------------------------------------------------------------------*/

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  static int16_t counter;
  int16_t x, y, z;
  int16_t light, temperature, humidity;
  uint16_t offset;

  int16_t len = packetbuf_datalen();
  int16_t * buf = (int16_t *)packetbuf_dataptr();

  offset = 0;
  counter = *(int16_t *)&buf[offset];
  offset += sizeof(counter);

  x = *(int16_t *)&buf[offset];
  offset += sizeof(x);

  y = *(int16_t *)&buf[offset];
  offset += sizeof(y);

  z = *(int16_t *)&buf[offset];
  offset += sizeof(z);
  
  temperature =  *(int16_t *)&buf[offset];
  offset += sizeof(temperature);

  humidity = *(int16_t *)&buf[offset];
  offset += sizeof(humidity);

  light = *(int16_t *)&buf[offset];
  offset += sizeof(light);

  if((counter&0x3) == 0) {
    leds_toggle(LEDS_GREEN);
  } else if((counter&0x3) == 1) {
    leds_toggle(LEDS_YELLOW);
  } else if((counter&0x3) == 2) {
    leds_toggle(LEDS_ORANGE);
  } else if((counter&0x3) == 3) {
    leds_toggle(LEDS_RED);
  }

  printf("\n\nReceived %u bytes: counter = %u\n", len, counter);
  printf("Temperature: %u.%uC\n", temperature / 100, temperature  % 100);
  printf("Humidity: %u.%u%%\n", humidity / 100, humidity  % 100);
  printf("Light: %u.%ulux\n", light / 100, light  % 100);
  printf("X Acceleration: %d.%u G\n", x / 1000, x % 1000);
  printf("Y Acceleration: %d.%u G\n", y / 1000, y % 1000);
  printf("Z Acceleration: %d.%u G\n", z / 1000, z % 1000);

}

/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(openmote_gt_process, ev, data)
{
  static struct etimer et;
  static int16_t counter;
  int16_t x, y, z;
  uint16_t adxl346_present, sht21_present, max44009_present;
  int16_t light, temperature, humidity;
  int16_t buf[64];
  uint16_t offset;

  PROCESS_EXITHANDLER(broadcast_close(&bc))

  PROCESS_BEGIN();

  /* Initialize and calibrate the ADXL346 sensor */
  adxl346_present = SENSORS_ACTIVATE(adxl346);
  if(adxl346_present == ADXL346_ERROR) {
    printf("ADXL346 sensor is NOT present!\n");
    leds_on(LEDS_YELLOW);
  } else {
    adxl346.configure(ADXL346_CALIB_OFFSET, 0);
  }

  /* Initialize the MAX44009 sensor */
  max44009_present = SENSORS_ACTIVATE(max44009);
  if(max44009_present == MAX44009_ERROR) {
    printf("MAX44009 sensor is NOT present!\n");
    leds_on(LEDS_ORANGE);
  }

  /* Initialize the SHT21 sensor */
  sht21_present = SENSORS_ACTIVATE(sht21);
  if(sht21_present == SHT21_ERROR) {
    printf("SHT21 sensor is NOT present!\n");
    leds_on(LEDS_RED);
  }

  counter = 0;
  broadcast_open(&bc, BROADCAST_CHANNEL, &bc_rx);

  printf("****************************************\n");

  while(1) {
    etimer_set(&et, CLOCK_SECOND);

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      printf("*");
    }

    if(ev == sensors_event) {
      if(data == &button_sensor) {
        if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) == BUTTON_SENSOR_PRESSED_LEVEL) {
          if((counter&0x3) == 0) {
            leds_toggle(LEDS_GREEN);
          } else if((counter&0x3) == 1) {
            leds_toggle(LEDS_YELLOW);
          } else if((counter&0x3) == 2) {
            leds_toggle(LEDS_ORANGE);
          } else if((counter&0x3) == 3) {
            leds_toggle(LEDS_RED);
          }

          counter++;
          offset = 0;

          memcpy(&buf[offset], (int16_t*)&counter, sizeof(counter));
          offset += sizeof(counter);

          if(adxl346_present != ADXL346_ERROR) {
            x = adxl346.value(ADXL346_READ_X_mG);
            memcpy(&buf[offset], (int16_t*)&x, sizeof(x));
            offset += sizeof(x);
            y = adxl346.value(ADXL346_READ_Y_mG);
            memcpy(&buf[offset], (int16_t*)&y, sizeof(y));
            offset += sizeof(y);
            z = adxl346.value(ADXL346_READ_Z_mG);
            memcpy(&buf[offset], (int16_t*)&z, sizeof(z));
            offset += sizeof(z);
          } else {
            memset(&buf[offset], 0xc, sizeof(x) * 3);
            offset += sizeof(x) * 3;
          }

          if(sht21_present != SHT21_ERROR) {
            temperature = sht21.value(SHT21_READ_TEMP);
            memcpy(&buf[offset], (int16_t*)&temperature, sizeof(temperature));
            offset += sizeof(temperature);
            humidity = sht21.value(SHT21_READ_RHUM);
            memcpy(&buf[offset], (int16_t*)&humidity, sizeof(humidity));
            offset += sizeof(humidity);
          } else {
            memset(&buf[offset], 0xa, sizeof(temperature) + sizeof(humidity));
            offset += sizeof(temperature) + sizeof(humidity);
          }

          if(max44009_present != MAX44009_ERROR) {
            light = max44009.value(MAX44009_READ_LIGHT);
            memcpy(&buf[offset], (int16_t*)&light, sizeof(light));
            offset += sizeof(light);
          } else {
            memset(&buf[offset], 0xb, sizeof(light));
            offset += sizeof(light);
          }

          packetbuf_copyfrom(&buf[0], offset + packetbuf_hdrlen());
          broadcast_send(&bc);
	  
  	  
        }
      }
    }
  }

  PROCESS_END();
}

