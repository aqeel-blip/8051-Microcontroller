#include <stdio.h>

extern int a;   // Declaration: a is defined somewhere else

int main()
{
    printf("%d", a);   // Using/calling the variable
    return 0;
}