set(lib toyml_classifier)
set(srcs
  classifier.cpp
  perception.cpp
)

add_library(${lib} ${srcs})
target_link_libraries(${lib} glog)

