#pragma once
#include "configure.h"

class RecvVideo : public QThread
{
	Q_OBJECT

public slots:
	void StartRecv();
signals:
	void VideoReady(const QImage img);
protected:
	void run();
};
