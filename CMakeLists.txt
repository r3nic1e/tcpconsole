cmake_minimum_required(VERSION 2.8)
project(tcpconsole)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(SOURCE_FILES
		defines.h
		drop_caches.c
		drop_caches.h
		dump.c
		dump.h
		error.c
        error.h
		help.c
		help.h
		io.c
		io.h
		kill.c
		kill.h
		serve.c
		serve.h
		sysrq.c
		sysrq.h
        tc.c
		)

option(VERSION dev)
add_definitions(-DVERSION="${VERSION}")

add_executable(tcpconsole ${SOURCE_FILES})
