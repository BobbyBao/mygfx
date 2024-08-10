#pragma once

#include <functional>

#include <SDL3/SDL.h>

namespace mygfx {

class SDL {
public:
    static void showOpenFileDialog(SDL_Window* window, const SDL_DialogFileFilter* filters, const char* default_location, SDL_bool allow_many, const std::function<void(const char* const* filelist)>& fn);
    static void showSaveFileDialog(SDL_Window* window, const SDL_DialogFileFilter* filters, const char* default_location, const std::function<void(const char* const* filelist)>& fn);
    static void showOpenFolderDialog(SDL_Window* window, const char* default_location, SDL_bool allow_many, const std::function<void(const char* const* filelist)>& fn);
};

}