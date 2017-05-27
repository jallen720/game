#include "Game/APIs/Audio_Manager.hpp"

#include <map>
#include <vector>
#include "Nito/APIs/Audio.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/Collection.hpp"


using std::string;
using std::map;
using std::vector;

// Nito/APIs/Audio.hpp
using Nito::create_audio_source;
using Nito::play_audio_source;
using Nito::stop_audio_source;
using Nito::audio_source_playing;
using Nito::set_audio_source_buffer;
using Nito::set_audio_source_volume;

// Nito/APIs/Scene.hpp
using Nito::set_scene_load_handler;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;

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
static const float GLOBAL_VOLUME = 0.1f;
static string current_music_id;
static vector<string> sound_audio_source_pool;


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
    static bool music_started = false;

    set_scene_load_handler(SCENE_CHANGE_HANDLER_ID, [&](const string & /*scene_name*/) -> void
    {
        if (!music_started)
        {
            create_audio_source("music", "resources/audio/androids.wav", true, GLOBAL_VOLUME);
            play_music("music");
            music_started = true;
        }
    });
}


void play_sound(const string & path, float volume = GLOBAL_VOLUME)
{
#if __gnu_linux__
    string sound_audio_source_id;


    // Check for an existing audio source that isn't playing to be used to play the sound.
    for (const string & sound_audio_source_pool_id : sound_audio_source_pool)
    {
        if (!audio_source_playing(sound_audio_source_pool_id))
        {
            sound_audio_source_id = sound_audio_source_pool_id;
            break;
        }
    }


    // If an existing audio source in the sound audio source pool couldn't be found that wasn't currently playing,
    // create a new audio source and add it to the pool.
    if (sound_audio_source_id.empty())
    {
        sound_audio_source_id = "sound_audio_source " + to_string(sound_audio_source_pool.size());
        sound_audio_source_pool.push_back(sound_audio_source_id);
        create_audio_source(sound_audio_source_id);
    }


    set_audio_source_buffer(sound_audio_source_id, path);
    set_audio_source_volume(sound_audio_source_id, volume);
    play_audio_source(sound_audio_source_id);
#endif
}


} // namespace Game
