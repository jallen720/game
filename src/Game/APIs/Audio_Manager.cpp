#include "Game/APIs/Audio_Manager.hpp"

#include <string>
#include <map>
#include "Nito/APIs/Audio.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Collection.hpp"


using std::string;
using std::map;

// Nito/APIs/Audio.hpp
using Nito::create_audio_source;
using Nito::play_audio_source;
using Nito::stop_audio_source;

// Nito/APIs/Scene.hpp
using Nito::set_scene_load_handler;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Scene_Music
{
    string id;
    string path;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string SCENE_CHANGE_HANDLER_ID("audio_manager_api");
static string current_music_id;
static bool engine_started = false;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void play_music(const string & music_id)
{
    if (!current_music_id.empty())
    {
        stop_audio_source(current_music_id);
    }

    play_audio_source(music_id);
    current_music_id = music_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void audio_manager_api_init()
{
    static const map<string, const Scene_Music> SCENE_MUSICS
    {
        {
            "default",
            {
                "main menu music",
                "resources/audio/loading_loop.wav"
            }
        },
        {
            "game",
            {
                "game music",
                "resources/audio/androids.wav"
            }
        },
    };

    set_scene_load_handler(SCENE_CHANGE_HANDLER_ID, [&](const string & scene_name) -> void
    {
        if (!engine_started)
        {
            for_each(SCENE_MUSICS, [](const string & /*scene_name*/, const Scene_Music & scene_music) -> void
            {
                create_audio_source(scene_music.id, scene_music.path, true, 0.1f);
            });

            engine_started = true;
        }

        play_music(SCENE_MUSICS.at(scene_name).id);
    });
}


} // namespace Game
