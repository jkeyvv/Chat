#include "recvvideo.h"

void RecvVideo::StartRecv()
{
	this->start();
}

void RecvVideo::run()
{
	avdevice_register_all();

	AVFrame *frame = av_frame_alloc();
	AVFrame *frameRGB32 = av_frame_alloc();
	AVPacket *packet = av_packet_alloc();
	SwsContext *swsCtx = sws_alloc_context();
	AVCodec *decode = avcodec_find_decoder(AV_CODEC_ID_H264);
	AVCodecContext *decodeCtx = avcodec_alloc_context3(decode);
	avcodec_open2(decodeCtx, decode, NULL);

	RTPSession session;
	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0 / 90000);
	sessionparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(DEST_PORT_VIDEO);
	session.Create(sessionparams, &transparams);

	uint8_t frameBuf[FRAME_BUF_SIZE_VIDEO];
	memset(frameBuf, 0, FRAME_BUF_SIZE_VIDEO);
	int frameLength = 0;
	uint8_t *payLoadData;
	size_t payLoadLength;
	RTPPacket *pack;

	for (;;)
	{
		session.BeginDataAccess();
		if (session.GotoFirstSourceWithData())
		{
			while ((pack = session.GetNextPacket()) != NULL)
			{
				payLoadData = pack->GetPayloadData();
				payLoadLength = pack->GetPayloadLength();

				if ((payLoadData[0] & 0x1f) != 28)
				{
					frameBuf[frameLength] = 0;
					frameBuf[frameLength + 1] = 0;
					frameBuf[frameLength + 2] = 0;
					frameBuf[frameLength + 3] = 1;
					frameLength += 4;

					memcpy(&frameBuf[frameLength], payLoadData, payLoadLength);
					frameLength += payLoadLength;
				}
				else if ((payLoadData[1] & 0xe0) == 0x80)
				{
					frameBuf[frameLength] = 0;
					frameBuf[frameLength + 1] = 0;
					frameBuf[frameLength + 2] = 0;
					frameBuf[frameLength + 3] = 1;
					frameLength += 4;

					memcpy(&frameBuf[frameLength], payLoadData + 1, payLoadLength - 1);
					frameBuf[frameLength] &= 0x1f;
					frameBuf[frameLength] |= (payLoadData[0] & 0xe0);
					frameLength += payLoadLength - 1;
				}
				else
				{
					memcpy(&frameBuf[frameLength], payLoadData + 2, payLoadLength - 2);
					frameLength += payLoadLength - 2;
				}

				if (pack->HasMarker())
				{
					av_new_packet(packet, frameLength);
					
					packet->data = frameBuf;
					packet->size = frameLength;

					if (!avcodec_send_packet(decodeCtx, packet))
					{
						avcodec_receive_frame(decodeCtx, frame);
						frameRGB32->format = AV_PIX_FMT_RGB32;
						frameRGB32->height = decodeCtx->height;
						frameRGB32->width = decodeCtx->width;
						av_frame_get_buffer(frameRGB32, 1);

						swsCtx = sws_getContext(
							decodeCtx->width, decodeCtx->height, AV_PIX_FMT_YUV422P,
							decodeCtx->width, decodeCtx->height, AV_PIX_FMT_RGB32,
							SWS_BICUBIC, NULL, NULL, NULL);

						sws_scale(swsCtx, (const uint8_t * const*)frame->data,
							frame->linesize, 0, frame->height, frameRGB32->data, frameRGB32->linesize);

						QImage img((uchar *)frameRGB32->data[0], frameRGB32->width, frameRGB32->height, QImage::Format_RGB32);
						emit VideoReady(img);
					}
					memset(frameBuf, 0, FRAME_BUF_SIZE_VIDEO);
					frameLength = 0;
				}
			}
		}
		session.EndDataAccess();
	}
	av_packet_free(&packet);
	av_frame_free(&frame);
	av_frame_free(&frameRGB32);
	sws_freeContext(swsCtx);
	avcodec_free_context(&decodeCtx);
}