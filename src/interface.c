#include <libopencm3/stm32/f1/gpio.h>

#include <stdio.h>
#include <string.h>

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

// Now playing screen, shows title, artist and progress
void interface_now_playing() {
  char *msg;
  static int artist_dir = -1;
  static int artist_index = 0;
  static int title_dir = -1;
  static int title_index = 0;
  static int album_dir = -1;
  static int album_index = 0;
  char buf[12];
  
  if(player_is_playing()) {
    msg = player_get_artist();
    frame_print_at(artist_index,24,msg);
    if(strlen(msg) > 10) {
      // need to scroll the artist name
      if(artist_dir == 1) {
        if(artist_index == 0) {
          artist_dir = -10;
        } else {
          artist_index += artist_dir;
        }
      } else if(artist_dir == -1) {
        if(artist_index == (((int)strlen(msg) * -6) + 64)) {
          artist_dir = 10;
        } else {
          artist_index += artist_dir;
        }
      } else {
        if(artist_dir > 0) {
          artist_dir --;
        } else {
          artist_dir ++;
        }
      }
    } else {
      artist_index = 0;
    }
    
    msg = player_get_title();
    frame_print_at(title_index,32,msg);
    if(strlen(msg) > 10) {
      // need to scroll the artist name
      if(title_dir == 1) {
        if(title_index == 0) {
          title_dir = -10;
        } else {
          title_index += title_dir;
        }
      } else if(title_dir == -1) {
        if(title_index == (((int)strlen(msg) * -6) + 64)) {
          title_dir = 10;
        } else {
          title_index += title_dir;
        }
      } else {
        if(title_dir > 0) {
          title_dir --;
        } else {
          title_dir ++;
        }
      }
    } else {
      title_index = 0;
    }
    
    msg = player_get_album();
    frame_print_at(album_index,40,msg);
    if(strlen(msg) > 10) {
      // need to scroll the artist name
      if(album_dir == 1) {
        if(album_index == 0) {
          album_dir = -10;
        } else {
          album_index += album_dir;
        }
      } else if(album_dir == -1) {
        if(album_index == (((int)strlen(msg) * -6) + 64)) {
          album_dir = 10;
        } else {
          album_index += album_dir;
        }
      } else {
        if(album_dir > 0) {
          album_dir --;
        } else {
          album_dir ++;
        }
      }
    } else {
      album_index = 0;
    }
    
    sniprintf(buf, sizeof(buf), "%3lu%%", (100 * player_get_position()) / player_get_length());
    frame_print_at(12,56,buf);
  }
}

void interface_main_menu() {
  frame_print_at(5, 16, "Now");
  frame_print_at(10, 24, "Playing");
  frame_print_at(5, 36, "Playlist");
  frame_print_at(5, 52, "Media");
  frame_print_at(5, 68, "Settings");
  
  frame_draw_rect(0,16,64,16,FILL_TYPE_INVERT, FILL_TYPE_INVERT);
}

static void interface_task(void *parameter __attribute__((unused))) {
  int volume = 16;
  char msg[30];
  struct player_job player_job_to_do;
  int mode = 0;
  
  screen_init();
  
  screen_backlight(65535);
  
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
    if(gpio_get(MENU_BTN_PORT, MENU_BTN_PIN)) {
      mode ^= 1;
    }
    
    // update the screen
    
    frame_clear();
    
    if(usb_get_status() == USB_STATUS_ACTIVE) {
      frame_print_at(20, 100, "data");
    } else if(usb_get_status() == USB_STATUS_CHARGER) {
      frame_print_at(11, 100, "charger");
    }
      
    
    sniprintf(msg, 10, "%3d.%ddB", -1 * (volume / 2), ((volume & 1) * 5));
    frame_print_at(8, 112,msg);
    sniprintf(msg, 10, "%d.%03dV", power_latest_battery() / 1000, power_latest_battery() % 1000);
    frame_print_at(14,120,msg);
    
    if(mode) {
      interface_now_playing();
    } else {
      interface_main_menu();
    }
    frame_show();
    
    vTaskDelay(100);
  }
  // interface loop shouldn't exit
}

void start_interface_task() {
  iprintf("Start interface %ld\r\n", xTaskCreate( interface_task, (const signed char * const)"INTERFACE", INTERFACE_TASK_STACK_SIZE, NULL, INTERFACE_TASK_PRIORITY, NULL));
}
