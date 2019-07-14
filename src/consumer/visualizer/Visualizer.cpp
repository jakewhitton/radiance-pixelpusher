#include "Visualizer.h"
#include "misc/Log.h"

Visualizer::Visualizer()
    : _running(true)
    , _sdlInitialized(false)
{}

Visualizer::~Visualizer()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Visualizer::initializeSDL()
{
    int status = SDL_Init(SDL_INIT_VIDEO);
    if (status < 0)
    {
        ERR("Could not initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("Radiance Visualizer", //title
                              SDL_WINDOWPOS_UNDEFINED, //x
                              SDL_WINDOWPOS_UNDEFINED, //y
                              SQUARE_SIZE*TEST_WIDTH, //w
                              SQUARE_SIZE*TEST_HEIGHT, //h
                              SDL_WINDOW_SHOWN);
    
    if (!window)
    {
        ERR("Could not create an SDL window: %s", SDL_GetError());
    }

    // screenSurface = SDL_GetWindowSurface(window);
    screenRenderer = SDL_CreateRenderer(window, -1, 0);
    if (screenRenderer == nullptr) 
    {
        ERR("Could not create an SDL renderer: %s", SDL_GetError());
    }
    // SDL_RenderSetLogicalSize( screenRenderer, SQUARE_SIZE*16, SQUARE_SIZE*12 );

    // // SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
    // // Set render color to red ( background will be rendered in this color )
    //     SDL_SetRenderDrawColor( screenRenderer, 255, 0, 0, 255 );

    //     // Clear winow
    //     SDL_RenderClear( screenRenderer );

}

void Visualizer::consumeFrames(FrameQueue & frameQueue)
{
    Frame frame;

    if (!_sdlInitialized)
    {
        initializeSDL();
        _sdlInitialized = true;
    }

    while (_running)
    {
        frameQueue.take(frame, std::chrono::milliseconds(100));
        
        // INFO("%d",frame.data());

        uint32_t seqnum = *((uint32_t *)frame.data());
        INFO("frame received %d", seqnum);
        // uint8_t index = *((uint8_t *)frame.data() + 4);
        // INFO("%d %d", seqnum, index); //breaks here

        uint8_t * data = ((uint8_t *)frame.data()) + 5; // 4 for seq, 1 for index

        int i = 0;

        for (; i < TEST_WIDTH * TEST_HEIGHT; ++i)
        {
            int row = (int)(i / TEST_WIDTH);
            int column = i % TEST_WIDTH;

            uint8_t r = data[3*i];
            uint8_t g = data[3*i + 1];
            uint8_t b = data[3*i + 2];
            fillSquare(row, column, r, g, b);
        }
        SDL_RenderPresent(screenRenderer);
    }
}

void Visualizer::fillSquare(int row, int col, uint8_t r, uint8_t g, uint8_t b)
{

    SDL_Rect square = {row*SQUARE_SIZE, col*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE}; //x,y,w,h
    INFO("(%d,%d) %d %d %d", row, col, r, g, b);

    SDL_SetRenderDrawColor(screenRenderer, r, g, b, 255);
    SDL_RenderFillRect(screenRenderer, &square);

    // SDL_FillRect(screenSurface, &square, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);
}

void Visualizer::stop()
{
    _running = false;
}
