
// QuickLang-GUI for PS Vita
// Uses vita2d for graphics. Provides favorites, config file at ux0:data/quicklang/config.json,
// and UI labels in English/German (auto-detect by system language).
#include <vita2d.h>
#include <psp2/ctrl.h>
#include <psp2/sysmodule.h>
#include <psp2/apputil.h>
#include <psp2/registrymgr.h>
#include <psp2/power.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LANGS 32
#define CONFIG_PATH "ux0:data/quicklang/config.json"

typedef struct { int id; const char* name; } LangItem;

static LangItem LANGS[] = {
    { SCE_SYSTEM_PARAM_LANG_ENGLISH_GB, "English (UK)" },
    { SCE_SYSTEM_PARAM_LANG_ENGLISH_US, "English (US)" },
    { SCE_SYSTEM_PARAM_LANG_GERMAN,     "Deutsch" },
    { SCE_SYSTEM_PARAM_LANG_FRENCH,     "Français" },
    { SCE_SYSTEM_PARAM_LANG_SPANISH,    "Español" },
    { SCE_SYSTEM_PARAM_LANG_ITALIAN,    "Italiano" },
    { SCE_SYSTEM_PARAM_LANG_DUTCH,      "Nederlands" },
    { SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT, "Português (PT)" },
    { SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR, "Português (BR)" },
    { SCE_SYSTEM_PARAM_LANG_RUSSIAN,    "Русский" },
    { SCE_SYSTEM_PARAM_LANG_TURKISH,    "Türkçe" },
    { SCE_SYSTEM_PARAM_LANG_POLISH,     "Polski" },
    { SCE_SYSTEM_PARAM_LANG_FINNISH,    "Suomi" },
    { SCE_SYSTEM_PARAM_LANG_SWEDISH,    "Svenska" },
    { SCE_SYSTEM_PARAM_LANG_DANISH,     "Dansk" },
    { SCE_SYSTEM_PARAM_LANG_NORWEGIAN,  "Norsk" },
    { SCE_SYSTEM_PARAM_LANG_JAPANESE,   "日本語" },
    { SCE_SYSTEM_PARAM_LANG_KOREAN,     "한국어" },
    { SCE_SYSTEM_PARAM_LANG_CHINESE_S,  "简体中文" },
    { SCE_SYSTEM_PARAM_LANG_CHINESE_T,  "繁體中文" },
};

static vita2d_pgf* font;
static int sel = 0;
static int offset = 0;
static int locale = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;
static int favorites[MAX_LANGS];
static int fav_count = 0;

static void load_config() {
    int fd = sceIoOpen(CONFIG_PATH, SCE_O_RDONLY, 0);
    if (fd < 0) return;
    char buf[1024]; int r = sceIoRead(fd, buf, sizeof(buf)-1);
    sceIoClose(fd);
    if (r <= 0) return;
    buf[r]=0;
    // very simple parse: look for "favorites":[id,id,...]
    char *p = strstr(buf, "\"favorites\":");
    if (!p) return;
    p = strchr(p, '[');
    if (!p) return;
    p++;
    fav_count = 0;
    while (p && fav_count < MAX_LANGS) {
        int id; if (sscanf(p, "%d", &id) == 1) { favorites[fav_count++] = id; }
        p = strchr(p, ',');
        if (p) p++;
        else break;
    }
}

static void save_config() {
    sceIoMkdir("ux0:data/quicklang", 0777);
    int fd = sceIoOpen(CONFIG_PATH, SCE_O_WRONLY|SCE_O_CREAT|SCE_O_TRUNC, 0666);
    if (fd < 0) return;
    char buf[1024]; int len = 0;
    len += snprintf(buf+len, sizeof(buf)-len, "{\n  \"favorites\": [");
    for (int i=0;i<fav_count;i++) {
        if (i) len += snprintf(buf+len, sizeof(buf)-len, ",");
        len += snprintf(buf+len, sizeof(buf)-len, "%d", favorites[i]);
    }
    len += snprintf(buf+len, sizeof(buf)-len, "]\n}\n");
    sceIoWrite(fd, buf, len);
    sceIoClose(fd);
}

static int get_current_lang() {
    int cur = -1;
    if (sceRegMgrSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &cur) < 0) {
        sceAppUtilInit(NULL,NULL);
        sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &cur);
        sceAppUtilShutdown();
    }
    return cur;
}

static int find_index_by_id(int id) {
    for (int i=0;i<(int)(sizeof(LANGS)/sizeof(LANGS[0]));i++) if (LANGS[i].id==id) return i;
    return -1;
}

