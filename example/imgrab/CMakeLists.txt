# -----------------------------------------------------------------------------
# 
# -----------------------------------------------------------------------------

# ...
cmake_minimum_required(VERSION 3.0)
project(imgrab_ps3)

# ...
SET(CAM_DRIVER ../../driver/PS3Eye_Experiments/driver/)
include_directories(${CAM_DRIVER})

# ...
execute_process(
  COMMAND pkg-config --cflags libusb-1.0 
  OUTPUT_VARIABLE LIBUSB_CFLAGS
)

# ...
find_package(OpenCV REQUIRED)

# ...
add_compile_options(${LIBUSB_CFLAGS})
add_compile_options(-std=c++11 -o3 - Wall -g)

# ...
file(GLOB SOURCES *.cpp ${CAM_DRIVER}/*.cpp)

# ...
add_executable(imgrab_ps3 ${SOURCES})
target_link_libraries(imgrab_ps3 usb-1.0 pthread ${OpenCV_LIBS} zmq czmq)
