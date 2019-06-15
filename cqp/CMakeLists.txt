set(HDRS cqp.h options.h symtab.h tree.h eval.h corpmanag.h
	regex2dfa.h output.h
	ranges.h builtins.h treemacros.h
	groups.h targets.h matchlist.h
	concordance.h
	parse_actions.h attlist.h context_descriptor.h
	print-modes.h ascii-print.h sgml-print.h html-print.h latex-print.h
	variables.h print_align.h
	macro.h hash.h
	table.h
	../CQi/server.h ../CQi/auth.h ../CQi/cqi.h
	)

set(SRCS cqp.c symtab.c eval.c tree.c options.c corpmanag.c
	regex2dfa.c output.c ranges.c builtins.c groups.c targets.c
	matchlist.c
	concordance.c
	parse_actions.c attlist.c context_descriptor.c
	print-modes.c ascii-print.c sgml-print.c html-print.c latex-print.c
	variables.c print_align.c
	macro.c hash.c
	table.c
	../CQi/server.c ../CQi/auth.c
	parser.y
	parser.l
	${HDRS}
	)

if(MSVC)
	set(SRCS
		../include/msvc.h
		${SRCS})
endif()

BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c)
FLEX_TARGET(Scanner parser.l ${CMAKE_CURRENT_SOURCE_DIR}/lex.yy.c)
set(SRCS
	${BISON_Parser_OUTPUTS}
	${FLEX_Scanner_OUTPUTS}
	${SRCS})

add_library(cqp SHARED ${SRCS})
target_link_libraries(cqp PUBLIC cqpcl)
set_target_properties(cqp PROPERTIES COMPILE_DEFINITIONS LIBCQP_EXPORTS=1)

add_executable(cqp-bin llquery.c)
target_link_libraries(cqp-bin PRIVATE cqp)
set_target_properties(cqp-bin PROPERTIES OUTPUT_NAME cqp)

add_executable(cqpcl-bin cqpcl.c)
target_link_libraries(cqpcl-bin PRIVATE cqp)
set_target_properties(cqpcl-bin PROPERTIES OUTPUT_NAME cqpcl)

add_executable(cqpserver ../CQi/cqpserver.c)
target_link_libraries(cqpserver PRIVATE cqp)

install(TARGETS cqp cqp-bin cqpcl-bin cqpserver
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
