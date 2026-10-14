#ifndef TAMAGOTCHI_H
#define TAMAGOTCHI_H

#include <Arduino.h>
#include "config.h"

enum PetType {
    PET_DINO = 0,
    PET_CHIK = 1,
    PET_CAT  = 2
};

enum PetState {
    STATE_EGG,        // Huevo inicial
    STATE_BABY,       // Mascota pequeña
    STATE_TEEN,       // Mascota joven / adolescente
    STATE_ADULT,      // Mascota adulta
    STATE_EATING,     // Animación de comer
    STATE_POOPING,    // Animación de hacer caca
    STATE_PLAYING,    // Animación de jugar / feliz
    STATE_SLEEPING,   // Durmiendo con luz apagada
    STATE_SICK,       // Enfermo
    STATE_DEAD        // Fallecido (fantasma)
};

struct PetStats {
    uint8_t pet_type;     // 0 = Dino, 1 = Chik, 2 = Cat
    uint8_t hunger;       // 0 = Lleno, 100 = Hambriento
    uint8_t happiness;    // 100 = Feliz, 0 = Deprimido
    uint8_t energy;       // 100 = Máxima energía, 0 = Exhausto
    uint8_t hygiene;      // 100 = Limpio, 0 = Sucio
    uint8_t poop_count;   // 0 a MAX_POOPS
    uint32_t age_seconds;
    uint32_t care_score;  // Puntuación acumulada de buen cuidado
    PetState state;
    bool is_sleeping;
    bool is_sick;
    bool has_chosen_pet;  // True si ya se eligió mascota inicial
};

class Tamagotchi {
public:
    Tamagotchi();
    void init();
    void update();

    // Acciones táctiles del jugador
    void select_pet(uint8_t type);
    bool feed();
    bool clean();
    bool play();
    bool toggle_sleep();
    bool cure();
    void reset();
    void full_reset();

    // Getters
    const PetStats& getStats() const { return _stats; }
    PetState getState() const;
    uint8_t getPetType() const { return _stats.pet_type; }
    bool hasChosenPet() const { return _stats.has_chosen_pet; }
    uint32_t getCareScore() const { return _stats.care_score; }
    uint8_t getHunger() const { return _stats.hunger; }
    uint8_t getHappiness() const { return _stats.happiness; }
    uint8_t getEnergy() const { return _stats.energy; }
    uint8_t getHygiene() const { return _stats.hygiene; }
    uint8_t getPoopCount() const { return _stats.poop_count; }
    uint32_t getAgeSeconds() const { return _stats.age_seconds; }
    bool isSleeping() const { return _stats.is_sleeping; }
    bool isSick() const { return _stats.is_sick; }

    void setStats(const PetStats& stats) { _stats = stats; }

private:
    PetStats _stats;
    uint32_t _last_tick_time;
    uint32_t _last_save_time;
    uint32_t _action_end_time;
    PetState _action_state;
    uint8_t  _meals_since_poop;

    void _tick();
};

extern Tamagotchi pet;

#endif // TAMAGOTCHI_H
