/*
@file: encoder.cpp
@description: 一个简单的编码器，将yuv文件编码为h264文件
@reference: https://ffmpeg.org/doxygen/5.1/encode_video_8c-example.html
@author:wmh
@time:2023/10/5 21:39
@version:1.0
**/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <format>
#include <string>
#include <print>
using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}
#pragma warning(disable : 4996)

static void mencode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt,
	FILE* outfile)
{
	int ret;

	/* send the frame to the encoder */
	if (frame)
		//printf("Send frame %3"PRId64"\n", frame->pts);
		std::print("Send frame {0}\n", frame->pts);

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}
	//一个frame可能包含多个packet，所以需要循环读取frame
	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		//printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
		print("Write packet {0} (size={1})\n", pkt->pts, pkt->size);
		fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt);
	}
}

int main(int argc, char** argv)
{
	const char* filename, * codec_name;
	const AVCodec* codec;
	AVCodecContext* c = NULL;
	int i, ret;
	FILE* f;
	AVFrame* frame;
	AVPacket* pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	if (argc <= 2) {
		fprintf(stderr, "Usage: %s <output file> <codec name>\n", argv[0]);
		exit(0);
	}
	filename = argv[1];
	codec_name = argv[2];

	/* find the mpeg1video encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	//codec = avcodec_find_encoder_by_name(codec_name);
	if (!codec) {
		fprintf(stderr, "Codec '%s' not found\n", codec_name);
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	/* put sample parameters */
	c->bit_rate = 400000;
	/* resolution must be a multiple of two */
	c->width = 480;
	c->height = 272;
	/* frames per second */
	c->time_base = AVRational{ 1, 25 };
	c->framerate = AVRational{ 25, 1 };

	/* emit one intra frame every ten frames
	 * check frame pict_type before passing frame
	 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	 * then gop_size is ignored and the output of encoder
	 * will always be I frame irrespective to gop_size
	 */
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	/* open it */
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		char errbuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
		fprintf(stderr, "Could not open codec: %s\n", av_make_error_string(errbuf, 64, ret));
		exit(1);
	}

	f = fopen(filename, "wb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;

	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}

	char infile_name[256] = { 0 };
	/* encode 4 second of video */
	for (i = 1; i <= 250; i++) {
		fflush(stdout);

		/* Make sure the frame data is writable.
		   On the first round, the frame is fresh from av_frame_get_buffer()
		   and therefore we know it is writable.
		   But on the next rounds, encode() will have called
		   avcodec_send_frame(), and the codec may have kept a reference to
		   the frame in its internal structures, that makes the frame
		   unwritable.
		   av_frame_make_writable() checks that and allocates a new buffer
		   for the frame only if necessary.
		 */
		ret = av_frame_make_writable(frame);
		if (ret < 0)
			exit(1);
		memset(infile_name, 0, sizeof(infile_name));
		sprintf(infile_name, "./infile/out-%d.yuv", i);

		uint8_t* buf = (uint8_t*)malloc(frame->width * frame->height * 3 / 2);
		FILE* inf = fopen(infile_name, "rb");

		int ret = fread(buf, 1, frame->width * frame->height * 3 / 2, inf);
		if (ret != frame->width * frame->height * 3 / 2) {
			print("read file error\n");
			exit(1);
		}

		frame->data[0] = buf;
		frame->data[1] = buf + frame->width * frame->height;
		frame->data[2] = buf + frame->width * frame->height * 5 / 4;
		fclose(inf);
		frame->pts = i;

		/* encode the image */
		mencode(c, frame, pkt, f);
	}

	/* flush the encoder */
	mencode(c, NULL, pkt, f);

	/* Add sequence end code to have a real MPEG file.
	   It makes only sense because this tiny examples writes packets
	   directly. This is called "elementary stream" and only works for some
	   codecs. To create a valid file, you usually need to write packets
	   into a proper file format or protocol; see muxing.c.
	 */
	if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
		fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	return 0;
}