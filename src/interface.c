#include <libopencm3/stm32/f1/gpio.h>

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "vs1053.h"
#include "screen.h"
#include "power.h"
#include "usb.h"
#include "interface.h"
#include "config.h"

#define INTERFACE_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 2048)
#define INTERFACE_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

extern xQueueHandle player_queue;

static void interface_task(void *parameter __attribute__((unused))) {
  int volume = 16;
  char msg[30];
  struct player_job player_job_to_do;
  
  screen_init();
  
  while(1) {
    // check the buttons
    if(gpio_get(VOL_UP_PORT, VOL_UP_PIN)) {
      if(volume > 0)
        volume--;
      player_job_to_do.type = PLAYER_VOLUME_COMMAND;
      player_job_to_do.data = volume | (volume << 8);
      xQueueSendToBack(player_queue, &player_job_to_do, 0);
    }
    if(gpio_get(VOL_DOWN_PORT, VOL_DOWN_PIN)) {
      if(volume < 0xFD)
        volume++;
      player_job_to_do.type = PLAYER_VOLUME_COMMAND;
      player_job_to_do.data = volume | (volume << 8);
      xQueueSendToBack(player_queue, &player_job_to_do, 0);
    }
    
    // update the screen
    
    frame_clear();
    
    if(usb_get_status() == USB_STATUS_ACTIVE) {
      frame_print_at(20, 100, "data");
    } else if(usb_get_status() == USB_STATUS_CHARGER) {
      frame_print_at(11, 100, "charger");
    }
      
    
    sniprintf(msg, 10, "%d.%03dV", power_latest_battery() / 1000, power_latest_battery() % 1000);
    frame_print_at(14,120,msg);
    
    frame_show();
    
    vTaskDelay(100);
  }
  // interface loop shouldn't exit
}

void start_interface_task() {
  xTaskCreate( interface_task, (const signed char * const)"INTERFACE", INTERFACE_TASK_STACK_SIZE, NULL, INTERFACE_TASK_PRIORITY, NULL);
}
