/******************************************************************************
 * Includes
******************************************************************************/
#include <string.h>
#include "MKL46Z4.h"
#include "SrecParser.h"
#include "uart.h"
#include "Flash.h"
/******************************************************************************
 * Definitions
******************************************************************************/
#define _16BIT_ADDRESS_LENGTH    4u
#define _24BIT_ADDRESS_LENGTH    6u
#define _32BIT_ADDRESS_LENGTH    8u
#define _16BIT_LINE_COUNT_LENGTH 4u
#define _24BIT_LINE_COUNT_LENGTH 6u
#define TYPE_LENGTH              2u
#define TYPE_POSITION            1u
#define S_POSITION               0u
#define BYTE_COUNT_LENGTH        2u
#define CHECKSUM_LENGTH          2u
#define LINE_TYPE_LENGTH         2u
#define BYTE_LENGTH              2u /*2 character per byte*/
#define TEMP_STRING_LENGTH       7u
#define HEXA_SHIFT               4u /* x*16 == x<<4*/

#define RED_LED_PIN_NUM     29u
#define RED_LED_PIN         (1 << RED_LED_PIN_NUM)
/*Convert a character to hexa number*/
#define ASCII_TO_HEX(character) (character > '9' ? \
        ((character & 0xdf) - 'A' + 10) : (character - '0'))
/******************************************************************************
 * Variable
******************************************************************************/
/*Update line Count after each record line*/
static uint16_t lineCount = 0;
/******************************************************************************
 * Prototype
******************************************************************************/
typedef enum {
    HEADER                    = 0u,
    _16_BIT_DATA              = 1u,
    _24_BIT_DATA              = 2u,
    _32_BIT_DATA              = 3u,
    _16_BIT_COUNT             = 5u,
    _24_BIT_COUNT             = 6u,
    _16_BIT_START_ADDRESS     = 9u,
    _24_BIT_START_ADDRESS     = 8u,
    _32_BIT_START_ADDRESS     = 7u,
} RecordType;

/**
 * @brief check record type using first character
 * 
 * @param character The risrt character of the record line
 * @return ErrorCode 
 */
static ErrorCode checkS(const uint8_t character);

/**
 * @brief check byte count of line
 * 
 * @param[in] byteCount Byte count value
 * @param[in] line The record line
 * @return ErrorCode 
 */
static ErrorCode checkByteCount(const uint8_t byteCount,
                                const uint8_t* line);

/**
 * @brief check if check sum value is valid
 * 
 * @param[in] checksum The original checksum value in record line
 * @param[in] byteCount Byte count value
 * @param[in] address pointer to address array
 * @param[in] data pointer to data array
 * @return ErrorCode 
 */
static ErrorCode checkSum(const uint8_t checksum,
                          const uint8_t byteCount,
                          const  uint8_t *address,
                          const  uint8_t *data);

/**
 * @brief This function to check line count
 * 
 * @param[in] data the line count data 
 * @return ErrorCode 
 */
static ErrorCode checkLineCount(const uint8_t *data);


/**
 * @brief Check if Hexa Code in line is valid
 * 
 * @param[in] line The line of record
 * @return uint8_t Error Code
 */
static ErrorCode checkHexa(const uint8_t *line);

/**
 * @brief Get the Data in record line
 * 
 * @param[in] line Pointer to record  line
 * @param[in] type Type of record line 
 * @param[in] address Pointer to address array 
 * @param[in] data Pointer to data array
 */
static void getData(const uint8_t *line,
                    const uint8_t type,
                          uint8_t *address,
                          uint8_t *data);

/**
 * @brief Get the the data of type 0 1 2 3 7 8 9 record line
 * 
 * @param[in] line Record line
 * @param[in] type The type of record line
 * @param[in] address Pointer to address array
 * @param[in] data Pointer to data array
 * @param[in] addressLength The length of address (bit)
 */
