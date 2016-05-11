cmake_minimum_required(VERSION 2.8.11)

project(SlowControlTBL C CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  	set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
  endif ()
else ()
	set (CMAKE_CXX_STANDARD 11)
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)

include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories("include")

set(LIBS ${LIBS} ${JSONCPP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

qt5_wrap_cpp(Interfaces_SRC include/Interface.h include/HVGroup.h)

set(SOURCES
    "src/main.cpp"
    "src/Interface.cpp"
    "src/LoggingManager.cpp"
    "src/Setup.cpp"
    "src/HVGroup.cpp"
    ${Interfaces_SRC}
    )

add_executable(main ${SOURCES})

qt5_use_modules(main Widgets)

target_link_libraries(main ${LIBS})
