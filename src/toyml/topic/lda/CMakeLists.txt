set(lib toyml_lda)
set(srcs
  lda.cc
  gibbs_lda.cc
)

add_library(${lib} ${srcs})
target_link_libraries(${lib} toyml_topic glog)
