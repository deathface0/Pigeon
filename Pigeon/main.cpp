#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

int main(int argc, char** argv) {
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Crear una ventana con SDL
    SDL_Window* window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window) {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Bucle principal
    bool quit = false;
    while (!quit) {
        // Manejar eventos de SDL
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Aquí puedes agregar lógica para la renderización y gestión de la interfaz de usuario con SDL

        // Intercambiar el búfer de la ventana
        SDL_GL_SwapWindow(window);
    }

    // Liberar recursos
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}