static void getType0123789Data(const uint8_t *line,
                               const uint8_t type,
                                    uint8_t *address,
                                    uint8_t *data,  
                                const uint8_t addressLength);

/**
 * @brief Get the data of type 5 6 record line
 * 
 * @param[in] line Record line
 * @param[in] data Pointer to data area
 * @param[in] lineCountLength The length of linecount value
 */
static void getType56Data(const uint8_t *line, 
                                uint8_t *data, 
                          const uint8_t lineCountLength);


/**
 * @brief Convert text to hexa number
 * 
 * @param[in] str The text pointer
 * @return uint64_t hexa number result
 */
static uint16_t TEXT_TO_HEX(const uint8_t *str);

/**
 * @brief Send address and data of the line which has no error
 * 
 * @param[in] address Pointer to address array
 * @param[in] data Pointer to data array
 */
static void sendData(const uint8_t * address,
                     const uint8_t *data);

/**
 * @brief Send error message
 * 
 * @param[in] error error code to generate the message
 * @param[in] line when there is error in a line, the line will 
 * be sent to the terminal for checking again
 */
static void sendError(const ErrorCode error,
                      const uint8_t *line);

/**
 * @brief Write 4 bytes to the flash
 * 
 * @param[in] address the start address
 * @param[in] data 4 bytes data
 */
static void write_to_flash(const uint8_t *address,
                           const uint8_t *data);
/******************************************************************************
 * Codes
******************************************************************************/

ErrorCode parseRecordLine(const uint8_t *line){
    uint8_t type;
    uint8_t address[10];
    uint8_t byteCount;
    uint8_t data[80];
    uint8_t checksum;
    ErrorCode errorCode;
    uint8_t temp[TEMP_STRING_LENGTH];
    /*Update the line count*/
    lineCount ++;
    /*Init the original errorCode*/
    errorCode = SUCCESS;

    /* Get the line type*/
    type = line[TYPE_POSITION] - '0';
    /*Get the byte count*/
    strncpy(temp, line + LINE_TYPE_LENGTH, BYTE_COUNT_LENGTH); 
    temp[BYTE_COUNT_LENGTH] = '\0';
    byteCount = TEXT_TO_HEX(temp);

    /*Get the check sum value*/
    /*Check sum data is last 2 character of the line*/
    strncpy(temp, line + strlen(line) - CHECKSUM_LENGTH, CHECKSUM_LENGTH);
    temp[CHECKSUM_LENGTH] = '\0';
    checksum = TEXT_TO_HEX(temp);

    /* Check the fomart of record line*/
    errorCode = checkHexa(line);
    /*Check the S character at the firt of line*/
    if(errorCode == SUCCESS){
        errorCode = checkS(line[S_POSITION]);
    }
    /*Check the byte count*/
    if(errorCode == SUCCESS){
        errorCode = checkByteCount(byteCount, line);
    }
    /* Get the address and data*/
    if(errorCode == SUCCESS){
        getData(line, type, address, data);
        /*Check the checkSum value*/
        errorCode = checkSum(checksum, byteCount, address, data);
    }
    if((type == _16_BIT_COUNT) || (type == _24_BIT_COUNT)){
        /*Check line count*/
        /*In type 5 6, line count is contained by data*/
        if(errorCode == SUCCESS){
            errorCode = checkLineCount((data));
        }
    }
    /*Send error to the terminal*/
    if(errorCode != SUCCESS){
        sendError(errorCode, line);
    }
    // flash to the memory if no error
    if((errorCode == SUCCESS) && ((type > HEADER) && (type <= _32_BIT_DATA))){
        write_to_flash(address, data);
    }
    /*When flashed done*/
    if(type > _32_BIT_START_ADDRESS){
        write_done = 1;
        FPTE->PSOR |= RED_LED_PIN;
        uint8_t message[30] = "";
        strcpy(message, "--> Done flash to memory\n");
        sendMessage(message);
    }

    return errorCode;
}

