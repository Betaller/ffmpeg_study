#include<SDL.h>

int main(int argc, char* argv[])
{
	//��ʼ��SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		SDL_Log("can not init SDL:%s", SDL_GetError());
		return -1;
	}

	return 0;
}