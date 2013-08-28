
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/f1/rtc.h>
#include <libopencm3/cm3/systick.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "oggbox.h"

#define mainFLASH_DELAY 1000
#define LED_TASK_PRIORITY ( tskIDLE_PRIORITY + 0 )
#define LED_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 50)

void hardware_setup() {
  
  rcc_clock_setup_in_hse_8mhz_out_72mhz();
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);

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

}

static void FlashLEDTask( void *pvParameters )
{
    portTickType xLastExecutionTime;

    /* Initialise the xLastExecutionTime variable on task entry. */
    xLastExecutionTime = xTaskGetTickCount();

    while(1) {
        /* Simple toggle the LED periodically.  This just provides some timing
            verification. */
        vTaskDelay(1000);
        gpio_toggle(GREEN_LED_PORT, GREEN_LED_PIN);
    }
}

int main(void) {
  hardware_setup();


  // setup the systick counter for timeouts etc.
  systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8);
  // 6000000 counts per second @ 48MHz
  systick_set_reload(6000);
  // reload every millisecond
  
  xTaskCreate( FlashLEDTask, "LED", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, NULL);

  vTaskStartScheduler();

  // the previous call should never return
  return 0;
}

void vApplicationTickHook( void ) {
  
}
