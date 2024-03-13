/******************************************************************************
 * Includes
******************************************************************************/
#include "uart.h"
/******************************************************************************
 * Prototype
******************************************************************************/

/**
 * @brief Find the best OSR for smallest baud rate error
 * 
 * @param[in] baud_rate The baud rate of desired connection
 * @return uint8_t OSR value
 */
static uint8_t findOSR(uint32_t baud_rate);
/******************************************************************************
 * Codes
******************************************************************************/

static uint8_t findOSR(uint32_t baud_rate){
    uint8_t index;
    uint8_t osr;
    uint16_t error;
    uint16_t sbr;
    uint16_t min_error = 5000u;
    uint32_t temp_baud_rate;
    osr = MIN_OSR;
    for(index = MIN_OSR; index <= MAX_OSR; index ++){
        /*calculate sbr with osr*/
        sbr = (uint16_t)(DEFAULT_SYSTEM_CLOCK / ((index + 1) * baud_rate));
        /*calculate baudrate with recent sbr*/
        temp_baud_rate = (uint32_t)(DEFAULT_SYSTEM_CLOCK / ((index + 1) * sbr));
        /*find the best osr with smallest baud rate error*/
        error = abs(temp_baud_rate - baud_rate);
        if(error < min_error){
            min_error = error;
            osr = index;
        }
    }
    return osr;
}

void UART0_Init(uint32_t baud_rate) {
    uint8_t osr;
    uint16_t sbr;
    /*Select clock source for UART0*/
    SIM->SOPT2 = SIM_SOPT2_UART0SRC(MCGFLLCLK);
    /*turn on clock for uart0*/
    SIM->SCGC4 = SIM_SCGC4_UART0_MASK;
    /*Turn on clock for tx pin at port A*/
    SIM->SCGC5 = SIM_SCGC5_PORTA_MASK;
    /* Set the PTA1, PTA2 (RX, TX) pin multiplexer to UART mode*/
    PORTA->PCR[RX0_PIN_NUMBER] = PORT_PCR_MUX(UART_MODE);
    PORTA->PCR[TX0_PIN_NUMBER] = PORT_PCR_MUX(UART_MODE);
    /*Find the best OSR value*/
    osr = findOSR(baud_rate);
    sbr = (uint16_t)((DEFAULT_SYSTEM_CLOCK ) / ((osr + 1) * baud_rate));
    
    /*Set the OSR at c4 register*/
    UART0->C4 = osr;
    // Set the Baud Rate Divisor
    UART0->BDH = UART_BDH_SBR((sbr >> 8) & 0x1F);
    UART0->BDL = UART_BDL_SBR(sbr & 0xFF);
    
    /*Set other UART configurations*/
    /*No parity*/
    UART0->C1 = 0; // No parity 
    /*Enable interupt for RX*/
    UART0->C2 = UART0_C2_RIE(1);
    // Enable UART0
    UART0->C2 |= UART_C2_RE_MASK | UART_C2_TE_MASK;

    NVIC_EnableIRQ(UART0_IRQn);
}

void sendMessage(const uint8_t *message){
    uint8_t index;
    uint8_t len = strlen(message);

    for(index = 0; index < len; index ++){
        transmit(message[index]);
    }
}

void transmit(const uint8_t character){
    /*Wait ultil the buffer is empty*/
    while (!(UART0->S1 & UART_S1_TDRE_MASK)) {}
    UART0->D = character;
}
/******************************************************************************
 * End of file
******************************************************************************/