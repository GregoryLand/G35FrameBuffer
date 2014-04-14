### Variables Needed
CURRENT_DIR   = $(shell basename $(CURDIR))
PROJECT_DIR   := $(HOME)/Programming/Arduino/G35FrameBuffer
ARDMK_DIR     = /usr/share/arduino
ARDUINO_DIR   = /usr/share/arduino
USER_LIB_PATH := $(PROJECT_DIR)/libraries
BOARD_TAG     = uno
ARDUINO_LIBS  = G35Arduino
#AVR_TOOLS_DIR = /usr
AVRDDUDE      = /usr/bin/avrdude
CPPFLAGS      = -pedantic -Wall -Wextra
MONITOR_PORT  = /dev/ttyACM*
OBJDIR        = $(PROJECT_DIR)/bin/$(BOARD_TAG)/$(CURRENT_DIR)
#OPTIMIZATION_LEVEL = 3 # Screw software serial


include $(ARDMK_DIR)/Arduino.mk
