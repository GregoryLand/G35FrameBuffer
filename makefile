### Variables Needed
CURRENT_DIR   = $(shell basename $(CURDIR))
PROJECT_DIR   = ~/Programming/Ardwino/IDE\ Workgroup/G35FrameBuffer
ARDMK_DIR     = /usr/share/arduino
ARDUINO_DIR   = /usr/share/arduino
USER_LIB_PATH = $(PROJECT_DIR)/Libraries
BOARD_TAG     = uno
AVR_TOOLS_DIR = /usr/bin
AVRDDUDE      = /usr/bin/avrdude
CPPFLAGS      = -pedantic -Wall -Wextra
MONITOR_PORT  = /dev/ttyACM*
OBJDIR        = $(PROJECT_DIR)/bin/$(BOARD_TAG)/$(CURRENT_DIR)

include $(ARDMK_DIR)/Arduino.mk
