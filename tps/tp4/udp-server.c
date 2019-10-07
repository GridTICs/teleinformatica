/*
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

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/resolv.h"

#include "sys/etimer.h"

#include "dev/button-sensor.h"
#include "dev/adxl346.h"
#include "dev/max44009.h"
#include "dev/sht21.h"
#include "dev/leds.h"

#include <string.h>
#include <stdbool.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;

int chosen_task = 8;
int client_detector = 1;

static uint16_t seq_id = 0;

long int SEND_INTERVAL;
long int time_value = 15;
int time_error      = 0;
int time_set        = 0;

PROCESS (udp_server_process, "UDP server process");
AUTOSTART_PROCESSES (&resolv_process,&udp_server_process);

/*----------------------------------------------------------------------------*/
/*-----------------Function converter_char_to_int-----------------------------*/
int
converter_char_to_int (char characterIn)
{
  int integerOut;
  switch (characterIn) {

		case '0':	{
						integerOut = 0;
						break;
					}
		case '1':	{
						integerOut = 1;
						break;
					}
		case '2':	{
						integerOut = 2;
						break;
					}
		case '3':	{
						integerOut = 3;
						break;
					}
		case '4':	{
						integerOut = 4;
						break;
					}
		case '5':	{
						integerOut = 5;
						break;
					}
		case '6':	{
						integerOut = 6;
						break;
					}
		case '7':	{
						integerOut = 7;
						break;
					}
		case '8':	{
						integerOut = 8;
						break;
					}
		case '9':	{
						integerOut = 9;
						break;
					}
		default:	{	integerOut =-1;
					}
  }

  return integerOut;

}

