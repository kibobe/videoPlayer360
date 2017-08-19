#include "player.hpp"


Player::Player()
{
    m_formatCtx = NULL;
    m_codecCtxOrig = NULL;
    m_codecCtx = NULL;
    m_codec = NULL;
    m_screen = NULL;

    m_sws_ctxToRGB = NULL;
    m_sws_ctxToYUV= NULL;

    m_frame = NULL;
    m_frameRGB = NULL;
    m_frameConvert = NULL;

    m_rotation = 0;
    m_UpDownAngle = 0;
    m_changeLR = false;
    m_playQuit = false;
}

bool Player::init()
{
    av_register_all();

    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        cout << "Could not initialize SDL!" << endl;
        return  false;
    }

    return true;
}

bool Player::openFile(const char * video)
{
    // Open video file
    if (avformat_open_input(&m_formatCtx, video, NULL, NULL) != 0)
    {
        cout << "Could not open file!" << endl;
        return false;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(m_formatCtx, NULL) <0)
    {
        cout << "Could not find stream information!" << endl;
        return false;
    }

    // Dump information about file 
    cout << "Dump infotmation about file: " << endl;
    av_dump_format(m_formatCtx, 0, video, 0);

    return true;
}

bool Player::initCodec(int & videoStream, double & fpsMicro){

    videoStream = -1;
    for (unsigned i = 0; i < m_formatCtx->nb_streams; i++)
    {
        if (m_formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
          videoStream = i;
          break;
        }
    }
    
    if (videoStream == -1)
    {
        cout << "Didn't find a video stream!" << endl;
        return false;
    }

    fpsMicro = (av_q2d (m_formatCtx->streams[videoStream]->r_frame_rate)) * MICRO;

    // Get a pointer to the codec context for the video stream
    m_codecCtxOrig = m_formatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    cout << "Find decoder for the first stream - avcodec_find_decoder" << endl;
    m_codec = avcodec_find_decoder(m_codecCtxOrig->codec_id);

    if (m_codec == NULL)
    {
        // Codec not found
        cout << "Unsupported codec!" << endl;
        return false;
    }

    // Copy context
    m_codecCtx = avcodec_alloc_context3(m_codec);
    if (avcodec_copy_context(m_codecCtx, m_codecCtxOrig) != 0)
    {
        cout << "Could not copy codec context!" << endl;
        return false;
    }

    // Open codec
    if (avcodec_open2(m_codecCtx, m_codec, NULL) < 0)
    {
        cout << "Could not open codec!" << endl;
        return false;
    }

    return true;
}

bool Player::SDLSetUpScreen()
{
    m_screen = SDL_SetVideoMode(m_codecCtx->width, m_codecCtx->height, SCREEN_BPP, SDL_SWSURFACE);
    if (!m_screen)
    {
        cout << "SDL: could not set video mode" << endl;
        exit(1);
    }

    return true;
}

bool Player::allocateVideoFrame()
{
    int numBytes;
    uint8_t *buffer = NULL;

    // Allocate video frame
    cout << "Allocate video frame" << endl;
    m_frame = av_frame_alloc();
    m_frameConvert = av_frame_alloc();
    m_frameRGB = av_frame_alloc();
    if (m_frameRGB == NULL)
    {
        return -1;
    }

    numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, m_codecCtx->width, m_codecCtx->height);
    buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    avpicture_fill((AVPicture *)m_frameRGB, buffer, AV_PIX_FMT_BGR24, m_codecCtx->width, m_codecCtx->height);

    //av_image_get_buffer_size used because avpicture_get_size is deprecated
    numBytes = av_image_get_buffer_size((AVPixelFormat)0, m_codecCtx->width, m_codecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    avpicture_fill((AVPicture *)m_frameConvert, buffer, m_codecCtx->pix_fmt, m_codecCtx->width, m_codecCtx->height);

    return true;
}

void Player::setSwsContext()
{
    m_sws_ctxToRGB = sws_getContext( m_codecCtx->width,
                                    m_codecCtx->height,
                                    m_codecCtx->pix_fmt,
                                    m_codecCtx->width,
                                    m_codecCtx->height,
                                    AV_PIX_FMT_BGR24,
                                    SWS_BILINEAR,
                                    NULL,
                                    NULL,
                                    NULL
                      );

    m_sws_ctxToYUV= sws_getContext(m_codecCtx->width,
                m_codecCtx->height,
                m_codecCtx->pix_fmt,
                m_codecCtx->width,
                m_codecCtx->height,
                AV_PIX_FMT_YUV420P,
                SWS_BILINEAR,
                NULL,
                NULL,
                NULL
                   );
}

