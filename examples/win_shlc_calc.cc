#include "wshlchdr.h"

void scmain() {

  // fix the imports before you call any API and ensure that the data are writeable
  //impFixDefaultK32Only();
  impFixNameCommon();
  WinExec("calc", SW_SHOWNORMAL);
  ExitProcess(0); 
}

int main(int argc, char *argv[]) {
  /*
  pscmain scentry_ptr = 0;
  void* scentry_args[MAX_SCMAIN_ARGS_COUNT];

  scentry_ptr must be set and some args is allowed if you fill in scmain_args array
  */
  scentry_ptr = (pscmain)scmain;
  return mainDefault(argc, argv, 0);
}
