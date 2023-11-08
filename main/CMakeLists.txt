# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS "main.c" "SIM_UART.c" "AT_function.c" "SDCard.c" "mypH.c" "nvs_interface.c" "myDO.c" "myTDS.c" "i2cdev.c" "ds1307.c" "ds3231.c" "EC.c" "ssd1306.c" "font.c"        # list the source files of this component
    INCLUDE_DIRS        # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
