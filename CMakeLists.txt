cmake_minimum_required(VERSION 3.22)
project(libstuff)

set(CMAKE_CXX_STANDARD 23)

option(LibStuffUseFMT "" ON)

add_library(${PROJECT_NAME}
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
        Inc/Stuff/Maths/Scalar/Other.hpp
        Inc/Stuff/Maths/Scalar/Scalar.hpp
        Inc/Stuff/Maths/Scalar/Trig.hpp
        Inc/Stuff/Maths/Bit.hpp
        Inc/Stuff/Maths/CRC.hpp
        Inc/Stuff/Maths/Fmt.hpp
        Inc/Stuff/Maths/Maths.hpp
        Inc/Stuff/Maths/Scalar.hpp
        Inc/Stuff/Maths/Std.hpp

        Inc/Stuff/Refl/Refl.hpp
        Inc/Stuff/Refl/Serde.hpp

        Inc/Stuff/Util/Conv.hpp
        Inc/Stuff/Util/DummyIterator.hpp
        Inc/Stuff/Util/Error.hpp
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

add_subdirectory(Thirdparty/googletest)

add_executable(${PROJECT_NAME}_tests
        Tests/IO/Delim.cpp
        Tests/IO/GPS.cpp
        Tests/IO/Packetman.cpp

        Tests/Maths/Bit.cpp
        Tests/Maths/CRC.cpp
        Tests/Maths/Scalar.cpp
        Tests/Maths/Vector.cpp

        Tests/Refl/Serde.cpp

        Tests/Util/Conv.cpp
        )

target_link_libraries(${PROJECT_NAME}_tests
        gtest gtest_main
        ${PROJECT_NAME}
        )

### EXAMPLES ###

add_executable(${PROJECT_NAME}_examples_GPS
        Examples/GPS.cpp)
target_link_libraries(${PROJECT_NAME}_examples_GPS
        ${PROJECT_NAME})