add_executable(plsa_main plsa_main.cc)
target_link_libraries(plsa_main toyml_topic glog gflags)

add_executable(dataset_main dataset_main.cc)
target_link_libraries(dataset_main toyml_topic glog gflags)