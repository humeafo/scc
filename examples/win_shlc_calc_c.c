#include "wshlchdr.h"

void scmain() {
  impFixHashK32Only();
  WinExec("calc", SW_SHOWNORMAL);
#ifndef _WIN64
  ExitProcess(0);
#endif
}

int main(int argc, char *argv[]) {
  impFixMeth = impFix_hash;
  return mainDefault(argc, argv, 0);
}
