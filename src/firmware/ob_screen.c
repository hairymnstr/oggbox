#include <libopenstm32/gpio.h>
#include <libopenstm32/timer.h>
#include <libopenstm32/rcc.h>
#include "config.h"
#include "ob_screen.h"

void ob_screen_setup() {

  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_TIM1EN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
  /* === setup GPIO === */
   /* PWM output for backlight brightness */
  gpio_set_mode(SCREEN_BL_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, SCREEN_BL);

   /* PWM output for contrast control */
  gpio_set_mode(SCREEN_CON_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, SCREEN_CON);
  
   /* Reset line */
  gpio_set_mode(SCREEN_RST_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, SCREEN_RST);
  gpio_clear(SCREEN_RST_PORT, SCREEN_RST);    /* put chip in reset to start with */

   /* Chip selects, enable, r/w and d/i */
  gpio_set_mode(SCREEN_CS1_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, SCREEN_CS1);
  gpio_set_mode(SCREEN_CS2_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, SCREEN_CS2);
  gpio_set_mode(SCREEN_E_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, SCREEN_E);
  gpio_set_mode(SCREEN_DI_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, SCREEN_DI);
  gpio_set_mode(SCREEN_RW_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, SCREEN_RW);

   /* leave the actual data bus floating for now, will set dir later */

  /* === Setup PWM peripherals for brightness/contrast === */
   /* global settings for timer 1 */
  TIM1_CR1 = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
  TIM1_ARR = 65535;
  TIM1_PSC = 0;
  TIM1_EGR = TIM_EGR_UG;

   /* timer 1 channel 2 (screen brightness) */
  TIM1_CCMR1 |= TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC2PE;
  TIM1_CCER |= TIM_CCER_CC2E;
  TIM1_CCR2 = 0;

   /* timer 1 channel 3 (screen contrast) */
  TIM1_CCMR2 |= TIM_CCMR2_OC3M_PWM1 | TIM_CCMR2_OC3PE;
  TIM1_CCER |= TIM_CCER_CC3E;
  TIM1_CCR3 = 0;

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

void ob_screen_set_contrast(unsigned short level) {
  TIM1_CCR3 = level;
  return;
}

void ob_screen_instruction(unsigned char code, unsigned char val) {
  int i, t;

  /* instruction mode */
  gpio_clear(SCREEN_DI_PORT, SCREEN_DI);
  /* read mode */
  gpio_set(SCREEN_RW_PORT, SCREEN_RW);
  
  /* wait for screen to be ready */
  do {
    for(i=0;i<1000;i++);
    gpio_set(SCREEN_E_PORT, SCREEN_E);
    for(i=0;i<1000;i++);
    t = SCREEN_DATA_IN & 0x80;     /* mask for the busy flag */
    gpio_clear(SCREEN_E_PORT, SCREEN_E);
  } while(t);

  /* set write mode */
  gpio_clear(SCREEN_RW_PORT, SCREEN_RW);

  /* set the data bus to write */
  gpio_set_mode(SCREEN_DATA_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, 0xFF);

  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | code | val;

  for(i=0;i<1000;i++);
  gpio_set(SCREEN_E_PORT, SCREEN_E);
  for(i=0;i<1000;i++);
  gpio_clear(SCREEN_E_PORT, SCREEN_E);

  /* set the data bus back to input only */
  gpio_set_mode(SCREEN_DATA_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, 0xFF);

  return;
}

void ob_screen_data(unsigned char val) {
  int i, t;

  /* instruction mode */
  gpio_clear(SCREEN_DI_PORT, SCREEN_DI);
  /* read mode */
  gpio_set(SCREEN_RW_PORT, SCREEN_RW);
  
  ob_printf("checking screen status\n");

  /* wait for screen to be ready */
  do {
    for(i=0;i<1000;i++);
    gpio_set(SCREEN_E_PORT, SCREEN_E);
    for(i=0;i<10000000;i++);
    t = SCREEN_DATA_IN & 0x80;     /* mask for the busy flag */
    gpio_clear(SCREEN_E_PORT, SCREEN_E);
    ob_printf("data is %x\n", SCREEN_DATA_IN);
  } while(t);

  ob_printf("okay, writing data\n");

  /* set write mode */
  gpio_clear(SCREEN_RW_PORT, SCREEN_RW);
  /* data mode */
  gpio_set(SCREEN_DI_PORT, SCREEN_DI);

  /* set the data bus to write */
  gpio_set_mode(SCREEN_DATA_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, 0xFF);

  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | val;

  for(i=0;i<1000;i++);
  gpio_set(SCREEN_E_PORT, SCREEN_E);
  for(i=0;i<1000;i++);
  gpio_clear(SCREEN_E_PORT, SCREEN_E);

  /* set the data bus back to input only */
  gpio_set_mode(SCREEN_DATA_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, 0xFF);

  return;
}

void ob_screen_startup() {
  int i;

  gpio_clear(SCREEN_E_PORT, SCREEN_E);
  gpio_set(SCREEN_CS1_PORT, SCREEN_CS1);
  gpio_set(SCREEN_CS2_PORT, SCREEN_CS2);
  gpio_set(SCREEN_DI_PORT, SCREEN_DI);
  gpio_set(SCREEN_RW_PORT, SCREEN_RW);
  gpio_set(SCREEN_RST_PORT, SCREEN_RST);

  for(i=0;i<10000;i++);

  gpio_clear(SCREEN_CS1_PORT, SCREEN_CS1);
  gpio_clear(SCREEN_CS2_PORT, SCREEN_CS2);


  ob_printf("setting screen on\n");
  ob_screen_instruction(SCREEN_ON, 0);
  ob_printf("setting screen col=0\n");
  ob_screen_instruction(SCREEN_COL, 0);
  ob_printf("setting screen row=0\n");
  ob_screen_instruction(SCREEN_ROW, 0);

  ob_printf("clearing screen \n");

  ob_screen_cls();
  return;
}

void ob_screen_cls() {
  return;
}

void ob_screen_test() {

  ob_printf("byte 0\n");
  ob_screen_data(0xAA);
  ob_printf("byte 1\n");
  ob_screen_data(0x55);
  ob_printf("byte 2\n");
  ob_screen_data(0xFF);

  return;
}