static void write_to_flash(const uint8_t *address,
                           const uint8_t *data){
    uint8_t index;
    uint8_t temp_string[3];
    uint8_t data_array[16];
    uint8_t len = strlen(data);
    /*Convert to hex number string*/
    for(index = 0; index < 16; index ++){
        strncpy(temp_string, data + index * 2, 2);
        temp_string[2] = '\0';
        data_array[index] = TEXT_TO_HEX(temp_string);
    }
    if(strlen(data) <32){
        for(index = len / 2; index < 16; index ++){
            data_array[index] = 0xFF;
        }
    }
    /*Write each byte to the flash*/
    for(index = 0; index < 4; index ++){
        __disable_irq();/*disable interrupt*/
        Program_LongWord_8B(TEXT_TO_HEX(address) + index *4, data_array + index * 4);
        __enable_irq();/*enable interrupt*/
    }
}

static uint16_t TEXT_TO_HEX(const uint8_t *str) {
    uint16_t result;
    uint8_t len;
    uint8_t index;

    len = strlen(str);
    result = 0; 
    for (index = 0; index < len; index++) { 
        result = (result << HEXA_SHIFT) + ASCII_TO_HEX(str[index]); 
    }

    return result;
}

static ErrorCode checkHexa(const uint8_t *line){
    ErrorCode errorCode;
    uint8_t index;

    errorCode = SUCCESS;
    for (index = 0; (index < (strlen(line) -1)) && (errorCode == SUCCESS); index++){
        /*The valindexd characters are 0->9, S, A->F*/
        if(! ((line[index] == 'S') || ((line[index] >= 'A') && (line[index] <= 'F')) || ((line[index] >= '0') && line[index] <= '9'))){
            errorCode = HEXA_INVALID;
        }
    }
    return errorCode;
}

static ErrorCode checkS(const uint8_t character){
    ErrorCode errorCode;

    errorCode = SUCCESS;
    if(character != 'S'){
        errorCode = NO_S;
    }

    return errorCode;
}

static ErrorCode checkByteCount(const uint8_t byteCount, const uint8_t* line){
    ErrorCode errorCode;

    errorCode = SUCCESS;
    if(byteCount != ((strlen(line) - TYPE_LENGTH - BYTE_COUNT_LENGTH) / BYTE_LENGTH)){
        errorCode = BYTE_COUNT_ERROR;
    }

    return errorCode;
}

static ErrorCode checkSum(const uint8_t checksum,
                          const uint8_t byteCount,
                          const  uint8_t *address,
                          const  uint8_t *data){
    ErrorCode errorCode;
    uint16_t sum;
    uint8_t index;
    uint8_t temp[TEMP_STRING_LENGTH];
    errorCode = SUCCESS;
    /*Sum = byteCount + Address + Data*/
    sum = byteCount;
    /*Sum with address*/
    for (index = 0; index < strlen(address); index = index + 2){
        strncpy(temp, address + index, 2);
        temp[2] = '\0';
        sum += TEXT_TO_HEX(temp);
    }
    /*Sum with data*/
    for (index = 0; index < strlen(data); index = index + 2){
        strncpy(temp, data + index, 2);
        temp[2] = '\0';
        sum +=TEXT_TO_HEX(temp);
    }    
    /*Calculate the sum value again*/
    sum = 0xFF - (sum & 0xFF);
    if(sum != checksum){
        errorCode = CHECK_SUM_ERROR;
    }

    return errorCode;
}

