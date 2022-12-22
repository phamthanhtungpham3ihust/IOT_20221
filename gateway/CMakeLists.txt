cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURENT_LIST_DIR}/components)
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURENT_LIST_DIR}/common)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)



project(gateway)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "html/web.html" TEXT)

