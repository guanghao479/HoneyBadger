make clean
make
if [ ! -d "/tmp/HB_log" ]; then
  mkdir "/tmp/HB_log"
  echo "/tmp/HB_log created"
fi
GLOG_log_dir="/tmp/HB_log" GLOG_minloglevel=0 GLOG_stderrthreshold=2 ./server
