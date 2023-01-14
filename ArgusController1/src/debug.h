
//---------------------------------------------------------
// Argus Controller (Open Hardware)
// debug.h
// Copyright 2020-2023 Argotronic GmbH
//
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, ShareAlike
//---------------------------------------------------------

#ifndef _DEBUG_H_

#ifdef DEBUG_OUTPUT

template <class T> inline void dbgPrint(T str)
{
    Serial.print(str);
}
template <class T> inline void dbgPrintln(T str)
{
    dbgPrint(str);
    dbgPrint(F("\n"));
}
inline void dbgHex(uint8_t b)
{
    if (b < 16) {
        Serial.print('0');
    }
    Serial.print(b, HEX);
}
template <typename TYPE> inline void dbgDec(TYPE b)
{
    Serial.print(b, DEC);
}
template <typename TYPE> inline void dbgDecln(TYPE b)
{
    dbgDec(b);
    dbgPrint(F("\n"));
}

#else

#define dbgPrint(str)
#define dbgPrintln(str)
#define dbgHex(b...)
#define dbgDec(b)
#define dbgDecln(b)

#endif

#endif
