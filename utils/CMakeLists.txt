foreach(src atoi itoa makeall decode lexdecode huffcode compress-rdx
	s-encode s-decode describe-corpus decode-nqrfile scan-corpus
	align-show align-encode align-decode
	#check-input
	)
	add_executable("cwb-${src}" "cwb-${src}.c")
	target_link_libraries("cwb-${src}" PRIVATE cqpcl)

	install(TARGETS "cwb-${src}"
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		)
endforeach()

add_executable(cwb-encode cwb-encode.c ${DIRENT})
target_link_libraries(cwb-encode PRIVATE cqpcl)

add_executable(cwb-align cwb-align.c barlib.h barlib.c feature_maps.h feature_maps.c)
target_link_libraries(cwb-align PRIVATE cqpcl)

install(TARGETS cwb-encode cwb-align
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
