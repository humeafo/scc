/*********************************************************************/
// Program Name:			Calculator
// Programmer:			Arati Rajbhandari
// modified by Humeafo
/*********************************************************************/
#include "wshlchdr.h"
#include <windows.h>
#include <math.h>

#define BUTTON0 100
#define BUTTON1 101
#define BUTTON2 102
#define BUTTON3 103
#define BUTTON4 104
#define BUTTON5 105
#define BUTTON6 106
#define BUTTON7 107
#define BUTTON8 108
#define BUTTON9 109

#define BUTTONC 110 //CLEAR EDIT BOX
#define BUTTONB 111 //BACKSPACE

#define BUTTONA 112 //ADDITION
#define BUTTONS 113 //SUBTRACTION
#define BUTTONX 114 //MULTIPLICATION
#define BUTTOND 115 //DIVISION

#define BUTTONE 116 //EQUALS
#define BUTTONO 117 //DOT
#define BUTTONH 118 //PLUS/MINUS

#define BUTTONR 119 //SQUARE ROOT
#define BUTTONF 120 //FACTORIAL
#define BUTTONP 121 //RECIPROCAL

#define EDITTXT 522 //EDIT BOX

#define MAXOUT 25
#define MAXIN  10

/**************FUNCTION PROTOTYPES********************/
void setvalue(const char* val);
void calculations(void);
double Factorial(double val);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
HWND SetUpWindow(const char* cClass, const char* cTitle, int nWidth, int nHeight, HINSTANCE hInstance);
/*****************************************************/


/***********GLOBAL PARAMETERS*************************/
HWND hWndMe,
hB0, hB1, hB2, hB3, hB4, hB5, hB6, hB7, hB8, hB9,
hE1, hBC, hBB,
hBE, hBO,
hBA, hBS, hBX, hBD,
hBR, hBF, hBP, hBH;

int nFunc = 0;     // Function in use: 0=None, 1=Add, 2=Subtract, 3=Multiply, 4=Divide
int nNew = 1;	   // New/continuing digits
double dTotal = 0; // Current buffer total
char cBuf[MAXOUT]; // char buffer
double dbOne = 1.0;
double dbZero = 0.0;
/**************FUNCTION DEFINITIONS*******************/

//SET FONT
inline void SetDefaultFont(int identifier, HWND hwnd) {
  SendDlgItemMessage(hwnd, identifier, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(true, 0));
}

