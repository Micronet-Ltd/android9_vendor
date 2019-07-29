#ifndef WVCDM_METRICS_TIMER_METRIC_H_
#define WVCDM_METRICS_TIMER_METRIC_H_

#include <stdint.h>

namespace wvcdm {
namespace metrics {

class TimerMetric {

 public:
  // Starts the clock running. If the clock was previously set, this resets it.
  // IsStarted will return true after this call.
  void Start();
  // Returns whether or not the timer has started.
  bool IsStarted() const { return is_started_; };
  // Stops the clock and clears the current value.  IsStarted will return false
  // after this call.
  void Clear();
  // Returns the current clock value as milliseconds (AsMs) or microseconds
  // (AsUs).
  double AsMs() const;
  double AsUs() const;

 private:
  double sec_;
  double usec_;
  bool is_started_;

};

}  // namespace metrics
}  // namespace wvcdm
#endif
