#pragma once
#include "configure.h"

class RecvAudio : public QThread
{
	Q_OBJECT

public slots:
	void StartRecv();
signals:
	void AudioReady(const QByteArray arry);
protected:
	void run();
private:
	void AddADTSToPacket(uint8_t *buf, uint8_t *packet, int packetLen);
};
