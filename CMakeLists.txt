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
find_package(ROOT REQUIRED 6)

set(Python_ADDITIONAL_VERSIONS 2.6 2.7)
find_package(PythonLibs REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/CosmicTrigger/include)

include_directories(${JSONCPP_INCLUDE_DIRS}/${JSONCPP_INCLUDE_PREFIX})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${ROOT_INCLUDE_DIR})

set(LIBS 
    ${LIBS}
    ${JSONCPP_LIBRARY}
    ${PYTHON_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_CURRENT_SOURCE_DIR}/CosmicTrigger/lib/libCAEN.so
    ${ROOT_LIBRARIES}
    )

# Generate ROOT dictionary for Event class
ROOT_GENERATE_DICTIONARY("DICT__event" "Event.h" LINKDEF "Linkdef.h") 

qt5_wrap_cpp(Interfaces_SRC
    include/Interface.h
    include/HVGroup.h
    include/Trigger_TDC_Group.h
    include/DiscriSettingsWindow.h)
    

set(SOURCES
    "src/main.cpp"
    "src/Interface.cpp"
    "src/LoggingManager.cpp"
    "src/ConditionManager.cpp"
    "src/HVGroup.cpp"
    "src/Trigger_TDC_Group.cpp"
    "src/RealSetupManager.cpp"
    "src/FakeSetupManager.cpp"
    "src/DiscriSettingsWindow.cpp"
    "src/PythonDB.cpp"
    "DICT__event.cxx"
    ${Interfaces_SRC}
    )

add_executable(SlowControlTBL ${SOURCES})

qt5_use_modules(SlowControlTBL Widgets)

target_link_libraries(SlowControlTBL ${LIBS})

