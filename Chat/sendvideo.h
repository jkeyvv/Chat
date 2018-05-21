#pragma once
#include "configure.h"

class SendVideo : public QThread
{
	Q_OBJECT

public slots:

	void StartSend();

private:

	void SendNalu(uint8_t* pNaluBuf, size_t len, RTPSession &sess, bool mark);

protected:

	void run();
};