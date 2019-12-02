set(HDRS globals.h macros.h
	list.h lexhash.h ngram-hash.h
	bitfields.h storage.h fileutils.h
	special-chars.h regopt.h
	corpus.h attributes.h makecomps.h
	cdaccess.h
	bitio.h
	endian.h
	compression.h
	binsert.h
	class-mapping.h
	cl.h
	)

set(SRCS globals.c macros.c
	list.c lexhash.c ngram-hash.c
	bitfields.c storage.c fileutils.c
	special-chars.c regopt.c
	corpus.c attributes.c makecomps.c
	cdaccess.c
	bitio.c
	endian.c
	compression.c
	binsert.c
	class-mapping.c
	registry.y
	registry.l
	${HDRS}
	)

if(MSVC)
	set(SRCS
		windows-mmap.c windows-mmap.h
		../include/msvc.h
		${SRCS})
endif()

BISON_TARGET(Parser registry.y ${CMAKE_CURRENT_SOURCE_DIR}/registry.tab.c
	COMPILE_FLAGS "-p creg -v")
FLEX_TARGET(Scanner registry.l ${CMAKE_CURRENT_SOURCE_DIR}/lex.creg.c
	COMPILE_FLAGS "-Pcreg")
set(SRCS
	${BISON_Parser_OUTPUTS}
	${FLEX_Scanner_OUTPUTS}
	${SRCS})

add_library(cqpcl SHARED ${SRCS})
target_link_libraries(cqpcl PUBLIC ${GLIB_LIBRARIES} ${PCRE_LIB} ${GTOD_LIB} ${GETOPT_LIB} ${XXHASH_LIB})
set_target_properties(cqpcl PROPERTIES COMPILE_DEFINITIONS LIBCQPCL_EXPORTS=1)
if(WIN32)
	target_link_libraries(cqpcl PUBLIC "wsock32.lib")
endif()
if(NOT MSVC)
	target_link_libraries(cqpcl PUBLIC "m")
endif()

install(TARGETS cqpcl
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
