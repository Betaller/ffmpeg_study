#include<SDL.h>

int main(int argc, char* argv[])
{
	//≥ı ºªØSDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		SDL_Log("can not init SDL:%s", SDL_GetError());
		return -1;
	}

	return 0;
}