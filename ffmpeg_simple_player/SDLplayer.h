#pragma once
#ifndef SDLPLAYER_H
#define SDLPLAYER_H
#include<print>
#include <SDL.h>
extern "C" {
#include<libavcodec/avcodec.h>
}
#include<print>
class SDLplayer
{
public:
	SDLplayer();
	~SDLplayer();
	int init(int width, int height);
	bool isInit() { return isInitFlag; };
	int render(AVFrame* pframe);
	int destory();

private:
	bool isInitFlag;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Rect rect;
};
#endif // SDLPLAYER_H
