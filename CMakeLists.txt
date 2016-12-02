CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

PROJECT(kiv-pd-vtpbuddy)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_MAINTAINER "Martin Ubl")
SET(CPACK_PACKAGE_NAME "vtpbuddy")
SET(CPACK_PACKAGE_VERSION "1.0-dev")
INCLUDE(CPack)

ADD_DEFINITIONS(--std=c++11 -g)

FILE(GLOB tmp_src
	src/*.h
	src/*.hpp
	src/*.c
	src/*.cpp
)

ADD_EXECUTABLE(kiv-pd-vtpbuddy ${tmp_src})

INSTALL(TARGETS kiv-pd-vtpbuddy DESTINATION /usr/sbin)
INSTALL(FILES misc/vtpbuddy.cfg DESTINATION /etc/vtpbuddy)

TARGET_LINK_LIBRARIES(kiv-pd-vtpbuddy pthread)
