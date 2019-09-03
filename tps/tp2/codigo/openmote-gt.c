#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "net/rime/broadcast.h"

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
  uint16_t  x, y, z;
  float light, temperature, humidity;
  uint8_t offset;

  uint16_t len = packetbuf_datalen();
  uint8_t * buf = (uint8_t *)packetbuf_dataptr();

  offset = 0;
  counter = *(uint32_t *)&buf[offset];
  offset += sizeof(counter);
  temperature = *(float *)&buf[offset];
  offset += sizeof(temperature);
  humidity = *(float *)&buf[offset];
  offset += sizeof(humidity);
  light = *(float *)&buf[offset];
  offset += sizeof(light);
  x = *(uint32_t *)&buf[offset];
  offset += sizeof(x);
  y = *(uint32_t *)&buf[offset];
  offset += sizeof(y);
  z = *(uint32_t *)&buf[offset];
  offset += sizeof(z);

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
  printf("Temperature: %u.%uC\n", (unsigned int)temperature, (unsigned int)(temperature * 100) % 100);
  printf("Humidity: %u.%u%%\n", (unsigned int)humidity, (unsigned int)(humidity * 100) % 100);
  printf("Light: %u.%ulux\n", (unsigned int)light, (unsigned int)(light * 100) % 100);
  printf("X: %u, Y: %u, Z: %u\n\n", x, y, z);
}

/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(openmote_gt_process, ev, data)
{
  static struct etimer et;
  static int16_t counter;
  static uint16_t axis;
  static uint16_t adxl346_present, sht21_present, max44009_present;
  static int16_t light, temperature, humidity;
  static uint8_t buf[64];
  static uint8_t offset;

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

          memcpy(buf+offset, (void*)&counter, sizeof(counter));
          offset += sizeof(counter);

          if(sht21_present != SHT21_ERROR) {
            temperature = sht21.value(SHT21_READ_TEMP);
            memcpy(buf+offset, (void*)&temperature, sizeof(temperature));
            offset += sizeof(temperature);
            humidity = sht21.value(SHT21_READ_RHUM);
            memcpy(buf+offset, (void*)&humidity, sizeof(humidity));
            offset += sizeof(humidity);
          } else {
            memset(buf+offset, 0xa, sizeof(temperature) + sizeof(humidity));
            offset += sizeof(temperature) + sizeof(humidity);
          }

          if(max44009_present != MAX44009_ERROR) {
            light = max44009.value(MAX44009_READ_LIGHT);
            memcpy(buf+offset, (void*)&light, sizeof(light));
            offset += sizeof(light);
          } else {
            memset(buf+offset, 0xb, sizeof(light));
            offset += sizeof(light);
          }

          if(adxl346_present != ADXL346_ERROR) {
            axis = adxl346.value(ADXL346_READ_X_mG);
            memcpy(buf+offset, (void*)&axis, sizeof(axis));
            offset += sizeof(axis);
            axis = adxl346.value(ADXL346_READ_Y_mG);
            memcpy(buf+offset, (void*)&axis, sizeof(axis));
            offset += sizeof(axis);
            axis = adxl346.value(ADXL346_READ_Z_mG);
            memcpy(buf+offset, (void*)&axis, sizeof(axis));
            offset += sizeof(axis);
          } else {
            memset(buf+offset, 0xc, sizeof(axis) * 3);
            offset += sizeof(axis) * 3;
          }

          packetbuf_copyfrom(buf, offset);
          broadcast_send(&bc);
        }
      }
    }
  }

  PROCESS_END();
}


