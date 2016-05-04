cmake_minimum_required(VERSION 2.8.11)

project(test C CXX)

set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  	set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
  endif ()
else ()
	set (CMAKE_CXX_STANDARD 11)
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories("include")

find_package(Qt5Widgets)
find_package(Threads)
find_package(jsoncpp)

set(SOURCES
    "src/main.cpp"
    "src/Interface.cpp"
    "src/LoggingManager.cpp"
    "src/Setup.cpp"
    )

add_executable(main ${SOURCES})

qt5_use_modules(main Widgets)

#target_link_libraries(helloworld Qt4::Widgets)
target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(main ${JSONCPP_LIBRARIES})
