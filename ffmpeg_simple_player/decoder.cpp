#include "decoder.h"

decoder::decoder(const string filename)
{
	std::print("open file {0} and start decode\n", filename.c_str());
	this->filename = filename;
	avformat_network_init();
	fmt_ctx = NULL;
	if (avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL) != 0)
	{
		print("avformat_open_input failed\n");
		exit(1);
	}
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
	{
		print("avformat_find_stream_info failed\n");
		exit(1);
	}

	//print("{}", avformat_configuration());
	av_dump_format(fmt_ctx, 0, filename.c_str(), 0);
	videoindex = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	/*for (int i = 0; i < fmt_ctx->nb_streams; i++)
	{
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}*/
	if (videoindex < 0)
	{
		print("Could not find video stream in input file '{}'\n", filename);
		exit(1);
	}
	this->codec_id = fmt_ctx->streams[videoindex]->codecpar->codec_id;

	print("codec_id = {0}\n", avcodec_get_name(codec_id));

	pkt = av_packet_alloc();
	if (!pkt)
	{
		print("av_packet_alloc failed\n");
		exit(1);
	}

	codec = avcodec_find_decoder(codec_id);
	if (!codec)
	{
		print("avcodec_find_decoder failed\n");
		exit(1);
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx)
	{
		print("avcodec_alloc_context3 failed\n");
		exit(1);
	}

	int rtn = avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[videoindex]->codecpar);
	if (rtn < 0)
	{
		print("avcodec_parameters_to_context failed\n");
		exit(1);
	}

	if (avcodec_open2(codec_ctx, codec, NULL) < 0)
	{
		print("avcodec_open2 failed\n");
		exit(1);
	}

	win_h_max = 600;
	win_w_max = 800;
	if (codec_ctx->width > win_w_max || codec_ctx->height > win_h_max)
	{
		float r1 = (float)codec_ctx->width / win_w_max;
		float r2 = (float)codec_ctx->height / win_h_max;

		r1 = max(r1, r2);

		win_h_max = codec_ctx->height / r1;
		win_w_max = codec_ctx->width / r1;
	}
	frame = av_frame_alloc();
	if (!frame)
	{
		print("av_frame_alloc failed\n");
		exit(1);
	}
	frameyuv = av_frame_alloc();
	if (!frameyuv)
	{
		print("av_frame_alloc failed\n");
		exit(1);
	}
	av_image_alloc(frameyuv->data,
		frameyuv->linesize,
		win_w_max,
		win_h_max,
		AV_PIX_FMT_YUV420P,
		1);

	img_convert_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
		win_w_max, win_h_max, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	if (!img_convert_ctx) {
		print("sws_getContext failed\n");
		exit(1);
	}

	sdlplayer = new SDLplayer();

	std::print("open file {0} and start decode success\n", filename.c_str());
}

decoder::~decoder()
{
	destory();
}

int decoder::decode_display()
{
	int ret;
	print("start decode and display\n");
	int err;
	while (av_read_frame(fmt_ctx, pkt) >= 0) {
		if (pkt->stream_index != videoindex) {
			cout << "pkt->stream_index != videoindex" << endl;
			continue;
		}
		//print("send packet\n");
		ret = avcodec_send_packet(codec_ctx, pkt);
		if (ret < 0) {
			fprintf(stderr, "Error sending a packet for decoding\n");
			exit(1);
		}

		while (ret >= 0) {
			ret = avcodec_receive_frame(codec_ctx, frame);
			if (ret == AVERROR(EAGAIN)) {
				//print("decode finish ret={}\n", ret);
				break;
			}
			else if (ret == AVERROR_EOF) {
				print("decode finish\n");
				return -1;
			}
			else if (ret < 0) {
				print("Error during decoding\n");
				exit(1);
			}
			sws_scale_frame(img_convert_ctx, frameyuv, frame);
			if (!sdlplayer->isInit()) {
				sdlplayer->init(win_w_max, win_h_max);
			}
			//print("render frame\n");
			sdlplayer->render(frameyuv);
		}
		av_packet_unref(pkt);
	}
	print("decode finish\n");
	return 0;
}

int decoder::destory()
{
	delete sdlplayer;
	sws_freeContext(img_convert_ctx);
	av_frame_free(&frame);
	av_frame_free(&frameyuv);
	avcodec_free_context(&codec_ctx);
	avformat_close_input(&fmt_ctx);
	av_packet_free(&pkt);
	return 0;
}