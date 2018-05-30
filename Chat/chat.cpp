#include "chat.h"

Chat::Chat(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	SendVideo *sendVideo = new SendVideo;
	RecvVideo *recvVideo = new RecvVideo;
	SendAudio *sendAudio = new SendAudio;
	RecvAudio *recvAudio = new RecvAudio;

	connect(ui.start, &QPushButton::clicked, this, &Chat::OpenAudioDevice);
	connect(ui.start, &QPushButton::clicked, sendVideo, &SendVideo::StartSend);
	connect(ui.start, &QPushButton::clicked, sendAudio, &SendAudio::StartSend);
	connect(ui.start, &QPushButton::clicked, recvVideo, &RecvVideo::StartRecv);
	connect(ui.start, &QPushButton::clicked, recvAudio, &RecvAudio::StartRecv);

	connect(recvVideo, &RecvVideo::VideoReady, this, &Chat::HandleVideo);
	connect(recvAudio, &RecvAudio::AudioReady, this, &Chat::HandleAudio);
}

void Chat::OpenAudioDevice()
{
	QAudioFormat audioFormat;
	audioFormat.setChannelCount(2);
	audioFormat.setSampleRate(44100);
	audioFormat.setByteOrder(QAudioFormat::LittleEndian);
	audioFormat.setSampleSize(16);
	audioFormat.setSampleType(QAudioFormat::UnSignedInt);
	audioFormat.setCodec("audio/pcm");
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
	if (!info.isFormatSupported(audioFormat)) {
		qDebug() << "default format not supported try to use nearest";
		audioFormat = info.nearestFormat(audioFormat);
	}
	QAudioOutput *audioOutput = new QAudioOutput(audioFormat, this);
	this->pAudioOutDevice = audioOutput->start();
}

void Chat::HandleVideo(const QImage img)
{
	ui.label->setPixmap(QPixmap::fromImage(img));
}

void Chat::HandleAudio(const QByteArray array)
{
	this->pAudioOutDevice->write(array);
}