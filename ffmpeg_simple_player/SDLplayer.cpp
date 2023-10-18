#include "SDLplayer.h"

SDLplayer::SDLplayer()
{
	isInitFlag = false;
	window = nullptr;
	renderer = nullptr;
	texture = nullptr;
	rect = { 0 };
}

SDLplayer::~SDLplayer()
{
	destory();
}

int SDLplayer::init(int width, int height)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	window = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_OPENGL);
	if (!window) {
		std::print("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		std::print("SDL: could not create renderer - exiting:%s\n", SDL_GetError());
		return -1;
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);

	rect.x = 0;
	rect.y = 0;
	rect.w = width;
	rect.h = height;
	isInitFlag = true;
	return 0;
}

int SDLplayer::render(AVFrame* pframe)
{
	if (!pframe)
		return -1;
	static int cnt = 0;
	//std::print("render frame {}\n", cnt++);
	int rtn = SDL_UpdateYUVTexture(texture, &rect,
		pframe->data[0], pframe->linesize[0],
		pframe->data[1], pframe->linesize[1],
		pframe->data[2], pframe->linesize[2]);
	if (rtn) {
		std::print("SDL_UpdateYUVTexture error:{0}\n", SDL_GetError());
		return -1;
	}

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_RenderPresent(renderer);
	SDL_Delay(40);
	return 0;
}

int SDLplayer::destory()
{
	if (texture)SDL_DestroyTexture(texture);
	if (renderer)SDL_DestroyRenderer(renderer);
	if (window)SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}