cmake_minimum_required(VERSION 3.25)

project(rffep LANGUAGES C CXX)

# Compiler options
#add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-Wall -Wextra )

add_executable(rffe 
    main.c
    src/dac/dac_app.c
    src/helpers/helpers_app.c
    src/rffe/rffe_app.c
    src/adc/pac1720_app.c
    src/pd/pd_app.c
)

# Path to headers if not in the system dir
target_include_directories(rffe PUBLIC 
    /usr/lib/arm-linux-gnueabihf
    /usr/lib/aarch64-linux-gnu
    src/dac
    src/helpers
    src/rffe
    src/adc
    src/pd
)

# Path to libs if not in the system dir
#target_link_directories(rffe /usr/lib/aarch64-linux-gnu)

# What libraries to use
#target_link_libraries(rffe pthread rt iio)
target_link_libraries(rffe iio gpiod)
