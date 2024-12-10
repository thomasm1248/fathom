#include <SDL.h>
#include <iostream>
#include "classes/TextBox.h"
#include "classes/View.h"
#include "classes/ViewFileJSON.h"

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

    // Initialize label node font
    SDL_Color labelFontForeground{255, 255, 255, 255};
    SDL_Color labelFontBackground{21, 21, 21, 255};
    std::shared_ptr<Font> labelNodeFont = std::make_shared<Font>("Aovel Sans Rounded.ttf", 40, labelFontForeground, labelFontBackground);

    // Create default view
    // TODO check if argv[1] exists, and make two different constructors for View
    View* currentView = new View(renderer, window, std::make_shared<ViewFileJSON>(std::string(argv[1]), renderer, labelNodeFont), labelNodeFont);

    // Create cursor
    uint8_t mask[] = {
        0xff, 0xff, 0xff,
        0xff, 0xff, 0xfe,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0xc0, 0x00, 0x00,
        0x80, 0x00, 0x00
    };
    uint8_t data[] = {
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00
    };
    SDL_Cursor* cursor = SDL_CreateCursor(data, mask, 24, 24, 0, 0);

    // Change cursor
    SDL_SetCursor(cursor);

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

        // Process dynamic content
        currentView->processDynamicContent();

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
    SDL_FreeCursor(cursor);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
