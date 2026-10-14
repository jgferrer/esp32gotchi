#include "ui.h"
#include "game/tamagotchi.h"
#include "game/pet_sprites.h"
#include "hal/buzzer.h"
#include "hal/wifi_manager.h"
#include <stdio.h>

// ==========================================
// CONTENEDORES PRINCIPALES DE VISTAS
// ==========================================
static lv_obj_t *main_screen;
static lv_obj_t *game_main_cont;     // Contenedor del juego activo
static lv_obj_t *select_screen_cont; // Pantalla de selección de mascota
static lv_obj_t *settings_modal_cont;// Pantalla de ajustes (Brillo, WiFi)

// ==========================================
// WIDGETS DE LA PANTALLA DE JUEGO
// ==========================================
static lv_obj_t *top_bar_cont;
static lv_obj_t *game_area_cont;
static lv_obj_t *bottom_bar_cont;

// Barras de progreso de estado
static lv_obj_t *bar_hunger;
static lv_obj_t *bar_happy;
static lv_obj_t *bar_energy;
static lv_obj_t *bar_hygiene;

// Etiquetas de texto superiores
static lv_obj_t *btn_cfg;
static lv_obj_t *lbl_title;
static lv_obj_t *lbl_clock;
static lv_obj_t *lbl_speech;

// Widget principal del pet y objetos
static lv_obj_t *img_pet;
static lv_obj_t *img_action;
static lv_obj_t *img_poops[4];
static lv_obj_t *btn_revive;

// Botones de acción inferiores
static lv_obj_t *btn_feed;
static lv_obj_t *btn_clean;
static lv_obj_t *btn_play;
static lv_obj_t *btn_sleep;
static lv_obj_t *btn_cure;

// ==========================================
// WIDGETS DE LA PANTALLA DE AJUSTES
// ==========================================
static lv_obj_t *slider_brightness;
static lv_obj_t *lbl_bright_val;
static lv_obj_t *sw_wifi;
static lv_obj_t *btn_scan;
static lv_obj_t *dd_networks;
static lv_obj_t *ta_ssid;
static lv_obj_t *ta_pass;
static lv_obj_t *btn_connect;
static lv_obj_t *lbl_wifi_status;
static lv_obj_t *kb_settings;

// ==========================================
// VARIABLES DE ESTADO Y CONTROL
// ==========================================
static uint32_t last_anim_tick = 0;
static uint32_t last_clock_tick = 0;
static bool anim_frame_alt = false;
static bool night_mode_active = false;

// ==========================================
// FUNCIONES DE SOPORTE DE SPRITES
// ==========================================
static const lv_img_dsc_t* get_pet_sprite(uint8_t pet_type, PetState state, bool alt_frame) {
    if (state == STATE_EGG) {
        return &sprite_egg;
    }
    if (state == STATE_DEAD) {
        return &sprite_baby_dead;
    }

    if (pet_type == PET_DINO) {
        switch (state) {
            case STATE_BABY:
                return alt_frame ? &sprite_dino_baby_idle1 : &sprite_dino_baby_idle2;
            case STATE_TEEN:
                return alt_frame ? &sprite_dino_teen_idle1 : &sprite_dino_teen_idle2;
            case STATE_ADULT:
                return alt_frame ? &sprite_dino_adult_idle1 : &sprite_dino_adult_idle2;
            case STATE_EATING:
                return alt_frame ? &sprite_dino_baby_eat1 : &sprite_dino_baby_eat2;
            case STATE_PLAYING:
                return &sprite_dino_baby_happy;
            case STATE_SLEEPING:
                return &sprite_dino_baby_sleep;
            case STATE_SICK:
                return &sprite_dino_baby_sick;
            default:
                return &sprite_dino_baby_idle1;
        }
    } else if (pet_type == PET_CHIK) {
        switch (state) {
            case STATE_BABY:
                return alt_frame ? &sprite_chik_baby_idle1 : &sprite_chik_baby_idle2;
            case STATE_TEEN:
                return alt_frame ? &sprite_chik_teen_idle1 : &sprite_chik_teen_idle2;
            case STATE_ADULT:
                return alt_frame ? &sprite_chik_adult_idle1 : &sprite_chik_adult_idle2;
            case STATE_EATING:
                return alt_frame ? &sprite_chik_baby_eat1 : &sprite_chik_baby_eat2;
            case STATE_PLAYING:
                return &sprite_chik_baby_happy;
            case STATE_SLEEPING:
                return &sprite_chik_baby_sleep;
            case STATE_SICK:
                return &sprite_chik_baby_sick;
            default:
                return &sprite_chik_baby_idle1;
        }
    } else { // PET_CAT
        switch (state) {
            case STATE_BABY:
                return alt_frame ? &sprite_cat_baby_idle1 : &sprite_cat_baby_idle2;
            case STATE_TEEN:
                return alt_frame ? &sprite_cat_teen_idle1 : &sprite_cat_teen_idle2;
            case STATE_ADULT:
                return alt_frame ? &sprite_cat_adult_idle1 : &sprite_cat_adult_idle2;
            case STATE_EATING:
                return alt_frame ? &sprite_cat_baby_eat1 : &sprite_cat_baby_eat2;
            case STATE_PLAYING:
                return &sprite_cat_baby_happy;
            case STATE_SLEEPING:
                return &sprite_cat_baby_sleep;
            case STATE_SICK:
                return &sprite_cat_baby_sick;
            default:
                return &sprite_cat_baby_idle1;
        }
    }
}

