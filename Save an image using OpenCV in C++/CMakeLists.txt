project(tcamopencvsaveimage)
 
cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

set(LOCAL_INSTALATION_DIRECTORY "$ENV{HOME}/.local")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${LOCAL_INSTALATION_DIRECTORY}/lib/pkgconfig")
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
pkg_check_modules(TCAMLIB tcam)

include_directories( include ${CMAKE_CURRENT_BINARY_DIR} ${GSTREAMER_INCLUDE_DIRS} ${TCAMLIB_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

link_directories(${LOCAL_INSTALATION_DIRECTORY}/lib)
add_executable(tcamopencvsaveimage main.cpp tcamcamera.cpp common.cpp image_saver.cpp)
#add_executable(tcamopencvsaveimage main.cpp tcamcamera.cpp)

target_link_libraries(tcamopencvsaveimage ${TCAMLIB_LIBRARIES} ${GSTREAMER_LIBRARIES} ${OpenCV_LIBS} )

#install(TARGETS tcamopencvsaveimage RUNTIME DESTINATION bin)