# Hal header path
INCLUDES := -I./include \
			-I../Inc \
			-I../../Drivers/CMSIS/Device/ST/STM32F3xx/Include \
			-I../../Drivers/STM32F3xx_HAL_Driver/Inc \

# Ensure the user defines HAL_PATH to the STM32 HAL include directory
# ifndef HAL_PATH
#   $(error HAL_PATH is not defined. Please specify the path to the STM32 HAL include directory.)
# endif
# Define the STM32F3-specific flags (modify if using a different STM32 series)
# MCU := -DSTM32F3

# Compiler and flags
CC := arm-none-eabi-gcc
AR := arm-none-eabi-ar
CFLAGS += -I$(HAL_PATH)/Inc

# MSFP source files
SRCS := $(CURDIR)/src/msfp_api.c \
        $(CURDIR)/src/msfp_parser.c \
        $(CURDIR)/src/msfp_task.c

# MSFP object files (one object file per source file)
OBJS := $(SRCS:.c=.o)

# The name of the static library
LIBRARY := libmsfp.a

# Rule to compile each source file into an object file
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(MCU) -c $< -o $@

# Rule to create a static library
$(LIBRARY): $(OBJS)
	$(AR) rcs $@ $(OBJS)

# Clean rule for MSFP object files and library
clean:
	rm -f $(OBJS) $(LIBRARY)

# Export the MSFP static library to the main project (can be used in the main Makefile)
export_lib:
	@echo "MSFP static library created: $(LIBRARY)"
