#pragma once


#include <string>


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void audio_manager_api_init();
void play_sound(const std::string & path, float volume);


} // namespace Game
