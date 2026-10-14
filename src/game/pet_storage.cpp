#include "pet_storage.h"
#include <Preferences.h>

static Preferences prefs;
static const char* PREF_NAMESPACE = "tamagotchi";

void pet_storage_init() {
    // Inicialización del subsistema NVS
}

bool pet_storage_save(const PetStats& stats) {
    if (!prefs.begin(PREF_NAMESPACE, false)) {
        return false;
    }
    prefs.putUChar("type", stats.pet_type);
    prefs.putUChar("hunger", stats.hunger);
    prefs.putUChar("happy", stats.happiness);
    prefs.putUChar("energy", stats.energy);
    prefs.putUChar("hygiene", stats.hygiene);
    prefs.putUChar("poops", stats.poop_count);
    prefs.putUInt("age", stats.age_seconds);
    prefs.putUInt("care", stats.care_score);
    prefs.putUChar("state", (uint8_t)stats.state);
    prefs.putBool("sleeping", stats.is_sleeping);
    prefs.putBool("sick", stats.is_sick);
    prefs.putBool("chosen", stats.has_chosen_pet);
    prefs.putBool("has_data", true);
    prefs.end();
    return true;
}

bool pet_storage_load(PetStats& stats) {
    if (!prefs.begin(PREF_NAMESPACE, true)) {
        return false;
    }
    bool has_data = prefs.getBool("has_data", false);
    if (!has_data) {
        prefs.end();
        return false;
    }

    stats.pet_type        = prefs.getUChar("type", 0);
    stats.hunger          = prefs.getUChar("hunger", 0);
    stats.happiness       = prefs.getUChar("happy", 100);
    stats.energy          = prefs.getUChar("energy", 100);
    stats.hygiene         = prefs.getUChar("hygiene", 100);
    stats.poop_count      = prefs.getUChar("poops", 0);
    stats.age_seconds     = prefs.getUInt("age", 0);
    stats.care_score      = prefs.getUInt("care", 0);
    stats.state           = (PetState)prefs.getUChar("state", (uint8_t)STATE_BABY);
    stats.is_sleeping     = prefs.getBool("sleeping", false);
    stats.is_sick         = prefs.getBool("sick", false);
    stats.has_chosen_pet  = prefs.getBool("chosen", true); // Compatibilidad con guardados previos

    prefs.end();
    return true;
}

void pet_storage_clear() {
    if (prefs.begin(PREF_NAMESPACE, false)) {
        prefs.clear();
        prefs.end();
    }
}
