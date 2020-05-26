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
#include"Rect.h"
//#include"AABB.h"
#include"Texture.h"
#include"Box.h"


using namespace std;


#define MAXFLOAT 100000
#define MRAND 0x100000000LL
#define CRAND 0xB16
#define ARAND 0x5DEECE66DLL

#define RENDER_TO_PICTURE 1

//static unsigned long long seed = 1;
SDL_Window* window;
SDL_Texture* s_texture;
SDL_Renderer* renderer;
SDL_Thread* thread;
SDL_Event event;

const int texWidth = 500;
const int texHeight = 500;
bool running = true;
bool update_flag = false;

uint8_t pixels[texWidth * texHeight * 4];


double drand48(void);
//{
//    seed = (ARAND * seed + CRAND) & 0xFFFFFFFFFFFFLL;
//    unsigned int x = seed >> 16;
//    return 	((double)x / (double)MRAND);
//
//}

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
        // 散射后的光线
        ray scattered;
        // 衰减
        vec3 attenuation;
        // 记录自发光的颜色
        vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            // 递归 衰减
            return emitted + attenuation * color(scattered, world, depth + 1);
        }
        else
        {
            return emitted;
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
    s_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, texWidth, texHeight);
    Uint64 start = SDL_GetPerformanceCounter();
    while (1)
    {
        Sleep(16);
        if (false&&!update_flag)continue;

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

        SDL_UpdateTexture(s_texture, NULL, &pixels[0], texWidth * 4);

        SDL_RenderCopy(renderer, s_texture, NULL, NULL);
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
//Hitable* random_scene() {
//    int n = 500;
//    Hitable** list = new Hitable * [n + 1];
//    list[0] = new sphere(vec3(0, -700, 0), 700, new Lambertian(vec3(0.5, 0.5, 0.5)));
//    int i = 1;
//    for (int a = -11; a < 11; a++) {
//        for (int b = -11; b < 11; b++) {
//            float choose_mat = drand48();
//            vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());
//            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
//                if (choose_mat < 0.8) {  // diffuse
//                    // 运动模糊的小球
//                    list[i++] = new sphere(center, 40,
//                        new Lambertian(new constant_texture(vec3(drand48() * drand48(), drand48() * drand48(),
//                            drand48() * drand48()))));
//                }
//                else if (choose_mat < 0.95) { // metal
//                    list[i++] = new sphere(center, 0.2,
//                        new Metal(vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()),
//                            0.5 * (1 + drand48())), 0.5 * drand48()));
//                }
//                else {  // glass
//                    list[i++] = new sphere(center, 0.2, new Dielectric(1.5));
//                }
//            }
//        }
//    }
//
//    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new Dielectric(2.5));
//    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new Lambertian(vec3(0.4, 0.2, 0.1)));
//    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(1, 1, 1), 0.0));
//
//    return new HitableList(list, i);
//}

Hitable* cornell_box() {
    Hitable** list = new Hitable * [8];
    int i = 0;
    Material* red = new Lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
    Material* white = new Lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
    Material* green = new Lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
    Material* light = new DiffuseLight(new constant_texture(vec3(100, 100, 100)));
    
    Material* gloss = new Dielectric(0.75f);
    Material* gloss2 = new Dielectric(0.95f);

    Material* gold = new Metal(vec3(0.8, 0.6, 0.2), 0.75f);
    Material* silver = new Metal(vec3(0.8, 0.8, 0.8), 0.35f);
    Material* iron = new Metal(vec3(0.8, 0.8, 0.8),0.1f);

    //Material* g
    list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
    list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
    list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
    list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
    //list[i++] = new xy_rect(0, 555, 0, 955, 0, red);

    //box
    //list[i++] = new translate(new rotate_y(new Box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    //list[i++] = new translate(new rotate_y(new Box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));
    
    //sphere
    list[i++] = new sphere(vec3(150, 90, 150), 90, gloss2);
    list[i++] = new sphere(vec3(350, 130, 350), 130, iron);

    return new HitableList(list, i);
}
Hitable* test1() {
    Hitable** list = new Hitable * [4];
    int i = 0;
    Material* red = new Lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
    Material* white = new Lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
    Material* green = new Lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
    Material* light = new DiffuseLight(new constant_texture(vec3(100, 100, 100)));

    Material* gloss = new Dielectric(0.75f);
    Material* gloss2 = new Dielectric(0.95f);

    Material* gold = new Metal(vec3(0.8, 0.6, 0.2), 0.75f);
    Material* silver = new Metal(vec3(0.8, 0.8, 0.8), 0.35f);
    Material* iron = new Metal(vec3(0.8, 0.8, 0.8), 0.1f);

    //Material* g
    list[0] = new sphere(vec3(0, 0, -1), 0.5, white);
    list[1] = new sphere(vec3(0, -100.5, -1), 100, white);
    list[2] = new sphere(vec3(1, 0, -1), 0.5, white);
    list[3] = new sphere(vec3(-1, 0, -1), 0.5, white);
    

    return new HitableList(list, i);
}
int main(int argc, char** argv)
{


    thread = SDL_CreateThread(update, "render", (void*)NULL);
    cout << thread;

    //vector< unsigned char > pixels(texWidth * texHeight * 4, 0);

    ////////////////////

    int const xsize = texWidth, ysize = texHeight;
    int const ns = 1; //内循环采样次数
    int const nss = 100; //外循环迭代次数
    uint8_t* p = pixels;
    //unsigned char rgb[xsize * ysize * 3], * p = rgb;
    unsigned x, y;

    vec3 lower_left_corner(-2.0, -1.0, -1.0);
    vec3 horizontal(4.0, 0.0, 0.0);
    vec3 vertical(0.0, 2.0, 0.0);
    vec3 origin(0.0, 0.0, 0.0);

    Hitable* world = cornell_box();
    
    vec3 lookfrom(278, 278, -700);
    vec3 lookat(278, 278, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;
    float vfov = 40.0;
    Camera cam(lookfrom, lookat, vec3(0, 100, 0), vfov, float(xsize) / float(ysize));
    //Camera cam(vec3(0, 0, 2.2), vec3(0, 0, 0), vec3(0, 1, 0), 30, float(texWidth) / float(texHeight));

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
    system("pause");

    return 1;
}

