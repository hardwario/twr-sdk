################################################################################
#   __  __       _         __ _ _                                              #
#  |  \/  | __ _| | _____ / _(_) | ___                                         #
#  | |\/| |/ _` | |/ / _ \ |_| | |/ _ \                                        #
#  | |  | | (_| |   <  __/  _| | |  __/                                        #
#  |_|  |_|\__,_|_|\_\___|_| |_|_|\___|                                        #
#                                                                              #
################################################################################

# TODO Implement "help" target

################################################################################
# Verbose build?                                                               #
################################################################################

ifeq ("$(BUILD_VERBOSE)","1")
Q :=
ECHO = @echo
else
MAKE += -s
Q := @
ECHO = @echo
endif

################################################################################
# Directories                                                                  #
################################################################################

APP_DIR ?= app
OBJ_DIR ?= obj
OUT_DIR ?= out
SDK_DIR ?= sdk

################################################################################
# Output name                                                                  #
################################################################################

OUT ?= firmware
TYPE ?= debug

################################################################################
# Output extensions                                                            #
################################################################################

ELF ?= $(OUT_DIR)/$(TYPE)/$(OUT).elf
MAP ?= $(OUT_DIR)/$(TYPE)/$(OUT).map
BIN ?= $(OUT_DIR)/$(TYPE)/$(OUT).bin

################################################################################
# Linker script                                                                #
################################################################################

LINKER_SCRIPT ?= $(SDK_DIR)/sys/lkr/stm32l083cz.ld

################################################################################
# Include directories                                                          #
################################################################################

INC_DIR += $(APP_DIR)
INC_DIR += $(SDK_DIR)/bcl/inc
INC_DIR += $(SDK_DIR)/bcl/stm/inc
INC_DIR += $(SDK_DIR)/stm/hal/inc
INC_DIR += $(SDK_DIR)/stm/spirit1/inc
INC_DIR += $(SDK_DIR)/stm/usb/inc
INC_DIR += $(SDK_DIR)/sys/inc

################################################################################
# Source directories                                                           #
################################################################################

SRC_DIR += $(APP_DIR)
SRC_DIR += $(SDK_DIR)/bcl/src
SRC_DIR += $(SDK_DIR)/bcl/stm/src
SRC_DIR += $(SDK_DIR)/stm/hal/src
SRC_DIR += $(SDK_DIR)/stm/spirit1/src
SRC_DIR += $(SDK_DIR)/stm/usb/src
SRC_DIR += $(SDK_DIR)/sys/src

################################################################################
# Toolchain                                                                    #
################################################################################

TOOLCHAIN ?= arm-none-eabi-
CC = $(TOOLCHAIN)gcc
GDB = $(TOOLCHAIN)gdb
OBJCOPY = $(TOOLCHAIN)objcopy
SIZE = $(TOOLCHAIN)size
OZONE ?= Ozone
DFU_UTIL ?= dfu-util
DOXYGEN ?= doxygen
VSCODE ?= code

################################################################################
# Compiler flags for "c" files                                                 #
################################################################################

CFLAGS += -mcpu=cortex-m0plus
CFLAGS += -mthumb
CFLAGS += -mlittle-endian
CFLAGS += -Wall
CFLAGS += -pedantic
CFLAGS += -Wextra
CFLAGS += -Wmissing-include-dirs
CFLAGS += -Wswitch-default
CFLAGS += -Wswitch-enum
CFLAGS += -D'__weak=__attribute__((weak))'
CFLAGS += -D'__packed=__attribute__((__packed__))'
CFLAGS += -D'USE_HAL_DRIVER'
CFLAGS += -D'STM32L083xx'
CFLAGS += -D'HAL_IWDG_MODULE_ENABLED'
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -std=c11
CFLAGS_DEBUG += -g3
CFLAGS_DEBUG += -Og
CFLAGS_DEBUG += -D'DEBUG'
CFLAGS_RELEASE += -Os
CFLAGS_RELEASE += -D'RELEASE'

BAND ?= 868
CFLAGS += -D'BAND=$(BAND)'

################################################################################
# Compiler flags for "s" files                                                 #
################################################################################

ASFLAGS += -mcpu=cortex-m0plus
ASFLAGS += -mthumb
ASFLAGS += -mlittle-endian
ASFLAGS_DEBUG += -g3
ASFLAGS_DEBUG += -Og
ASFLAGS_RELEASE += -Os

################################################################################
# Linker flags                                                                 #
################################################################################

LDFLAGS += -mcpu=cortex-m0plus
LDFLAGS += -mthumb
LDFLAGS += -mlittle-endian
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -Wl,-lc
LDFLAGS += -Wl,-lm
LDFLAGS += -static
LDFLAGS += -Wl,-Map=$(MAP)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-u,__errno
LDFLAGS += --specs=nosys.specs

################################################################################
# Create list of files for compilation                                         #
################################################################################

SRC_C = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))
SRC_S = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.s))

################################################################################
# Create list of object files and their dependencies                           #
################################################################################

OBJ_C = $(SRC_C:%.c=$(OBJ_DIR)/$(TYPE)/%.o)
OBJ_S = $(SRC_S:%.s=$(OBJ_DIR)/$(TYPE)/%.o)
OBJ = $(OBJ_C) $(OBJ_S)
DEP = $(OBJ:%.o=%.d)

################################################################################
# Debug target                                                                 #
################################################################################

.PHONY: debug
debug:
	$(Q)$(MAKE) .clean-out
	$(Q)$(MAKE) .obj-debug
	$(Q)$(MAKE) elf
	$(Q)$(MAKE) size
	$(Q)$(MAKE) bin

################################################################################
# Release target                                                               #
################################################################################

