cmake_minimum_required(VERSION 2.8)

project(HLLib)

set(HLLIB_MAJOR_VERSION 2)
set(HLLIB_MINOR_VERSION 3)
set(HLLIB_PATCH_VERSION 0)

set(HLLIB_NAME HLLib${HLLIB_MAJOR_VERSION}${HLLIB_MINOR_VERSION})
set(HLLIB_VERSION ${HLLIB_MAJOR_VERSION}.${HLLIB_MINOR_VERSION}.${HLLIB_PATCH_VERSION})

# ---- options ----------------------------------------------------------------
option(BUILD_DOCS "Build doxygen documentation" OFF)

# ---- compiler flags ---------------------------------------------------------
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -funroll-loops -fvisibility=hidden -O3")
endif()

# compiling the library now:
add_definitions(-DHLLIB_EXPORTS)

# ---- pkg config -------------------------------------------------------------
# for CMAKE_INSTALL_LIBDIR:
include(GNUInstallDirs)
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate_symlink_code.cmake")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/HLLib.pc.in
	${CMAKE_CURRENT_BINARY_DIR}/${HLLIB_NAME}.pc @ONLY)

generate_symlink_code(HLLIB_PC_INSTALL_CODE
	${HLLIB_NAME}.pc HLLib.pc
	${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/${HLLIB_NAME}.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(CODE ${HLLIB_PC_INSTALL_CODE})

# ---- library target ---------------------------------------------------------
set(hllib_SRCS BSPFile.cpp Checksum.cpp DebugMemory.cpp DirectoryFile.cpp
	DirectoryFolder.cpp DirectoryItem.cpp Error.cpp FileMapping.cpp FileStream.cpp
	GCFFile.cpp GCFStream.cpp HLLib.cpp Mapping.cpp MappingStream.cpp MemoryMapping.cpp
	MemoryStream.cpp NCFFile.cpp NullStream.cpp Package.cpp PAKFile.cpp ProcStream.cpp
	SGAFile.cpp Stream.cpp StreamMapping.cpp Utility.cpp VBSPFile.cpp VPKFile.cpp
	WADFile.cpp Wrapper.cpp XZPFile.cpp ZIPFile.cpp)

set(hllib_HDRS BSPFile.h Checksum.h DebugMemory.h DirectoryFile.h DirectoryFolder.h
	DirectoryItem.h DirectoryItems.h Error.h FileMapping.h FileStream.h GCFFile.h
	GCFStream.h HLLib.h Mapping.h Mappings.h MappingStream.h MemoryMapping.h MemoryStream.h
	NCFFile.h NullStream.h Package.h Packages.h PAKFile.h ProcStream.h resource.h
	SGAFile.h stdafx.h Stream.h StreamMapping.h Streams.h Utility.h VBSPFile.h VPKFile.h
	WADFile.h Wrapper.h XZPFile.h ZIPFile.h)

add_library(hllib SHARED ${hllib_SRCS} ${hllib_HDRS} HLLib.rc)
set_target_properties(hllib PROPERTIES SOVERSION ${HLLIB_MAJOR_VERSION} VERSION ${HLLIB_VERSION})

install(TARGETS hllib DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../lib/HLLib.h" DESTINATION "include/${HLLIB_NAME}")

# ---- docs tareget -----------------------------------------------------------
if(BUILD_DOCS)
	find_package(Doxygen)
	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen is needed to build the documentation.")
	endif()

	configure_file(Doxyfile.in
		"${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)

	add_custom_target(docs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
		SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif()

# ---- uninstall target -------------------------------------------------------
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
