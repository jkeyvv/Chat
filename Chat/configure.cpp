#include "configure.h"

void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		qDebug() << QString::fromStdString(RTPGetErrorString(rtperr)) << endl;
	}
}