#include "recvaudio.h"

void RecvAudio::StartRecv()
{
	this->start();
}

void RecvAudio::AddADTSToPacket(uint8_t *buf, uint8_t *packet, int packetLen)
{
	int profile = 2; //lz
	int freqIdx = 4; //44100Hz
	int channels = 2;
	int bufLen = packetLen + 7;

	buf[0] = (byte)0xFF;
	buf[1] = (byte)0xF9;
	buf[2] = (byte)(((profile - 1) << 6) + (freqIdx << 2) + (channels >> 2));
	buf[3] = (byte)(((channels & 3) << 6) + (bufLen >> 11));
	buf[4] = (byte)((bufLen & 0x7FF) >> 3);
	buf[5] = (byte)(((bufLen & 7) << 5) + 0x1F);
	buf[6] = (byte)0xFC;

	memcpy(buf + 7, packet, packetLen);
}

void RecvAudio::run()
{
	avdevice_register_all();

	AVFrame *frame = av_frame_alloc();
	AVFrame *framePCMS16 = av_frame_alloc();
	AVPacket *packet = av_packet_alloc();
	SwrContext *swrCtx = swr_alloc();

	AVCodec *decode = avcodec_find_decoder(AV_CODEC_ID_AAC);
	AVCodecContext *decodeCtx = avcodec_alloc_context3(decode);
	avcodec_open2(decodeCtx, decode, NULL);

	RTPSession session;
	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessionparams;
	RTPPacket *pack;
	sessionparams.SetOwnTimestampUnit(1.0 / 44100);
	sessionparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(DEST_PORT_AUDIO);
	session.Create(sessionparams, &transparams);

	uint8_t frameBuf[FRAME_BUF_SIZE_AUDIO];
	uint8_t *payLoadData;
	size_t payLoadLength;

	for (;;)
	{
		session.BeginDataAccess();
		if (session.GotoFirstSourceWithData())
		{
			while ((pack = session.GetNextPacket()) != NULL)
			{
				payLoadData = pack->GetPayloadData();
				payLoadLength = pack->GetPayloadLength();

				AddADTSToPacket(frameBuf, payLoadData + 4, payLoadLength - 4);

				av_new_packet(packet, payLoadLength + 3);
				packet->data = frameBuf;
				packet->size = payLoadLength + 3;

				avcodec_send_packet(decodeCtx, packet);
				avcodec_receive_frame(decodeCtx, frame);

				framePCMS16->format = AV_SAMPLE_FMT_S16;
				framePCMS16->sample_rate = frame->sample_rate;
				framePCMS16->nb_samples = frame->nb_samples;
				framePCMS16->channels = 2;
				framePCMS16->channel_layout = AV_CH_LAYOUT_STEREO;
				av_frame_get_buffer(framePCMS16, 1);

				av_opt_set_int(swrCtx, "ich", decodeCtx->channels, 0);
				av_opt_set_int(swrCtx, "och", 2, 0);
				av_opt_set_channel_layout(swrCtx, "in_channel_layout", decodeCtx->channel_layout, 0);
				av_opt_set_channel_layout(swrCtx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
				av_opt_set_int(swrCtx, "in_sample_rate", decodeCtx->sample_rate, 0);
				av_opt_set_int(swrCtx, "out_sample_rate", decodeCtx->sample_rate, 0);
				av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", decodeCtx->sample_fmt, 0);
				av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
				swr_init(swrCtx);

				swr_convert(swrCtx, framePCMS16->data, framePCMS16->nb_samples,
					(const uint8_t **)frame->data, frame->nb_samples);

				QByteArray array((char *)framePCMS16->data[0], framePCMS16->linesize[0]);
				emit AudioReady(array);
				msleep(23);
			}
		}
		session.EndDataAccess();
	}
	av_frame_free(&frame);
	av_frame_free(&framePCMS16);
	av_packet_free(&packet);
	avcodec_free_context(&decodeCtx);
	swr_free(&swrCtx);
}