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
  /*
  pscmain scentry_ptr = 0;
  void* scentry_args[MAX_SCMAIN_ARGS_COUNT];

  scentry_ptr must be set and some args is allowed if you fill in scmain_args array
  */
  scentry_ptr = (pscmain)scmain;
  return mainDefault(argc, argv, 0);
}
