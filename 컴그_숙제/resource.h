#pragma once

#include "dtd.h"
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

extern float sound_v;
extern float effect_v;
extern FMOD::Sound* start_bgm;
extern FMOD::Sound* main_bgm;
extern FMOD::Sound* run_bgm;
extern FMOD::Sound* winner_bgm;
extern FMOD::Sound* walk_effect;
extern FMOD::Channel* bgmChannel;
extern FMOD::Channel* effectChannel;
extern FMOD::System* _system;
extern int Time;
extern int power;
namespace dtd {
	extern glm::vec3 v;
}


void Sound_Create();
void PlayBGM();
void PlayEffect();
void UpdateSound();
void StartShake(int duration, int power);
void UpdateShake();