#include "contiki.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);

  while(1)
  {
    PROCESS_WAIT_EVENT();
    if(ev == sensors_event && data == &button_sensor)
    {           
			printf("Hello, world\n");
      leds_toggle(LEDS_ALL);

      PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
    }
  }

  PROCESS_END();
}
