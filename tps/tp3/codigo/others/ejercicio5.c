/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/*
 * Utilizando el ejemplo del punto 3, escriba un nuevo programa llamado ejercicio4,
 * que agregue una variable que se incremente cada vez que el botón sea presionado,
 * mostrando el valor actual, junto al valor del sensor. (local variables are not
 * stored across a blocking wait. A workaround consists in declaring your variable 
 * as static)
**/

#include "contiki.h"

#include "dev/button-sensor.h"
#include "dev/adxl346.h"
#include "dev/max44009.h"
#include "dev/sht21.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  static uint16_t adxl346_present, max44009_present, sht21_present;
  static int16_t temperature, humidity;
  static int16_t accel;
  static uint32_t contador;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);

  /* Initialize and calibrate the ADXL346 sensor */
  adxl346_present = SENSORS_ACTIVATE(adxl346);
  if(adxl346_present == ADXL346_ERROR)
  {
    printf("ADXL346 sensor is NOT present!\n");
    leds_on(LEDS_YELLOW);
  }
  else
    adxl346.configure(ADXL346_CALIB_OFFSET, 0);


  /* Initialize the MAX44009 sensor */
  max44009_present = SENSORS_ACTIVATE(max44009);
  if(max44009_present == MAX44009_ERROR)
  {
     printf("MAX44009 sensor is NOT present!\n");
     leds_on(LEDS_ORANGE);
  }

  sht21_present = SENSORS_ACTIVATE(sht21);
  if(sht21_present == SHT21_ERROR) {
    printf("SHT21 sensor is NOT present!\n");
    leds_on(LEDS_RED);
  }


  while(1)
  {
    PROCESS_WAIT_EVENT();
    if(ev == sensors_event && data == &button_sensor)
    {
      if(adxl346_present != ADXL346_ERROR)
      {
        accel = adxl346.value(ADXL346_READ_X_mG);
        printf("X Acceleration: %d.%u G\n", accel / 1000, accel % 1000);
        accel = adxl346.value(ADXL346_READ_Y_mG);
        printf("Y Acceleration: %d.%u G\n", accel / 1000, accel % 1000);
        accel = adxl346.value(ADXL346_READ_Z_mG);
        printf("Z Acceleration: %d.%u G\n", accel / 1000, accel % 1000);
      }
      else
      {
        printf("XYZ Acceleration: ERROR\n");
        leds_toggle(LEDS_YELLOW);
      }

      if(max44009_present != MAX44009_ERROR)
        printf("Light: \%u\n", max44009.value(MAX44009_READ_LIGHT));
      else
      {
        printf("Light: ERROR");
        leds_toggle(LEDS_GREEN);
      }

      if(sht21_present != SHT21_ERROR)
      {
        temperature = sht21.value(SHT21_READ_TEMP);
        printf("Temperature: %u.%uC\n", temperature / 100, temperature % 100);
        humidity = sht21.value(SHT21_READ_RHUM);
        printf("Rel. humidity: %u.%u%%\n", humidity / 100, humidity % 100);

      }
      else
      {
        printf("SHT21: ERROR\n");
        leds_toggle(LEDS_RED);
      }

      printf("Contador %lu\n", ++contador);

      leds_toggle(LEDS_ALL);

      /* para el evento de soltar botón */
      PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
    }
  }

  printf("Hello, world\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
