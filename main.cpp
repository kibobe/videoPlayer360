#include "button.hpp"
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

using namespace cv;
using namespace std;

// struct for  thread data
typedef struct
{
    Player *player;
    int videoStream;
    double fps;
} ThreadData;

// thread function playVideo() - playing video
// playVideo() : read  frames, transform the frames and shows transformed frame on display
int playVideo (void *data)
{

    // get thread data for input params data
    ThreadData *tdata = (ThreadData *)data;
    Player * player = tdata->player;
    int videoStream = tdata->videoStream;
    int fpsMicro = (int)tdata->fps;

    player->play(videoStream, fpsMicro);

    cout << "set quit " << endl;
    player->setPlayQuit(true);

    return 0;
}

int main( int argc, char* args[])
{

    SDL_Thread *thread = NULL;
    Player *player = new Player();

    // Mouse event; mouse motion event, clicked event, ...
    SDL_Event event;

    int streamVdieo;
    cout << M_PI << endl;

    // data for Thread 
    ThreadData *data;

    double fpsMicro;

    if(argc <= 1)
    {
        cerr << "Set Arguments" << endl;
        return 1;
    }
    const char* video = args[1];

    //Initialize all SDL subsystems and open video file
    if (player->init(video, streamVdieo, fpsMicro) == false)
    {
        cout << "player init = false" << endl;
        return 1;
    }

    // Creating a Display -- method
    // SDL_SetVideoMode set up a screen with the given width and height
    if (player->SDLSetUpScreen() == false)
    {
        cout << "player set up screen = false" << endl;
        return 1;
    }

    player->allocateVideoFrame();
    player->setSwsContext();

    Button myButton(player);

    data = (ThreadData *)malloc(sizeof(ThreadData));
    data->player = player;
    data->videoStream = streamVdieo;
    data->fps = fpsMicro;

    // create SDL thread for doing playVideo function
    thread = SDL_CreateThread(playVideo, data);

    while (!player->getPlayQuit())
    {
        //If there's events to handle
        if (SDL_PollEvent(&event))
        {
            //Handle button events
            myButton.handleEvents(event);
        }
    }

    SDL_WaitThread(thread, NULL);

    player->cleanUp(); 

    player->freeFrame();

    player->closeCodec();

    player->closeVideoFile();

    return 0;
}
