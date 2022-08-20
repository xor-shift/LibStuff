cmake_minimum_required(VERSION 3.22)
project(libstuff)

set(CMAKE_CXX_STANDARD 23)

option(LibStuffUseFMT "" ON)
option(LibStuffCompileTests "" ON)
option(LibStuffCompileBenchmarks "" ON)
option(LibStuffCompileExamples "" ON)

set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
set(${CMAKE_EXE_LINKER_FLAGS} "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC Inc)

target_sources(${PROJECT_NAME} PUBLIC
        Src/IO/Delim.cpp
        Src/IO/GPS.cpp
        Src/IO/SoftUART.cpp
        )

if (LibStuffUseFMT)
    target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
    target_compile_definitions(${PROJECT_NAME} PUBLIC LIBSTUFF_FMT)
endif ()

### TESTS ###

if (LibStuffCompileTests)

    add_subdirectory(Thirdparty/googletest)

    add_executable(${PROJECT_NAME}_tests
            Tests/Graphics/Image.cpp

            Tests/IO/Delim.cpp
            Tests/IO/GPS.cpp
            Tests/IO/Packetman.cpp

            Tests/Maths/Bit.cpp
            Tests/Maths/CRC.cpp
            Tests/Maths/Scalar.cpp
            Tests/Maths/Vector.cpp

            Tests/Refl/Serde.cpp

            Tests/Serde/Serde.cpp

            Tests/Util/Alloc.cpp
            Tests/Util/Conv.cpp
            Tests/Util/Scope.cpp
            Tests/Util/UTF8.cpp
            )

    target_link_libraries(${PROJECT_NAME}_tests
            gtest gtest_main
            ${PROJECT_NAME}
            )

endif ()

### BENCHMARKS ###

if (LibStuffCompileBenchmarks)

    add_subdirectory(Thirdparty/benchmark)

    add_executable(${PROJECT_NAME}_benchmarks
            Benchmarks/main.cpp

            Benchmarks/Gfx/Util/Alloc.cpp
            #Benchmarks/Gfx/Image/QoI.cpp

            Benchmarks/Maths/Random.cpp
            )

    target_link_libraries(${PROJECT_NAME}_benchmarks
            benchmark
            ${PROJECT_NAME})

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME}_benchmarks PUBLIC
                -fsanitize=address -fsanitize=undefined)

        target_link_options(${PROJECT_NAME}_benchmarks PUBLIC
                -fsanitize=address -fsanitize=undefined)
    endif()

endif ()

### EXAMPLES ###

if (LibStuffCompileExamples)

    add_executable(${PROJECT_NAME}_examples_GPS
            Examples/GPS.cpp)
    target_link_libraries(${PROJECT_NAME}_examples_GPS
            ${PROJECT_NAME})

endif ()
