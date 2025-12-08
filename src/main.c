#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "config.h"
#include "spi.h"
#include "usart.h"
#include "joystick.h"
#include "adc.h"
#include "pwm.h"
#include "nrf.h"
#include "nrf24l01.h"

int main(){
  // Init
  usart_init();
  switch_init();
  adc_init();
  spi_init();
  nrf_init();
  nrf_config_tx();
  
  int16_t x_left, y_left, x_right, y_right;
  uint8_t sw_left, sw_right, pressed_left, pressed_right;
  char buffer[80];

  // Measured offsets
  int16_t x_left_offset = 23;
  int16_t y_left_offset = -6;
  int16_t x_right_offset = 6;
  int16_t y_right_offset = 7;
  
  DualJoystickPacket joysticks;

  while(1){
    // Read left joystick
    x_left = adc_read(PIN_X_LEFT);
    y_left = adc_read(PIN_Y_LEFT);
    sw_left = PIND & PIN_SW_LEFT;
    pressed_left = (sw_left == 0) ? 1 : 0;
    
    // Read right joystick
    x_right = adc_read(PIN_X_RIGHT);
    y_right = adc_read(PIN_Y_RIGHT);
    sw_right = PIND & PIN_SW_RIGHT;
    pressed_right = (sw_right == 0) ? 1 : 0;

    // Map from -127 to 128
    x_left = map((x_left+x_left_offset), 0, 1023, 128, -127);
    y_left = map((y_left+y_left_offset), 0, 1023, 128, -127);
    x_right = map((x_right+x_right_offset), 0, 1023, 128, -127);
    y_right = map((y_right+y_right_offset), 0, 1023, 128, -127);

    x_left = -x_left;
    y_left = -y_left;
    x_right = -x_right;
    y_right = -y_right;

    // Add values to joysticks
    joysticks.left.x = x_left;
    joysticks.left.y = y_left;
    joysticks.left.button = sw_left;
    joysticks.right.x = x_right;
    joysticks.right.y = y_right;
    joysticks.right.button = sw_right;

    // Transfer data
    nrf_write_payload((uint8_t*)&joysticks, sizeof(joysticks));
    nrf_write_register(NRF_REG_STATUS, 0x70);

    // Output for debugging
    sprintf(buffer, "(%4d, %4d, %4d), (%4d, %4d, %4d)", x_left, y_left, pressed_left, x_right, y_right, pressed_right);
    usart_tx(buffer);  
    usart_tx("\n");
  }
} 