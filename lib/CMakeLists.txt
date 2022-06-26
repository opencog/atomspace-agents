
include(CMakePackageConfigHelpers)

set(ConfigPackageLocation lib/cmake/CogAgents)

install(EXPORT CogAgentsTargets
	FILE CogAgentsTargets.cmake
	DESTINATION ${ConfigPackageLocation}
)

configure_package_config_file(CogAgentsConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/CogAgentsConfig.cmake
	INSTALL_DESTINATION ${ConfigPackageLocation}
	PATH_VARS CMAKE_INSTALL_PREFIX
)

SET(SEMANTIC_VERSION 0.1.0)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/CogAgentsConfigVersion.cmake"
	VERSION ${SEMANTIC_VERSION}
	COMPATIBILITY SameMajorVersion
)

INSTALL(FILES
	${CMAKE_CURRENT_BINARY_DIR}/CogAgentsConfigVersion.cmake
	${CMAKE_CURRENT_BINARY_DIR}/CogAgentsConfig.cmake
	DESTINATION ${ConfigPackageLocation}
)
