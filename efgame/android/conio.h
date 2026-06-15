/* Android/Linux stand-in for <conio.h> (kbhit/getch — BehavEd-only paths). */
#include "ef_android_compat.h"
static inline int kbhit(void){ return 0; }
static inline int getch(void){ return 0; }
