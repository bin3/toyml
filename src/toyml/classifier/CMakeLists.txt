set(lib toyml_classifier)
set(srcs
  classifier.cpp
  perceptron.cpp
)

add_library(${lib} ${srcs})
target_link_libraries(${lib} glog)

