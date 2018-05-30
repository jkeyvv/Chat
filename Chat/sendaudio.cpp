#include "sendaudio.h"

void SendAudio::StartSend()
{
	this->start();
}

void SendAudio::AddRtpHeaderToPacket(uint8_t *buf, uint8_t *packet, int packetLen)
{
	buf[0] = (byte)0x00;
	buf[1] = (byte)0x10;
	buf[2] = (byte)(packetLen >> 5);
	buf[3] = (byte)(packetLen << 3);

	memcpy(buf + 4, packet, packetLen);
}

void SendAudio::run()
{
	avdevice_register_all();

	AVFrame *frame = av_frame_alloc();
	AVFrame *frameFLTP = av_frame_alloc();
	AVFrame *frameTmp = av_frame_alloc();
	AVPacket *packet = av_packet_alloc();
	AVPacket *packetAAC = av_packet_alloc();
	AVFormatContext *formatCtx = avformat_alloc_context();

	AVInputFormat *ifmt = av_find_input_format("dshow");
	avformat_open_input(&formatCtx, AUDIO_INPUT_DEVICE, ifmt, NULL);
	AVCodec *decode = avcodec_find_decoder(formatCtx->streams[0]->codecpar->codec_id);
	AVCodecContext *decodeCtx = avcodec_alloc_context3(decode);
	avcodec_parameters_to_context(decodeCtx, formatCtx->streams[0]->codecpar);
	avcodec_open2(decodeCtx, decode, NULL);

	AVCodec *encode = avcodec_find_encoder(AV_CODEC_ID_AAC);
	AVCodecContext *encodeCtx = avcodec_alloc_context3(encode);
	encodeCtx->sample_rate = decodeCtx->sample_rate;
	encodeCtx->time_base = decodeCtx->time_base;
	encodeCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	if (encodeCtx->channel_layout == 0)
	{
		encodeCtx->channel_layout = AV_CH_LAYOUT_STEREO;
		encodeCtx->channels = av_get_channel_layout_nb_channels(encodeCtx->channel_layout);
	}
	avcodec_open2(encodeCtx, encode, NULL);

	SwrContext *swrCtx = swr_alloc();
	av_opt_set_int(swrCtx, "ich", decodeCtx->channels, 0);
	av_opt_set_int(swrCtx, "och", encodeCtx->channels, 0);
	av_opt_set_channel_layout(swrCtx, "in_channel_layout", decodeCtx->channel_layout, 0);
	av_opt_set_channel_layout(swrCtx, "out_channel_layout", encodeCtx->channel_layout, 0);
	av_opt_set_int(swrCtx, "in_sample_rate", decodeCtx->sample_rate, 0);
	av_opt_set_int(swrCtx, "out_sample_rate", encodeCtx->sample_rate, 0);
	av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", decodeCtx->sample_fmt, 0);
	av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", encodeCtx->sample_fmt, 0);
	swr_init(swrCtx);

	uint8_t localip[] = { 127,0,0,1 };
	RTPIPv4Address addr(localip, DEST_PORT_AUDIO);
	RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0 / 44100.0);
	RTPUDPv4TransmissionParams transparams;
	transparams.SetPortbase(BASE_PORT_AUDIO);
	RTPSession session;
	session.Create(sessionparams, &transparams);
	session.AddDestination(addr);
	session.SetDefaultPayloadType(96);
	session.SetDefaultMark(false);
	session.SetDefaultTimestampIncrement(1024);

	while (av_read_frame(formatCtx, packet) >= 0)
	{
		avcodec_send_packet(decodeCtx, packet);
		avcodec_receive_frame(decodeCtx, frame);

		frameFLTP->format = encodeCtx->sample_fmt;
		frameFLTP->nb_samples = frame->nb_samples;
		frameFLTP->channels = encodeCtx->channels;
		frameFLTP->channel_layout = AV_CH_LAYOUT_STEREO;
		av_frame_get_buffer(frameFLTP, 1);

		swr_convert(swrCtx, frameFLTP->data, frameFLTP->nb_samples,
			(const uint8_t **)frame->data, frame->nb_samples);

		AVAudioFifo *fifo = av_audio_fifo_alloc(encodeCtx->sample_fmt, frameFLTP->channels, frameFLTP->nb_samples);
		av_audio_fifo_write(fifo, (void **)frameFLTP->data, frameFLTP->nb_samples);

		while (av_audio_fifo_size(fifo) >= encodeCtx->frame_size)
		{
			frameTmp->nb_samples = encodeCtx->frame_size;
			frameTmp->channel_layout = encodeCtx->channel_layout;
			frameTmp->format = encodeCtx->sample_fmt;
			frameTmp->sample_rate = encodeCtx->sample_rate;
			av_frame_get_buffer(frameTmp, 1);

			av_audio_fifo_read(fifo, (void **)frameTmp->data, encodeCtx->frame_size);

			avcodec_send_frame(encodeCtx, frameTmp);
			avcodec_receive_packet(encodeCtx, packetAAC);

			uint8_t *packetTmp = new uint8_t[packetAAC->size + 4];
			AddRtpHeaderToPacket(packetTmp, packetAAC->data, packetAAC->size);
			session.SendPacket(packetTmp, packetAAC->size + 4, 96, true, 1024);
			delete [] packetTmp;
		}
	}
	av_frame_free(&frame);
	av_frame_free(&frameFLTP);
	av_frame_free(&frameTmp);
	av_packet_free(&packet);
	av_packet_free(&packetAAC);
	swr_free(&swrCtx);
	avformat_free_context(formatCtx);
	avcodec_free_context(&encodeCtx);
	avcodec_free_context(&decodeCtx);
}