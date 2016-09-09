cmake_minimum_required(VERSION 2.8.11)

project(SlowControlTBL C CXX)

add_definitions(-DLINUX)

set(CMAKE_BUILD_TYPE ReleaseWithDebug)
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

set(Python_ADDITIONAL_VERSIONS 2.6 2.7)
find_package(PythonLibs REQUIRED)

include_directories(${JSONCPP_INCLUDE_DIRS}/${JSONCPP_INCLUDE_PREFIX})
include_directories("include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/CosmicTrigger/include)
include_directories(${PYTHON_INCLUDE_DIRS})

set(LIBS ${LIBS} ${JSONCPP_LIBRARIES} ${PYTHON_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_CURRENT_SOURCE_DIR}/CosmicTrigger/lib/libCAEN.so)

qt5_wrap_cpp(Interfaces_SRC include/Interface.h include/HVGroup.h
    include/DiscriSettingsWindow.h)

set(SOURCES
    "src/main.cpp"
    "src/Interface.cpp"
    "src/LoggingManager.cpp"
    "src/ConditionManager.cpp"
    "src/HVGroup.cpp"
    "src/RealSetupManager.cpp"
    "src/FakeSetupManager.cpp"
    "src/DiscriSettingsWindow.cpp"
    "src/PythonDB.cpp"
    ${Interfaces_SRC}
    )

add_executable(main ${SOURCES})

qt5_use_modules(main Widgets)

target_link_libraries(main ${LIBS})
