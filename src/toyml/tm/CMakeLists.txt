set(lib toyml_tm)
set(srcs
  dataset.cc
  utils.cc
  plsa/plsa.cc
  plsa/ex_plsa.cc
  plsa/background_plsa.cc
  lda/lda.cc
  lda/gibbs_lda.cc
)

add_library(${lib} ${srcs})
target_link_libraries(${lib} glog)

