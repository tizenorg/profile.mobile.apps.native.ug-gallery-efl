CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-gallery-efl C)
SET(GALLERY_UG_PROJECT_NAME ug-gallery-efl)
SET(GE_EDC_DIR edje)
SET(GE_IMG_DIR edje/images)
SET(GE_SOUND_DIR edje/sounds)
SET(GE_IMG_EDC_DIR edje/images)
SET(GE_IMG_C_DIR edje/images)

#IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
#	SET(CMAKE_BUILD_TYPE "Release")
#ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
#MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/inc")
SET(VERSION 1.0)

#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src MAIN_UG_SRCS)
#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/data DATA_UG_SRCS)
#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/util UTIL_UG_SRCS)
#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/view VIEW_UG_SRCS)
#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/widget WIDGET_UG_SRCS)

SET(SRCS
	gallery-efl.c
	data/ge-data.c
	data/ge-data-type.c
	data/ge-data-util.c
	data/ge-local-data.c
	util/ge-db-update.c
	util/ge-debug.c
	util/ge-exif.c
	util/ge-ext-exec.c
	util/ge-ext-ug-load.c
	util/ge-file-util.c
	util/ge-lang.c
	util/ge-ui-util.c
	util/ge-util.c
	view/ge-albums.c
	view/ge-gridview.c
	view/ge-main-view.c
	widget/ge-button.c
	widget/ge-nocontent.c
	widget/ge-rotate-bg.c
	widget/ge-thumb.c
	view/ge-albums.c
	widget/ge-tile.c
)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/inc/)



INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	elementary
	evas
	dlog
	glib-2.0
	capi-appfw-application
	capi-content-media-content
	efl-extension
	notification
	appsvc
	storage
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -fvisibility=hidden -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

ADD_DEFINITIONS("-D_USE_DLOG_")
ADD_DEFINITIONS("-D_USE_SECURE_LOG_")
ADD_DEFINITIONS("-D_USE_ROTATE_BG_GE")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PROJECT_NAME}\"")
ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

#Set executable
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

#ADD_LIBRARY(${PROJECT_NAME} SHARED ${UG_SRCS})
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.0.1)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} "-ldl")


# install binary
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})



# i18n
#ADD_SUBDIRECTORY(res/po)




