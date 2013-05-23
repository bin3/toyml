set(lib toyml_dl)
set(srcs
  layer.cpp
  sigmoid_layer.cpp
  neural_network.cpp
)

add_library(${lib} ${srcs})

