#include <libopenstm32/gpio.h>
#include <libopenstm32/timer.h>
#include <libopenstm32/rcc.h>
#include "config.h"
#include "ob_screen.h"

void ob_screen_setup() {

  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_TIM1EN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO9);

  /* global settings for timer 1 */
  TIM1_CR1 = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
  TIM1_ARR = 65535;
  TIM1_PSC = 0;
  TIM1_EGR = TIM_EGR_UG;

  /* timer 1 channel 1 (screen brightness) */
  TIM1_CCMR1 |= TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC2PE;
  TIM1_CCER |= TIM_CCER_CC2E;
  TIM1_CCR2 = 0;

  TIM1_CR1 |= TIM_CR1_ARPE;
  TIM1_CR1 |= TIM_CR1_CEN;

  TIM1_BDTR |= TIM_BDTR_MOE;

}

void ob_screen_set_bl(unsigned short level) {
/*  if(level > 127) {
    gpio_set(SCREEN_BL_PORT, SCREEN_BL);
  } else {
    gpio_clear(SCREEN_BL_PORT, SCREEN_BL);
  }*/

  TIM1_CCR2 = level;

  return;
}

