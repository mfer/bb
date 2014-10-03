#ifndef BREAK_BEAL_H_INCLUDED
#define BREAK_BEAL_H_INCLUDED

#include "BigIntegerLibrary.hh"

BigInteger z;
BigInteger x;
BigInteger y;
BigInteger c;
BigInteger a;
BigInteger b;

BigInteger mdc(BigInteger n1, BigInteger n2);
BigInteger bigPow(BigInteger base, BigInteger expoent);
BigInteger mdc3(BigInteger n1, BigInteger n2, BigInteger n3);
int break_beal(BigInteger bmin, BigInteger bmax, BigInteger pmin, BigInteger pmax);

#endif // BREAK_BEAL_H_INCLUDED