void Player::freeFrame()
{
    av_frame_free(&m_frame);
    av_frame_free(&m_frameConvert);
    av_frame_free(&m_frameRGB);
}

void Player::closeCodec() 
{
    avcodec_close(m_codecCtx);
    avcodec_close(m_codecCtxOrig);
}

void Player::closeVideoFile()
{
    avformat_close_input(&m_formatCtx);
}

//Quit SDL
void Player::cleanUp()
{
    SDL_Quit();
}

// get methods

double Player::getUpDownAngle()
{
    return  m_UpDownAngle;
}

int Player::getRotationAngle()
{
    return m_rotation;
}

bool Player::getChangeLR()
{
    return m_changeLR;
}

bool Player::getPlayQuit()
{
    return m_playQuit;
}


// set methods
void Player::setUpDownAngle(double value)
{
    m_UpDownAngle = value;
}

void Player::setRotationAngle(int value)
{
    m_rotation = value;
    m_rotation %= m_codecCtx->width;
}

void Player::setChangeLR(bool value)
{
    m_changeLR = value;
}

void Player::setPlayQuit(bool value)
{
    m_playQuit = value;
}

void Player::lockPlayerMutex()
{
    SDL_LockMutex(m_change_mutex);
}

void Player::unlockPlayerMutex()
{
    SDL_UnlockMutex(m_change_mutex);
}

void Player::play(int videoStream, int fpsMicro)
{

    AVPacket        packet;
    SDL_Overlay     *bmp;
    SDL_Rect        rect;

    int frameFinished;

    // variables for timing set and showing frames
    static long long int microStop;
    static long long int microStart;
    static struct timespec startStruct, stopStruct;
    static bool nextFrameNotShow = false;

    // Allocate a place to put our YUV image on that screen
    bmp = SDL_CreateYUVOverlay(m_codecCtx->width, m_codecCtx->height, SDL_YV12_OVERLAY, m_screen);

    // Read frames and save first five frames to disk
    while (av_read_frame(m_formatCtx, &packet)>=0 && (m_playQuit == false))
    {
        // Is this a packet from the video stream?
        if (packet.stream_index==videoStream)
        {
            // Decode video frame
            avcodec_decode_video2(m_codecCtx, m_frame, &frameFinished, &packet);

              // Did we get a video frame?
            if (frameFinished)
            {
                if(!nextFrameNotShow)
                {
                    sws_scale
                    (
                    m_sws_ctxToRGB,
                    (uint8_t const * const *)(m_frame->data),
                    m_frame->linesize,
                    0,
                    m_codecCtx->height,
                    m_frameRGB->data,
                    m_frameRGB->linesize
                    );

                    
                    transformFrame(m_codecCtx->width, m_codecCtx->height);
                    
                    // Display image
                    SDL_LockYUVOverlay(bmp);
                    AVFrame pict;

                    pict.data[0] = bmp->pixels[0];
                    pict.data[1] = bmp->pixels[2];
                    pict.data[2] = bmp->pixels[1];

                    pict.linesize[0] = bmp->pitches[0];
                    pict.linesize[1] = bmp->pitches[2];
                    pict.linesize[2] = bmp->pitches[1];

                    // Convert the image into YUV format that SDL uses
                    sws_scale
                    (
                    m_sws_ctxToYUV,
                    (uint8_t const * const *)(m_frameConvert->data),
                    m_frameConvert->linesize,
                    0,
                    m_codecCtx->height,
                    pict.data,
                    pict.linesize
                    );

                    SDL_UnlockYUVOverlay(bmp);

                    rect.x = 0;
                    rect.y = 0;
                    rect.w = m_codecCtx->width;
                    rect.h = m_codecCtx->height;

                    nextFrameNotShow = false;
                    SDL_DisplayYUVOverlay(bmp, &rect);

                    clock_gettime( CLOCK_MONOTONIC, &stopStruct);
                    microStop = ((stopStruct.tv_sec)*1000000000LL + stopStruct.tv_nsec)/1000;

                    if(((microStop-microStart)< fpsMicro))
                    {
                        nextFrameNotShow = true;
                    }
                    microStart = microStop;

                }
                else
                {
                    // frame not show
                    clock_gettime( CLOCK_MONOTONIC, &startStruct);
                    microStart = ((stopStruct.tv_sec)*1000000000LL + startStruct.tv_nsec)/1000;
                    nextFrameNotShow = false;
                }
            }
        }
        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }
}

