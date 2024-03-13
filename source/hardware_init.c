/******************************************************************************
 * Includes
******************************************************************************/
#include "hardware_init.h"
/******************************************************************************
 * Codes
******************************************************************************/
void initButton(void){
    /*Set up clock for port C*/
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    /*Set the PTC3 pin multiplexer to GPIO mode*/
    PORTC->PCR[BUTTON_1_PIN_NUM] |= PORT_PCR_MUX(GPIO_MODE);
    /*Set button 1 as input*/
    GPIOC->PDDR &= (~BUTTON_1_PIN);
    /*Enable PULL for PTC3*/
    PORTC->PCR[BUTTON_1_PIN_NUM] |= PORT_PCR_PE(PULL_ENABLE);
    /*Set up PULL for PTC3*/
    PORTC->PCR[BUTTON_1_PIN_NUM] |= PORT_PCR_PS(PULLUP);
}

void initLed(void){
    /* Enable clock for PORTE & PORTD */
    SIM->SCGC5 |= (SIM_SCGC5_PORTE_MASK);

    /* Initialize the RED LED (PTE29) */
    /* Set the PTE29 pin multiplexer to GPIO mode */
    PORTE->PCR[29]= PORT_PCR_MUX(1);
    /* Set the pin's direction to output */
    FPTE->PDDR |= (1<<29);
    /* Set the initial output state to high */
    FPTE->PSOR |= (1<<29);
}
/******************************************************************************
 * End of file
******************************************************************************/