int WINAPI calcMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdline, int iCmdShow) {
  const char* cClass = "Calculator";	// Name of class
  const char* cTitle = "Calculator";	// Text in window title bar
  const int nWidth = 320;			// Width of window
  const int nHeight = 320;		// Height of window

  MSG msg;

  // CREATE WINDOWS
  hWndMe = SetUpWindow(cClass, cTitle, nWidth, nHeight, hInstance);

  hB7 = CreateWindow("button", "7", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 20, 92, 40, 30, hWndMe, (HMENU)BUTTON7, hInstance, NULL);
  hB8 = CreateWindow("button", "8", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 65, 92, 40, 30, hWndMe, (HMENU)BUTTON8, hInstance, NULL);
  hB9 = CreateWindow("button", "9", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 110, 92, 40, 30, hWndMe, (HMENU)BUTTON9, hInstance, NULL);

  hB4 = CreateWindow("button", "4", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 20, 124, 40, 30, hWndMe, (HMENU)BUTTON4, hInstance, NULL);
  hB5 = CreateWindow("button", "5", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 65, 124, 40, 30, hWndMe, (HMENU)BUTTON5, hInstance, NULL);
  hB6 = CreateWindow("button", "6", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 110, 124, 40, 30, hWndMe, (HMENU)BUTTON6, hInstance, NULL);

  hB1 = CreateWindow("button", "1", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 20, 156, 40, 30, hWndMe, (HMENU)BUTTON1, hInstance, NULL);
  hB2 = CreateWindow("button", "2", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 65, 156, 40, 30, hWndMe, (HMENU)BUTTON2, hInstance, NULL);
  hB3 = CreateWindow("button", "3", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 110, 156, 40, 30, hWndMe, (HMENU)BUTTON3, hInstance, NULL);

  hB0 = CreateWindow("button", "0", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 20, 188, 40, 30, hWndMe, (HMENU)BUTTON0, hInstance, NULL);
  hBO = CreateWindow("button", ".", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 65, 188, 40, 30, hWndMe, (HMENU)BUTTONO, hInstance, NULL);
  hBH = CreateWindow("button", "+/-", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 110, 188, 40, 30, hWndMe, (HMENU)BUTTONH, hInstance, NULL);

  hBA = CreateWindow("button", "+", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 160, 92, 40, 30, hWndMe, (HMENU)BUTTONA, hInstance, NULL);
  hBS = CreateWindow("button", "-", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 160, 124, 40, 30, hWndMe, (HMENU)BUTTONS, hInstance, NULL);
  hBX = CreateWindow("button", "x", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 160, 156, 40, 30, hWndMe, (HMENU)BUTTONX, hInstance, NULL);
  hBD = CreateWindow("button", "/", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 160, 188, 40, 30, hWndMe, (HMENU)BUTTOND, hInstance, NULL);

  hBR = CreateWindow("button", "sqrt", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 205, 92, 40, 30, hWndMe, (HMENU)BUTTONR, hInstance, NULL);
  hBF = CreateWindow("button", "x!", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 205, 124, 40, 30, hWndMe, (HMENU)BUTTONF, hInstance, NULL);
  hBP = CreateWindow("button", "1/x", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 205, 156, 40, 30, hWndMe, (HMENU)BUTTONP, hInstance, NULL);
  hBE = CreateWindow("button", "=", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 205, 188, 40, 30, hWndMe, (HMENU)BUTTONE, hInstance, NULL);

  hE1 = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", WS_CHILD | WS_VISIBLE | ES_RIGHT, 20, 20, 225, 25, hWndMe, (HMENU)EDITTXT, hInstance, NULL);
  hBB = CreateWindow("button", "Backspace", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 20, 55, 130, 30, hWndMe, (HMENU)BUTTONB, hInstance, NULL);
  hBC = CreateWindow("button", "C", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 160, 55, 85, 30, hWndMe, (HMENU)BUTTONC, hInstance, NULL);

  cBuf[0] = 0;
  SetWindowText(hE1, "0.");

  ShowWindow(hWndMe, iCmdShow);
  UpdateWindow(hWndMe);

  SetDefaultFont(BUTTON0, hWndMe);
  SetDefaultFont(BUTTON1, hWndMe);
  SetDefaultFont(BUTTON2, hWndMe);
  SetDefaultFont(BUTTON3, hWndMe);
  SetDefaultFont(BUTTON4, hWndMe);
  SetDefaultFont(BUTTON5, hWndMe);
  SetDefaultFont(BUTTON6, hWndMe);
  SetDefaultFont(BUTTON7, hWndMe);
  SetDefaultFont(BUTTON8, hWndMe);
  SetDefaultFont(BUTTON9, hWndMe);

  SetDefaultFont(BUTTONC, hWndMe);
  SetDefaultFont(BUTTONB, hWndMe);

  SetDefaultFont(BUTTONA, hWndMe);
  SetDefaultFont(BUTTONS, hWndMe);
  SetDefaultFont(BUTTONX, hWndMe);
  SetDefaultFont(BUTTOND, hWndMe);

  SetDefaultFont(BUTTONE, hWndMe);
  SetDefaultFont(BUTTONO, hWndMe);
  SetDefaultFont(BUTTONH, hWndMe);

  SetDefaultFont(BUTTONR, hWndMe);
  SetDefaultFont(BUTTONF, hWndMe);
  SetDefaultFont(BUTTONP, hWndMe);

  SetDefaultFont(EDITTXT, hWndMe);

  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
  static bool dot;
  static bool backsp;

  char cTemp[MAXOUT];

  switch (iMsg) {
    case WM_DESTROY:				// Program closed
    {
      PostQuitMessage(0);
      return 0;
      break;
    }
    case WM_COMMAND:
    {
      if (lParam && LOWORD(wParam) == EDITTXT) {
        switch (HIWORD(wParam)) {
          case EN_SETFOCUS:
          {
            SetFocus(hWndMe);
            break;
          }
        }
        break;
      }
      switch (LOWORD(wParam)) {
        case BUTTON0:			// BUTTON0 clicked
        {
          if (nNew) {
            cBuf[0] = 0;
            SetWindowText(hE1, "0.");
            nNew = 0;
          }

          if ((strlen(cBuf) > 0) && (strlen(cBuf) < MAXIN)) {
            strcat(&cBuf[0], "0");
            SetWindowText(hE1, cBuf);
          }

          backsp = true;

          break;
        }
        case BUTTON1:			// BUTTON1 clicked
        {
          setvalue("1");
          backsp = true;

          break;
        }
        case BUTTON2:			// BUTTON2 clicked
        {
          setvalue("2");
          backsp = true;

          break;
        }
        case BUTTON3:			// BUTTON3 clicked
        {
          setvalue("3");
          backsp = true;

          break;
        }
        case BUTTON4:			// BUTTON4 clicked
        {
          setvalue("4");
          backsp = true;

          break;
        }
        case BUTTON5:			// BUTTON5 clicked
        {
          setvalue("5");
          backsp = true;

          break;
        }
        case BUTTON6:			// BUTTON6 clicked
        {
          setvalue("6");
          backsp = true;

          break;
        }
        case BUTTON7:			// BUTTON7 clicked
        {
          setvalue("7");
          backsp = true;
          break;
        }
        case BUTTON8:			// BUTTON8 clicked
        {
          setvalue("8");
          backsp = true;
          break;
        }
        case BUTTON9:			// BUTTON9 clicked
        {
          setvalue("9");
          backsp = true;
          break;
        }
        case BUTTONO:			// Dot clicked
        {
          if (nNew) {
            cBuf[0] = 0;
            SetWindowText(hE1, cBuf);
            nNew = 0;
          }

          if ((!dot) && (strlen(cBuf) < MAXIN)) {
            strcat(&cBuf[0], ".");
            SetWindowText(hE1, cBuf);
            dot = true;
          }

          backsp = true;

          break;
        }
        case BUTTONC:					// BUTTON C clicked
        {
          dTotal = 0;					// Clear total
          cBuf[0] = 0;				// Clear char buffer
          SetWindowText(hE1, "0.");
          nNew = 1;					// Set new digit
          nFunc = 0;					// set function to none
          dot = false;
          backsp = false;
          break;
        }
        case BUTTONB:					// Backspace clicked
        {
          if (backsp) {
            if (strlen(cBuf) > 1) {
              char cLast[1];
              int iSLen = strlen(cBuf);
              cLast[0] = cBuf[iSLen - 1];
              if (strncmp(cLast, ".", 1) == 0) {
                dot = false;
              }
              strncpy(cTemp, cBuf, (iSLen - 1));
              cTemp[(iSLen - 1)] = '\0';
              strcpy(cBuf, cTemp);
              SetWindowText(hE1, cBuf);
            } else {
              if (strlen(cBuf) == 1) {
                cBuf[0] = 0;		// Clear char buffer
                SetWindowText(hE1, "0.");
                nNew = 1;			// Set new digit
                dot = false;
              }
            }
          }
          break;
        }
        case BUTTONA:				// '+' clicked
        {
          calculations();
          nFunc = 1;				// Set function to add
          dot = false;
          backsp = false;

          break;
        }
        case BUTTONS:				// '-' clicked
        {
          calculations();
          nFunc = 2;				// Set function to subtract
          dot = false;
          backsp = false;
          break;
        }
        case BUTTONX:				// 'x' clicked
        {
          calculations();
          nFunc = 3;				// Set function to multiply
          dot = false;
          backsp = false;
          break;
        }
        case BUTTOND:				// '/' clicked
        {
          calculations();
          nFunc = 4;				// Set function to divide
          dot = false;
          backsp = false;
          break;
        }
        case BUTTONE:				// '=' clicked
        {
          calculations();
          nFunc = 0;
          dot = false;
          backsp = false;
          break;
        }
        case BUTTONR:				// 'sqrt' clicked
        {
          double dRoot;

          GetWindowText(hE1, cBuf, MAXOUT);
          dRoot = sqrt(atof(cBuf));

          cBuf[0] = 0;			// Empty char buffer
          _gcvt(dRoot, MAXOUT, cBuf);
          SetWindowText(hE1, cBuf);
          nNew = 1;				// Set new digit
          dot = false;
          backsp = false;

          break;
        }
        case BUTTONP:				// '1/x' clicked
        {
          double dRecip;

          GetWindowText(hE1, cBuf, MAXOUT);
          dRecip = dbOne / atof(cBuf);

          cBuf[0] = 0;			// Empty char buffer
          _gcvt(dRecip, MAXOUT, cBuf);
          SetWindowText(hE1, cBuf);
          nNew = 1;				// Set new digit
          dot = false;
          backsp = false;
          break;
        }
        case BUTTONF:				// 'x!' clicked
        {
          double dFact;
          double dFraction, dInt;

          GetWindowText(hE1, cBuf, MAXOUT);

          if (atof(cBuf) < dbZero) {
            strcpy(cBuf, "-E-");
            dTotal = dbZero;
          } else {
            dFraction = modf(atof(cBuf), &dInt);

            if ((dFraction == dbZero) && (dInt >= dbZero)) {
              dFact = Factorial(atof(cBuf));
              cBuf[0] = 0;			// Empty char buffer
              _gcvt(dFact, MAXOUT, cBuf);
            } else {
              strcpy(cBuf, "-E-");
              dTotal = dbZero;
            }
          }

          SetWindowText(hE1, cBuf);
          nNew = 1;					// Set new digit
          dot = false;
          backsp = false;
          break;
        }
        case BUTTONH:
        {
          double dSignChg;

          GetWindowText(hE1, cBuf, MAXOUT);
          dSignChg = 0 - atof(cBuf);

          cBuf[0] = 0;				// Empty char buffer
          _gcvt(dSignChg, MAXOUT, cBuf);
          SetWindowText(hE1, cBuf);
          nNew = 1;					// Set new digit
          dot = false;
          backsp = false;
          break;
        }
      }
      break;
    }
  }

  return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

HWND SetUpWindow(const char* cClass, const char* cTitle, int nWidth, int nHeight, HINSTANCE hInstance) {
  WNDCLASS wClass;
  HWND hWnd;

  wClass.style = CS_HREDRAW | CS_VREDRAW;
  wClass.lpfnWndProc = WndProc;
  wClass.cbClsExtra = 0;
  wClass.cbWndExtra = 0;
  wClass.hInstance = hInstance;
  wClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wClass.lpszMenuName = NULL;
  wClass.lpszClassName = cClass;

  RegisterClass(&wClass);

  hWnd = CreateWindow(cClass, cTitle, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, NULL);

  return hWnd;
}

void setvalue(const char* val) {
  if (nNew) {
    cBuf[0] = 0;
    SetWindowText(hE1, cBuf);
    nNew = 0;
  }

  if (strlen(cBuf) < MAXIN) {
    strcat(&cBuf[0], val);
    SetWindowText(hE1, cBuf);
  }
}

void calculations() {
  switch (nFunc) {
    case 0:
    {
      GetWindowText(hE1, cBuf, MAXOUT);
      dTotal = atof(cBuf);
      break;
    }
    case 1:
    {
      GetWindowText(hE1, cBuf, MAXOUT);
      dTotal += atof(cBuf);
      break;
    }
    case 2:
    {
      GetWindowText(hE1, cBuf, MAXOUT);
      dTotal -= atof(cBuf);
      break;
    }
    case 3:
    {
      GetWindowText(hE1, cBuf, MAXOUT);
      dTotal = dTotal * atof(cBuf);
      break;
    }
    case 4:
    {
      GetWindowText(hE1, cBuf, MAXOUT);
      dTotal = dTotal / atof(cBuf);
      break;
    }
  }
  cBuf[0] = 0;			// Empty char buffer
  _gcvt(dTotal, MAXOUT, cBuf);
  SetWindowText(hE1, cBuf);
  nNew = 1;				// Set new digit
}

double Factorial(double val) {
  int i;
  double f = dbOne;

  for (i = dbOne; i <= int(val); ++i)
    f = f*i;
  return f;
}

void scmain() {
  impFixNameCommon();
  calcMain(GetModuleHandle(NULL), NULL, NULL, TRUE);
  ExitProcess(0);
}

int main(int argc, char* argv[]) {
  /*
  pscmain scentry_ptr = 0;
  void* scentry_args[MAX_SCMAIN_ARGS_COUNT];

  scentry_ptr must be set and some args is allowed if you fill in scmain_args array
  */
  scentry_ptr = (pscmain)scmain;
  return mainDefault(argc, argv, 0);
  return 0;
}
