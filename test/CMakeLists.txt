
add_compile_options(-Wall -Wextra -Werror)

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