// ==========================================
// EVENTOS DE LA PANTALLA DE JUEGO
// ==========================================
static void btn_feed_cb(lv_event_t *e) {
    buzzer_play_click();
    if (pet.feed()) {
        lv_label_set_text(lbl_speech, "Nam nam! Que rico!");
    } else {
        lv_label_set_text(lbl_speech, "No tengo hambre.");
    }
}

static void btn_clean_cb(lv_event_t *e) {
    buzzer_play_click();
    if (pet.clean()) {
        lv_label_set_text(lbl_speech, "Todo limpio!");
    } else {
        lv_label_set_text(lbl_speech, "Ya esta limpio!");
    }
}

static void btn_play_cb(lv_event_t *e) {
    buzzer_play_click();
    if (pet.play()) {
        lv_label_set_text(lbl_speech, "A jugar! Yupi!");
    } else {
        if (pet.isSick()) lv_label_set_text(lbl_speech, "Estoy malito...");
        else if (pet.isSleeping()) lv_label_set_text(lbl_speech, "Durmiendo... Zzz");
        else lv_label_set_text(lbl_speech, "Estoy cansado...");
    }
}

static void btn_sleep_cb(lv_event_t *e) {
    buzzer_play_click();
    if (pet.toggle_sleep()) {
        if (pet.isSleeping()) {
            lv_label_set_text(lbl_speech, "Buenas noches... Zzz");
        } else {
            lv_label_set_text(lbl_speech, "Buenos dias!");
        }
    }
}

static void btn_cure_cb(lv_event_t *e) {
    buzzer_play_click();
    if (pet.cure()) {
        lv_label_set_text(lbl_speech, "Me siento mejor!");
    } else {
        lv_label_set_text(lbl_speech, "No estoy malito!");
    }
}

