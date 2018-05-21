#pragma once
#include "configure.h"

class SendAudio : public QThread
{
	Q_OBJECT

public slots:

	void StartSend();

private:

	void AddRtpHeaderToPacket(uint8_t *buf, uint8_t *packet, int packetLen);

protected:

	void run();
};