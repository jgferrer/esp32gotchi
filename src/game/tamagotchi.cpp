#include "tamagotchi.h"
#include "pet_storage.h"
#include "hal/buzzer.h"
#include "hal/status_led.h"

Tamagotchi pet;

Tamagotchi::Tamagotchi() {
    _last_tick_time = 0;
    _last_save_time = 0;
    _action_end_time = 0;
    _action_state = STATE_BABY;
    _meals_since_poop = 0;
}

void Tamagotchi::init() {
    if (!pet_storage_load(_stats)) {
        // No hay partida previa guardada -> requerir selección inicial
        _stats.has_chosen_pet = false;
        _stats.pet_type = PET_DINO;
        reset();
    } else {
        _last_tick_time = millis();
        _last_save_time = millis();
    }
}

void Tamagotchi::select_pet(uint8_t type) {
    _stats.pet_type = (type < 3) ? type : 0;
    _stats.has_chosen_pet = true;
    reset();
    pet_storage_save(_stats);
}

void Tamagotchi::reset() {
    _stats.hunger = 20;
    _stats.happiness = 90;
    _stats.energy = 100;
    _stats.hygiene = 100;
    _stats.poop_count = 0;
    _stats.age_seconds = 0;
    _stats.care_score = 0;
    _stats.state = STATE_EGG;
    _stats.is_sleeping = false;
    _stats.is_sick = false;
    _meals_since_poop = 0;
    _action_end_time = 0;

    _last_tick_time = millis();
    _last_save_time = millis();
    if (_stats.has_chosen_pet) {
        pet_storage_save(_stats);
    }
    status_led_set(LED_COLOR_GREEN);
    buzzer_play_happy();
}

void Tamagotchi::full_reset() {
    pet_storage_clear();
    _stats.has_chosen_pet = false;
    _stats.pet_type = PET_DINO;
    reset();
}

PetState Tamagotchi::getState() const {
    if (_action_end_time > millis()) {
        return _action_state;
    }
    return _stats.state;
}

bool Tamagotchi::feed() {
    if (_stats.state == STATE_DEAD || _stats.is_sleeping) return false;

    if (_stats.hunger > 0) {
        if (_stats.hunger >= 25) _stats.hunger -= 25;
        else _stats.hunger = 0;
    }

    if (_stats.happiness < 100) {
        _stats.happiness = min(100, _stats.happiness + 5);
    }

    _meals_since_poop++;
    _action_state = STATE_EATING;
    _action_end_time = millis() + 2000;

    status_led_set(LED_COLOR_GREEN);
    buzzer_play_eat();

    // Cada 2 o 3 comidas, hace caca
    if (_meals_since_poop >= 2 && _stats.poop_count < MAX_POOPS) {
        _meals_since_poop = 0;
        _stats.poop_count++;
    }

    pet_storage_save(_stats);
    return true;
}

bool Tamagotchi::clean() {
    if (_stats.state == STATE_DEAD) return false;

    if (_stats.poop_count > 0 || _stats.hygiene < 100) {
        _stats.poop_count = 0;
        _stats.hygiene = 100;
        _stats.happiness = min(100, _stats.happiness + 15);

        status_led_set(LED_COLOR_CYAN);
        buzzer_play_clean();
        pet_storage_save(_stats);
        return true;
    }
    return false;
}

bool Tamagotchi::play() {
    if (_stats.state == STATE_DEAD || _stats.is_sleeping || _stats.is_sick) {
        buzzer_play_sick();
        return false;
    }

    _stats.happiness = min(100, _stats.happiness + 25);
    _stats.hunger = min(100, _stats.hunger + 10);
    if (_stats.energy >= 15) _stats.energy -= 15;
    else _stats.energy = 0;

    _action_state = STATE_PLAYING;
    _action_end_time = millis() + 2000;

    status_led_set(LED_COLOR_MAGENTA);
    buzzer_play_happy();
    pet_storage_save(_stats);
    return true;
}

bool Tamagotchi::toggle_sleep() {
    if (_stats.state == STATE_DEAD) return false;

    _stats.is_sleeping = !_stats.is_sleeping;
    if (_stats.is_sleeping) {
        status_led_set(LED_COLOR_BLUE);
        buzzer_beep(600, 100);
        buzzer_beep(400, 150);
    } else {
        status_led_set(LED_COLOR_GREEN);
        buzzer_play_happy();
    }

    pet_storage_save(_stats);
    return true;
}

