/* Super Mario For Nintendo Switch Port
 * Made by 25ratcma@protonmail.com
 * Yes the code is messy...I will fix it soon. I will also optimize cpu
 */
#include <time.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <switch.h>
// some switch buttons
#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_PLUS  10
#define JOY_MINUS 11
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14 
#define JOY_DOWN  15
#define SCREEN_W 1280
#define SCREEN_H 720
/* NES specific */
#include <nes.h>
#include <ppu.h>
#include <cpu.h>
#include <cartridge.h>
#include <controller.h>


//define global keys
int kg [100] = {0};



#define debug_print(fmt, ...) \
            do { if (DEBUG_MAIN) printf(fmt, __VA_ARGS__); } while (0)
void die (const char * format, ...)
{
    va_list vargs;
    va_start (vargs, format);
    vfprintf (stderr, format, vargs);
    va_end (vargs);
    exit (1);
}

static int kp = 99;

SDL_Texture * render_text(SDL_Renderer *renderer, const char* text, TTF_Font *font, SDL_Color color, SDL_Rect *rect) 
{
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = TTF_RenderText_Solid(font, text, color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect->w = surface->w;
    rect->h = surface->h;

    SDL_FreeSurface(surface);

    return texture;
}

int rand_range(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}
#define FPS 60
#define FPS_UPDATE_TIME_MS (1000/FPS)
uint8_t nes_key_state(uint8_t b)
{
    switch (b)
    {
        case 0: // On / Off
            return 1;
        case 1: // A
            if(kg[0] == 1){
                return 1;
            }else{
                return 0;
            }
        case 2: // B
            if(kg[1] == 1){
                return 1;
            }else{
                return 0;
            }
        case 3: // SELECT
            if(kg[11] == 1){
                return 1;
            }else{
                return 0;
            }
        case 4: // START
            if(kg[10] == 1){
                return 1;
            }else{
                return 0;
            }
        case 5: // UP
            if(kg[17] == 1){
                return 1;
            }else{
                return 0;
            }
        case 6: // DOWN
            if(kg[19] == 1){
                return 1;
            }else{
                return 0;
            }
        case 7: // LEFT
            if(kg[16] == 1){
                return 1;
            }else{
                return 0;
            }
        case 8: // RIGHT
            if(kg[18] == 1){
                return 1;
            }else{
                return 0;
            }
        default:
            return 1;
    }
}
uint8_t nes_key_state_ctrl2(uint8_t b)
{
    switch (b)
    {
        case 0: // On / Off
            return 1;
        case 1: // A
            return 0;
        case 2: // B
            return 0;
        case 3: // SELECT
            return 0;
        case 4: // START
            return 0;
        case 5: // UP
            return 0;
        case 6: // DOWN
            return 0;
        case 7: // LEFT
            return 0;
        case 8: // RIGHT
            return 0;
        default:
            return 1;
    }
}
int main(int argc, char** argv) {

    romfsInit();
    chdir("romfs:/");


    static nes_ppu_t nes_ppu;
    static nes_cpu_t nes_cpu;
    static nes_cartridge_t nes_cart;
    static nes_mem_td nes_memory = { 0 };

    uint32_t cpu_clocks = 0;
    uint32_t ppu_clocks = 0;
    uint32_t ppu_rest_clocks = 0;
    uint32_t ppu_clock_index = 0;
    uint8_t ppu_status = 0;


    nes_cart_init(&nes_cart, &nes_memory);
    nes_cart_load_rom(&nes_cart, "data/rom.nes");
    nes_cpu_init(&nes_cpu, &nes_memory);
    nes_cpu_reset(&nes_cpu);

    /* init ppu */
    nes_ppu_init(&nes_ppu, &nes_memory);
    int exit_requested = 0;
    int trail = 0;
    int wait = 25;

    SDL_Texture *switchlogo_tex = NULL, *sdllogo_tex = NULL, *helloworld_tex = NULL;
    SDL_Rect pos = { 0, 0, 0, 0 }, sdl_pos = { 0, 0, 0, 0 };
    Mix_Music *music = NULL;
    Mix_Chunk *sound[4] = { NULL };
    SDL_Event event;
    unsigned int lastTime = 0, currentTime;
    SDL_Color colors[] = {
        { 128, 128, 128, 0 }, // gray
        { 255, 255, 255, 0 }, // white
        { 255, 0, 0, 0 },     // red
        { 0, 255, 0, 0 },     // green
        { 0, 0, 255, 0 },     // blue
        { 255, 255, 0, 0 },   // brown
        { 0, 255, 255, 0 },   // cyan
        { 255, 0, 255, 0 },   // purple
    };
    int col = 0, snd = 0;

    srand(time(NULL));
    int vel_x = rand_range(1, 5);
    int vel_y = rand_range(1, 5);

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    Mix_Init(MIX_INIT_OGG);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("sdl2+mixer+image+ttf demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_Texture * texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 256, 240);
    // load logos from file
    

    

    col = rand_range(0, 7);

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_JoystickOpen(0);
    
    while (1)
    {
        // Get the next event
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                break;
            }



            switch (event.type)
            {
            case SDL_JOYBUTTONDOWN:
                kg[event.jbutton.button] = 1;
                break;
            case SDL_JOYBUTTONUP:
                kg[event.jbutton.button] = 0;
                break;
            }




        }

        /* NES core loop */
        for(;;)
        {
            cpu_clocks = 0;
            if(!ppu_rest_clocks)
            {
                if(ppu_status & PPU_STATUS_NMI)
                    cpu_clocks += nes_cpu_nmi(&nes_cpu);
                cpu_clocks += nes_cpu_run(&nes_cpu);
            }

            /* the ppu runs at a 3 times higher clock rate than the cpu
            so we need to give the ppu some clocks here to catchup */
            ppu_clocks = (cpu_clocks*3) + ppu_rest_clocks;
            ppu_status = 0;
            for(ppu_clock_index=0;ppu_clock_index<ppu_clocks;ppu_clock_index++)
            {
                ppu_status |= nes_ppu_run(&nes_ppu, nes_cpu.num_cycles);
                if(ppu_status & PPU_STATUS_FRAME_READY) break;
                else ppu_rest_clocks = 0;
            }

            ppu_rest_clocks = (ppu_clocks - ppu_clock_index);

            nes_ppu_dump_regs(&nes_ppu);

            if(ppu_status & PPU_STATUS_FRAME_READY) break;
        }

        /* Commented DEBUG code */
        // for(int i=0x0000;i<0x0FFF;i++)
        // {
        //     printf("Pattern Table 0: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }
        // for(int i=0x1000;i<0x1FFF;i++)
        // {
        //     printf("Pattern Table 1: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }
        // /* Nametable 0 contents */
        // for(int i=0x2000;i<0x23FF;i++)
        // {
        //     printf("Nametable 0: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }
        // /* Nametable 1 contents */
        // for(int i=0x2400;i<0x27FF;i++)
        // {
        //     printf("Nametable 1: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }
        // for(int i=0x3F00;i<=0x3F1F;i++)
        // {
        //     printf("PALLETE: %x %x\n", i, ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }
        // for(int i=0;i<256;i++)
        // {
        //     printf("OAM: %d %x\n", i, nes_memory.oam_memory[i]);
        // }

        SDL_UpdateTexture(texture, NULL, nes_ppu.screen_bitmap, 256 * sizeof(Uint32));

        SDL_RenderClear(renderer);  
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        /* 60 FPS framerate limit */
        while ((currentTime = SDL_GetTicks()) < (lastTime + FPS_UPDATE_TIME_MS));
        lastTime = currentTime;
        SDL_RenderPresent(renderer);
    }

    // Tidy up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
