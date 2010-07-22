#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include <time.h>
#include "../dt.h"

#define FPS (30)
#define MS_PER_FRAME (1000/FPS)

#define BALL_NUM (200)

static const int height = 600;

static const int width = 800;

// all ball have same radius and mass
static const float radius = 10;

static const float max_speed = 5;

typedef struct {
    vertex position;
    float vx;
    float vy;
} Ball;

void collide(Ball * a, Ball * b) {
    float distx = a->position.x - b->position.x;
    float disty = a->position.y - b->position.y;
    float dist2 = distx * distx + disty * disty;

    // not close enough
    if (dist2 > (2 * radius) * (2 * radius))
        return;

    float va_proj_coeff = (a->vx * distx + a->vy * disty)/dist2;
    float vax_proj = va_proj_coeff * distx;
    float vay_proj = va_proj_coeff * disty;
    float vax_orth = a->vx - vax_proj;
    float vay_orth = a->vy - vay_proj;

    float vb_proj_coeff = (b->vx * distx + b->vy * disty)/dist2;
    float vbx_proj = vb_proj_coeff * distx;
    float vby_proj = vb_proj_coeff * disty;
    float vbx_orth = b->vx - vbx_proj;
    float vby_orth = b->vy - vby_proj;

    a->vx = vax_orth + vbx_proj;
    a->vy = vay_orth + vby_proj;
    b->vx = vbx_orth + vax_proj;
    b->vy = vby_orth + vay_proj;
}

void collide_with_edge(Ball * b) {
    if (b->position.x < 0 || b->position.x > width - radius)
        b->vx = -b->vx;

    if (b->position.y < 0 || b->position.y > height - radius)
        b->vy = -b->vy;
}

float random() {
    return (float)rand()/(float)(RAND_MAX + 1);
}

void fill_random_ball(Ball * b) {
    b->position.x = random() * width;
    b->position.y = random() * height;
    b->vx = -max_speed + 2 * random() * max_speed;
    b->vy = -max_speed + 2 * random() * max_speed;
}

void display_ball(Ball * ball, SDL_Surface * screen, SDL_Surface * bmp) {
    SDL_Rect dstrect;
    dstrect.x = ball->position.x;
    dstrect.y = ball->position.y;

    SDL_BlitSurface(bmp, 0, screen, &dstrect);
}

void detect_collisioin(void * _, const vertex * v1, const vertex * v2) {
    Ball * b1 = (Ball *)v1;
    Ball * b2 = (Ball *)v2;
    collide(b1, b2);
}

int main ( int argc, char** argv )
{
    int i;
    Ball * pball;
    Ball balls[BALL_NUM];
    const vertex * vertexes[BALL_NUM];
    srand(time(NULL));
    for (i = 0; i < BALL_NUM; ++i) {
        fill_random_ball(&balls[i]);
        vertexes[i] = (vertex *)&balls[i];
    }

    myDt dt;
    dt_create(&dt);
    dt_set_edge_handler(dt, detect_collisioin, 0);

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(width, height, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set %dx%d video: %s\n", width, height, SDL_GetError());
        return 1;
    }

    // load an image
    SDL_Surface* bmp = SDL_LoadBMP("ball.bmp");
    if (!bmp)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetColorKey(bmp, SDL_SRCCOLORKEY, SDL_MapRGB(bmp->format, 255, 255, 255));

    // program main loop
    boolean_t done = 0;
    Uint32 curr_tick, prev_tick;
    prev_tick = SDL_GetTicks();
    while (!done)
    {
        // don't run too fast
        curr_tick = SDL_GetTicks();
        if (curr_tick - prev_tick < MS_PER_FRAME)
            SDL_Delay(curr_tick - prev_tick);
        prev_tick = curr_tick;

        // message processing loop
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = 1;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
                {
                    // exit if ESCAPE is pressed
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        done = 1;
                    break;
                }
            } // end switch
        } // end of message processing

        dt_run_vertexes(dt, vertexes, BALL_NUM);
        for (i = 0; i < BALL_NUM; ++i)
            collide_with_edge(&balls[i]);

        for (i = 0; i < BALL_NUM; ++i) {
            pball = &balls[i];
            pball->position.x += pball->vx;
            pball->position.y += pball->vy;
        }

        // DRAWING STARTS HERE

        // clear screen
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

        for (i = 0; i < BALL_NUM; ++i)
            display_ball(&balls[i], screen, bmp);

        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);
    } // end main loop

    // free loaded bitmap
    SDL_FreeSurface(bmp);
    dt_destroy(&dt);

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}
