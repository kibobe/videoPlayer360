#ifndef _BUTTON_H
#define _BUTTON_H

#include "player.hpp"
#include <iostream>
#include <string>
#include <stdio.h>
#include <inttypes.h>
#include <opencv2/opencv.hpp>
#include <map>
#include <iostream>
#include <fstream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#include <time.h>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
}

#define MICRO 1000000
#define CUBEFACE 600
#define PI 180

#define UD_ANGLE 15
#define RL_ANGLE 10

class Button
{
public:
    Button(Player *player);
    
    //Handle button events
    void handleEvents(SDL_Event newEvent);

private:
    // x and y value where button down was clicked
    // mouse offset
    int m_beforeX;
    int m_beforeY;

    // action - on how pixel react player
    int m_action;
    // mouse button up was clicked or no
    bool m_up ;

    // Player which playing video
    Player *m_player;

    // Mouse event; mouse motion event, clicked event, ...
    SDL_Event m_event;
};

#endif
