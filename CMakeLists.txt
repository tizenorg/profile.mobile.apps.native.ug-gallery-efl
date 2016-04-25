CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-gallery-efl C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
#SET(PREFIX ${CMAKE_INSTALL_PREFIX})
#SET(RESDIR "${IMGDIRRES}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/inc")

SET(GALLERY_UG_PROJECT_NAME ug-gallery-efl)
SET(GE_EDC_DIR edje)
SET(GE_IMG_DIR edje/images)
SET(GE_SOUND_DIR edje/sounds)
SET(GE_IMG_EDC_DIR edje/images)
SET(GE_IMG_C_DIR edje/images)

# subdir
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(src/data)
ADD_SUBDIRECTORY(src/util)
ADD_SUBDIRECTORY(src/view)
ADD_SUBDIRECTORY(src/widget)
ADD_SUBDIRECTORY(res/edje)
ADD_SUBDIRECTORY(res/icon)
ADD_SUBDIRECTORY(res/edje/images)
ADD_SUBDIRECTORY(res/po)
ADD_SUBDIRECTORY(res/edje/sounds)

CONFIGURE_FILE(ug-gallery-efl.manifest-3.0.in ${PKGNAME}.manifest)

# EDJ
ADD_CUSTOM_COMMAND(OUTPUT gallery-efl.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_DIR}
				-sd  ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_SOUND_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_EDC_DIR}/gallery-efl.edc
				${CMAKE_BINARY_DIR}/gallery-efl.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_EDC_DIR}/gallery-efl.edc
)

ADD_CUSTOM_COMMAND(OUTPUT gallery-efl-images.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_EDC_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_EDC_DIR}/gallery-efl-images.edc ${CMAKE_BINARY_DIR}/gallery-efl-images.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_EDC_DIR}/gallery-efl-images.edc
)

ADD_CUSTOM_TARGET(gallery_efl_edj_build DEPENDS gallery-efl.edj gallery-efl-images.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} gallery_efl_edj_build)

INSTALL(FILES ${CMAKE_BINARY_DIR}/gallery-efl.edj DESTINATION ${RESDIR}/edje)
INSTALL(FILES ${CMAKE_BINARY_DIR}/gallery-efl-images.edj DESTINATION ${RESDIR}/edje/images)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_C_DIR} DESTINATION ${RESDIR}/edje)

INSTALL(FILES ug-gallery-efl.manifest.xml DESTINATION ${MANIFESTDIR} RENAME ${PKGNAME}.xml)
INSTALL(FILES shared/res/ug-gallery-efl.png DESTINATION ${ICONDIR})
#INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/images/ DESTINATION ${IMGDIR})
INSTALL(FILES ${PROJECT_NAME}.xml DESTINATION ${MANIFESTDIR})


# Install appcontrol icon
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/icon/ug-gallery-efl.png DESTINATION ${ICONDIR})