.PHONY: release
release:
	$(Q)$(MAKE) clean TYPE=release
	$(Q)$(MAKE) .obj-release TYPE=release
	$(Q)$(MAKE) elf TYPE=release
	$(Q)$(MAKE) size TYPE=release
	$(Q)$(MAKE) bin TYPE=release
	$(Q)$(MAKE) .clean-obj TYPE=release

################################################################################
# Clean target                                                                 #
################################################################################

.PHONY: clean
clean:
	$(Q)$(MAKE) .clean-obj
	$(Q)$(MAKE) .clean-out

.PHONY: .clean-obj
.clean-obj:
	$(Q)$(ECHO) "Removing object directory..."
	$(Q)rm -rf $(OBJ_DIR)/$(TYPE)

.PHONY: .clean-out
.clean-out:
	$(Q)$(ECHO) "Clean output ..."
	$(Q)rm -f "$(ELF)" "$(MAP)" "$(BIN)"

################################################################################
# Flash firmware using DFU bootloader                                          #
################################################################################

.PHONY: dfu-release
dfu-release:
	$(Q)$(MAKE) dfu TYPE=release

.PHONY: dfu-debug
dfu-debug:
	$(Q)$(MAKE) dfu

.PHONY: dfu
dfu: $(BIN)
	$(Q)$(ECHO) "Flashing $(BIN)..."
	$(Q)$(DFU_UTIL) -d 0483:df11 -a 0 -s 0x08000000:leave -D $(BIN)

################################################################################
# Generate Doxygen documentation                                               #
################################################################################

.PHONY: doc
doc:
	$(Q)$(ECHO) "Generating documentation..."
	$(Q)sh -c 'cd $(SDK_DIR) && $(DOXYGEN) Doxyfile'

################################################################################
# Debug firmware using GDB debugger (using Segger J-Link probe)                #
################################################################################

.PHONY: gdb
gdb: debug
	$(Q)$(ECHO) "Launching GDB debugger..."
	$(Q)$(GDB) -x $(SDK_DIR)/tools/gdb/gdbinit $(ELF)

################################################################################
# Debug firmware using Ozone debugger (from Segger)                            #
################################################################################

.PHONY: ozone
ozone: debug
	$(Q)$(ECHO) "Launching Ozone debugger..."
	$(Q)$(OZONE) $(SDK_DIR)/tools/ozone/ozone.jdebug

################################################################################
# Open project in Visual Studio Code                                           #
################################################################################

.PHONY: code
code:
	$(Q)$(ECHO) "Opening project in Visual Code..."
	$(Q)$(VSCODE) .

################################################################################
# J-Link                                          #
################################################################################

.PHONY: jlink-flash
jlink-flash:
ifeq ($(OS),Windows_NT)
	JLink -device stm32l083cz -CommanderScript $(SDK_DIR)/tools/jlink/flash.jlink
else
	JLinkExe -device stm32l083cz -CommanderScript $(SDK_DIR)/tools/jlink/flash.jlink
endif

.PHONY: jlink-gdbserver
jlink-gdbserver:
ifeq ($(OS),Windows_NT)
	JLinkGDBServerCL -singlerun -device stm32l083cz -if swd -speed 4000 -localhostonly -reset
else
	JLinkGDBServer -singlerun -device stm32l083cz -if swd -speed 4000 -localhostonly -reset
endif

.PHONY: jlink
jlink:
	$(Q)$(MAKE) jlink-flash
	$(Q)$(MAKE) jlink-gdbserver

################################################################################
# Link object files                                                            #
################################################################################

.PHONY: elf
elf: $(ELF)

$(ELF): $(OBJ)
	$(Q)$(ECHO) "Linking object files..."
	$(Q)mkdir -p $(OUT_DIR)/$(TYPE)
	$(Q)$(CC) $(LDFLAGS) $(OBJ) -o $(ELF)

################################################################################
# Print information about size of sections                                     #
################################################################################

.PHONY: size
size: $(ELF)
	$(Q)$(ECHO) "Size of sections:"
	$(Q)$(SIZE) $(ELF)

################################################################################
# Create binary file                                                           #
################################################################################

.PHONY: bin
bin: $(BIN)

$(BIN): $(ELF)
	$(Q)$(ECHO) "Creating $(BIN) from $(ELF)..."
	$(Q)$(OBJCOPY) -O binary $(ELF) $(BIN)
	$(Q)rm -f $(OUT).bin
	$(Q)cp $(BIN) $(OUT).bin

################################################################################
# Compile source files                                                         #
################################################################################

.PHONY: .obj-debug
.obj-debug: CFLAGS += $(CFLAGS_DEBUG)
.obj-debug: ASFLAGS += $(ASFLAGS_DEBUG)
.obj-debug: $(OBJ)

.PHONY: .obj-release
.obj-release: CFLAGS += $(CFLAGS_RELEASE)
.obj-release: ASFLAGS += $(ASFLAGS_RELEASE)
.obj-release: $(OBJ)

################################################################################
# Compile "c" files                                                            #
################################################################################

$(OBJ_DIR)/$(TYPE)/%.o: %.c
	$(Q)$(ECHO) "Compiling: $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -MMD -c $(CFLAGS) $(foreach d,$(INC_DIR),-I$d) $< -o $@

################################################################################
# Compile "s" files                                                            #
################################################################################

$(OBJ_DIR)/$(TYPE)/%.o: %.s
	$(Q)$(ECHO) "Compiling: $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -MMD -c $(ASFLAGS) $< -o $@

################################################################################
# Include dependencies                                                         #
################################################################################

-include $(DEP)

################################################################################
# End of file                                                                  #
################################################################################
