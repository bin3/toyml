/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-14
 */

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <toyml/tm/lda/gibbs_lda.h>

DECLARE_int32(stderrthreshold);
//DECLARE_string(log_dir);

DEFINE_string(docpath, "../data/lda/doc.dat", "input file of documents");
DEFINE_string(dictpath, "../data/lda/dict.dat", "output file of dictionary");

DEFINE_int32(topics, 10, "number of topics");
DEFINE_int32(iters, 100, "number of iterators");
DEFINE_double(eps, 0.1, "EPS");
DEFINE_int32(nlog, 10, "log interval");
DEFINE_int32(nsave, 40, "save interval");
DEFINE_string(datadir, "../data/lda/", "output data directory");
DEFINE_bool(random, false, "whether to randomly initialize probability");

int main(int argc, char **argv) {
  FLAGS_stderrthreshold = 0;
//  FLAGS_log_dir = "../log/";
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  VLOG(0) << "------" << argv[0] << "------";

  toyml::DocumentSet dataset;
  CHECK(dataset.Load(FLAGS_docpath)) << "Failed to load file " << FLAGS_docpath;
  VLOG(0) << "docpath=" << FLAGS_docpath;
  CHECK(dataset.SaveDetailedDict(FLAGS_dictpath)) << "Failed to save dictionary file " << FLAGS_dictpath;
  VLOG(0) << "DocumentSet: " << dataset.StatString();

  toyml::LDAOptions options;
  options.topics = FLAGS_topics;
  options.iters = FLAGS_iters;
  options.eps = FLAGS_eps;
  options.nlog = FLAGS_nlog;
  options.nsave = FLAGS_nsave;
  options.datadir = FLAGS_datadir;
  options.random = FLAGS_random;
  VLOG(0) << "LDAOptions: " << options.ToString();

  toyml::GibbsLDA lda;
  CHECK(lda.Init(options, dataset));
  VLOG(0) << "GibbsLDA: " << lda.ToString();

  boost::posix_time::ptime start =
      boost::posix_time::microsec_clock::local_time();

  std::size_t niters = lda.Train();

  boost::posix_time::ptime end =
      boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration elapsed = end - start;
  double duration_per_iter = static_cast<double>(elapsed.total_seconds()) / niters;
  VLOG(0) << "niters=" << niters << ", duration_per_iter=" << duration_per_iter << "s";

  return 0;
}
