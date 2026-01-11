#ifndef STRUT_H
#define STRUT_H

#define HEAD1 "head"
#include <qglobal.h>

typedef struct
{
    char    head1[4];
    quint32 cmd;
    quint32 param;
    quint32 len;
} TcpComHeadT;

typedef struct
{
    TcpComHeadT comHead;
    ushort signal[2048];   // ABABAB...
} SignalSpectrumInfo;

#endif // STRUT_H
