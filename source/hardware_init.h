#ifndef _HARDWARE_INIT_H_
#define _HARDWARE_INIT_H_
/******************************************************************************
 * Includes
******************************************************************************/
#include "MKL46Z4.h"
/******************************************************************************
 * Definitions
******************************************************************************/
#define BUTTON_1_PIN_NUM    3u
#define BUTTON_1_PIN        (1 << BUTTON_1_PIN_NUM)
#define RED_LED_PIN_NUM     29u
#define RED_LED_PIN         (1 << RED_LED_PIN_NUM)
#define PULL_ENABLE         1u
#define PULLDOWN            0u
#define PULLUP              1u
#define GPIO_MODE           1u
/******************************************************************************
 * APIs
******************************************************************************/

/**
 * @brief Init the build in button sw1 on port C
 * 
 */
void initButton(void);
/**
 * @brief Init the LED
 * 
 */
void initLed(void);

#endif /*UART protocol setup*/
/******************************************************************************
 * End of file
******************************************************************************/
