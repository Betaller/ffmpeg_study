/**
 * 最简单的FFmpeg Helloworld程序
 * Simplest FFmpeg HelloWorld
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * 本程序是基于FFmpeg函数的最简单的程序。它可以打印出FFmpeg类库的下列信息：
 * Protocol:  FFmpeg类库支持的协议
 * AVFormat:  FFmpeg类库支持的封装格式
 * AVCodec:   FFmpeg类库支持的编解码器
 * AVFilter:  FFmpeg类库支持的滤镜
 * Configure: FFmpeg类库的配置信息
 *
 * This is the simplest program based on FFmpeg API. It can show following
 * informations about FFmpeg library:
 * Protocol:  Protocols supported by FFmpeg.
 * AVFormat:  Container format supported by FFmpeg.
 * AVCodec:   Encoder/Decoder supported by FFmpeg.
 * AVFilter:  Filters supported by FFmpeg.
 * Configure: configure information of FFmpeg.
 *
 */

#include <stdio.h>
#include <iostream>
#define __STDC_CONSTANT_MACROS

#ifdef _WIN32

 //Windows
#pragma warning(disable : 4996)
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
};

#else
 //Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#ifdef __cplusplus
};
#endif
#endif

//FIX
struct URLProtocol;
/**
 * Protocol Support Information
 */
char* urlprotocolinfo() {
	char* info = (char*)malloc(40000);
	memset(info, 0, 40000);

	//av_register_all();

	struct URLProtocol* pup = NULL;
	//Input
	struct URLProtocol** p_temp = &pup;
	avio_enum_protocols((void**)p_temp, 0);
	while ((*p_temp) != NULL) {
		sprintf(info, "%s[In ][%10s]\n", info, avio_enum_protocols((void**)p_temp, 0));
	}
	pup = NULL;
	//Output
	avio_enum_protocols((void**)p_temp, 1);
	while ((*p_temp) != NULL) {
		sprintf(info, "%s[Out][%10s]\n", info, avio_enum_protocols((void**)p_temp, 1));
	}

	return info;
}

/**
 * AVFormat Support Information
 */
char* avformatinfo() {
	char* info = (char*)malloc(40000);
	//memset(info, 0, 40000);

	//av_register_all();
	void* p_opaque = NULL;
	const AVInputFormat* if_temp = NULL;
	while ((if_temp = av_demuxer_iterate(&p_opaque))) {
		if (if_temp->name && if_temp->long_name)
			std::cout << "[Demuxer]" << if_temp->name << std::endl;
	}
	return info;
}

///**
// * AVCodec Support Information
// */
char* avcodecinfo()
{
	void* p_opaque = NULL;
	const AVCodec* p_avcodec = NULL;
	while ((p_avcodec = av_codec_iterate(&p_opaque))) {
		if (p_avcodec->name == NULL)continue;
		//std::cout << p_avcodec->long_name << std::endl;
		switch (p_avcodec->type) {
		case AVMEDIA_TYPE_VIDEO:
			std::cout << "[CODEC][Video]" << p_avcodec->name << std::endl;
			break;
		case AVMEDIA_TYPE_AUDIO:
			std::cout << "[CODEC][Audio]" << p_avcodec->name << std::endl;
			break;
		default:
			std::cout << "[CODEC][Other]" << p_avcodec->name << std::endl;
			break;
		}
	}
	return NULL;
}
/**
 * AVFilter Support Information
 */
char* avfilterinfo()
{
	char* info = (char*)malloc(40000);
	memset(info, 0, 40000);

	void* p_opaque = NULL;

	const AVFilter* f_temp = NULL;

	while ((f_temp = av_filter_iterate(&p_opaque))) {
		std::cout << f_temp->name << std::endl;
	}
	return info;
}
//
/**
 * Configuration Information
 */
char* configurationinfo()
{
	char* info = (char*)malloc(40000);
	memset(info, 0, 40000);

	//av_register_all();

	sprintf(info, "%s\n", avcodec_configuration());

	return info;
}

int main(int argc, char* argv[])
{
	//打印ffmpeg版本信息
	printf("FFmpeg Version: %s\n", av_version_info());
	char* infostr = NULL;
	infostr = configurationinfo();
	printf("\n<<Configuration>>\n%s", infostr);
	free(infostr);

	infostr = urlprotocolinfo();
	printf("\n<<URLProtocol>>\n%s", infostr);
	free(infostr);

	printf("\n<<AVFormat>>\n");
	infostr = avformatinfo();
	free(infostr);

	printf("\n<<AVCodec>>\n");
	infostr = avcodecinfo();
	//free(infostr);

	printf("\n<<AVFilter>>\n");
	infostr = avfilterinfo();
	free(infostr);

	return 0;
}