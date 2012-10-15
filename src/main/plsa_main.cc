/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-14
 */

#include <iostream>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <toyml/topic/plsa.h>

DECLARE_int32(stderrthreshold);
//DECLARE_string(log_dir);

DEFINE_string(docpath, "../data/topic/testdocs.dat", "input file of documents");
DEFINE_string(dictpath, "../data/topic/dict.dat", "output file of dictionary");

DEFINE_int32(topics, 100, "number of topics");
DEFINE_int32(iterators, 100, "number of iterators");
DEFINE_string(topicpath, "../data/topic/topics.dat", "output file of topics");
DEFINE_string(modelpath, "../data/topic/plsa_model.dat", "output file of plsa model");

int main(int argc, char **argv) {
  FLAGS_stderrthreshold = 0;
//  FLAGS_log_dir = "../log/";
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  VLOG(0) << "------" << argv[0] << "------";

  toyml::Dataset dataset;
  CHECK(dataset.Load(FLAGS_docpath)) << "Failed to load file " << FLAGS_docpath;
  CHECK(dataset.SaveDict(FLAGS_dictpath)) << "Failed to save dictionary file " << FLAGS_dictpath;
  VLOG(0) << "dataset.StatString: " << dataset.StatString();
  VLOG(4) << "dataset.Doc(0): " << dataset.Doc(0).ToString();

  toyml::PLSA plsa;
  plsa.Init(dataset, FLAGS_topics, FLAGS_iterators);
  plsa.Train();
  CHECK(plsa.SaveTopics(FLAGS_topicpath));
  CHECK(plsa.SaveModel(FLAGS_modelpath));

  return 0;
}
