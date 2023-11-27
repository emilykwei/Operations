#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrfx_saadc.h"

#include "microbit_v2.h"

// Digital outputs
// Breakout pins 13, 14, and 15
// These are GPIO pin numbers that can be used in nrf_gpio_* calls
#define LED_RED   EDGE_P13
#define LED_GREEN EDGE_P14
#define LED_BLUE  EDGE_P15

// Analog inputs
// Breakout pins 1
// These are GPIO pin numbers that can be used in ADC configurations
// AIN1 is breakout pin 1. 
#define ANALOG_TWEEZER_IN  NRF_SAADC_INPUT_AIN1

// ADC channel configurations
// These are ADC channel numbers that can be used in ADC calls
#define ADC_TWEEZER_CHANNEL  0

// Global variables
APP_TIMER_DEF(led_timer);

// Function prototypes
static void gpio_init(void);
static void adc_init(void);
static float adc_sample_blocking(uint8_t channel);
static bool touch_conversion(float volts);
static void led_callback(void* _unused);

static void gpio_init(void) {
  // Initialize output pins
  nrf_gpio_cfg_output(LED_RED);
  nrf_gpio_cfg_output(LED_GREEN);
  nrf_gpio_cfg_output(LED_BLUE);

  // Set LEDs off initially
  nrf_gpio_pin_set(LED_RED);
  nrf_gpio_pin_set(LED_BLUE);
  nrf_gpio_pin_set(LED_GREEN);
}

static void saadc_event_callback(nrfx_saadc_evt_t const* _unused) {
  // don't care about saadc events
  // ignore this function
}

static void adc_init(void) {
  // Initialize the SAADC
  nrfx_saadc_config_t saadc_config = {
    .resolution = NRF_SAADC_RESOLUTION_12BIT,
    .oversample = NRF_SAADC_OVERSAMPLE_DISABLED,
    .interrupt_priority = 4,
    .low_power_mode = false,
  };
  ret_code_t error_code = nrfx_saadc_init(&saadc_config, saadc_event_callback);
  APP_ERROR_CHECK(error_code);

  // Initialize tweezer sensor channel
  nrf_saadc_channel_config_t tweezer_channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(ANALOG_TWEEZER_IN);
  error_code = nrfx_saadc_channel_init(ADC_TWEEZER_CHANNEL, &tweezer_channel_config);
  APP_ERROR_CHECK(error_code);
}

static float adc_sample_blocking(uint8_t channel) {
  // read ADC counts (0-4095)
  // this function blocks until the sample is ready
  int16_t adc_counts = 0;
  ret_code_t error_code = nrfx_saadc_sample_convert(channel, &adc_counts);
  APP_ERROR_CHECK(error_code);

  // convert ADC counts to volts
  // 12-bit ADC with range from 0 to 3.6 Volts
  // TODO
  float volts = (adc_counts*3.6)/(pow(2, 12));

  // return voltage measurement
  return volts;
}

static bool touch_conversion(float volts) {
  return volts >= 3.0;
}

static void led_callback(void* _unused) {
  bool is_touching = touch_conversion(adc_sample_blocking(ADC_TWEEZER_CHANNEL));
  if (is_touching) {
    printf("touching!\n");
    nrf_gpio_pin_clear(LED_RED);
  } else {
    nrf_gpio_pin_set(LED_RED);
  }
}

int main(void) {
  printf("Board started!\n");
  
  // initialize GPIO
  gpio_init();

  // initialize ADC
  adc_init();

  // initialize app timers
  app_timer_init();
  app_timer_create(&led_timer, APP_TIMER_MODE_REPEATED, led_callback);

  // start timer
  // change the rate to whatever you want
  app_timer_start(led_timer, 32768 / 4, led_callback);

  // loop forever
  while (1) {
    // Don't put any code in here.
    nrf_delay_ms(1000);
  }
}

