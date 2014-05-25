#include <stdio.h>

class A {
public:
  A() {}
  int a = 'A';
};

class D : public A {
public:
  D() {}
  int b = 'D';
};

int main(int argc, char *argv[]) {
  A a;
  D d;

  D* pd = &d;
  A* pa = pd;

  pd->a = 'A';
  printf("Hello %d\n", pa->a);
  return 0;
}
