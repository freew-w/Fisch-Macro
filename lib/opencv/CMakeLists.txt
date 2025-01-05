add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE ../../include/)
target_include_directories(opencv INTERFACE ../../include/opencv2/)
target_link_directories(opencv INTERFACE ./)
target_link_libraries(opencv INTERFACE
opencv_world4100
libwebp libtiff
libjpeg-turbo
IlmImf
zlib
libpng
libopenjp2
ade
libprotobuf
)
