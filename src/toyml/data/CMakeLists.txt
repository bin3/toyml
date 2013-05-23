set(lib toyml_data)
set(srcs
  data_transformer.cpp
)

add_library(${lib} ${srcs})

add_test(csv_test)
