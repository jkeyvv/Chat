#pragma once
#include <QObject>
#include <Qtdebug>
#include <QIODevice>
#include <QtCore/QThread>
#include <QtGui/Qpixmap>
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>
#include <QtMultimedia/QAudioformat>
#include <QtMultimedia/QAudiooutput>

#include <rtpsession.h>
#include <rtpsessionparams.h>
#include <rtpipv4address.h>
#include <rtpudpv4transmitter.h>
#include <rtperrors.h>
#include <rtplibraryversion.h>
#include <rtppacket.h>

#define __STDC_CONSTANT_MACROS
#define DEST_IP					{127, 0, 0, 1}
#define DEST_PORT_VIDEO			10040
#define BASE_PORT_VIDEO			10020
#define DEST_PORT_AUDIO			10080
#define BASE_PORT_AUDIO			10060
#define MAX_RTP_PKT_LENGTH		1300
#define TIME_STAMP_INC			3600
#define FRAME_BUF_SIZE_VIDEO	100000
#define FRAME_BUF_SIZE_AUDIO	1024
#define VIDEO_INPUT_DEVICE		u8"video=BisonCam, NB Pro"
#define AUDIO_INPUT_DEVICE		u8"audio=Âó¿Ë·ç (Realtek High Definition Audio)"

extern "C"
{
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
};

using namespace std;
using namespace jrtplib;

void checkerror(int rtperr);
