/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <toyml/topic/ex_plsa.h>

DEFINE_string(docpath, "../data/explsa/doc.dat.test", "input file of documents");
DEFINE_string(followeepath, "../data/explsa/followee.dat.test", "input file of documents");
DEFINE_string(dictpath, "../data/explsa/dict.dat", "output file of dictionary");
DEFINE_string(celpath, "../data/explsa/cel.dat", "output file of celebrities");

DEFINE_int32(topics, 10, "number of topics");
DEFINE_int32(iters, 100, "number of maximum iterations");
DEFINE_double(eps, 0.1, "EPS");
DEFINE_double(lambda, 0.2, "for background");
DEFINE_int32(log_interval, 1, "log interval");
DEFINE_int32(em_log_interval, 1000, "EMStep log interval");
DEFINE_int32(save_interval, 40, "save interval");
DEFINE_int32(threads, 0, "the number of threads");
DEFINE_string(datadir, "../data/explsa/", "output data directory");
DEFINE_bool(random, false, "whether to randomly initialize probability");

int main(int argc, char **argv) {
  FLAGS_stderrthreshold = 0;
//  FLAGS_log_dir = "log/";
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  VLOG(0) << "------" << argv[0] << "------";

  toyml::Dataset document_data;
  CHECK(document_data.Load(FLAGS_docpath)) << "Failed to load document file " << FLAGS_docpath;
  VLOG(0) << "docpath=" << FLAGS_docpath;
  CHECK(document_data.SaveDetailedDict(FLAGS_dictpath)) << "Failed to save dictionary file " << FLAGS_dictpath;
  VLOG(0) << "document_data: " << document_data.StatString();

  toyml::Dataset followee_data;
  CHECK(followee_data.Load(FLAGS_followeepath)) << "Failed to load followee file " << FLAGS_followeepath;
  VLOG(0) << "followeepath=" << FLAGS_followeepath;
  CHECK(followee_data.SaveDetailedDict(FLAGS_celpath)) << "Failed to save celebrities file " << FLAGS_dictpath;
  VLOG(0) << "followee_data: " << followee_data.StatString();

  toyml::ExPLSAOptions options;
  options.ntopics = FLAGS_topics;
  options.niters = FLAGS_iters;
  options.lambda = FLAGS_lambda;
  options.eps = FLAGS_eps;
  options.log_interval = FLAGS_log_interval;
  options.save_interval = FLAGS_save_interval;
  options.em_log_interval = FLAGS_em_log_interval;
  options.threads = FLAGS_threads ? FLAGS_threads : boost::thread::hardware_concurrency();
  options.datadir = FLAGS_datadir;
  options.random = FLAGS_random;
  VLOG(0) << "options: " << options.ToString();

  toyml::ExPLSA explsa;
  CHECK(explsa.Init(options, document_data, followee_data));
  VLOG(0) << "ExPLSA: " << explsa.ToString();
  boost::posix_time::ptime start =
      boost::posix_time::microsec_clock::local_time();
  std::size_t niters = explsa.Train();
  boost::posix_time::ptime end =
      boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration elapsed = end - start;
  double duration_per_iter = static_cast<double>(elapsed.total_seconds()) / niters;
  VLOG(0) << "niters=" << niters << ", duration_per_iter=" << duration_per_iter << "s";

  return 0;
}
