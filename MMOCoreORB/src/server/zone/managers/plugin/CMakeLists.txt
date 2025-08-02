# Plugin System Headers
set(PLUGIN_HEADERS
    PluginInterface.h
    EventDispatcher.h
    PluginLoader.h
)

# Export headers for inclusion by other components
foreach(header ${PLUGIN_HEADERS})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${header}
                   ${CMAKE_BINARY_DIR}/include/server/zone/managers/plugin/${header}
                   COPYONLY)
endforeach()

# Add to parent scope
set(ZONE_MANAGER_SOURCES ${ZONE_MANAGER_SOURCES} PARENT_SCOPE)