static void btn_revive_cb(lv_event_t *e) {
    buzzer_play_click();
    // Volver a la pantalla de selección para que pueda elegir de nuevo
    lv_obj_add_flag(game_main_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(select_screen_cont, LV_OBJ_FLAG_HIDDEN);
}

// Abrir pantalla de ajustes
static void btn_cfg_cb(lv_event_t *e) {
    buzzer_play_click();
    uint8_t b = wifi_manager_get_brightness();
    lv_slider_set_value(slider_brightness, b, LV_ANIM_OFF);
    char buf[32];
    snprintf(buf, sizeof(buf), "BRILLO: %d%%", (int)(b * 100 / 255));
    lv_label_set_text(lbl_bright_val, buf);

    WifiSettings s = wifi_manager_get_settings();
    if (s.enabled) lv_obj_add_state(sw_wifi, LV_STATE_CHECKED);
    else lv_obj_clear_state(sw_wifi, LV_STATE_CHECKED);

    lv_textarea_set_text(ta_ssid, s.ssid);
    lv_textarea_set_text(ta_pass, s.password);
    lv_label_set_text(lbl_wifi_status, wifi_manager_get_status_str().c_str());

    lv_obj_clear_flag(settings_modal_cont, LV_OBJ_FLAG_HIDDEN);
}

// ==========================================
// EVENTOS DE LA PANTALLA DE SELECCIÓN
// ==========================================
static void card_select_pet_cb(lv_event_t *e) {
    uintptr_t type = (uintptr_t)lv_event_get_user_data(e);
    buzzer_play_click();
    pet.select_pet(type);

    lv_obj_add_flag(select_screen_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(game_main_cont, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(lbl_speech, "Ha nacido un huevo!");
    buzzer_play_happy();
}

// ==========================================
// EVENTOS DE LA PANTALLA DE AJUSTES
// ==========================================
static void btn_close_settings_cb(lv_event_t *e) {
    buzzer_play_click();
    lv_obj_add_flag(settings_modal_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(kb_settings, LV_OBJ_FLAG_HIDDEN);
}

static void slider_brightness_cb(lv_event_t *e) {
    int32_t val = lv_slider_get_value(slider_brightness);
    wifi_manager_set_brightness((uint8_t)val);
    char buf[32];
    snprintf(buf, sizeof(buf), "BRILLO: %d%%", (int)(val * 100 / 255));
    lv_label_set_text(lbl_bright_val, buf);
}

static void sw_wifi_cb(lv_event_t *e) {
    bool en = lv_obj_has_state(sw_wifi, LV_STATE_CHECKED);
    buzzer_play_click();
    wifi_manager_set_enabled(en);
    lv_label_set_text(lbl_wifi_status, wifi_manager_get_status_str().c_str());
}

static void btn_scan_cb(lv_event_t *e) {
    buzzer_play_click();
    lv_label_set_text(lbl_wifi_status, "Escaneando redes...");
    int n = wifi_manager_scan_networks();
    if (n <= 0) {
        lv_label_set_text(lbl_wifi_status, "No se hallaron redes");
        lv_dropdown_set_options(dd_networks, "Ninguna red");
        return;
    }
    String opts = "";
    for (int i = 0; i < n && i < 15; i++) {
        String ssid = wifi_manager_get_scanned_ssid(i);
        if (ssid.length() > 0) {
            if (opts.length() > 0) opts += "\n";
            opts += ssid;
        }
    }
    lv_dropdown_set_options(dd_networks, opts.c_str());
    char buf[48];
    snprintf(buf, sizeof(buf), "%d redes encontradas", n);
    lv_label_set_text(lbl_wifi_status, buf);
}

static void dd_networks_cb(lv_event_t *e) {
    char buf[40];
    lv_dropdown_get_selected_str(dd_networks, buf, sizeof(buf));
    if (strlen(buf) > 0 && strcmp(buf, "Ninguna red") != 0) {
        lv_textarea_set_text(ta_ssid, buf);
    }
}

static void btn_connect_cb(lv_event_t *e) {
    buzzer_play_click();
    const char *s = lv_textarea_get_text(ta_ssid);
    const char *p = lv_textarea_get_text(ta_pass);
    if (!s || strlen(s) == 0) {
        lv_label_set_text(lbl_wifi_status, "Escribe un SSID!");
        return;
    }
    lv_obj_add_state(sw_wifi, LV_STATE_CHECKED);
    wifi_manager_connect(s, p);
    lv_label_set_text(lbl_wifi_status, "Conectando...");
}

static void btn_reset_cb(lv_event_t *e) {
    buzzer_beep(400, 80);
    delay(40);
    buzzer_beep(250, 120);

    pet.full_reset();

    // Cerrar ventana de ajustes y teclado
    lv_obj_add_flag(settings_modal_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(kb_settings, LV_OBJ_FLAG_HIDDEN);

    // Ocultar juego y mostrar pantalla de selección de mascota
    lv_obj_add_flag(game_main_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(select_screen_cont, LV_OBJ_FLAG_HIDDEN);
}

static void ta_click_cb(lv_event_t *e) {
    lv_obj_t *ta = lv_event_get_target(e);
    lv_keyboard_set_textarea(kb_settings, ta);
    lv_obj_clear_flag(kb_settings, LV_OBJ_FLAG_HIDDEN);
}

static void kb_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb_settings, LV_OBJ_FLAG_HIDDEN);
    }
}

// ==========================================
// CONSTRUCCIÓN DE LA INTERFAZ (UI_INIT)
// ==========================================
void ui_init() {
    main_screen = lv_scr_act();
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x0F172A), 0);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);

    // ==========================================
    // 1. PANTALLA DE JUEGO (GAME_MAIN_CONT)
    // ==========================================
    game_main_cont = lv_obj_create(main_screen);
    lv_obj_set_size(game_main_cont, 240, 320);
    lv_obj_set_pos(game_main_cont, 0, 0);
    lv_obj_set_style_bg_color(game_main_cont, lv_color_hex(0x1E1B4B), 0);
    lv_obj_set_style_pad_all(game_main_cont, 0, 0);
    lv_obj_set_style_border_width(game_main_cont, 0, 0);
    lv_obj_set_style_radius(game_main_cont, 0, 0);
    lv_obj_clear_flag(game_main_cont, LV_OBJ_FLAG_SCROLLABLE);

    // 1.1 ÁREA SUPERIOR (TOP BAR)
    top_bar_cont = lv_obj_create(game_main_cont);
    lv_obj_set_size(top_bar_cont, 240, 68);
    lv_obj_set_pos(top_bar_cont, 0, 0);
    lv_obj_set_style_bg_color(top_bar_cont, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_radius(top_bar_cont, 0, 0);
    lv_obj_set_style_border_width(top_bar_cont, 0, 0);
    lv_obj_set_style_pad_all(top_bar_cont, 2, 0);
    lv_obj_clear_flag(top_bar_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Botón Ajustes [CFG]
    btn_cfg = lv_btn_create(top_bar_cont);
    lv_obj_set_size(btn_cfg, 34, 16);
    lv_obj_set_pos(btn_cfg, 4, 1);
    lv_obj_set_style_bg_color(btn_cfg, lv_color_hex(0x334155), 0);
    lv_obj_set_style_radius(btn_cfg, 4, 0);
    lv_obj_set_style_pad_all(btn_cfg, 0, 0);
    lv_obj_add_event_cb(btn_cfg, btn_cfg_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_cfg_txt = lv_label_create(btn_cfg);
    lv_label_set_text(lbl_cfg_txt, "CFG");
    lv_obj_set_style_text_font(lbl_cfg_txt, &lv_font_montserrat_10, 0);
    lv_obj_center(lbl_cfg_txt);

    // Título de la mascota
    lbl_title = lv_label_create(top_bar_cont);
    lv_label_set_text(lbl_title, "TAMAGOTCHI");
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x38BDF8), 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 1);

    // Reloj digital en tiempo real
    lbl_clock = lv_label_create(top_bar_cont);
    lv_label_set_text(lbl_clock, "12:00");
    lv_obj_set_style_text_font(lbl_clock, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_clock, lv_color_hex(0xFEF08A), 0);
    lv_obj_set_pos(lbl_clock, 196, 1);

    // Fila 1: HAM (Hambre) izquierda, FEL (Felicidad) derecha
    lv_obj_t *lbl_h = lv_label_create(top_bar_cont);
    lv_label_set_text(lbl_h, "HAM");
    lv_obj_set_style_text_font(lbl_h, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_h, lv_color_hex(0xF97316), 0);
    lv_obj_set_pos(lbl_h, 6, 18);

    bar_hunger = lv_bar_create(top_bar_cont);
    lv_obj_set_size(bar_hunger, 48, 8);
    lv_obj_set_pos(bar_hunger, 34, 20);
    lv_bar_set_range(bar_hunger, 0, 100);
    lv_obj_set_style_bg_color(bar_hunger, lv_color_hex(0x334155), 0);
    lv_obj_set_style_bg_color(bar_hunger, lv_color_hex(0xEA580C), LV_PART_INDICATOR);

    lv_obj_t *lbl_hap = lv_label_create(top_bar_cont);
    lv_label_set_text(lbl_hap, "FEL");
    lv_obj_set_style_text_font(lbl_hap, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_hap, lv_color_hex(0xEC4899), 0);
    lv_obj_set_pos(lbl_hap, 122, 18);

    bar_happy = lv_bar_create(top_bar_cont);
    lv_obj_set_size(bar_happy, 48, 8);
    lv_obj_set_pos(bar_happy, 148, 20);
    lv_bar_set_range(bar_happy, 0, 100);
    lv_obj_set_style_bg_color(bar_happy, lv_color_hex(0x334155), 0);
    lv_obj_set_style_bg_color(bar_happy, lv_color_hex(0xDB2777), LV_PART_INDICATOR);

    // Fila 2: ENE (Energía) izquierda, HIG (Higiene) derecha
    lv_obj_t *lbl_ene = lv_label_create(top_bar_cont);
    lv_label_set_text(lbl_ene, "ENE");
    lv_obj_set_style_text_font(lbl_ene, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_ene, lv_color_hex(0xEAB308), 0);
    lv_obj_set_pos(lbl_ene, 6, 32);

    bar_energy = lv_bar_create(top_bar_cont);
    lv_obj_set_size(bar_energy, 48, 8);
    lv_obj_set_pos(bar_energy, 34, 34);
    lv_bar_set_range(bar_energy, 0, 100);
    lv_obj_set_style_bg_color(bar_energy, lv_color_hex(0x334155), 0);
    lv_obj_set_style_bg_color(bar_energy, lv_color_hex(0xCA8A04), LV_PART_INDICATOR);

    lv_obj_t *lbl_hig = lv_label_create(top_bar_cont);
    lv_label_set_text(lbl_hig, "HIG");
    lv_obj_set_style_text_font(lbl_hig, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_hig, lv_color_hex(0x06B6D4), 0);
    lv_obj_set_pos(lbl_hig, 122, 32);

    bar_hygiene = lv_bar_create(top_bar_cont);
    lv_obj_set_size(bar_hygiene, 48, 8);
    lv_obj_set_pos(bar_hygiene, 148, 34);
    lv_bar_set_range(bar_hygiene, 0, 100);
    lv_obj_set_style_bg_color(bar_hygiene, lv_color_hex(0x334155), 0);
    lv_obj_set_style_bg_color(bar_hygiene, lv_color_hex(0x0891B2), LV_PART_INDICATOR);

    // Fila 3: Globo de diálogo / mensaje centrado
    lbl_speech = lv_label_create(top_bar_cont);
    lv_label_set_text(lbl_speech, "Hola amigo!");
    lv_obj_set_style_text_font(lbl_speech, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_speech, lv_color_hex(0xFEF08A), 0);
    lv_obj_align(lbl_speech, LV_ALIGN_TOP_MID, 0, 48);

    // 1.2 ÁREA CENTRAL: SALA DE JUEGO (GAME AREA)
    game_area_cont = lv_obj_create(game_main_cont);
    lv_obj_set_size(game_area_cont, 240, 182);
    lv_obj_set_pos(game_area_cont, 0, 68);
    lv_obj_set_style_bg_color(game_area_cont, lv_color_hex(0x312E81), 0);
    lv_obj_set_style_radius(game_area_cont, 0, 0);
    lv_obj_set_style_border_width(game_area_cont, 0, 0);
    lv_obj_clear_flag(game_area_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Suelo decorativo estilo 16-bit
    lv_obj_t *floor_rect = lv_obj_create(game_area_cont);
    lv_obj_set_size(floor_rect, 240, 40);
    lv_obj_align(floor_rect, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(floor_rect, lv_color_hex(0x1E1B4B), 0);
    lv_obj_set_style_radius(floor_rect, 0, 0);
    lv_obj_set_style_border_width(floor_rect, 0, 0);
    lv_obj_clear_flag(floor_rect, LV_OBJ_FLAG_SCROLLABLE);

    // Objeto central del Pet (con zoom 2x = 64x64 px)
    img_pet = lv_img_create(game_area_cont);
    lv_img_set_src(img_pet, &sprite_egg);
    lv_img_set_zoom(img_pet, 512);
    lv_img_set_antialias(img_pet, false);
    lv_obj_align(img_pet, LV_ALIGN_CENTER, 0, 10);

    // Icono flotante de acción (comida, esponja, etc.)
    img_action = lv_img_create(game_area_cont);
    lv_img_set_src(img_action, &sprite_apple);
    lv_img_set_zoom(img_action, 384);
    lv_img_set_antialias(img_action, false);
    lv_obj_align(img_action, LV_ALIGN_CENTER, 48, -25);
    lv_obj_add_flag(img_action, LV_OBJ_FLAG_HIDDEN);

    // Cacas en el suelo
    const int poop_coords[4][2] = {
        {18, 125},
        {190, 125},
        {45, 135},
        {165, 135}
    };
    for (int i = 0; i < 4; i++) {
        img_poops[i] = lv_img_create(game_area_cont);
        lv_img_set_src(img_poops[i], &sprite_poop);
        lv_img_set_zoom(img_poops[i], 320);
        lv_img_set_antialias(img_poops[i], false);
        lv_obj_set_pos(img_poops[i], poop_coords[i][0], poop_coords[i][1]);
        lv_obj_add_flag(img_poops[i], LV_OBJ_FLAG_HIDDEN);
    }

    // Botón de revivir
    btn_revive = lv_btn_create(game_area_cont);
    lv_obj_set_size(btn_revive, 140, 36);
    lv_obj_align(btn_revive, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_bg_color(btn_revive, lv_color_hex(0x10B981), 0);
    lv_obj_add_event_cb(btn_revive, btn_revive_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_rev = lv_label_create(btn_revive);
    lv_label_set_text(lbl_rev, "NUEVO HUEVO");
    lv_obj_set_style_text_font(lbl_rev, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_rev);
    lv_obj_add_flag(btn_revive, LV_OBJ_FLAG_HIDDEN);

    // 1.3 ÁREA INFERIOR: BOTONES DE ACCIÓN (BOTTOM BAR)
    bottom_bar_cont = lv_obj_create(game_main_cont);
    lv_obj_set_size(bottom_bar_cont, 240, 70);
    lv_obj_set_pos(bottom_bar_cont, 0, 250);
    lv_obj_set_style_bg_color(bottom_bar_cont, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_radius(bottom_bar_cont, 0, 0);
    lv_obj_set_style_border_width(bottom_bar_cont, 0, 0);
    lv_obj_set_style_pad_all(bottom_bar_cont, 2, 0);
    lv_obj_clear_flag(bottom_bar_cont, LV_OBJ_FLAG_SCROLLABLE);

    auto make_btn = [](lv_obj_t *parent, const char *txt, uint32_t bg_hex, int x, lv_event_cb_t cb) -> lv_obj_t* {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, 44, 58);
        lv_obj_set_pos(btn, x, 6);
        lv_obj_set_style_bg_color(btn, lv_color_hex(bg_hex), 0);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, txt);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(lbl);
        return btn;
    };

    btn_feed  = make_btn(bottom_bar_cont, "COMER\n-HAM",  0xEA580C, 2,   btn_feed_cb);
    btn_clean = make_btn(bottom_bar_cont, "LIMP\n+HIG",   0x0284C7, 49,  btn_clean_cb);
    btn_play  = make_btn(bottom_bar_cont, "JUGAR\n+FEL",  0x7C3AED, 96,  btn_play_cb);
    btn_sleep = make_btn(bottom_bar_cont, "DORM\n+ENE",   0xD97706, 143, btn_sleep_cb);
    btn_cure  = make_btn(bottom_bar_cont, "MED\n+SAL",    0x059669, 190, btn_cure_cb);

    // ==========================================
    // 2. PANTALLA DE SELECCIÓN DE MASCOTA
    // ==========================================
    select_screen_cont = lv_obj_create(main_screen);
    lv_obj_set_size(select_screen_cont, 240, 320);
    lv_obj_set_pos(select_screen_cont, 0, 0);
    lv_obj_set_style_bg_color(select_screen_cont, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_pad_all(select_screen_cont, 8, 0);
    lv_obj_set_style_border_width(select_screen_cont, 0, 0);
    lv_obj_clear_flag(select_screen_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_sel_title = lv_label_create(select_screen_cont);
    lv_label_set_text(lbl_sel_title, "ELIGE TU MASCOTA");
    lv_obj_set_style_text_font(lbl_sel_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_sel_title, lv_color_hex(0x38BDF8), 0);
    lv_obj_align(lbl_sel_title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *lbl_sel_sub = lv_label_create(select_screen_cont);
    lv_label_set_text(lbl_sel_sub, "Toca una para comenzar:");
    lv_obj_set_style_text_font(lbl_sel_sub, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_sel_sub, lv_color_hex(0x94A3B8), 0);
    lv_obj_align(lbl_sel_sub, LV_ALIGN_TOP_MID, 0, 30);

    // Función auxiliar para tarjetas de mascota
    auto make_pet_card = [](lv_obj_t *parent, int y, const char *name, const char *desc,
                            const lv_img_dsc_t *icon, uint32_t bg_hex, uintptr_t type) {
        lv_obj_t *card = lv_btn_create(parent);
        lv_obj_set_size(card, 224, 76);
        lv_obj_set_pos(card, 0, y);
        lv_obj_set_style_bg_color(card, lv_color_hex(bg_hex), 0);
        lv_obj_set_style_radius(card, 10, 0);
        lv_obj_set_style_pad_all(card, 6, 0);
        lv_obj_add_event_cb(card, card_select_pet_cb, LV_EVENT_CLICKED, (void*)type);

        lv_obj_t *img = lv_img_create(card);
        lv_img_set_src(img, icon);
        lv_img_set_zoom(img, 384);
        lv_img_set_antialias(img, false);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 6, 0);

        lv_obj_t *lbl_n = lv_label_create(card);
        lv_label_set_text(lbl_n, name);
        lv_obj_set_style_text_font(lbl_n, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(lbl_n, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(lbl_n, 65, 12);

        lv_obj_t *lbl_d = lv_label_create(card);
        lv_label_set_text(lbl_d, desc);
        lv_obj_set_style_text_font(lbl_d, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(lbl_d, lv_color_hex(0xFEF08A), 0);
        lv_obj_set_pos(lbl_d, 65, 34);

        return card;
    };

    make_pet_card(select_screen_cont, 56,  "1. DINO (Dragon)",  "Fuerte y comilon", &sprite_dino_baby_idle1, 0x065F46, PET_DINO);
    make_pet_card(select_screen_cont, 140, "2. POLLI (Pollito)", "Alegre y cantarin", &sprite_chik_baby_idle1, 0xB45309, PET_CHIK);
    make_pet_card(select_screen_cont, 224, "3. GATITO (Mimi)",   "Carinoso y jugueton", &sprite_cat_baby_idle1, 0x4338CA, PET_CAT);

    // ==========================================
    // 3. PANTALLA DE AJUSTES (SETTINGS_MODAL_CONT)
    // ==========================================
    settings_modal_cont = lv_obj_create(main_screen);
    lv_obj_set_size(settings_modal_cont, 240, 320);
    lv_obj_set_pos(settings_modal_cont, 0, 0);
    lv_obj_set_style_bg_color(settings_modal_cont, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_pad_all(settings_modal_cont, 6, 0);
    lv_obj_set_style_border_width(settings_modal_cont, 0, 0);
    lv_obj_add_flag(settings_modal_cont, LV_OBJ_FLAG_HIDDEN); // Oculto por defecto

    // Encabezado
    lv_obj_t *lbl_set_title = lv_label_create(settings_modal_cont);
    lv_label_set_text(lbl_set_title, "CONFIGURACION");
    lv_obj_set_style_text_font(lbl_set_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_set_title, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_pos(lbl_set_title, 8, 6);

    lv_obj_t *btn_close = lv_btn_create(settings_modal_cont);
    lv_obj_set_size(btn_close, 60, 22);
    lv_obj_set_pos(btn_close, 160, 2);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x475569), 0);
    lv_obj_add_event_cb(btn_close, btn_close_settings_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_close = lv_label_create(btn_close);
    lv_label_set_text(lbl_close, "VOLVER");
    lv_obj_set_style_text_font(lbl_close, &lv_font_montserrat_10, 0);
    lv_obj_center(lbl_close);

    // Control de Brillo
    lbl_bright_val = lv_label_create(settings_modal_cont);
    lv_label_set_text(lbl_bright_val, "BRILLO: 85%");
    lv_obj_set_style_text_font(lbl_bright_val, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_bright_val, lv_color_hex(0xE2E8F0), 0);
    lv_obj_set_pos(lbl_bright_val, 8, 30);

    slider_brightness = lv_slider_create(settings_modal_cont);
    lv_obj_set_size(slider_brightness, 216, 10);
    lv_obj_set_pos(slider_brightness, 8, 44);
    lv_slider_set_range(slider_brightness, 20, 255);
    lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0x334155), 0);
    lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0x38BDF8), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider_brightness, slider_brightness_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Control WiFi
    lv_obj_t *lbl_wifi_t = lv_label_create(settings_modal_cont);
    lv_label_set_text(lbl_wifi_t, "WIFI:");
    lv_obj_set_style_text_font(lbl_wifi_t, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_wifi_t, lv_color_hex(0xE2E8F0), 0);
    lv_obj_set_pos(lbl_wifi_t, 8, 64);

    sw_wifi = lv_switch_create(settings_modal_cont);
    lv_obj_set_size(sw_wifi, 42, 20);
    lv_obj_set_pos(sw_wifi, 46, 60);
    lv_obj_add_event_cb(sw_wifi, sw_wifi_cb, LV_EVENT_VALUE_CHANGED, NULL);

    btn_scan = lv_btn_create(settings_modal_cont);
    lv_obj_set_size(btn_scan, 120, 22);
    lv_obj_set_pos(btn_scan, 104, 60);
    lv_obj_set_style_bg_color(btn_scan, lv_color_hex(0x0284C7), 0);
    lv_obj_add_event_cb(btn_scan, btn_scan_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_scan_txt = lv_label_create(btn_scan);
    lv_label_set_text(lbl_scan_txt, "BUSCAR REDES");
    lv_obj_set_style_text_font(lbl_scan_txt, &lv_font_montserrat_10, 0);
    lv_obj_center(lbl_scan_txt);

    // Dropdown de Redes
    dd_networks = lv_dropdown_create(settings_modal_cont);
    lv_obj_set_size(dd_networks, 216, 28);
    lv_obj_set_pos(dd_networks, 8, 88);
    lv_dropdown_set_options(dd_networks, "Toca 'Buscar Redes'");
    lv_obj_set_style_text_font(dd_networks, &lv_font_montserrat_10, 0);
    lv_obj_add_event_cb(dd_networks, dd_networks_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Entradas de texto (SSID y Pass)
    ta_ssid = lv_textarea_create(settings_modal_cont);
    lv_obj_set_size(ta_ssid, 216, 28);
    lv_obj_set_pos(ta_ssid, 8, 122);
    lv_textarea_set_placeholder_text(ta_ssid, "Nombre de red (SSID)");
    lv_textarea_set_one_line(ta_ssid, true);
    lv_obj_set_style_text_font(ta_ssid, &lv_font_montserrat_10, 0);
    lv_obj_add_event_cb(ta_ssid, ta_click_cb, LV_EVENT_CLICKED, NULL);

    ta_pass = lv_textarea_create(settings_modal_cont);
    lv_obj_set_size(ta_pass, 216, 28);
    lv_obj_set_pos(ta_pass, 8, 154);
    lv_textarea_set_placeholder_text(ta_pass, "Contrasena WiFi");
    lv_textarea_set_password_mode(ta_pass, true);
    lv_textarea_set_one_line(ta_pass, true);
    lv_obj_set_style_text_font(ta_pass, &lv_font_montserrat_10, 0);
    lv_obj_add_event_cb(ta_pass, ta_click_cb, LV_EVENT_CLICKED, NULL);

    // Botón Conectar
    btn_connect = lv_btn_create(settings_modal_cont);
    lv_obj_set_size(btn_connect, 104, 24);
    lv_obj_set_pos(btn_connect, 8, 186);
    lv_obj_set_style_bg_color(btn_connect, lv_color_hex(0x10B981), 0);
    lv_obj_add_event_cb(btn_connect, btn_connect_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_conn_txt = lv_label_create(btn_connect);
    lv_label_set_text(lbl_conn_txt, "CONECTAR");
    lv_obj_set_style_text_font(lbl_conn_txt, &lv_font_montserrat_10, 0);
    lv_obj_center(lbl_conn_txt);

    // Botón RESET (Borra partida y vuelve a selección de mascota)
    lv_obj_t *btn_reset = lv_btn_create(settings_modal_cont);
    lv_obj_set_size(btn_reset, 104, 24);
    lv_obj_set_pos(btn_reset, 120, 186);
    lv_obj_set_style_bg_color(btn_reset, lv_color_hex(0xDC2626), 0);
    lv_obj_add_event_cb(btn_reset, btn_reset_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_res_txt = lv_label_create(btn_reset);
    lv_label_set_text(lbl_res_txt, "RESET");
    lv_obj_set_style_text_font(lbl_res_txt, &lv_font_montserrat_10, 0);
    lv_obj_center(lbl_res_txt);

    // Estado WiFi / NTP
    lbl_wifi_status = lv_label_create(settings_modal_cont);
    lv_label_set_text(lbl_wifi_status, "Desconectado");
    lv_obj_set_style_text_font(lbl_wifi_status, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_wifi_status, lv_color_hex(0xFEF08A), 0);
    lv_obj_set_pos(lbl_wifi_status, 8, 216);

    // Teclado virtual flotante en pantalla
    kb_settings = lv_keyboard_create(settings_modal_cont);
    lv_obj_set_size(kb_settings, 240, 130);
    lv_obj_align(kb_settings, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(kb_settings, LV_OBJ_FLAG_HIDDEN); // Oculto hasta tocar un campo
    lv_obj_add_event_cb(kb_settings, kb_event_cb, LV_EVENT_ALL, NULL);

    // Si aún no se ha elegido mascota, mostrar selección; si ya hay, mostrar juego
    if (!pet.hasChosenPet()) {
        lv_obj_add_flag(game_main_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(select_screen_cont, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(game_main_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(select_screen_cont, LV_OBJ_FLAG_HIDDEN);
    }
}

// ==========================================
// BUCLE DE ACTUALIZACIÓN (UI_UPDATE)
// ==========================================
void ui_update() {
    uint32_t now = millis();

    // Actualizar reloj de la barra superior cada 1 segundo
    if (now - last_clock_tick >= 1000) {
        last_clock_tick = now;
        String time_str = wifi_manager_get_time_str(false);
        lv_label_set_text(lbl_clock, time_str.c_str());
    }

    // Si estamos en la pantalla de selección, no hace falta actualizar el juego
    if (!pet.hasChosenPet()) return;

    // Actualizar barras de estado
    lv_bar_set_value(bar_hunger, pet.getHunger(), LV_ANIM_OFF);
    lv_bar_set_value(bar_happy, pet.getHappiness(), LV_ANIM_OFF);
    lv_bar_set_value(bar_energy, pet.getEnergy(), LV_ANIM_OFF);
    lv_bar_set_value(bar_hygiene, pet.getHygiene(), LV_ANIM_OFF);

    // Alternar frame de animación cada 400ms
    if (now - last_anim_tick >= 400) {
        last_anim_tick = now;
        anim_frame_alt = !anim_frame_alt;
    }

    // Modo noche / dormir
    if (pet.isSleeping() != night_mode_active) {
        night_mode_active = pet.isSleeping();
        if (night_mode_active) {
            lv_obj_set_style_bg_color(game_area_cont, lv_color_hex(0x09090B), 0);
        } else {
            lv_obj_set_style_bg_color(game_area_cont, lv_color_hex(0x312E81), 0);
        }
    }

    // Actualizar nombre y fase en el título
    uint8_t ptype = pet.getPetType();
    PetState state = pet.getState();

    const char* type_name = (ptype == PET_DINO) ? "DINO" : ((ptype == PET_CHIK) ? "POLLI" : "GATITO");
    char title_buf[32];
    if (state == STATE_EGG) snprintf(title_buf, sizeof(title_buf), "HUEVO %s", type_name);
    else if (state == STATE_BABY) snprintf(title_buf, sizeof(title_buf), "%s BEBE", type_name);
    else if (state == STATE_TEEN) snprintf(title_buf, sizeof(title_buf), "%s JOVEN", type_name);
    else if (state == STATE_ADULT) snprintf(title_buf, sizeof(title_buf), "%s ADULTO", type_name);
    else snprintf(title_buf, sizeof(title_buf), "%s", type_name);
    lv_label_set_text(lbl_title, title_buf);

    // Obtener sprite correspondiente según tipo, estado y frame
    const lv_img_dsc_t *current_sprite = get_pet_sprite(ptype, state, anim_frame_alt);

    // Iconos de acción flotantes
    if (state == STATE_EATING) {
        lv_img_set_src(img_action, &sprite_apple);
        lv_obj_clear_flag(img_action, LV_OBJ_FLAG_HIDDEN);
    } else if (state == STATE_PLAYING) {
        lv_img_set_src(img_action, &sprite_ball);
        lv_obj_clear_flag(img_action, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(img_action, LV_OBJ_FLAG_HIDDEN);
    }

    // Si ha muerto, mostrar botón de revivir / nueva mascota
    if (state == STATE_DEAD) {
        lv_obj_clear_flag(btn_revive, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(lbl_speech, "R.I.P. Ha fallecido...");
    } else {
        lv_obj_add_flag(btn_revive, LV_OBJ_FLAG_HIDDEN);
    }

    lv_img_set_src(img_pet, current_sprite);

    // Actualizar cacas visibles
    uint8_t poops = pet.getPoopCount();
    for (int i = 0; i < 4; i++) {
        if (i < poops) {
            lv_obj_clear_flag(img_poops[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(img_poops[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}
