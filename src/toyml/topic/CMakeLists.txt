set(lib toyml_topic)
set(srcs
  dataset.cc
  plsa.cc
  ex_dataset.cc
  ex_plsa.cc
)

add_library(${lib} ${srcs})
target_link_libraries(${lib} glog)
