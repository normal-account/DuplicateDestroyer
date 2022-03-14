include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG beb9e98806bb84bcc130a2cebfbcbbc6ce62b335) # The commit hash for 1.7.2. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

cmake_minimum_required(VERSION 3.17)
project(DuplicateDestroyer)

set(CMAKE_CXX_STANDARD 20)


add_executable(DuplicateDestroyer main.cpp api.cpp entities.cpp)

target_link_libraries(DuplicateDestroyer PRIVATE cpr::cpr)

