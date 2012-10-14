set(lib toyml_topic)
set(srcs
  dataset.cc
  plsa.cc
)

add_library(${lib} ${srcs})
target_link_libraries(${lib} glog)
