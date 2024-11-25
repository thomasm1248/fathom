#include <SDL.h>
#include <iostream>
#include "classes/TextBox.h"
#include "classes/View.h"

int main(int argc, char** argv) {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Error initializing SDL: " << SDL_GetError() << '\n';
        return 1;
    }
    /*SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);*/

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Fathom",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        500, 500,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );
    if(!window) {
        std::cout << "Error creating window: " << SDL_GetError() << '\n';
        return 1;
    }

    // Create SDL renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        printf("Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize TTF
    TTF_Init();

    // Create default view
    View* currentView = new View(renderer, window);

    // Main loop
    while(true){
        // Poll events
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            // Stop if window is closed
            if( event.type == SDL_QUIT ){
                exit(0);
            }
            // Send events to View
            currentView->handleEvent(event);
        }

        // Let View do first pass to see what needs to be drawn
        currentView->checkWhatNeedsToBeRedrawn();

        // Render UI
        if(currentView->requestsToBeRedrawn())
            currentView->render();

        // Display backbuffer
        SDL_RenderPresent(renderer);

        // Wait for 1/60th of a second
        SDL_Delay(17);
    }

    // Delete views
    delete currentView;
    currentView = nullptr;

    // Free resources
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
