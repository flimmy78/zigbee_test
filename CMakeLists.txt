cmake_minimum_required(VERSION 2.6)
project(zigbee)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -std=c99 -D_BSD_SOURCE")

add_executable(zb_controler controler.c zigbee.c serial.c zigbee_protocol.c display.c)
add_executable(zb_end_device end_device.c zigbee.c serial.c zigbee_protocol.c display.c)
add_executable(zb_unit_test unit_test.c zigbee.c serial.c)
add_executable(test_float test_float.c)
add_executable(end_device_mini end_device_mini.c serial.c)

#install(TARGETS zb_unit_test RUNTIME DESTINATION bin)

