CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

PROJECT(kiv-pd-vtpbuddy)

ADD_DEFINITIONS(--std=c++11)

FILE(GLOB tmp_src
	src/*.h
	src/*.hpp
	src/*.c
	src/*.cpp
)

ADD_EXECUTABLE(kiv-pd-vtpbuddy ${tmp_src})

TARGET_LINK_LIBRARIES(kiv-pd-vtpbuddy pthread)
