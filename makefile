# Tags needed for the make file to know what todo
BOARD_TAG     = uno
ARDMK_DIR     = Arduino-Makefile

# Add libraries needed for project
USER_LIB_PATH = libraries
ARDUINO_LIBS  = G35Arduino

# Call the make file
include $(ARDMK_DIR)/Arduino.mk
