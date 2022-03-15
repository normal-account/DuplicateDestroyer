include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG beb9e98806bb84bcc130a2cebfbcbbc6ce62b335) # The commit hash for 1.7.2. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

cmake_minimum_required(VERSION 3.17)
project(DuplicateDestroyer)

find_package( OpenCV REQUIRED ) # of course, OpenCV needs to be installed on the system

include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_CXX_STANDARD 20)

add_executable(DuplicateDestroyer src/main.cpp src/api_wrapper.cpp src/reddit_entities.cpp src/image_manipulation.cpp)

SET(OpenCV_LIBS opencv_core opencv_highgui opencv_imgproc) # linker errors without this line

target_link_libraries(DuplicateDestroyer cpr::cpr ${OpenCV_LIBS} gmpxx gmp)


