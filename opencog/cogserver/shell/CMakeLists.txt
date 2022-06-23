###
###ADD_LIBRARY (sexpr-shell SHARED
###	SexprShell
###	SexprShellModule
###)
###
###TARGET_LINK_LIBRARIES(sexpr-shell
###	network
###	server
###	${ATOMSPACE_LIBRARY}
###	${COGUTIL_LIBRARY}
###)
###
###INSTALL (TARGETS sexpr-shell
###	LIBRARY DESTINATION "lib${LIB_DIR_SUFFIX}/opencog/modules")
