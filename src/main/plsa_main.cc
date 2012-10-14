/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-14
 */

#include <iostream>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <toyml/topic/dataset.h>

DECLARE_int32(stderrthreshold);
DECLARE_string(log_dir);

DEFINE_string(fname, "../data/topic/trndocs.dat", "input file of documents");

int main(int argc, char **argv) {
//  std::cout << "------" << argv[0] << "------" << std::endl;
  FLAGS_stderrthreshold = 0;
//  FLAGS_log_dir = "../log/";
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  VLOG(0) << "------" << argv[0] << "------";

  toyml::Dataset dataset;
  CHECK(dataset.Load(FLAGS_fname)) << "Failed to load file " << FLAGS_fname;
  VLOG(0) << dataset.StatString();

  return 0;
}