static void getData(const uint8_t *line,
                    const uint8_t type,
                          uint8_t *address,
                          uint8_t *data){
    uint8_t index;
    switch (type){
        case HEADER:
            getType0123789Data(line,type, address, data, _16BIT_ADDRESS_LENGTH);
            break;
        case _16_BIT_DATA:
            getType0123789Data(line,type, address, data, _16BIT_ADDRESS_LENGTH);
            break;
        case _24_BIT_DATA:
            getType0123789Data(line,type, address, data, _24BIT_ADDRESS_LENGTH);
            break;
        case _32_BIT_DATA:
            getType0123789Data(line,type, address, data, _32BIT_ADDRESS_LENGTH);
            break;
        case _16_BIT_COUNT:
            getType56Data(line, data, _16BIT_LINE_COUNT_LENGTH);
            /*There is no address*/
            address[0] = '\0';
            break;
        case _24_BIT_COUNT:
            getType56Data(line, data, _24BIT_LINE_COUNT_LENGTH);
            /*There is no address*/
            address[0] = '\0';
            break;
        case _32_BIT_START_ADDRESS: 
            getType0123789Data(line, type, address, data, _32BIT_ADDRESS_LENGTH);
            break;
        case _24_BIT_START_ADDRESS: 
            getType0123789Data(line, type, address, data, _24BIT_ADDRESS_LENGTH);
            break;
        case _16_BIT_START_ADDRESS: 
            getType0123789Data(line,type, address, data, _16BIT_ADDRESS_LENGTH);
            break;
        default:
            break;
    }
}

static void getType0123789Data(const uint8_t *line,
                                    uint8_t type,
                                    uint8_t *address,
                                    uint8_t *data,  
                                const uint8_t addressLength){
    uint8_t addressPosition;
    uint8_t dataPosition;
    uint8_t dataLength;

    addressPosition = TYPE_LENGTH + BYTE_COUNT_LENGTH;
    strncpy(address, line + addressPosition, addressLength);
    address[addressLength] = '\0';
    /*Type 7 8 9 has no data*/
    if(type <= _32_BIT_DATA){
        dataPosition = addressPosition + addressLength;
        /*Get the length of data field*/
        dataLength = strlen(line) - addressPosition - addressLength - CHECKSUM_LENGTH ;
        strncpy(data, line + dataPosition, dataLength);
        data[dataLength] = '\0';
    } else{
        data[0] = '\0';
    }
}

static void getType56Data(const uint8_t *line, 
                                uint8_t *data, 
                            const uint8_t lineCountLength){
    /*Type 5 6 has no address*/
    uint8_t dataPosition;
    /*Get the possition of data field*/
    dataPosition = TYPE_LENGTH + BYTE_COUNT_LENGTH;
    strncpy(data, line + dataPosition, lineCountLength);
    data[lineCountLength] = '\0';
}

static ErrorCode checkLineCount(const uint8_t *data){
    uint16_t sum;

    ErrorCode errorCode = SUCCESS;
    /*convert line count data to hex number to commpare*/
    sum = TEXT_TO_HEX(data);
    if(sum != (lineCount - 2)){
        errorCode = LINE_COUNT_ERROR;
    }

    return errorCode;
}

static void sendData(const uint8_t * address,
                     const uint8_t *data){
    sendMessage(address);
    transmit('_');

    sendMessage(data);
    transmit('\n');
}

static void sendError(const ErrorCode error,
                      const uint8_t *line){
    uint8_t errorMessage[30] = "";

    switch (error)
    {
    case HEXA_INVALID:
        strcpy(errorMessage, "Invalid character\0");
        break;
    case NO_S:
        strcpy(errorMessage, "There is no S character\0");
        break;
    case BYTE_COUNT_ERROR:
        strcpy(errorMessage, "Invalid Bytecount\0");
        break;
    case CHECK_SUM_ERROR:
        strcpy(errorMessage, "Invalid Checksum\0");
        break;
    case LINE_COUNT_ERROR:
        strcpy(errorMessage, "Invalid Linecount\0");
        break;
    default:
        break;
    }
    /*Send error message*/
    sendMessage(errorMessage);
    transmit('_');
    /*Send the line for checking*/
    sendMessage(line);
    transmit('\n');
}
/******************************************************************************
 * End of file
******************************************************************************/