bool Tamagotchi::cure() {
    if (_stats.state == STATE_DEAD) return false;

    if (_stats.is_sick) {
        _stats.is_sick = false;
        _stats.happiness = min(100, _stats.happiness + 20);
        status_led_set(LED_COLOR_GREEN);
        buzzer_play_happy();
        pet_storage_save(_stats);
        return true;
    }
    return false;
}

void Tamagotchi::_tick() {
    if (!_stats.has_chosen_pet) return;

    _stats.age_seconds += (TICK_INTERVAL_MS / 1000);

    // Sistema de puntuación de buen cuidado:
    // Si la mascota está bien alimentada, feliz, aseada y sana, suma puntos
    if (_stats.hunger < 50 && _stats.happiness > 50 && _stats.hygiene > 50 && !_stats.is_sick) {
        _stats.care_score++;
    }

    // Evolución basada en edad y cuidados
    if (_stats.state == STATE_EGG && _stats.age_seconds >= 20) {
        _stats.state = STATE_BABY;
        buzzer_play_happy();
    } else if (_stats.state == STATE_BABY && _stats.age_seconds >= 180 && _stats.care_score >= 20) {
        _stats.state = STATE_TEEN; // Crece a Joven si se le cuida bien
        buzzer_play_happy();
    } else if (_stats.state == STATE_TEEN && _stats.age_seconds >= 420 && _stats.care_score >= 50) {
        _stats.state = STATE_ADULT; // Crece a Adulto evolucionado
        buzzer_play_happy();
    }

    if (_stats.state == STATE_DEAD) return;

    // Desgaste de necesidades
    if (_stats.is_sleeping) {
        _stats.energy = min(100, _stats.energy + 3);
        if (_stats.energy >= 100) {
            _stats.is_sleeping = false; // Se despierta al descansar totalmente
            status_led_set(LED_COLOR_GREEN);
        }
    } else {
        if (_stats.energy > 0) _stats.energy -= ENERGY_DECREASE_TICK;
        if (_stats.hunger < 100) _stats.hunger += HUNGER_DECREASE_TICK;
        if (_stats.happiness > 0) _stats.happiness -= HAPPINESS_DECREASE_TICK;
    }

    // Impacto de suciedad / cacas
    if (_stats.poop_count > 0) {
        uint8_t dirty_impact = _stats.poop_count * 2;
        if (_stats.hygiene > dirty_impact) _stats.hygiene -= dirty_impact;
        else _stats.hygiene = 0;
    }

    // Comprobar si enferma
    if (!_stats.is_sick) {
        if (_stats.hygiene == 0 || _stats.hunger >= 95 || _stats.energy == 0) {
            _stats.is_sick = true;
            status_led_set(LED_COLOR_RED);
            buzzer_play_sick();
        }
    }

    // Comprobar muerte por abandono severo
    if (_stats.is_sick && _stats.hunger >= 100 && _stats.hygiene == 0) {
        _stats.state = STATE_DEAD;
        _stats.is_sleeping = false;
        status_led_set(LED_COLOR_RED);
        buzzer_play_sick();
    }

    // Actualizar color LED según estado
    if (_stats.state == STATE_DEAD) {
        status_led_set(LED_COLOR_RED);
    } else if (_stats.is_sick) {
        status_led_set(LED_COLOR_RED);
    } else if (_stats.is_sleeping) {
        status_led_set(LED_COLOR_BLUE);
    } else if (_stats.poop_count > 0 || _stats.hunger > 70) {
        status_led_set(LED_COLOR_YELLOW);
    } else {
        status_led_set(LED_COLOR_GREEN);
    }
}

void Tamagotchi::update() {
    uint32_t now = millis();

    // Ticks periódicos de simulación
    if (now - _last_tick_time >= TICK_INTERVAL_MS) {
        _last_tick_time = now;
        _tick();
    }

    // Autoguardado en NVS
    if (now - _last_save_time >= SAVE_INTERVAL_MS) {
        _last_save_time = now;
        if (_stats.has_chosen_pet) {
            pet_storage_save(_stats);
        }
    }
}
