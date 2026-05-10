#include "ConsoleColor.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace ConsoleColor {
void enableAnsiOnWindows() {
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;
    mode |= 0x0004;  // ENABLE_VIRTUAL_TERMINAL_PROCESSING
    SetConsoleMode(hOut, mode);
#endif
}
}  // namespace ConsoleColor
