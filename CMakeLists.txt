Project (shippy)
CMake_Minimum_Required (VERSION 3.1)

#Path options
Set (BINDIR "bin" CACHE STRING "Where to install binaries")
Set (DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" CACHE STRING "Sets the root of data directories to a non-default location")
Set (ICONDIR "${DATAROOTDIR}/icons" CACHE STRING "Sets the icon directory for desktop entry to a non-default location.")

add_definitions(-DDATADIR=\"${DATAROOTDIR}\" -DUSE_SDL)

INCLUDE(FindPkgConfig)

#Find the required libraries.
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2_MIXER REQUIRED SDL2_mixer)
PKG_SEARCH_MODULE(GAMERZILLA OPTIONAL gamerzilla)

if (GAMERZILLA_LIBRARIES)
	message(STATUS "Gamerzilla found")
	add_definitions(-DGAMERZILLA)
endif()

#Add the include directories of the (found) libraries.
Include_Directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_MIXER_INCLUDE_DIRS}
	${GAMERZILLA_INCLUDE_DIRS}
)

#Set the output path and the source path.
Set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
Set (SRC_DIR ${PROJECT_SOURCE_DIR})

#List the source files.
File (GLOB SOURCES ${SRC_DIR}/*.c)

Add_Executable (${PROJECT_NAME} ${SOURCES})

Target_Link_Libraries (
	${PROJECT_NAME}
	${SDL2_LIBRARIES}
	${SDL2_MIXER_LIBRARIES}
	${GAMERZILLA_LIBRARIES}
	${SDL2MAIN_LIBRARIES}
 -lm
)

#Install locations
Install (DIRECTORY ${PROJECT_SOURCE_DIR}/data/gamerzilla DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/cutscenes.txt DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/die.wav DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/fanfare.wav DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/graphics.bmp DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/graphics2.bmp DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/helix.wav DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/hit.wav DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/shippy.xm DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/shot.wav DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/splash.bmp DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/splash2.bmp DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/splash.wav DESTINATION ${DATAROOTDIR}/shippy/)
Install (FILES ${PROJECT_SOURCE_DIR}/data/title.xm DESTINATION ${DATAROOTDIR}/shippy/)
Install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${BINDIR})

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	Install (FILES shippy.png DESTINATION ${ICONDIR}/hicolor/64x64/apps/)
	Install (FILES shippy.desktop DESTINATION ${DATAROOTDIR}/applications/)
	Install (FILES shippy.metainfo.xml DESTINATION ${DATAROOTDIR}/metainfo/)
endif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
