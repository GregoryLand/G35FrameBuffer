# Tags needed for the make file to know what todo
BOARD_TAG     = atmega328
ARDMK_DIR     = Arduino-Makefile
ARDUINO_DIR   = ../../../../Arduino
MONITOR_PORT  = com3

# Add libraries needed for project
USER_LIB_PATH = libraries
ARDUINO_LIBS  = G35Arduino

# Call the make file
include $(ARDMK_DIR)/Arduino.mk