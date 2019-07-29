#include "timer_metric.h"

#include <stddef.h>
#include <sys/time.h>

namespace wvcdm {
namespace metrics {

void TimerMetric::Start() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  sec_ = tv.tv_sec;
  usec_ = tv.tv_usec;
  is_started_ = true;
}

void TimerMetric::Clear() {
  is_started_ = false;
  sec_ = 0;
  usec_ = 0;
}

double TimerMetric::AsMs() const {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return usec_ > tv.tv_usec ?
      (tv.tv_sec - sec_ - 1) * 1000.0 + (tv.tv_usec - usec_ + 1000000.0) / 1000.0 :
      (tv.tv_sec - sec_) * 1000.0 + (tv.tv_usec - usec_) / 1000.0;
}

double TimerMetric::AsUs() const {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return usec_ > tv.tv_usec ?
      (tv.tv_sec - sec_ - 1) * 1000000.0 + (tv.tv_usec - usec_ + 1000000.0) :
      (tv.tv_sec - sec_) * 1000000.0 + (tv.tv_usec - usec_);
}

}  // namespace metrics
}  // namespace wvcdm
