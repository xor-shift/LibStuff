cmake_minimum_required(VERSION 3.22)
project(libstuff)

set(CMAKE_CXX_STANDARD 23)

option(LibStuffUseFMT "" ON)
option(LibStuffCompileTests "" ON)
option(LibStuffCompileBenchmarks "" ON)
option(LibStuffCompileExamples "" ON)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

add_subdirectory(Thirdparty/expected)

set(LIBSTUFF_HEADERS
        Inc/Stuff/Files/Format.hpp

        Inc/Stuff/Graphics/Image.hpp
        Inc/Stuff/Graphics/Traits.hpp

        Inc/Stuff/Intro/Introspectors/Array.hpp
        Inc/Stuff/Intro/Introspectors/Map.hpp
        Inc/Stuff/Intro/Introspectors/Span.hpp
        Inc/Stuff/Intro/Introspectors/Tuple.hpp
        Inc/Stuff/Intro/Introspectors/UnorderedMap.hpp
        Inc/Stuff/Intro/Introspectors/Vector.hpp
        Inc/Stuff/Intro/AggregateExplorers.hpp
        Inc/Stuff/Intro/Dump.hpp
        Inc/Stuff/Intro/HelperTypes.hpp
        Inc/Stuff/Intro/Intro.hpp
        Inc/Stuff/Intro/Introspector.hpp
        Inc/Stuff/Intro/SAgg.hpp

        Inc/Stuff/IO/ACS.hpp
        Inc/Stuff/IO/Delim.hpp
        Inc/Stuff/IO/GPS.hpp
        Inc/Stuff/IO/Nextion.hpp
        Inc/Stuff/IO/NTC.hpp
        Inc/Stuff/IO/SoftUART.hpp

        Inc/Stuff/Maths/BLAS/Concepts.hpp
        Inc/Stuff/Maths/BLAS/Matrix.hpp
        Inc/Stuff/Maths/BLAS/MatVec.hpp
        Inc/Stuff/Maths/BLAS/Quat.hpp
        Inc/Stuff/Maths/BLAS/Util.hpp
        Inc/Stuff/Maths/BLAS/Vector.hpp
        Inc/Stuff/Maths/Check/CRC.hpp
        Inc/Stuff/Maths/Crypt/DES/Funcs.hpp
        Inc/Stuff/Maths/Crypt/DES/SBox.hpp
        Inc/Stuff/Maths/Crypt/DES/Tables.hpp
        Inc/Stuff/Maths/Crypt/DES.hpp
        Inc/Stuff/Maths/Crypt/Tripcode.hpp
        Inc/Stuff/Maths/Scalar/Basic.hpp
        Inc/Stuff/Maths/Scalar/Classification.hpp
        Inc/Stuff/Maths/Scalar/Exponential.hpp
        Inc/Stuff/Maths/Scalar/FloatUtils.hpp
        Inc/Stuff/Maths/Scalar/Interpolation.hpp
        Inc/Stuff/Maths/Scalar/Manipulation.hpp
        Inc/Stuff/Maths/Scalar/Power.hpp
        Inc/Stuff/Maths/Scalar/Trig.hpp
        Inc/Stuff/Maths/Bit.hpp
        Inc/Stuff/Maths/Fmt.hpp
        Inc/Stuff/Maths/Random.hpp
        Inc/Stuff/Maths/Scalar.hpp
        )

add_library(${PROJECT_NAME} ${LIBSTUFF_HEADERS}
        Src/IO/Delim.cpp
        Src/IO/GPS.cpp
        Src/IO/SoftUART.cpp

        Src/Util/CPUID/Features.cpp
        Src/Util/MMap.cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${LIBSTUFF_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC Inc)

target_link_libraries(${PROJECT_NAME} PUBLIC expected)

if (LibStuffUseFMT)
    target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
    target_compile_definitions(${PROJECT_NAME} PUBLIC LIBSTUFF_FMT)
endif ()

### TESTS ###

if (LibStuffCompileTests)

    add_subdirectory(Thirdparty/googletest)

    add_executable(${PROJECT_NAME}_tests
            Tests/Graphics/Image.cpp

            Tests/Intro/Intro.cpp

            Tests/IO/Delim.cpp
            Tests/IO/GPS.cpp

            Tests/Maths/Bit.cpp
            Tests/Maths/CRC.cpp
            Tests/Maths/DES.cpp
            Tests/Maths/Hash.cpp
            Tests/Maths/Scalar.cpp
            Tests/Maths/Vector.cpp

            Tests/Serde/Serde.cpp

            Tests/Util/Alloc.cpp
            Tests/Util/Conv.cpp
            Tests/Util/Scope.cpp
            Tests/Util/Tuple.cpp
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

    add_executable(${PROJECT_NAME}_benchmark_des Benchmarks/main.cpp Benchmarks/Maths/DES.cpp)
    target_link_libraries(${PROJECT_NAME}_benchmark_des ${PROJECT_NAME} benchmark)
    target_compile_options(${PROJECT_NAME}_benchmark_des PRIVATE -march=native -mtune=native)

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
    endif ()

endif ()

### EXAMPLES ###

if (LibStuffCompileExamples)

    add_executable(${PROJECT_NAME}_examples_GPS
            Examples/GPS.cpp)
    target_link_libraries(${PROJECT_NAME}_examples_GPS
            ${PROJECT_NAME})

    add_executable(${PROJECT_NAME}_examples_CPUID
            Examples/CPUID.cpp)
    target_link_libraries(${PROJECT_NAME}_examples_CPUID
            ${PROJECT_NAME})

endif ()
