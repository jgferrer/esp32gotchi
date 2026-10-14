#include "buzzer.h"
#include "config.h"

void buzzer_init() {
    ledcSetup(BUZZER_CHANNEL, 2000, 8);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0);
}

void buzzer_beep(uint32_t freq, uint32_t duration_ms) {
    if (freq == 0) {
        ledcWrite(BUZZER_CHANNEL, 0);
    } else {
        ledcWriteTone(BUZZER_CHANNEL, freq);
    }
    delay(duration_ms);
    ledcWrite(BUZZER_CHANNEL, 0);
}

void buzzer_play_click() {
    buzzer_beep(1800, 15);
}

void buzzer_play_happy() {
    buzzer_beep(988, 70);  // B5
    buzzer_beep(1319, 70); // E6
    buzzer_beep(1760, 100);// A6
}

void buzzer_play_eat() {
    for (int i = 0; i < 3; i++) {
        buzzer_beep(600, 40);
        delay(30);
        buzzer_beep(900, 50);
        delay(40);
    }
}

void buzzer_play_poop() {
    buzzer_beep(400, 100);
    buzzer_beep(300, 150);
    buzzer_beep(200, 200);
}

void buzzer_play_clean() {
    buzzer_beep(1200, 60);
    buzzer_beep(1500, 60);
    buzzer_beep(1800, 80);
}

void buzzer_play_sick() {
    buzzer_beep(350, 120);
    buzzer_beep(300, 120);
    buzzer_beep(250, 180);
}
