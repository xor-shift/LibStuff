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

add_library(${PROJECT_NAME}
        Inc/Stuff/Files/Format.hpp

        Inc/Stuff/Graphics/Image/QoI.ipp
        Inc/Stuff/Graphics/Image.hpp
        Inc/Stuff/Graphics/Traits.hpp

        Inc/Stuff/IO/ACS.hpp
        Inc/Stuff/IO/Delim.hpp
        Inc/Stuff/IO/GPS.hpp
        Inc/Stuff/IO/Nextion.hpp
        Inc/Stuff/IO/NTC.hpp
        Inc/Stuff/IO/Packetman.hpp
        Inc/Stuff/IO/SoftUART.hpp

        Inc/Stuff/Maths/BLAS/Concepts.hpp
        Inc/Stuff/Maths/BLAS/Matrix.hpp
        Inc/Stuff/Maths/BLAS/MatVec.hpp
        Inc/Stuff/Maths/BLAS/Quat.hpp
        Inc/Stuff/Maths/BLAS/Util.hpp
        Inc/Stuff/Maths/BLAS/Vector.hpp

        Inc/Stuff/Maths/Scalar/Basic.hpp
        Inc/Stuff/Maths/Scalar/Classification.hpp
        Inc/Stuff/Maths/Scalar/Exponential.hpp
        Inc/Stuff/Maths/Scalar/FloatUtils.hpp
        Inc/Stuff/Maths/Scalar/Interpolation.hpp
        Inc/Stuff/Maths/Scalar/Manipulation.hpp
        Inc/Stuff/Maths/Scalar/Power.hpp
        Inc/Stuff/Maths/Scalar/Trig.hpp

        Inc/Stuff/Maths/Bit.hpp
        Inc/Stuff/Maths/CRC.hpp
        Inc/Stuff/Maths/Fmt.hpp
        Inc/Stuff/Maths/Random.hpp
        Inc/Stuff/Maths/Scalar.hpp

        Inc/Stuff/Refl/ReflNew.hpp
        Inc/Stuff/Refl/Serde.hpp

        Inc/Stuff/Serde/Adapters/BEncode.hpp
        Inc/Stuff/Serde/Adapters/JSON.hpp
        Inc/Stuff/Serde/Refl/Refl.hpp
        Inc/Stuff/Serde/BasicSerializers.hpp
        Inc/Stuff/Serde/Concepts.hpp
        Inc/Stuff/Serde/Serde.hpp

        Inc/Stuff/Util/Hacks/Concepts.hpp
        Inc/Stuff/Util/Hacks/Coroutines.hpp
        Inc/Stuff/Util/Hacks/Expected.hpp
        Inc/Stuff/Util/Hacks/Try.hpp
        Inc/Stuff/Util/Alloc.hpp
        Inc/Stuff/Util/Conv.hpp
        Inc/Stuff/Util/DummyIterator.hpp
        Inc/Stuff/Util/Hash.hpp
        Inc/Stuff/Util/Scope.hpp
        Inc/Stuff/Util/SpinLock.hpp
        Inc/Stuff/Util/Timer.hpp
        Inc/Stuff/Util/Tuple.hpp
        Inc/Stuff/Util/UTF8.hpp
        Inc/Stuff/Util/Visitor.hpp
        )

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
    endif ()

endif ()

### EXAMPLES ###

if (LibStuffCompileExamples)

    add_executable(${PROJECT_NAME}_examples_GPS
            Examples/GPS.cpp)
    target_link_libraries(${PROJECT_NAME}_examples_GPS
            ${PROJECT_NAME})

endif ()
