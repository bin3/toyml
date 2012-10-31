add_executable(plsa_main plsa_main.cc)
target_link_libraries(plsa_main toyml_topic glog gflags)

add_executable(dataset_main dataset_main.cc)
target_link_libraries(dataset_main toyml_topic glog gflags)

add_executable(explsa_main explsa_main.cc)
target_link_libraries(explsa_main toyml_topic boost_thread glog gflags)

add_executable(background_plsa_main background_plsa_main.cc)
target_link_libraries(background_plsa_main toyml_topic glog gflags)