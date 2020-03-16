#include<iostream>

#include <SDL.h>
#include <SDL_render.h>

#include <vector>
#include<stdlib.h>
#include<Windows.h> // 之后换成pthread
#include"vec3.h"
#include"ray.h"
#include"float.h"
#include"camera.h"
#include"svpng.inc"
#include"hitable_list.h"
#include"sphere.h"
#include"material.h"

using namespace std;


#define MAXFLOAT 100000
#define MRAND 0x100000000LL
#define CRAND 0xB16
#define ARAND 0x5DEECE66DLL

#define RENDER_TO_PICTURE 1

static unsigned long long seed = 1;
SDL_Window* window;
SDL_Texture* texture;
SDL_Renderer* renderer;
SDL_Thread* thread;
SDL_Event event;

const int texWidth = 1000;
const int texHeight = 500;
bool running = true;
bool update_flag = false;

uint8_t pixels[texWidth * texHeight * 4];


double drand48(void)
{
    seed = (ARAND * seed + CRAND) & 0xFFFFFFFFFFFFLL;
    unsigned int x = seed >> 16;
    return 	((double)x / (double)MRAND);

}

float hit_sphere(const vec3& center, float radius, const ray& r);

vec3 random_in_sphere() {
    vec3 p;
    do {
        p = 2.0 * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
    } while (p.squared_length() > 1.0);
    return p;
}

vec3 color(const ray& r, Hitable* world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, MAXFLOAT, rec)) {
        ray scattered; // 散射后的光线
        vec3 attenuation; // 衰减
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth + 1);
        }
        else {
            return vec3(0, 0, 0);
        }
    }
    //if (world->hit(r, 0.0, MAXFLOAT, rec)) {
    //    vec3 target = rec.p + rec.normal + random_in_sphere();
    //    return 0.5 * color(ray(rec.p, target - rec.p), world );
    //}
    else {
        vec3 unit_dir = unit_vector(r.direction());
        float t = 0.5 * (unit_dir.y() + 1.0);
        return(1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    }
}

float hit_sphere(const vec3& center, float radius, const ray& r) {
    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return -1;
    else return (-b - sqrt(discriminant)) / (2.0 * a);

}

static int update(void *ptr)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("Render Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, texWidth, texHeight, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, texWidth, texHeight);
    Uint64 start = SDL_GetPerformanceCounter();
    while (1)
    {
        Sleep(16);
        if (!update_flag)continue;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        while (SDL_PollEvent(&event))
        {
            if ((SDL_QUIT == event.type) ||
                (SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode))
            {
                running = false;
                break;
            }
        }

        // splat down some random pixels
        //for (unsigned int i = 0; i < 1000; i++)
        //{
        //    const unsigned int x = rand() % texWidth;
        //    const unsigned int y = rand() % texHeight;

        //    const unsigned int offset = (texWidth * 4 * y) + x * 4;
        //    pixels[offset + 0] = rand() % 256;        // b
        //    pixels[offset + 1] = rand() % 256;        // g
        //    pixels[offset + 2] = rand() % 256;        // r
        //    pixels[offset + 3] = SDL_ALPHA_OPAQUE;    // a
        //}

        //unsigned char* lockedPixels;
        //int pitch;
        //SDL_LockTexture
        //    (
        //    texture,
        //    NULL,
        //    reinterpret_cast< void** >( &lockedPixels ),
        //    &pitch
        //    );
        //std::copy( pixels.begin(), pixels.end(), lockedPixels );
        //SDL_UnlockTexture( texture );

        SDL_UpdateTexture(texture, NULL, &pixels[0], texWidth * 4);

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        const Uint64 end = SDL_GetPerformanceCounter();
        const static Uint64 freq = 10000000;//SDL_GetPerformanceFrequency();
        const double seconds = (end - start) / static_cast<double>(freq);
        cout << "Frame time: " << seconds << "s" << endl;
        start = SDL_GetPerformanceCounter(); 
        update_flag = false;
    }
    return 0;
}


int main(int argc, char** argv)
{


    thread = SDL_CreateThread(update, "render", (void*)NULL);
    cout << thread;

    //vector< unsigned char > pixels(texWidth * texHeight * 4, 0);

    ////////////////////

    int const xsize = texWidth, ysize = texHeight;
    int const ns = 1;
    int const nss = 100;
    uint8_t* p = pixels;
    //unsigned char rgb[xsize * ysize * 3], * p = rgb;
    unsigned x, y;

    vec3 lower_left_corner(-2.0, -1.0, -1.0);
    vec3 horizontal(4.0, 0.0, 0.0);
    vec3 vertical(0.0, 2.0, 0.0);
    vec3 origin(0.0, 0.0, 0.0);

    Hitable* list[4];
    list[0] = new sphere(vec3( 0,      0, -1), 0.5,  new Lambertian(vec3(0.0, 0.3, 0.3)));
    list[1] = new sphere(vec3( 0, -100.5, -1), 100,  new Lambertian(vec3(0.0, 0.1, 0.0)));
    list[2] = new sphere(vec3( 1,      0, -1), 0.5,  new      Metal(vec3(0.8, 0.6, 0.2), 0.6));
    list[3] = new sphere(vec3(-0.88,-0.1, -1), 0.4,  new      Metal(vec3(0.8, 0.8, 0.8), 0.9));
    Hitable* world = new HitableList(list, 4);

    Camera cam(vec3(0, 0, 2.2), vec3(0, 0, 0), vec3(0, 1, 0), 30, float(texWidth) / float(texHeight));

    for (int ii = 0; ii < nss; ii++)
    {
        for (int j = ysize - 1; j >= 0; j--)
            for (int i = 0; i < xsize; i++) {
                vec3 col = vec3(0, 0, 0);
                for (int s = 0; s < ns; s++)
                {
                    float u = float(i + drand48()) / float(xsize);
                    float v = float(j + drand48()) / float(ysize);
                    //ray r(origin, lower_left_corner +u*horizontal +v*vertical);
                    ray r = cam.get_ray(u, v);
                    vec3 _p = r.point_at_parameter(2.0);
                    col += color(r, world,0);
                }
                col = col / float(ns);
                col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2])); // gamma 2

                const unsigned int offset = (texWidth * 4 * (texHeight-j)) + i * 4;
                p[offset + 0] = (p[offset + 0] * ii + (unsigned char)(255 * col[2])) / (ii + 1);    /* R */
                p[offset + 1] = (p[offset + 1] * ii + (unsigned char)(255 * col[1])) / (ii + 1);   /* G */
                p[offset + 2] = (p[offset + 2] * ii + (unsigned char)(255 * col[0])) / (ii + 1);   /* B */
                p[offset + 3] = 255;// (unsigned char)(255 * 1);         /* A */
            }
        update_flag = true;
    }
    if (RENDER_TO_PICTURE)
    {
        /*FILE* fp = fopen("out.png", "wb");
        svpng(fp, xsize, ysize, pixels, 0);
        fclose(fp);*/
    }

    return 1;
}

