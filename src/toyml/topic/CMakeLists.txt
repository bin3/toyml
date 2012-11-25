set(lib toyml_topic)
set(srcs
  dataset.cc
  plsa.cc
  ex_plsa.cc
  background_plsa.cc
  utils.cc
)

add_library(${lib} ${srcs})
target_link_libraries(${lib} glog)

add_subdirectory(lda)
