#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/cm3/systick.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "block.h"
#include "fat.h"
#include "vs1053.h"
#include "partition.h"
#include "power.h"
#include "interface.h"
#include "config.h"

#define mainFLASH_DELAY 1000
#define LED_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define LED_TASK_STACK_SIZE (2048)

xQueueHandle player_queue;

void hardware_setup() {
  
//   rcc_clock_setup_in_hse_8mhz_out_72mhz();
  rcc_clock_setup_in_hsi_out_48mhz();
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);
  rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_PWREN);

  /* set the AUX_POWER line to on all the time, turning this off causes an error
     condition on the SPI bus to the CODEC.  In RevB this line is used for sensing
     the battery voltage and should be initialised as off */
//   gpio_set(GPIOB, GPIO5);
//   gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
//                 GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
  gpio_set_mode(AUX_POWER_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, AUX_POWER_PIN);
                
  gpio_set_mode(GREEN_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, RED_LED_PIN | GREEN_LED_PIN);

  /* Turn off the JTAG port until next reset */
  AFIO_MAPR = AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF;
}

void queue_setup() {
  player_queue = xQueueCreate(10, sizeof(struct player_job));
}

static void FlashLEDTask( void *pvParameters __attribute__((__unused__))) {
    while(1) {
        /* Simple toggle the LED periodically.  This just provides some timing
            verification. */
        vTaskDelay(1000);
        gpio_toggle(RED_LED_PORT, RED_LED_PIN);
        iprintf("LED task\r\n");
    }
}

int main(void) {
  int r, i, mounted=0;
  uint8_t buffer[512];
  struct partition *part_list;
  hardware_setup();
  /* configure the UART for printf debug console */
  usart_clock_setup();
  usart_setup();
        
  power_aux_on();

  // setup the systick counter for timeouts etc.
//   systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8);
  // 9000000 counts per second @ 72MHz
//   systick_set_reload(9000);
  // reload every millisecond
  
  iprintf("==================================================\r\n");
  r = block_init();
  iprintf("Block init: %d\r\n", r);
  if(r == 0) {
    r = fat_mount(0, block_get_volume_size(), 0);
    iprintf("Mount card root: %d\r\n", r);
    if(r != 0) {
      block_read(0, buffer);
      r = read_partition_table(buffer, block_get_volume_size(), &part_list);
      iprintf("Found %d partitions\r\n", r);
      if(r > 0) {
        for(i=0;i<r;i++) {
          if(fat_mount(part_list[i].start, part_list[i].length, part_list[i].type) == 0) {
            iprintf("Mounted partition %d\r\n", i);
            mounted = 1;
            break;
          }
        }
      }
    } else {
      mounted = 1;
    }
  }
  
  queue_setup();
  
  xTaskCreate( FlashLEDTask, (const signed char * const)"LED", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, NULL);
  
  start_player_task();
  
  start_power_management_task();
  
  start_interface_task();
  
  vTaskStartScheduler();

  // the previous call should never return
  return 0;
}

