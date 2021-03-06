FILE(GLOB ctl_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
FILE(GLOB ctl_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

QT5_WRAP_CPP(ctl_MOC_out ${ctl_INCLUDES})

IF(ITALC_BUILD_WIN32)
	SET(WINRC "${CMAKE_CURRENT_BINARY_DIR}/italc-ctl-rc.obj")
	ADD_CUSTOM_COMMAND(OUTPUT ${WINRC}
						COMMAND ${WINDRES}
							-I${CMAKE_CURRENT_SOURCE_DIR}
							-o${WINRC}
							-i${CMAKE_CURRENT_BINARY_DIR}/italc-ctl.rc
						DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/italc-ctl.rc)
ENDIF(ITALC_BUILD_WIN32)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
ADD_EXECUTABLE(italc-ctl ${ctl_UIC_out} ${ctl_SOURCES} ${ctl_INCLUDES} ${ctl_MOC_out} ${WINRC})
TARGET_LINK_LIBRARIES(italc-ctl italc-core Qt5::Network)

INSTALL(TARGETS italc-ctl RUNTIME DESTINATION bin)

IF(ITALC_BUILD_WIN32)
	SET_TARGET_PROPERTIES(italc-ctl PROPERTIES LINK_FLAGS -mconsole)
ENDIF(ITALC_BUILD_WIN32)

