include(FetchContent)
link_directories(/usr/local/mysql/connector-c++-8.0.28/lib64/debug) # necessary for mysql connector lib linking
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG beb9e98806bb84bcc130a2cebfbcbbc6ce62b335) # The commit hash for 1.7.2. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

cmake_minimum_required(VERSION 3.17)
project(DuplicateDestroyer)

find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_CXX_STANDARD 20)

add_executable(DuplicateDestroyer src/main.cpp src/api_wrapper.cpp src/reddit_entities.cpp src/image_manipulation.cpp)

SET(OpenCV_LIBS opencv_core opencv_highgui opencv_imgproc) # linker errors without this line

find_library(mysqlcppconn_LIB
        mysqlcppconn8
        HINTS /usr/local/mysql/connector-c++-8.0.28/lib64/debug)
#/usr/local/mysql/connector-c++-8.0.28/lib64/debug

target_link_libraries(DuplicateDestroyer cpr::cpr ${OpenCV_LIBS} gmpxx gmp ${mysqlcppconn_LIB})


### Libs
# OpenCv (Package manager)
# GMP (package manager
# JSON lib (just copy the folder)
# mysql connector 8.0 (Build from source : https://dev.mysql.com/doc/connector-cpp/8.0/en/connector-cpp-installation-source-cpp.html

# Boost installation path : /usr/include/boost
#