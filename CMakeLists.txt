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

ADD_EXECUTABLE(vtpbuddy ${tmp_src})

INSTALL(TARGETS vtpbuddy DESTINATION /usr/sbin)
INSTALL(FILES misc/vtpbuddy.cfg DESTINATION /etc/)
INSTALL(FILES misc/vtpbuddy.service DESTINATION /etc/systemd/system/)
INSTALL(DIRECTORY DESTINATION /var/lib/vtpbuddy)
INSTALL(CODE "execute_process(COMMAND systemctl enable /etc/systemd/system/vtpbuddy.service)")

TARGET_LINK_LIBRARIES(vtpbuddy pthread crypto ssl)



FILE(GLOB ctl_src
	ctl-src/*.h
	ctl-src/*.hpp
	ctl-src/*.c
	ctl-src/*.cpp
)

ADD_EXECUTABLE(vtpbuddyctl ${ctl_src})

INSTALL(TARGETS vtpbuddyctl DESTINATION /usr/sbin)

TARGET_LINK_LIBRARIES(vtpbuddyctl pthread crypto ssl)