void Player::rotationFrame(const Mat &in, Mat &out, double UDAngle)
{
    float inWidth = in.cols;
    float inHeight = in.rows;
    unsigned x = 0;
    unsigned y = 0;
    double tx = 0;
    double ty = 0;
    float xx = 0;
    float yy = 0;

    // don't need to rotate all pixel from one frame
    // Only the pixels used in the creation of the front face of cube are rotated
    // [h,hh] height is used for creating front cube face
    unsigned h = inHeight/6;
    unsigned hh = 5*inHeight/6;

    // don't need to rotate all pixel from one frame
    // Only the pixels used in the creation of the front face of cube are rotated
    // [w,ww] height is used for creating front cube face
    unsigned w = inWidth/4;
    unsigned ww = 3*inWidth/4;

    double edgeX = inWidth - 1;
    double edgeY = inHeight - 1;

    static double lgPvi[3] = {  0.0  };
    static double lgPvf[3] = { 0.0  };

    // Allocate map
    static map<int, Mat> UDXMaps;
    static map<int, Mat> UDYMaps;

    // firstTimeInUD is true in first rotationFrame function call
    static int firstTimeInUD = true;

    // in first call create maps for angle of rotation  ([-PI/2,PI/2])
    if (firstTimeInUD)
    {

        for(int j = -PI/2; j <= PI/2;)
        {
            UDXMaps.insert(make_pair(j, Mat(inHeight, inWidth, CV_32F)) );
            UDYMaps.insert(make_pair(j, Mat(inHeight, inWidth, CV_32F)) );

            for (y = h; y < hh; y++)
            {
                for (x = w; x < ww; x++)
                {
                    //cout << "chjange " << UDAngle << endl;
                    xx = ( ( double( x ) / edgeX ) * 2.0) * M_PI;
                    yy = ( ( double( y ) / edgeY ) -  0.5) * M_PI;

                    lgPvi[0] = cos( yy );
                    lgPvi[1] = sin( xx ) * lgPvi[0];
                    lgPvi[0] = cos( xx ) * lgPvi[0];
                    lgPvi[2] = sin( yy );

                    double dd = j * M_PI / PI;
                    lgPvf[0] =  cos(dd) * lgPvi[0] + sin(dd)  * lgPvi[2];
                    lgPvf[1] =     lgPvi[1];
                    lgPvf[2] =  -sin(dd) * lgPvi[0] + cos(dd) * lgPvi[2];

                    ty = asin(lgPvf[2]);
                    tx = acos(lgPvf[0]/cos(ty));

                    if(xx >= M_PI)
                    {
                        tx = abs(tx - (2 * M_PI));
                    }

                    xx = edgeX * tx / (2*M_PI);
                    yy = edgeY * ((ty / M_PI) + 0.5);

                    (UDXMaps[j]).at<float>(y, x) = xx;
                    (UDYMaps[j]).at<float>(y, x) = yy;
                }
            }
            j = j + UD_ANGLE;
        }

        firstTimeInUD = false;
    }
    // Recreate output image if it has wrong size or type.
    if(out.cols != CUBEFACE || out.rows != CUBEFACE || out.type() != in.type())
    {
        out = Mat(inHeight, inWidth, in.type());
    }

    // Do actual resampling using OpenCV's remap
    remap(in, out, UDXMaps[UDAngle], UDYMaps[UDAngle], CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
}



void Player::rotationFrameRL(const Mat &in, Mat &out, int rotation, bool changeLR)
{
    // firstTimeInRotationRL is true in first RotationFrameRL function call
    static bool firstTimeInRotationRL = true;

    // inWidth, inHeight - width and height of input image (Mat in)
    float inWidth = in.cols;
    float inHeight = in.rows;

    // allocate map
    // mapX - The first map of x values
    static Mat mapX(inHeight, inWidth, CV_32F);
    // mapY - The second map of y values
    static Mat mapY(inHeight, inWidth, CV_32F);

    // set mapX and mapY for mapping in first function call
    // mapX and mapY are changing if angle for rotation is changed - if changeLR is true
    if (firstTimeInRotationRL || changeLR)
    {

        firstTimeInRotationRL = false;
        changeLR = false;

        float u;
        int move;
        int pom;

        if (rotation >=0)
        {
            //to move frame to the right
            move =  rotation;
            pom = inWidth - move;
        }
        else
        {
            //to move frame to the left
            pom = -rotation;
            move = inWidth - pom;
        }

        for (int y = 0; y < inHeight; y++)
        {
            for (int x = 0; x < inWidth; x++)
            {
                if (x >= pom)
                {
                    u = x - pom;
                }
                else
                {
                    u = x + move;
                }

                mapX.at<float>(y, x) = u;
                mapY.at<float>(y, x) = y;
            }
        }
    }

    if (out.cols != inHeight || out.rows != inWidth || out.type() != in.type())
    {
        out = Mat(inHeight,inWidth, in.type());
    }
    
    // Do actual resampling using OpenCV's remap
    remap(in, out, mapX, mapY, CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
}

void Player::createCubeFace(const Mat &in, Mat &face, int width, int height)
{
    //cout << "createCubeFace" << endl;
    static bool firstTimeInCreateCube = true;

    float inWidth = in.cols;
    float inHeight = in.rows;

    // Allocate map
    static Mat mapx(height, width, CV_32F);
    static Mat mapy(height, width, CV_32F);

    //SDL_LockMutex(changeLR_mutex);
    if (firstTimeInCreateCube )
    {
        const float an = sin(M_PI / 4);
        const float ak = cos(M_PI / 4);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                
                float u, v;
                // Map face pixel coordinates to [-1, 1] on plane
                float nx = (float)y / (float)height - 0.5f;
                float ny = (float)x / (float)width - 0.5f;

                nx *= 2;
                ny *= 2;

                // Map [-1, 1] plane coords to [-an, an]
                // thats the coordinates in respect to a unit sphere
                // that contains our box.
                nx *= an;
                ny *= an;

                // Project from plane to sphere surface.
                u = atan2(nx, ak);
                v = atan2(ny * cos(u), ak);
                
                // Map from angular coordinates to [-1, 1], respectively.
                u = u / (M_PI);
                v = v / (M_PI / 2);

                // Warp around, if our coordinates are out of bounds.
                while (v < -1)
                {
                    v += 2;
                    u += 1;
                }
                
                while (v > 1)
                {
                    v -= 2;
                    u += 1;
                }

                while (u < -1)
                {
                    u += 2;
                }
                
                while (u > 1)
                {
                    u -= 2;
                }

                // Map from [-1, 1] to in texture space
                u = u / 2.0f + 0.5f;
                v = v / 2.0f + 0.5f;

                u = u * (inWidth - 1);
                v = v * (inHeight - 1);

                // Save the result for this pixel in map
                mapx.at<float>(x, y) = u;
                mapy.at<float>(x, y) = v;
            }
        }
        firstTimeInCreateCube = false;
    }

    // Recreate output image if it has wrong size or type.
    if (face.cols != height || face.rows != width || face.type() != in.type())
    {
        //cout << in.type() << " " << face.type() << endl;
        face = Mat(height,width, in.type());
    }

    remap(in, face, mapx, mapy, CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
}

void Player::transformFrame(int width, int height)
{
    Mat mat(height, width, CV_8UC3, m_frameRGB->data[0], m_frameRGB->linesize[0]);
    Mat rotationUD(height, width, CV_8UC3);
    Mat rotationRL(height, width, CV_8UC3);

    int newWidth = CUBEFACE;
    int newHeight = CUBEFACE;

    //cout << "newImage2" << endl;
    Mat newImage2(newHeight, newWidth, CV_8UC3);

    //cout << "pre mutexa" << endl;
    lockPlayerMutex();

    //cout << "pre rotation RL" << endl;
    rotationFrameRL(mat, rotationRL, m_rotation, m_changeLR);
    //cout << "posle rotation RL" << endl;

    //cout << "pre rotation UD" << endl;
    rotationFrame(rotationRL, rotationUD, m_UpDownAngle);
    //cout << "posle rotation UD" << endl;
    
    //cout << "pre createCubeFace" << endl;
    createCubeFace(rotationUD, newImage2, newWidth, newHeight);
    //cout << "posle createCubeFace" << endl;

    unlockPlayerMutex();

    SwsContext* swsctx = sws_getContext(
                          newImage2.size().width,
                          newImage2.size().height,
                    AV_PIX_FMT_BGR24,
                    width,
                    height,
                    (AVPixelFormat)(m_frame->format),
                    SWS_BICUBIC,
                    NULL,
                    NULL,
                    NULL);

    const int stride[] = { static_cast<int>(newImage2.step[0]) };

    sws_scale(swsctx,
            (uint8_t const * const *)&newImage2.data,
            stride,
            0,
            newImage2.rows,
            m_frameConvert->data,
            m_frameConvert->linesize);
}