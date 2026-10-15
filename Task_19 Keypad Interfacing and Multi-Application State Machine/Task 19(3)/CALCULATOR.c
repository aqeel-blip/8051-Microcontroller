#include "calculator.h"

long Calculate(long num1, long num2, char op) {
    switch(op) {
        case '+': return num1 + num2;
        case '-': return num1 - num2;
        case '*': return num1 * num2;
        case '/': return (num2 != 0) ? (num1 / num2) : 0;
        default:  return 0;
    }
}