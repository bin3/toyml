/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-31
 */

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <toyml/topic/background_plsa.h>

DECLARE_int32(stderrthreshold);
//DECLARE_string(log_dir);

DEFINE_string(docpath, "../data/bplsa/doc.dat", "input file of documents");
DEFINE_string(dictpath, "../data/bplsa/dict.dat", "output file of dictionary");
DEFINE_string(wprobpath, "../data/bplsa/wprob.dat", "output file of word probabilities");
DEFINE_string(top_wprobpath, "../data/bplsa/top_wprob.dat", "output file of top word probabilities");
DEFINE_int32(topn_wprob, 100, "topn of word probabilities");

DEFINE_int32(topics, 30, "number of topics");
DEFINE_int32(iterators, 100, "number of iterators");
DEFINE_double(eps, 0.1, "EPS");
DEFINE_double(lambda, 0.8, "weight of background model");
DEFINE_double(delta, 1e-3, "Delta added to loglikelihood");
DEFINE_int32(log_interval, 1, "log interval");
DEFINE_int32(save_interval, 40, "save interval");
DEFINE_string(datadir, "../data/bplsa/", "output data directory");
DEFINE_bool(random, true, "whether to randomly initialize probability");

int main(int argc, char **argv) {
  FLAGS_stderrthreshold = 0;
//  FLAGS_log_dir = "../log/";
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  VLOG(0) << "------" << argv[0] << "------";

  toyml::Dataset dataset;
  CHECK(dataset.Load(FLAGS_docpath)) << "Failed to load file " << FLAGS_docpath;
  VLOG(0) << "docpath=" << FLAGS_docpath;
  CHECK(dataset.SaveDict(FLAGS_dictpath)) << "Failed to save dictionary file " << FLAGS_dictpath;
  CHECK(dataset.SaveWordProb(FLAGS_wprobpath)) << "Failed to save word probabilities file " << FLAGS_wprobpath;
  CHECK(dataset.SaveTopWordProb(FLAGS_top_wprobpath, FLAGS_topn_wprob)) << "Failed to save top word probabilities file " << FLAGS_top_wprobpath;
  VLOG(0) << "dataset.StatString: " << dataset.StatString();
  VLOG(4) << "dataset.Doc(0): " << dataset.Doc(0).ToString();

  toyml::BackgroundPLSAOptions options;
  options.ntopics = FLAGS_topics;
  options.niters = FLAGS_iterators;
  options.eps = FLAGS_eps;
  options.lambda = FLAGS_lambda;
  options.delta = FLAGS_delta;
  options.log_interval = FLAGS_log_interval;
  options.save_interval = FLAGS_save_interval;
  options.datadir = FLAGS_datadir;
  options.random = FLAGS_random;
  VLOG(0) << "options: " << options.ToString();

  toyml::BackgroundPLSA plsa;
  CHECK(plsa.Init(options, dataset));
  boost::posix_time::ptime start =
      boost::posix_time::microsec_clock::local_time();
  std::size_t niters = plsa.Train();
  boost::posix_time::ptime end =
      boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration elapsed = end - start;
  double duration_per_iter = static_cast<double>(elapsed.total_seconds()) / niters;
  VLOG(0) << "niters=" << niters << ", duration_per_iter=" << duration_per_iter << "s";

  return 0;
}