/*----------------------------------------------------------------------------*/
/*--------------------Function tcpip_handler----------------------------------*/
static void
tcpip_handler(void)
{
 static char all[]  = "get /all";
 static char tem[]  = "get /tem";
 static char hum[]  = "get /hum";
 static char acc[]  = "get /acc";
 static char lgh[]  = "get /lgh";
 static char out[]  = "get /out";
 static char time[] = "set /";

 int hundreds;
 int tens;
 int units;
  
 client_detector = 0;
 seq_id = 0;
  
 char buf[MAX_PAYLOAD_LEN];
 
 char appdata[9];

 if (uip_newdata()) {
	((char *) uip_appdata) [uip_datalen()] = '\0';
	appdata[uip_datalen()] = '\0';            
	
	memcpy(appdata, uip_appdata, uip_datalen());
	PRINTF("Server received: %s  from ", appdata);
	PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
	PRINTF("\n");

	if(strcmp(appdata, all) == 0) {
		chosen_task = 1;
	}
	else if (strcmp(appdata, acc) == 0) {
		chosen_task = 2;
	}
	else if (strcmp(appdata, lgh) == 0) {
		chosen_task = 3;
	}
	else if (strcmp(appdata, tem) == 0) {
		chosen_task = 4;
	}
	else if (strcmp(appdata, hum) == 0) {
		chosen_task = 5;
	}
	else if (strcmp(appdata, out) == 0) {
		client_detector = 1;
		chosen_task = 6;

		uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
		server_conn->rport = UIP_UDP_BUF->srcport;

		PRINTF("Responding with message: \n");
		sprintf(buf,"Hello from the server!\nServer received: (%s)\n", appdata);
		PRINTF("%s\n", buf);

		uip_udp_packet_send(server_conn, buf, strlen(buf));
		/* Restore server connection to allow data from any node */
		memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
	}

	else if (uip_datalen() == 8) {
	if (appdata[0] == time[0]) {
	if (appdata[1] == time[1]) {
	if (appdata[2] == time[2]) {
	if (appdata[3] == time[3]) {
	if (appdata[4] == time[4]) {
    	time_set = 1;                                               
		hundreds = converter_char_to_int(appdata[5]);
		if (hundreds != -1) {
			tens = converter_char_to_int(appdata[6]);
			if (tens  != -1) {
				units = converter_char_to_int (appdata[7]) ;
				if (units != -1) {
					time_value  = hundreds * 100 + tens *10 + units;
					if (time_value == 0) {
						time_value = 15;
						time_error = 1;
						PRINTF ("ERROR TIME SET: Value by default ");
						PRINTF ("%ld seconds\n", time_value);
					}

					else {	
						PRINTF ("Set Time: Time Value %ld seconds\n"
						, time_value);
					}
				}

				else {
					time_value = 15;
					time_error = 1;
					PRINTF ("ERROR TIME SET: Value by default %ld seconds\n"
					, time_value);
				}
			}

			else{
				time_value = 15;
				time_error = 1;
				PRINTF ("ERROR TIME SET: Value by default %ld seconds\n", 
				time_value);
			}
		}
		else{
			time_value = 15;
			time_error = 1;
			PRINTF ("ERROR TIME SET: Value by default %ld seconds\n",
			time_value);
		}
	}
	}
	}
	}
	}
	}
	else {chosen_task = 7;}
 }
 uip_ipaddr_copy (&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
 server_conn->rport = UIP_UDP_BUF->srcport;
 PRINTF ("Responding with message: ");
 sprintf (buf, "Hello from the server!\nServer received: (%s)\n", appdata);
 PRINTF ("%s\n", buf);
 uip_udp_packet_send (server_conn, buf, strlen (buf));
}

/*----------------------------------------------------------------------------*/
/*-----------Function timeout_handler-----------------------------------------*/

static char buf[MAX_PAYLOAD_LEN];
static int16_t temperature, humidity, light, accelx, accely, accelz;
static uint16_t adxl346_present, max44009_present, sht21_present;

static void
timeout_handler (void)
{
 if (adxl346_present != ADXL346_ERROR) {
	accelx = adxl346.value (ADXL346_READ_X_mG);
	accely = adxl346.value (ADXL346_READ_Y_mG);
	accelz = adxl346.value (ADXL346_READ_Z_mG);
 }
 else {
	printf ("XYZ Acceleration: ERROR\n");
	leds_toggle (LEDS_YELLOW);
 }

 if (max44009_present != MAX44009_ERROR) {
	light = max44009.value (MAX44009_READ_LIGHT);
 }
 else {
	printf ("Light: ERROR\n");
	leds_toggle (LEDS_GREEN);
 }

 if (sht21_present != SHT21_ERROR) {
	temperature = sht21.value (SHT21_READ_TEMP);
	humidity = sht21.value (SHT21_READ_RHUM);
 }
 else {
	printf ("SHT21: ERROR\n");
	leds_toggle (LEDS_RED);
 }

 switch (chosen_task) {
 
 case 1 : {  
			if (seq_id == 0) {
				sprintf (buf, "%d, X, Y, Z, Light, Temp (°C), humidity (%%)\n", 
				seq_id++);
			}
			else{
				sprintf (buf,
				"%d, %d.%u, %d.%u, %d.%u, %u, %u.%u, %u.%u\n",
				seq_id++,
				accelx / 1000, accelx % 1000,
				accely / 1000, accely % 1000,
				accelz / 1000, accelz % 1000,
				light,
				temperature / 100, temperature % 100,
				humidity / 100, humidity % 100);
			}

			break;
		  }
 case 2 : {
			if (seq_id == 0) {
				sprintf (buf, "%d, X, Y, Z\n", seq_id++);
			}

			else {
				sprintf (buf,
				"%d, %d.%u, %d.%u, %d.%u\n",
				seq_id++,
				accelx / 1000, accelx % 1000,
				accely / 1000, accely % 1000,
				accelz / 1000, accelz % 1000);
			}

			break;
          }
 case 3 : {
			if (seq_id == 0) {
				sprintf (buf, "%d, Light\n", seq_id++);
			}

			else{
				sprintf (buf,
				"%d, %u",
				seq_id++,
				light);
			}

			break;  
		  }
 case 4 : {  
			if (seq_id == 0) {
				sprintf (buf, "%d, Temp (°C)\n", seq_id++);
			}

			else {
				sprintf (buf,
				"%d, %u.%u\n",
				seq_id++,
				temperature / 100, temperature % 100);
			}

			break;  
		  }
 case 5 : {  
			if (seq_id == 0) {
				sprintf (buf, "%d, humidity (%%)\n", seq_id++);
			}

			else {
				sprintf(buf,
				"\n%d, %u.%u",
				seq_id++,
				humidity / 100, humidity % 100);
		    }

			break;  
 		   }
 default : {
			sprintf(buf, "ERROR PROTOCOL: Please read README file\n");
			break;
		   }
 }

 if (time_set == 1) {
 	if (time_error == 1) {
		sprintf(buf,"ERROR TIME SET: Value by default 15 seconds\n");
		time_set   = 0;
		time_error = 0;
		seq_id     = 0;
 	}

 	else {
		sprintf(buf, "Set Time %ld seconds\n", time_value);
		time_set = 0;
		seq_id   = 0;
 	}
 }

 printf("msg: %s", buf);
 #if SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION
 uip_udp_packet_send(server_conn, buf, UIP_APPDATA_SIZE);
 #else
 /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
 uip_udp_packet_send(server_conn, buf, strlen (buf));
 #endif
 /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
}

/*----------------------------------------------------------------------------*/
/*--------------Function print_local_addresses--------------------------------*/

static void
print_local_addresses(void)
{ 
	int i;
	int8_t state;

	PRINTF("Server IPv6 addresses: ");
	for (i = 0; i < UIP_DS6_ADDR_NB; i++) {
		state = uip_ds6_if.addr_list[i].state;

		if (uip_ds6_if.addr_list[i].isused &&
            (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
			PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
			PRINTF("\n");
		}
	}
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
PROCESS_THREAD (udp_server_process, ev, data)
{

  static struct etimer et;
  #if UIP_CONF_ROUTER
  uip_ipaddr_t ipaddr;
  #endif 
  /* UIP_CONF_ROUTER */

  PROCESS_BEGIN ();
  PRINTF("UDP server started\n");

  /* Initialize and calibrate the ADXL346 sensor */
  adxl346_present = SENSORS_ACTIVATE (adxl346);
  if (adxl346_present == ADXL346_ERROR) {
    printf("ADXL346 sensor is NOT present!\n");
    leds_on(LEDS_YELLOW);
  }

  else {
    adxl346.configure(ADXL346_CALIB_OFFSET, 0);
  }

  /* Initialize the MAX44009 sensor */
  max44009_present = SENSORS_ACTIVATE(max44009);
  if (max44009_present == MAX44009_ERROR) {
     printf("MAX44009 sensor is NOT present!\n");
     leds_on(LEDS_ORANGE);
  }

  /* Initialize the SHT21 sensor */
  sht21_present = SENSORS_ACTIVATE (sht21);
  if (sht21_present == SHT21_ERROR) {
    printf("SHT21 sensor is NOT present!\n");
    leds_on(LEDS_RED);
  }

  #if RESOLV_CONF_SUPPORTS_MDNS
  resolv_set_hostname("contiki-udp-server");
  #endif

  #if UIP_CONF_ROUTER
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
  #endif /* UIP_CONF_ROUTER */

  print_local_addresses();

  while (1) {
   	while (client_detector) {
  		server_conn = udp_new (NULL, UIP_HTONS(0), NULL);
  		uip_udp_bind (server_conn, UIP_HTONS(3000));
  		printf("Waiting client...\n");
   		PROCESS_YIELD();

  		if (ev == tcpip_event) {
  			tcpip_handler();
  		}
  	}
    
   	SEND_INTERVAL = time_value * CLOCK_SECOND;
   	etimer_set (&et, SEND_INTERVAL);

   	PROCESS_YIELD();

   	if (etimer_expired(&et)) {
	    timeout_handler();
	    etimer_restart(&et);
   }

   else if (ev == tcpip_event) {
      tcpip_handler();
   }
  }

  PROCESS_END();
} 
/*----------------------------------------------------------------------------*/
