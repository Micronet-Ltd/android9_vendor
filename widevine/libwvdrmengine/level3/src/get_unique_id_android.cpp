#include "level3.h"

#include <string.h>

// The function property_get is defined differently if it comes from the IN_APP
// version or from the Android OS.
#if defined(IN_APP_FASTBALL)
#include "inapp/fastball/include/properties_fastball.h"
#else
#include <cutils/properties.h>
#endif

namespace wvoec3 {

const char *getUniqueID(size_t *len) {
  static char temp_value[PROPERTY_VALUE_MAX * 2];
  int actual_len = property_get("ro.serialno", temp_value, NULL);
  if (actual_len <= 0) {
    actual_len = property_get("net.hostname", temp_value, NULL);
  }
  if (actual_len <= 0) {
    strncpy(temp_value, "0123456789abc", PROPERTY_VALUE_MAX);
  }
#if defined(IN_APP_FASTBALL) || defined(IN_APP_MOVIES)
  actual_len +=
      property_get("package.name", temp_value + actual_len, "com.google.inapp");
#endif
  *len = actual_len;
  return temp_value;
}

}  // namespace wvoec3
