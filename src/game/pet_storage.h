#ifndef PET_STORAGE_H
#define PET_STORAGE_H

#include "tamagotchi.h"

void pet_storage_init();
bool pet_storage_save(const PetStats& stats);
bool pet_storage_load(PetStats& stats);
void pet_storage_clear();

#endif // PET_STORAGE_H
