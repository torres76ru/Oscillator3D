#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <fftw3.h>
#include "Constants.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

#define DecodeBE(stream) (((uint16_t)stream[0] << 8) | (uint16_t)stream[1])
#define DecodeLE(stream) ((uint16_t)stream[0] | ((uint16_t)stream[1] << 8))


typedef struct AudioData
{
    uint8_t* audio_stream;
    uint8_t* audio_pos;
    uint32_t audio_length;

    SDL_AudioFormat audio_format;
    SDL_AudioDeviceID audio_device;

    void advance(int len)
    {
        audio_pos += len;
        audio_length -= len;
    }
} AudioData;


static double SampleAudio(uint8_t* stream, SDL_AudioFormat format)
{
    uint16_t val = SDL_AUDIO_ISLITTLEENDIAN(format) ? DecodeLE(stream) : DecodeBE(stream);

    if (SDL_AUDIO_ISSIGNED(format))
        return ((int16_t)val) / 32768.0;

    return val / 65535.0;
}

typedef struct AudioFFTW
{
    fftw_plan plan;
    fftw_complex* in;
    fftw_complex* out;

    void populate_input(uint8_t* stream, SDL_AudioFormat format)
    {
        double multiplier;

        for (int i = 0; i < SAMPLES; i++)
        {
            multiplier = 0.5 * (1 - cos(2 * M_PI * i / SAMPLES));
            in[i][0] = SampleAudio(stream, format) * multiplier; // Real
            in[i][1] = 0.0; // Imaginary

            stream += 2;
        }
    }

    double get_magnitude(int freq)
    {
        int j = (freq * SAMPLES) / (float)SAMPLE_RATE;
        double re, im, magnitude;

        re = out[j][0];
        im = out[j][1];

        magnitude = sqrt((re * re) + (im * im));
        return magnitude;
    }

    double get_magnitude_range(int start_freq, int end_freq)
    {
        double magnitude, temp;

        magnitude = 0;
        for (int j = start_freq; j < end_freq; j++)
        {
            temp = get_magnitude(j);
            magnitude = MAX(magnitude, temp);
        }
        return magnitude;
    }

    void execute()
    {
        fftw_execute(plan);
    }
} AudioFFTW;

typedef struct AudioUserdata
{
    AudioData* audioData;
    AudioFFTW* audioFFTW;
} AudioUserdata;


static void audio_callback(void* udata, Uint8* stream, int len)
{
    AudioUserdata* wav_audio_userdata = (AudioUserdata*)udata;

    AudioData* wav_audio_data = wav_audio_userdata->audioData;
    AudioFFTW* wav_audio_fftw = wav_audio_userdata->audioFFTW;

    Uint32 length = (Uint32)len / CHANELS;

    length = MIN((int)wav_audio_data->audio_length, length);

   
    if (wav_audio_data->audio_length <= 0)
    {
        SDL_PauseAudioDevice(wav_audio_data->audio_device, 1);
        return;
    }

    SDL_memcpy(stream, wav_audio_data->audio_pos, length);

    wav_audio_fftw->populate_input(stream, wav_audio_data->audio_format);
    wav_audio_fftw->execute();
    
    wav_audio_data->advance(length);
}


class MyAudio
{
private:
    Mix_Chunk* sound;

    SDL_AudioSpec wav_audio_spec;
    SDL_AudioSpec obtained;
    SDL_AudioDeviceID device;

    AudioUserdata wav_audio_userdata;
    AudioData wav_audio_data;
    AudioFFTW* wav_audio_fftw;

    bool paused;
    bool loaded;

public:
    MyAudio() :
        paused(false)
    {
        loaded = true;
        // Load sound file
        Mix_Init(MIX_INIT_FLAC);
        Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 1024);
        

        wav_audio_fftw = (AudioFFTW*)malloc(sizeof(AudioFFTW));

        wav_audio_userdata.audioData = &wav_audio_data;
        wav_audio_userdata.audioFFTW = wav_audio_fftw;

        wav_audio_fftw->in = (fftw_complex*)malloc(sizeof(fftw_complex) * SAMPLES);
        wav_audio_fftw->out = (fftw_complex*)malloc(sizeof(fftw_complex) * SAMPLES);

        wav_audio_fftw->plan = fftw_plan_dft_1d(
            SAMPLES,
            wav_audio_fftw->in,
            wav_audio_fftw->out,
            FFTW_FORWARD,
            FFTW_MEASURE
        );

        if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO))
        {
            this->loaded = false;
            printf("Could not load sound devicce\n");
        }
            

        if (SDL_LoadWAV(MUSIC_PATH, &wav_audio_spec, &wav_audio_data.audio_stream, &wav_audio_data.audio_length) == NULL)
        {
            this->loaded = false;
            printf("Could not load sound devicce\n");
        }

        wav_audio_data.audio_pos = wav_audio_data.audio_stream;
        wav_audio_data.audio_format = wav_audio_spec.format;

        wav_audio_spec.userdata = &wav_audio_userdata;
        wav_audio_spec.callback = audio_callback;
        wav_audio_spec.channels = CHANELS;
        wav_audio_spec.samples = SAMPLES;
        wav_audio_spec.freq = SAMPLE_RATE;

        device = SDL_OpenAudioDevice(NULL, 0, &wav_audio_spec, &obtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

        if (device == 0)
        {
            this->loaded = false;
            printf("Could not load sound devicce\n");
        }

        wav_audio_data.audio_device = device;
        
        sound  = Mix_LoadWAV(MUSIC_PATH);

        if (!sound) {
            this->loaded = false;
            printf("Could not load sound file: %s\n", Mix_GetError());
        }
        if (loaded)
        {
            Mix_Volume(-1, MIX_MAX_VOLUME / 8);
        }
        

        
    }
    ~MyAudio()
    {
        // Clean up
        Mix_FreeChunk(sound);
        Mix_CloseAudio();
        Mix_Quit();
    }


    double getMagnitude(int freq1, int freq2)
    {
        return wav_audio_fftw->get_magnitude_range(freq1, freq2);
    }

    bool isPaused() { return this->paused; }

    void play()
    {
        if (loaded)
        {
            this->paused = false;
            Mix_Resume(-1);
            SDL_PauseAudioDevice(device, 0);
        }
        
    }

    void pause()
    {
        if (loaded)
        {
            this->paused = true;
            Mix_Pause(-1);
            SDL_PauseAudioDevice(device, 1);
        }
    }

    void stopAudio() {
        
        
        SDL_CloseAudioDevice(device);
        SDL_PauseAudioDevice(device, 1);
    }

};