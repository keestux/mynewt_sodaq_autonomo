/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hal/hal_gpio.h"
#include <assert.h>
#include <compiler.h>
#include "port.h"

 /* XXX: Notes
 * 4) The code probably does not handle "re-purposing" gpio very well. 
 * "Re-purposing" means changing a gpio from input to output, or calling 
 * gpio_init_in and expecting previously enabled interrupts to be stopped. 
 *  
 */

/* 
 * GPIO pin mapping
 *
 * The samD 21G has 48 pins and 38 GPIO.
 * 
 * They are split among 2 ports A (PA0-PA25) and B
 * 
 * , PB8, PB9, 
 *  PB10, PB11, 
 * PB3 PB2,  PB23, PB22,  
 * 
 * The port A Pins exposed on the CHIP are 
 *  PA0,  PA1,  PA2,  PA3,  PA4,  PA5,  PA6,  PA7,
 *  PA8,  PA9, PA10, PA11, PA12, PA13, PA14, PA15, 
 * PA16, PA17, PA18, PA19, PA20, PA21, PA22, PA23, 
 * PA24, PA25,       PA27, PA28,       PA30, PA31, 
 * 
 * The port B pins exposed the chip are
 *              PB2,  PB3, 
 *  PB8,  PB9, PB10,  B11,
 * 
 *                                     PA22, PA23, 
 *
 * 
 * We keep the mapping where ports A0-A31 are pins 0 - 31
 * and ports B0-B31 are pins 32 - 64;
 */ 

#define GPIO_PORT(pin)           (pin/32)
#define GPIO_PIN(pin)            (pin % 32)

#define GPIO_MAX_PORT   (1)

/* this defines which pins are valid for the ports */
int valid_pins[GPIO_MAX_PORT + 1] =
{
    0xdbffffff,
    0xc0000f0c,
};


int hal_gpio_init_out(int pin, int val)
{
    struct port_config cfg;
    
    int port = GPIO_PORT(pin);
    int port_pin = GPIO_PIN(pin);
    
    if(port > GPIO_MAX_PORT) {
        return -1;
    }
    
    if((port_pin & valid_pins[port]) == 0) {
        return -1;
    }
 
    cfg.direction = PORT_PIN_DIR_OUTPUT_WTH_READBACK;
    cfg.input_pull = SYSTEM_PINMUX_PIN_PULL_NONE;
    cfg.powersave = false;
    
    port_pin_set_config(pin, &cfg);
    
    if(val) {
        hal_gpio_set(pin);
    } else {
        hal_gpio_clear(pin);        
    }
        
    return 0;
}

/**
 * gpio set 
 *  
 * Sets specified pin to 1 (high) 
 * 
 * @param pin 
 */
void hal_gpio_set(int pin)
{
    int port = GPIO_PORT(pin);
    int port_pin = GPIO_PIN(pin);
    
    assert(port <= GPIO_MAX_PORT);
    assert(((1 << port_pin) & valid_pins[port]) != 0);
    
    port_pin_set_output_level(pin, true);            
}

/**
 * gpio clear
 *  
 * Sets specified pin to 0 (low). 
 * 
 * @param pin 
 */
void hal_gpio_clear(int pin)
{
    int port = GPIO_PORT(pin);
    int port_pin = GPIO_PIN(pin);
    
    assert(port <= GPIO_MAX_PORT);
    assert(((1 << port_pin) & valid_pins[port]) != 0);   
    
    port_pin_set_output_level(pin, false);    
}

int hal_gpio_init_in(int pin, gpio_pull_t pull) {
    struct port_config cfg;
    
    int port = GPIO_PORT(pin);
    int port_pin = GPIO_PIN(pin);
    
    if(port > GPIO_MAX_PORT) {
        return -1;
    }
    
    if((port_pin & valid_pins[port]) == 0) {
        return -1;
    }
 
    cfg.direction = PORT_PIN_DIR_INPUT;
    switch(pull) {
        case GPIO_PULL_NONE:
            cfg.input_pull = PORT_PIN_PULL_NONE;
            break;
        case GPIO_PULL_UP:
            cfg.input_pull = GPIO_PULL_UP;
            break;
        case GPIO_PULL_DOWN:
            cfg.input_pull = GPIO_PULL_DOWN;
            break;
        default:
            return -1;
    }

    cfg.powersave = false;
    
    port_pin_set_config(pin, &cfg);
        
    return 0;
}

/**
 * gpio read 
 *  
 * Reads the specified pin. 
 *  
 * @param pin Pin number to read
 * 
 * @return int 0: low, 1: high
 */
int hal_gpio_read(int pin)
{
    int rc;
    int port = GPIO_PORT(pin);
    int port_pin = GPIO_PIN(pin);
    
    assert(port <= GPIO_MAX_PORT);
    assert(((1 << port_pin) & valid_pins[port]) != 0);  
    
    rc = port_pin_get_input_level(pin);
    return rc;
}

/**
 * gpio write 
 *  
 * Write a value (either high or low) to the specified pin.
 * 
 * @param pin Pin to set
 * @param val Value to set pin (0:low 1:high)
 */
void hal_gpio_write(int pin, int val)
{
    if (val) {
        hal_gpio_set(pin);
    } else {
        hal_gpio_clear(pin);
    }
}

/**
 * gpio toggle 
 *  
 * Toggles the specified pin
 * 
 * @param pin Pin number to toggle
 */
int hal_gpio_toggle(int pin)
{
    if (hal_gpio_read(pin)) {
        hal_gpio_clear(pin);
    } else {
        hal_gpio_set(pin);
    }
    return hal_gpio_read(pin);
}