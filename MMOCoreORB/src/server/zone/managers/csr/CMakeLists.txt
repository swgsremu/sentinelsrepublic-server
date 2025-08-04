# Copyright (C) 2007-2019 SWGEmu
# See file COPYING for copying conditions.

set(server_zone_managers_csr_sources
    CSRCommandProcessor.cpp
)

set(server_zone_managers_csr_headers
    CSRCommandProcessor.h
)

include_directories(.)

if(ENABLE_REST_API)
    set(server_zone_managers_csr_sources ${server_zone_managers_csr_sources})
endif()

add_library(zone_managers_csr STATIC ${server_zone_managers_csr_sources} ${server_zone_managers_csr_headers})