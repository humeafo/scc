#include "wshlchdr.h"

void scmain() {

  // fix the imports before you call any API and ensure that the data are writeable
  //impFixDefaultK32Only();
  impFixNameCommon();
  WinExec("calc", SW_SHOWNORMAL);
  ExitProcess(0); 
}

int main(int argc, char *argv[]) {
  return mainDefault(argc, argv, 0);
}
