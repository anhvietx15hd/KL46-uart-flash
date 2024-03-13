################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/Flash.c \
../source/Le_Hung_Viet_Mock_Project.c \
../source/SrecParser.c \
../source/hardware_init.c \
../source/mtb.c \
../source/uart.c 

C_DEPS += \
./source/Flash.d \
./source/Le_Hung_Viet_Mock_Project.d \
./source/SrecParser.d \
./source/hardware_init.d \
./source/mtb.d \
./source/uart.d 

OBJS += \
./source/Flash.o \
./source/Le_Hung_Viet_Mock_Project.o \
./source/SrecParser.o \
./source/hardware_init.o \
./source/mtb.o \
./source/uart.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MKL46Z256VLL4_cm0plus -DCPU_MKL46Z256VLL4 -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I"F:\Study\2023\FPT SOFTWARE\MCP\WS\Le_Hung_Viet_Mock_Project\source" -I"F:\Study\2023\FPT SOFTWARE\MCP\WS\Le_Hung_Viet_Mock_Project" -I"F:\Study\2023\FPT SOFTWARE\MCP\WS\Le_Hung_Viet_Mock_Project\CMSIS" -I"F:\Study\2023\FPT SOFTWARE\MCP\WS\Le_Hung_Viet_Mock_Project\startup" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/Flash.d ./source/Flash.o ./source/Le_Hung_Viet_Mock_Project.d ./source/Le_Hung_Viet_Mock_Project.o ./source/SrecParser.d ./source/SrecParser.o ./source/hardware_init.d ./source/hardware_init.o ./source/mtb.d ./source/mtb.o ./source/uart.d ./source/uart.o

.PHONY: clean-source

