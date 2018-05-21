#include "sendvideo.h"


void SendVideo::StartSend()
{
	this->start();
}

void SendVideo::SendNalu(uint8_t* naluBuf, size_t len, RTPSession &sess, bool mark)
{

	uint8_t *buf = new uint8_t[len];
	memcpy(buf, naluBuf, len);

	uint8_t sendBuf[1400];
	memset(sendBuf, 0, 1400);

	int status;

	if (len <= MAX_RTP_PKT_LENGTH)
	{
		memcpy(sendBuf, buf, len);

		status = sess.SendPacket((void *)sendBuf, len, 96, false, 0);
		checkerror(status);
	}
	else if (len > MAX_RTP_PKT_LENGTH)
	{

		int k = 0, l = 0;
		k = len / MAX_RTP_PKT_LENGTH;
		l = len % MAX_RTP_PKT_LENGTH;
		int t = 0;

		char nalHeader = buf[0];

		while (t <= k)
		{
			if (t < k - 1 || (t == k - 1) && l > 0)
			{
				sendBuf[0] = (nalHeader & 0x60) | 28;
				sendBuf[1] = (nalHeader & 0x1f);
				if (0 == t)
				{
					sendBuf[1] |= 0x80;
				}

				memcpy(sendBuf + 2, &buf[t*MAX_RTP_PKT_LENGTH + 1], MAX_RTP_PKT_LENGTH);

				status = sess.SendPacket((void *)sendBuf, MAX_RTP_PKT_LENGTH + 2, 96, false, 0);
				checkerror(status);
			}
			else
			{
				sendBuf[0] = (nalHeader & 0x60) | 28;
				sendBuf[1] = (nalHeader & 0x1f);
				sendBuf[1] |= 0x40;

				if (l == 0 && t < k)
				{
					memcpy(sendBuf + 2, &buf[t*MAX_RTP_PKT_LENGTH + 1], MAX_RTP_PKT_LENGTH - 1);
					status = sess.SendPacket((void *)sendBuf, MAX_RTP_PKT_LENGTH + 1, 96, mark, mark ? TIME_STAMP_INC : 0);
					checkerror(status);

				}
				else if (l > 1)
				{
					memcpy(sendBuf + 2, &buf[t*MAX_RTP_PKT_LENGTH + 1], l - 1);
					status = sess.SendPacket((void *)sendBuf, l + 1, 96, mark, mark ? TIME_STAMP_INC : 0);
					checkerror(status);
				}
			}
			t++;
		}
	}
	delete [] buf;
}

void SendVideo::run()
{
	avdevice_register_all();
	
	AVFrame *frame = av_frame_alloc();
	AVPacket *packet = av_packet_alloc();
	AVPacket *packetH264 = av_packet_alloc();

	AVFormatContext *formatCtx = avformat_alloc_context();
	AVInputFormat *ifmt = av_find_input_format("dshow");
	avformat_open_input(&formatCtx, VIDEO_INPUT_DEVICE, ifmt, NULL);

	AVCodec *decode = avcodec_find_decoder(formatCtx->streams[0]->codecpar->codec_id);
	AVCodecContext *decodeCtx = avcodec_alloc_context3(decode);
	avcodec_parameters_to_context(decodeCtx, formatCtx->streams[0]->codecpar);
	avcodec_open2(decodeCtx, decode, NULL);

	AVCodec *encode = avcodec_find_encoder(AV_CODEC_ID_H264);
	AVCodecContext *encodeCtx = avcodec_alloc_context3(encode);
	encodeCtx->bit_rate = 4000000;
	encodeCtx->width = decodeCtx->width;
	encodeCtx->height = decodeCtx->height;
	encodeCtx->gop_size = 1;
	encodeCtx->time_base = AVRational{ 1, 25 };
	encodeCtx->pix_fmt = AV_PIX_FMT_YUV422P;
	av_opt_set(encodeCtx->priv_data, "tune", "zerolatency", 0);
	avcodec_open2(encodeCtx, encode, NULL);

	uint8_t localip[] = DEST_IP;
	RTPIPv4Address addr(localip, DEST_PORT_VIDEO);
	RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0 / 90000.0);
	RTPUDPv4TransmissionParams transparams;
	transparams.SetPortbase(BASE_PORT_VIDEO);
	RTPSession session;
	session.Create(sessionparams, &transparams);
	session.AddDestination(addr);
	session.SetDefaultPayloadType(96);
	session.SetDefaultMark(false);
	session.SetDefaultTimestampIncrement(TIME_STAMP_INC); //90000/25

	while (av_read_frame(formatCtx, packet) >= 0)
	{
		avcodec_send_packet(decodeCtx, packet);
		avcodec_receive_frame(decodeCtx, frame);
		avcodec_send_frame(encodeCtx, frame);
		avcodec_receive_packet(encodeCtx, packetH264);

		int i = 0, j = 0;
		size_t len = 0;
		while (i + 3 < packetH264->size)
		{
			if (packetH264->data[i] == 0 && packetH264->data[i + 1] == 0 &&
				packetH264->data[i + 2] == 1)
			{
				if (j < i + 3 && j != 0)
				{
					if (packetH264->data[i - 1] == 0)
						len = i - j - 1;
					else
						len = i - j;

					SendNalu(&packetH264->data[j], len, session, false);
				}
				j = i + 3;
			}
			if (i + 4 == packetH264->size)
			{
				len = i - j + 4;
				SendNalu(&packetH264->data[j], len, session, true);
			}
			i++;
		}
	}
	av_frame_free(&frame);
	av_packet_free(&packet);
	av_packet_free(&packetH264);
	avformat_free_context(formatCtx);
	avcodec_free_context(&encodeCtx);
	avcodec_free_context(&decodeCtx);
}