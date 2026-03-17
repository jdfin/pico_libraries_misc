cmake_minimum_required(VERSION 3.13)
project(misc_native_tests CXX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Werror -fstack-protector-strong -fsanitize=address)
add_link_options(-fsanitize=address)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(misc_tests
    test_main.cpp
    test_args.cpp
)

enable_testing()
add_test(NAME misc_tests COMMAND misc_tests)
