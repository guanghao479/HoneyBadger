make clean
make
GLOG_log_dir="/tmp/HB_log" GLOG_minloglevel=0 GLOG_stderrthreshold=2 ./server
