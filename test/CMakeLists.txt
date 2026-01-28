
add_compile_options(-Wall -Wextra -Werror)

# argv_demo

add_executable(argv_demo
    argv_demo.cpp
)

pico_enable_stdio_uart(argv_demo 0)
pico_enable_stdio_usb(argv_demo 1)

target_link_libraries(argv_demo PRIVATE
    pico_stdlib
    pico_stdio_usb
    misc
)

pico_add_extra_outputs(argv_demo)

# argv_test

add_executable(argv_test
    argv_test.cpp
)

pico_enable_stdio_uart(argv_test 0)
pico_enable_stdio_usb(argv_test 1)

target_link_libraries(argv_test PRIVATE
    pico_stdlib
    pico_stdio_usb
    misc
)

pico_add_extra_outputs(argv_test)

# i2c_extra_test

#add_executable(i2c_extra_test
#    i2c_extra_test.cpp
#)

#pico_enable_stdio_uart(i2c_extra_test 0)
#pico_enable_stdio_usb(i2c_extra_test 1)

#target_link_libraries(i2c_extra_test PRIVATE
#    pico_stdlib
#    pico_stdio_usb
#    touchscreen
#    misc
#)

#pico_add_extra_outputs(i2c_extra_test)

# i2c_dev_test

#add_executable(i2c_dev_test
#    i2c_dev_test.cpp
#)

#pico_enable_stdio_uart(i2c_dev_test 0)
#pico_enable_stdio_usb(i2c_dev_test 1)

#target_link_libraries(i2c_dev_test PRIVATE
#    pico_stdlib
#    pico_stdio_usb
#    touchscreen
#    misc
#)

#pico_add_extra_outputs(i2c_dev_test)

# misc_test

add_executable(misc_test
    misc_test.cpp
)

pico_enable_stdio_uart(misc_test 0)
pico_enable_stdio_usb(misc_test 1)

target_link_libraries(misc_test PRIVATE
    pico_stdlib
    pico_stdio_usb
    misc
)

pico_add_extra_outputs(misc_test)
