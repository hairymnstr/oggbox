#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/adc.h>

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "usb.h"
#include "power.h"
#include "config.h"

#define POWER_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define POWER_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 2048)

struct power_info power_status = {
  0,
};

void power_init() {
  
  // set up clocks for battery reading
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_ADC2EN);
  
  // set up the ADC pin
  gpio_set_mode(BAT_VAL_PORT, GPIO_MODE_INPUT, 
                GPIO_CNF_INPUT_ANALOG, BAT_VAL_PIN);
  
  // set up the voltage divider enable pin
  gpio_clear(BAT_CHK_PORT, BAT_CHK_PIN);
  gpio_set_mode(BAT_CHK_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, BAT_CHK_PIN);
  
  // set up the ADC
  adc_off(ADC2);
  
  adc_disable_scan_mode(ADC2);
  adc_set_single_conversion_mode(ADC2);
  adc_disable_external_trigger_regular(ADC2);
  adc_set_right_aligned(ADC2);
  adc_set_sample_time_on_all_channels(ADC2, ADC_SMPR_SMP_28DOT5CYC);
  
  adc_power_on(ADC2);
  // wait for ADC to start up
  vTaskDelay(2);
  
  adc_reset_calibration(ADC2);
  adc_calibration(ADC2);
}

void power_shutdown() {
  // disable the clocks used for battery reading
  
}

int power_read_battery() {
  uint8_t channel_array[16];
  int adc_reading;
  
  // enable the voltage divider that drives the ADC
  gpio_set(BAT_CHK_PORT, BAT_CHK_PIN);
  
  // read the ADC
  channel_array[0] = 1;
  adc_set_regular_sequence(ADC2, 1, channel_array);
  adc_start_conversion_direct(ADC2);
  while(!(ADC_SR(ADC2) & ADC_SR_EOC))
    __asm__("nop\n");
  
  adc_reading = ADC_DR(ADC2);
  
  // disable the voltage divider that drives the ADC
  gpio_clear(BAT_CHK_PORT, BAT_CHK_PIN);
  
  // return in millivolts
  // fix divide by two (potential divider chain)
  // 3000mV reference
  // 4096 counts over that range
  return ((adc_reading * 2 * 3000) / 4096);
}

static void power_management_task(void *parameters __attribute__((unused))) {
  int battery_voltage;

  power_init();
  usb_init();   // USB interface is mainly for charging so take care of it here for now
  while(1) {
    // just keep updating the power status structure so other tasks can query it
    battery_voltage = power_read_battery();
//     usb_power = power_check_usb();
//     update_charge_current();
    usb_sys_tick_handler();
    
    portENTER_CRITICAL();
    power_status.battery_voltage = battery_voltage;
    portEXIT_CRITICAL();
    
    vTaskDelay(1000);           // only do this once a second
  }
  // main loop should never exit
}

void start_power_management_task() {
  xTaskCreate( power_management_task, (const signed char * const)"POWER", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, NULL);
}

// these functions can be called from other threads
int power_latest_battery() {
  return power_status.battery_voltage;
}

void power_set_charge_full() {
  gpio_clear(CHG_500_PORT, CHG_500_PIN);
  gpio_set_mode(CHG_500_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, CHG_500_PIN);
  gpio_set_mode(CHG_100_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, CHG_100_PIN);
}

void power_set_charge_slow() {
  gpio_set_mode(CHG_500_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, CHG_500_PIN);
  gpio_set_mode(CHG_100_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, CHG_100_PIN);
}

void power_set_charge_none() {
  gpio_set_mode(CHG_500_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, CHG_500_PIN);
  gpio_clear(CHG_100_PORT, CHG_100_PIN);
  gpio_set_mode(CHG_100_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, CHG_100_PIN);
}

int power_get_usb_present() {
  if(gpio_get(USB_P_PORT, USB_P_PIN)) {
    return 1;
  }
  return 0;
}
