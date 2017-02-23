FILE(GLOB ic_INCLUDES
						${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/Configuration/*.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/Ldap/*.h
)
FILE(GLOB ic_SOURCES
						${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
						${CMAKE_CURRENT_SOURCE_DIR}/src/Configuration/*.cpp
						${CMAKE_CURRENT_SOURCE_DIR}/src/Ldap/*.cpp
)

SET(kldap_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/kldap/src)
SET(kldap_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/__/3rdparty/kldap/src)

SET(kldap_INCLUDES_MOCINC
  ${kldap_SOURCE_DIR}/ldapsearch.h
  ${kldap_SOURCE_DIR}/ldapmodel.h
)

SET(kldap_INCLUDES
  ${kldap_SOURCE_DIR}/ldapattributeproxymodel.h
  ${kldap_SOURCE_DIR}/ldapstructureproxymodel.h
)

SET(kldap_SOURCES
  ${kldap_SOURCE_DIR}/ber.cpp
  ${kldap_SOURCE_DIR}/ldif.cpp
  ${kldap_SOURCE_DIR}/ldapurl.cpp
  ${kldap_SOURCE_DIR}/ldapserver.cpp
  ${kldap_SOURCE_DIR}/ldapobject.cpp
  ${kldap_SOURCE_DIR}/ldapconnection.cpp
  ${kldap_SOURCE_DIR}/ldapoperation.cpp
  ${kldap_SOURCE_DIR}/ldapcontrol.cpp
  ${kldap_SOURCE_DIR}/ldapsearch.cpp
  ${kldap_SOURCE_DIR}/ldapdn.cpp
  ${kldap_SOURCE_DIR}/ldapmodelnode_p.cpp
  ${kldap_SOURCE_DIR}/ldapmodel.cpp
  ${kldap_SOURCE_DIR}/ldapmodel_p.cpp
  ${kldap_SOURCE_DIR}/ldapstructureproxymodel.cpp
  ${kldap_SOURCE_DIR}/ldapattributeproxymodel.cpp
)

FILE(GLOB ic_UI ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.ui)

IF(ITALC_BUILD_WIN32)
	SET(WIN32_AUTHSSP_SOURCES
${ultravnc_DIR}/addon/ms-logon/authSSP/authSSP.cpp
${ultravnc_DIR}/addon/ms-logon/authSSP/GenClientServerContext.cpp)
ELSE()
	SET(WIN32_AUTHSSP_SOURCES "")
ENDIF()

SET(libvncclient_SOURCES
	${libvncserver_DIR}/libvncclient/cursor.c
	${libvncserver_DIR}/libvncclient/listen.c
	${libvncserver_DIR}/libvncclient/rfbproto.c
	${libvncserver_DIR}/libvncclient/sockets.c
	${libvncserver_DIR}/libvncclient/tls_none.c
	${libvncserver_DIR}/libvncclient/vncviewer.c)


SET(QRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/core.qrc)
QT5_WRAP_CPP(ic_MOC_out ${ic_INCLUDES})
QT5_WRAP_CPP(kldap_MOCINC_out ${kldap_INCLUDES_MOCINC})
QT5_WRAP_CPP(kldap_MOC_out ${kldap_INCLUDES})
QT5_WRAP_UI(ic_UIC_out ${ic_UI})
QT5_ADD_RESOURCES(ic_RCC_out ${QRC_FILE} ${CMAKE_CURRENT_BINARY_DIR}/builddata.qrc)
#IF(WIN32)
#	QT5_ADD_RESOURCES(ic_qt_qm_out ${CMAKE_CURRENT_BINARY_DIR}/qt_qm.qrc)
#ENDIF()
#QT5_TRANSLATIONS_FOR_QRC(${QRC_FILE} ${ic_SOURCES} ${ic_INCLUDES})

IF(ITALC_BUILD_WIN32)
	LINK_LIBRARIES(-lws2_32 -lwtsapi32 -lnetapi32 -luserenv -ladvapi32 -linterception)
	INCLUDE_DIRECTORIES(${Qt5Gui_PRIVATE_INCLUDE_DIRS} ${ultravnc_DIR}/addon/ms-logon/authSSP)
ENDIF(ITALC_BUILD_WIN32)

ADD_DEFINITIONS(-DBUILD_ITALC_CORE_LIBRARY)

INCLUDE_DIRECTORIES(${Ldap_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/Ldap ${libvncserver_DIR}/common/ ${kldap_SOURCE_DIR} ${kldap_BINARY_DIR})
ADD_LIBRARY(italc-core SHARED ${ic_SOURCES} ${kldap_SOURCES} ${ic_INCLUDES} ${ic_MOC_out} ${kldap_MOC_out} ${ic_UIC_out} ${ic_RCC_out} ${ic_qt_qm_out} ${libvncclient_SOURCES} ${WIN32_AUTHSSP_SOURCES})

IF(ITALC_BUILD_WIN32)
	SET_TARGET_PROPERTIES(italc-core PROPERTIES PREFIX "")
	INSTALL(TARGETS italc-core RUNTIME DESTINATION ${LIB_DIR})
ELSE()
	INSTALL(TARGETS italc-core LIBRARY DESTINATION ${LIB_DIR})
ENDIF(ITALC_BUILD_WIN32)

TARGET_LINK_LIBRARIES(italc-core
	Qt5::Gui
	Qt5::Network
	Qt5::Widgets
	Qt5::Xml
	${Ldap_LIBRARIES}
	${LZO_LIBRARIES}
	)

ADD_DEPENDENCIES(italc-core GenerateTranslationFiles)

# create target for building included MOC files from kldap before building italc-core library
ADD_CUSTOM_TARGET(kldap_MOC ALL DEPENDS ${kldap_MOCINC_out})
ADD_DEPENDENCIES(italc-core kldap_MOC)
