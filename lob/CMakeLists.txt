cmake_minimum_required(VERSION 3.20)
project(lob LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# NASDAQ ITCH 5.0 replay
add_executable(itch_replay nasdaq_itch/itch_replay.cc)

enable_testing()

add_executable(test_order orders/testOrder.cc)
add_test(NAME test_order COMMAND test_order)

add_executable(test_vector_orders orders/testVectorOrders.cc)
add_test(NAME test_vector_orders COMMAND test_vector_orders)


