#include "platform.h"
#include "course_load.h"
#include "race_select.h" // for course_price
#include "tuxracer.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

static int overscan_percent = 0;
static char *player_name = 0;
static bool_t is_on_ouya_val = False;

#ifdef __ANDROID__

#define JNI(f) Java_com_moonlite_tuxracer_##f

JNIEXPORT jdouble JNICALL JNI(GameActivity_nativeSetPlayerData)(
    JNIEnv *env, jobject jobj, jstring path, jboolean on_ouya) {
  const char *name = (*env)->GetStringUTFChars(env, path, 0);
  if (player_name) {
    free(player_name);
  }
  player_name = (char *)malloc(strlen(name));
  strcpy(player_name, name);
}
#endif

bool_t is_on_ouya(void) { return 0; }

int get_overscan_percent(void) { return 0; }

char race_button_text[20];

char *get_race_text(void) { return "Race"; }

char *get_back_text(void) { return "Back"; }

char *get_continue_text(void) { return "Continue"; }

char *get_abort_text(void) { return "Exit Race"; }

char *get_select_text(void) { return "Select"; }

char *get_quit_text(void) { return "Quit"; }
