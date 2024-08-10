#include "SDL.h"
#include "utils/Log.h"

namespace mygfx {

static void dialogFileCallback(void* userdata, const char* const* filelist, int filter)
{
    if (filter == -1) {
        auto err = SDL_GetError();
        LOG_ERROR(err);
        return;
    }

    auto fn = (std::function<void(const char* const* filelist)>*)userdata;
    fn->operator()(filelist);
    delete fn;
}

void SDL::showOpenFileDialog(SDL_Window* window, const SDL_DialogFileFilter* filters, const char* default_location, SDL_bool allow_many, const std::function<void(const char* const* filelist)>& fn)
{
    SDL_ShowOpenFileDialog(&dialogFileCallback, new std::function<void(const char* const*)>(fn), window, filters, default_location, allow_many);
}

void SDL::showSaveFileDialog(SDL_Window* window, const SDL_DialogFileFilter* filters, const char* default_location, const std::function<void(const char* const* filelist)>& fn)
{
    SDL_ShowSaveFileDialog(&dialogFileCallback, new std::function<void(const char* const*)>(fn), window, filters, default_location);
}

void SDL::showOpenFolderDialog(SDL_Window* window, const char* default_location, SDL_bool allow_many, const std::function<void(const char* const* filelist)>& fn)
{
    SDL_ShowOpenFolderDialog(&dialogFileCallback, new std::function<void(const char* const*)>(fn), window, default_location, allow_many);
}
}