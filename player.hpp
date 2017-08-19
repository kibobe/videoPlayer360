#ifndef _PLAYER_H
#define _PLAYER_H

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
//Screen attributes
#define SCREEN_BPP 32

using namespace cv;
using namespace std;


class Player 
{
public:
    Player();

    // Initialize all SDL subsystems
    // and Open video file
    bool init(const char * video, int & videoStream, double &fpsMicro);
    
    // Creating a Display -- method
    // SDL_SetVideoMode set up a screen with the given width and height
    bool SDLSetUpScreen();
    
    bool allocateVideoFrame();
    void setSwsContext();
    void freeFrame();

    // Close the codec
    void closeCodec();
    // Close the video file
    void closeVideoFile();
    // Quit SDL
    void cleanUp();
    
    // get methods
    double getUpDownAngle();
    int getRotationAngle();
    bool getChangeLR();
    bool getPlayQuit();

    // set methods
    void setUpDownAngle(double value);
    void setRotationAngle(int value);
    void setChangeLR(bool value);
    void setPlayQuit(bool value);

    // lock and unlock mutex method
    void lockPlayerMutex();
    void unlockPlayerMutex();

    void play(int videoStream, int fpsMicro);

private:
    
    // rotationFrameRL()    
    // rotationFrameRL()   Mat in - input image(frame)
    //                       Mat out - output image(frame)
    //                        double UDAngle - angle of rotation up down
    //                     - creates output frame(Mat out) by rotation input frame(Mat in) up or down
    //                      - Mat in, Mat face are equirectengular projection
    //                        and rotation is implemented using formulas for spherical rotation above y axis
    //                   - using openCV remap function
    void rotationFrame(const Mat &in, Mat &out, double UDAngle);

    // rotationFrameRL()
    // rotationFrameRL() - Mat in - input image(frame)
    //                       Mat out - output image(frame)
    //                      - changeLR is true if angle for rotation is changed
    //                   - creates output frame(Mat out) by rotation input frame(Mat in) to the rigtht or to the left
    //                      - Mat in, Mat out are equirectengular projection
    //                      - rotation is implemented by cyclic shift of pixels
    //                   - using openCV remap function
    void rotationFrameRL(const Mat &in, Mat &out, int rotation, bool changeLR);

    // createCubeFace()
    // createCubeFace()  - create front cube face (Mat face) frame from equirectangular frame (Mat in)
    //                   - using formula for this mapping
    //                   - using openCV remap function
    void createCubeFace(const Mat &in, Mat &face, int width, int height);
    
    // function transformFrame()
    // transform frame from input frame to frame that whill be displayed 
    void transformFrame(int width, int height);

    SDL_Surface     *m_screen;

    AVFormatContext *m_formatCtx;
    AVFrame         *m_frame;
    AVFrame         *m_frameRGB;
    AVFrame         *m_frameConvert;

    AVCodecContext  *m_codecCtxOrig;
    AVCodecContext  *m_codecCtx;
    AVCodec         *m_codec;

    struct SwsContext *m_sws_ctxToRGB;
    struct SwsContext *m_sws_ctxToYUV;

    // rotation angle to the up or down
    double m_UpDownAngle;

    // rotation angle to the left or right
    int m_rotation;

    // button down was clicked and left-right angle changes
    bool m_changeLR;

    // mutex for synchronize rotation of frame and showing frame
    SDL_mutex* m_change_mutex;

    // playQuit - for stop playing
    bool m_playQuit;
};

#endif