# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../sdk/native/3rdparty/libs/arm64-v8a)
set(OpenCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../sdk/native/jni)
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../FFTW_Install)
set(FFTW3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../fftw-3.3.8)
add_subdirectory(${FFTW3_DIR} ${CMAKE_CURRENT_BINARY_DIR}/fftw_build)
include_directories(${FFTW3_DIR}/api)
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_definitions(-D USE_SINGLE)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../SH-3DScaner-linux)
find_package(3DScaner-core REQUIRED)
include_directories(${CORE_DIR}/include)

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             native-lib.cpp
             ImageProcess.cpp
             OptFlow.cpp
             ${CORE_DIR}/src/optflow_FFT.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       jnigraphics
                       ${OpenCV_LIBRARIES}
                       fftw3
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )