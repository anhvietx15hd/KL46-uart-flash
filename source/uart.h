#ifndef _UART_H_
#define _UART_H_
/******************************************************************************
 * Includes
******************************************************************************/
#include "MKL46Z4.h"
/******************************************************************************
 * Definitions
******************************************************************************/
#define UART_MODE           2u
#define MCGFLLCLK           1u
#define RX0_PIN_NUMBER      1u
#define TX0_PIN_NUMBER      2u

#define MIN_OSR             4u
#define MAX_OSR             31u
/******************************************************************************
 * APIs
******************************************************************************/

/**
 * @brief Init the UART0 connection for KL46z4 kit
 * 
 * @param[in] baud_rate The baud rate for connection, up to ... bits/s
 */
void UART0_Init(uint32_t baud_rate);
/**
 * @brief Send message on UART connection
 * 
 * @param[in] message The pointer to messsage array
 */
void sendMessage(const uint8_t *message);

/**
 * @brief Tranmit the character on UART0 connection
 * 
 * @param[in] character char (uint8_t) value
 */
void transmit(const uint8_t character);
#endif /*UART protocol setup*/
/******************************************************************************
 * End of file
******************************************************************************/
