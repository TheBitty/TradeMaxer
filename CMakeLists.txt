cmake_minimum_required(VERSION 3.16)
project(TradingSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt")
endif()

add_executable(trading_system main.cpp)

target_link_libraries(trading_system 
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(trading_system rt)
endif()

set_target_properties(trading_system PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_target(install_python_deps
    COMMAND python3 -m pip install --user numpy
    COMMENT "Installing Python dependencies"
)

add_dependencies(trading_system install_python_deps)