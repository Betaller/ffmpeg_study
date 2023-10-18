#pragma once

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <format>
#include <string>
#include <print>
#include"SDLplayer.h"
using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
	//#include <SDL2/SDL.h>
#include <libavutil/imgutils.h>
}
class decoder
{
public:
	decoder(const string filename);
	~decoder();
	int decode_display();
	int destory();
private:
	int win_h_max;
	int win_w_max;
	int videoindex = -1;
	SDLplayer* sdlplayer;
	string filename;
	AVFormatContext* fmt_ctx;
	const AVCodec* codec;
	AVCodecParserContext* parser;
	AVCodecContext* codec_ctx = NULL;
	AVFrame* frame, * frameyuv;
	AVPacket* pkt;
	enum AVCodecID codec_id;

	SwsContext* img_convert_ctx;
};
