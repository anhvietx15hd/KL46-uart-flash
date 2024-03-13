/******************************************************************************
 * Includes
******************************************************************************/
#include "MKL46Z4.h"
#include "SrecParser.h"
#include "uart.h"
#include "Flash.h"
#include "hardware_init.h"
/******************************************************************************
 * Definition
******************************************************************************/
#define QUEUE_SIZE          10u
#define LINE_LENGTH         100u
#define FLASH_SECTOR2_BASE_ADDRESS 0xA000
/******************************************************************************
 * Variable
******************************************************************************/
volatile char receivedChar;
volatile uint8_t dataArray[QUEUE_SIZE][LINE_LENGTH];
volatile uint8_t pushIndex = 0;
uint8_t getIndex = 0;
volatile uint8_t queue_elenments = 0;
volatile uint8_t lineIndex = 0;
uint8_t write_done = 1u;
/******************************************************************************
 * Codes
******************************************************************************/
void UART0_IRQHandler(void) {
    /*Extract the character*/
	receivedChar = UART0->D;
	if(receivedChar != '\0'){
        if(receivedChar == '\n'){
            /*Delete the new line character*/
            dataArray[pushIndex][lineIndex] = '\0';
            queue_elenments ++;
            /*reset the line position to write new line*/
            lineIndex = 0;
            if(pushIndex < QUEUE_SIZE -1){
                pushIndex ++;
            } else {
                pushIndex = 0;
            }
        } else{
            if(receivedChar != '\r'){
                dataArray[pushIndex][lineIndex] = receivedChar;
                lineIndex ++;
            }
        }
	}
}

void bootloader_jump_to_user_app(void)
{
    typedef void (*app_reset_handler)(void);
    app_reset_handler resethandler_address;

    //disbale interuppts
    __disable_irq();
    /*Set the vector table address off set*/
    SCB->VTOR = FLASH_SECTOR2_BASE_ADDRESS; //0xA000

    uint32_t msp_value = *(__IO uint32_t *)FLASH_SECTOR2_BASE_ADDRESS;
    __set_MSP(msp_value);

    resethandler_address = *(__IO uint32_t *) (FLASH_SECTOR2_BASE_ADDRESS + 4);
    /*Reset the address of vector table*/
    resethandler_address();
}
int main(void) {
    UART0_Init(19200);
    initButton();
    initLed();

    if((GPIOC->PDIR & BUTTON_1_PIN) == 0){
        uint8_t message[30] = "";
        write_done = 0;
        /*disable interrupt*/
        __disable_irq();
        /*Erase the flash to ready to new user program*/
        Erase_Multi_Sector(FLASH_SECTOR2_BASE_ADDRESS, 10);
        strcpy(message, "-->Done erase memory\n");
        sendMessage(message);
        /*enable interrupt*/
        __enable_irq();
        FPTE->PCOR |= RED_LED_PIN;
        strcpy(message, "-->Choose file to send\n");
        sendMessage(message);
    }
    while (1) {
        if(write_done){
            bootloader_jump_to_user_app();  
        } else{
            if(queue_elenments > 0){
            write_done = 0;
            ErrorCode error = parseRecordLine(dataArray[getIndex]);
            queue_elenments --;
            if(getIndex < (QUEUE_SIZE - 1)){
                getIndex ++;
            } else {
                getIndex = 0;
            }
            }
        }
    }
}
/******************************************************************************
 * End of file 
******************************************************************************/
