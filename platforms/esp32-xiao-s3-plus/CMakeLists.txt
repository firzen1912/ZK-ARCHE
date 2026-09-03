cmake_minimum_required(VERSION 3.16)

# Build the existing protocol/wire/crypto C sources as an ESP-IDF component.
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/components/zk_arche_core"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(zk_arche_xiao_esp32s3_plus)
