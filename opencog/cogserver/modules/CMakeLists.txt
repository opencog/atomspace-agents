###
###ADD_LIBRARY (wthru-shell SHARED
###	WriteThruShell
###	WriteThruShellModule
###)
###
###TARGET_LINK_LIBRARIES(wthru-shell
###	network
###	server
###	${ATOMSPACE_LIBRARY}
###	${COGUTIL_LIBRARY}
###)
###
###INSTALL (TARGETS wthru-shell
###	LIBRARY DESTINATION "lib${LIB_DIR_SUFFIX}/opencog/modules")
