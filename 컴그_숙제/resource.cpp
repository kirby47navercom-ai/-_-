#include "resource.h"


float sound_v = 0.15f;
float effect_v = 0.15f;
FMOD::Sound* start_bgm;
FMOD::Sound* main_bgm;
FMOD::Sound* run_bgm;
FMOD::Sound* winner_bgm;
FMOD::Sound* walk_effect;
FMOD::Channel* bgmChannel;
FMOD::Channel* effectChannel;
FMOD::System* _system;
int Time = 0;
int power = 5;
namespace dtd{
    glm::vec3 v(0.0f);
}



void Sound_Create() {
	FMOD::System_Create(&_system);
	_system->init(512, FMOD_INIT_NORMAL, 0);
	_system->createSound("1.mp3", FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &start_bgm);
	_system->createSound("2.mp3", FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &main_bgm);
	_system->createSound("3.mp3", FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &run_bgm);
	_system->createSound("4.mp3", FMOD_DEFAULT, 0, &winner_bgm);
	_system->createSound("walk.mp3", FMOD_DEFAULT, 0, &walk_effect);
}

void UpdateSound() {
	_system->update();
	bgmChannel->setVolume(sound_v);
	effectChannel->setVolume(effect_v);
}
void StartShake(int _time, int _power) {
    Time = _time;
    power = _power;
}


void UpdateShake() {
    if (Time > 0) {
        dtd::v.x = (rand() % (power * 2 + 1)) - power;
        dtd::v.y = (rand() % (power * 2 + 1)) - power;
        dtd::v.z = (rand() % (power * 2 + 1)) - power;
        Time -= 16;
    }
    else {
        dtd::v.x = 0;
        dtd::v.y = 0;
		dtd::v.z = 0;
    }
}