#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

void buzzer_init();
void buzzer_beep(uint32_t freq, uint32_t duration_ms);

// Efectos de sonido retro
void buzzer_play_happy();
void buzzer_play_eat();
void buzzer_play_poop();
void buzzer_play_clean();
void buzzer_play_sick();
void buzzer_play_click();

#endif // BUZZER_H
