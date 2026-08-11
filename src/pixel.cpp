#include "pixel.h"

// -----------------------------------------------------------------
// Some Helper Methods useful for testing the code or building it
// -----------------------------------------------------------------

#if defined(_WIN32) || defined(__MINGW32__)
#include <windows.h>
// Windows API uses 1 argument and ignores POSIX permissions completely
#define MAKE_DIR(path)                                                         \
  (CreateDirectoryA(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
#else
#include <sys/stat.h>
#include <sys/types.h>
// POSIX systems (Linux/macOS) use 2 arguments
#define MAKE_DIR(path) (mkdir(path, 0777) == 0)
#endif

void Pixel_CreateDirectories(const char *path) {
  char tempPath[512];
  strncpy(tempPath, path, sizeof(tempPath) - 1);
  tempPath[sizeof(tempPath) - 1] = '\0';

  for (char *p = tempPath; *p; p++) {
    if (*p == '/' || *p == '\\') {
      if (p == tempPath)
        continue;
      char originalChar = *p;
      *p = '\0';          // Temporarily truncate the string
      MAKE_DIR(tempPath); // Invoke the safe macro
      *p = originalChar;  // Restore character
    }
  }
}

void Pixel_SwapInt(uint32_t *value1, uint32_t *value2) {
  int temp = *value1;
  *value1 = *value2;
  *value2 = temp;
}
