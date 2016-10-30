#ifndef INVESTMENT_H
#define INVESTMENT_H

typedef struct Investment {
  struct Investment *next;
  unsigned long end;
  int amount;
} Investment;

#endif
