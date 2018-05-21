#pragma once
#include "configure.h"
#include "ui_chat.h"
#include "sendvideo.h"
#include "recvvideo.h"
#include "sendaudio.h"
#include "recvaudio.h"


class Chat : public QWidget
{
	Q_OBJECT

public:
	Chat(QWidget *parent = Q_NULLPTR);

private:

	Ui::chat ui;
	QIODevice *pAudioOutDevice;

private slots:

	void OpenAudioDevice();
	void HandleVideo(const QImage img);
	void HandleAudio(const QByteArray array);
};
