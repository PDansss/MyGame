#include <SDL3/SDL.h>
#include <cstring>
#include <iostream>
#include <vector>

#pragma once

using namespace std;

namespace my_audio
{
void MyCallBack(void* userdata, Uint8* stream, int len);

struct my_audio
{
    void set_data(const char* path, SDL_AudioSpec& device_info, bool loop)
    {
        SDL_AudioSpec wav;
        wav.freq     = 44100;
        wav.format   = AUDIO_S16;
        wav.channels = 2;
        wav.samples  = 1024;
        wav.callback = MyCallBack;
        wav.silence  = 0;
        wav.userdata = this;

        if (SDL_LoadWAV(path, &wav, &audio_buffer, &buffer_len) == NULL)
        {
            cout << "I can't load WAV file!" << endl;
        }
        else
        {
            cout << "I load WAV file!" << endl;
            cout << "file freq " << wav.freq << endl;
            cout << "file format " << wav.format << endl;
            cout << "file samples " << wav.samples << endl;
        }

        Uint8* output_bytes;
        int    output_length;

        int convert_status =
            SDL_ConvertAudioSamples(wav.format,
                                    wav.channels,
                                    wav.freq,
                                    audio_buffer,
                                    static_cast<int>(buffer_len),
                                    device_info.format,
                                    device_info.channels,
                                    device_info.freq,
                                    &output_bytes,
                                    &output_length);
        if (0 != convert_status)
        {
            cout << "failed to convert WAV byte stream: " << SDL_GetError();
            throw std::runtime_error("FATAL ERROR IN CONVERTING!");
        }
        cout << "I converted WAV!" << endl;
        SDL_free(audio_buffer);
        audio_buffer  = output_bytes;
        buffer_len    = static_cast<uint32_t>(output_length);
        looped        = loop;
        continue_play = false;
    }

    Uint8* audio_buffer = nullptr;
    size_t offset       = 0;
    Uint32 buffer_len   = 0;

    bool play          = false;
    bool continue_play = false;
    bool looped;

    void play_audio_from_beginning()
    {
        offset = 0;
        play   = true;
    }

    void stop_audio()
    {
        offset        = 0;
        play          = false;
        continue_play = false;
    }

    void play_audio() { play = true; }
};

vector<my_audio*> sounds;

void MyCallBack(void* userdata, Uint8* stream, int len)
{
    memset(stream, 0, static_cast<size_t>(len));
    size_t length = static_cast<size_t>(len);

    int vol;
    for (int i = 0; i < sounds.size(); i++)
    {
        if (i == 0)
            vol = 100;
        else
            vol = 128;
        my_audio* data = sounds[i];

        if (data->continue_play || data->play)
        {
            data->continue_play = true;
            while (length > 0)
            {
                data->play = false;
                const Uint8* current_position =
                    data->audio_buffer + data->offset;
                const size_t part = data->buffer_len - data->offset;
                if (length < part)
                {
                    SDL_MixAudioFormat(
                        stream, current_position, AUDIO_S16, length, vol);
                    data->offset += static_cast<size_t>(len);
                    break;
                }
                else
                {
                    SDL_MixAudioFormat(
                        stream, current_position, AUDIO_S16, part, vol);
                    data->offset = 0;
                    length -= part;
                    data->continue_play = data->looped;
                }
            }
        }
    }
}

} // namespace my_audio