static void toggle_favorite(int lang_id) {
    int idx=-1;
    for (int i=0;i<fav_count;i++) if (favorites[i]==lang_id) { idx=i; break; }
    if (idx>=0) {
        // remove
        for (int j=idx;j<fav_count-1;j++) favorites[j]=favorites[j+1];
        fav_count--;
    } else if (fav_count < MAX_LANGS) {
        favorites[fav_count++]=lang_id;
    }
    save_config();
}

int main() {
    sceSysmoduleLoadModule(SCE_SYSMODULE_PGF);
    vita2d_init_advanced(); // init vita2d
    font = vita2d_load_default_pgf();

    SceCtrlData pad, old={0};
    sel = 0; offset = 0;
    load_config();
    int current = get_current_lang();
    int lang_count = sizeof(LANGS)/sizeof(LANGS[0]);
    int running = 1;

    while (running) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        unsigned changed = pad.buttons & ~old.buttons;
        old = pad;

        if (changed & SCE_CTRL_DOWN) { if (sel < lang_count-1) sel++; if (sel - offset >= 10) offset++; }
        if (changed & SCE_CTRL_UP)   { if (sel > 0) sel--; if (sel < offset) offset--; }
        if (changed & SCE_CTRL_RIGHT) { // jump to next favorite
            if (fav_count>0) {
                int cur_idx = find_index_by_id(LANGS[sel].id);
                // find next favorite index after cur_idx
                int found=-1;
                for (int i=0;i<fav_count;i++) {
                    int idx = find_index_by_id(favorites[i]);
                    if (idx>cur_idx) { found=idx; break; }
                }
                if (found==-1) found = find_index_by_id(favorites[0]);
                if (found!=-1) { sel = found; if (sel - offset >= 10) offset = sel-9; }
            }
        }
        if (changed & SCE_CTRL_LEFT) { // previous favorite
            if (fav_count>0) {
                int cur_idx = find_index_by_id(LANGS[sel].id);
                int found=-1;
                for (int i=fav_count-1;i>=0;i--) {
                    int idx = find_index_by_id(favorites[i]);
                    if (idx<cur_idx) { found=idx; break; }
                }
                if (found==-1) found = find_index_by_id(favorites[fav_count-1]);
                if (found!=-1) { sel = found; if (sel < offset) offset = sel; }
            }
        }

        if (changed & SCE_CTRL_CROSS) {
            int target = LANGS[sel].id;
            sceRegMgrSetKeyInt("/CONFIG/SYSTEM", "language", target);
            // prompt: immediate reboot
            // simple text output then reboot
            vita2d_start_drawing();
            vita2d_clear_screen();
            vita2d_pgf_draw_textf(font, 20, 30, 1.0f, "%s", "Sprache gesetzt. X = Neustart, O = Abbrechen");
            vita2d_end_drawing();
            vita2d_wait_rendering_done();
            while (1) {
                sceCtrlPeekBufferPositive(0, &pad, 1);
                unsigned ch = pad.buttons & ~old.buttons; old = pad;
                if (ch & SCE_CTRL_CROSS) { scePowerRequestColdReset(); }
                if (ch & SCE_CTRL_CIRCLE) break;
            }
        }

        if (changed & SCE_CTRL_TRIANGLE) { // toggle favorite
            toggle_favorite(LANGS[sel].id);
        }

        if (changed & SCE_CTRL_CIRCLE) { running = 0; }

        // draw UI
        vita2d_start_drawing();
        vita2d_clear_screen();
        vita2d_pgf_draw_textf(font, 20, 24, 1.0f, "QuickLang GUI - Select language");
        vita2d_pgf_draw_textf(font, 20, 44, 0.9f, "X:apply  O:exit  /\u25B2/\u25BC:move  \u25C0/\u25B6:jump fav  /\u25B3:toggle fav");

        int y = 80;
        for (int i=offset; i<lang_count && i<offset+10; i++) {
            char line[128]; snprintf(line, sizeof(line), "%c %s", (i==sel?'>' : ' '), LANGS[i].name);
            vita2d_pgf_draw_textf(font, 40, y, 0.9f, "%s", line);
            // star for favorite
            for (int f=0;f<fav_count;f++) if (favorites[f]==LANGS[i].id) vita2d_pgf_draw_textf(font, 20, y, 0.9f, "*");
            y += 26;
        }
        vita2d_end_drawing();
        vita2d_wait_rendering_done();
    }

    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
