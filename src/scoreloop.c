#include "tuxracer.h"
#include "audio.h"
#include "game_config.h"
#include "multiplayer.h"
#include "gl_util.h"
#include "fps.h"
#include "render_util.h"
#include "phys_sim.h"
#include "view.h"
#include "course_render.h"
#include "tux.h"
#include "tux_shadow.h"
#include "keyboard.h"
#include "loop.h"
#include "hud.h"
#include "game_logic_util.h"
#include "fonts.h"
#include "ui_mgr.h"
#include "ui_theme.h"
#include "ui_snow.h"
#include "gui_slider.h"
#include "gui_button.h"
#include "gui_abstraction.h"
#include "joystick.h"
#include "scoreloop.h"

#ifdef __ANDROID__
#include "jni.h"
#define JNI(f)	Java_com_moonlite_tuxracer_ ## f
#endif

widget_t* enter_nickname_btn=NULL;
widget_t* enter_email_btn=NULL;

char alias_buf[100]="Set Alias";
bool_t update_info=False;

typedef enum TEXT_ENTRY_MODE
{
    TEXT_ENTRY_NONE,
    TEXT_ENTRY_ALIAS,
    TEXT_ENTRY_EMAIL,
} TEXT_ENTRY_MODE;

TEXT_ENTRY_MODE text_entry=TEXT_ENTRY_NONE;

void start_dialog(char* title, char* message, bool_t isUsername)
{
#ifdef __ANDROID__
    JNIEnv* env = Android_JNI_GetEnv();
    jstring j_title=(*env)->NewStringUTF(env, title);
    jstring j_message=(*env)->NewStringUTF(env, message);
    if (!env)
    {
        return;
    }
    jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/GameActivity");
    jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "displayTextInputDialog", "(Ljava/lang/String;Ljava/lang/String;Z)V");
    if (!mid)
    {
        return ;
    }
    (*env)->CallStaticVoidMethod(env, mActivityClass, mid, j_title, j_message, isUsername);
    (*env)->DeleteLocalRef(env, j_title);
    (*env)->DeleteLocalRef(env, j_message);
#endif
}

void nickname_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    start_dialog(ALIAS_PROMPT_TITLE, ALIAS_PROMPT_MESSAGE, True);
    text_entry=TEXT_ENTRY_ALIAS;
}

void email_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    start_dialog("Enter Email" JNI_NULL_CHAR, "Enter an email address. This is entirely optional, but recommended if you like your alias and might want to use it again on another device." JNI_NULL_CHAR, False);
    text_entry=TEXT_ENTRY_EMAIL;
}

void scoreloop_add_widgets()
{
    if (!update_info)
    {
        update_info=True;
        gui_add_widget(enter_nickname_btn=create_button(alias_buf, nickname_click_cb), NULL);
        update_info=False;
    }
    else
    {
        gui_add_widget(enter_nickname_btn=create_button("Set Alias", nickname_click_cb), NULL);
    }
    gui_add_widget(enter_email_btn=create_button("Set Email", email_click_cb), NULL);
}

void scoreloop_update_widgets()
{
    if (update_info)
    {
        button_set_text(enter_nickname_btn, alias_buf);
        update_info=False;
    }
}

#ifdef __ANDROID__
JNIEXPORT void JNICALL JNI(ScoreActivity_nativeTextCallback)(JNIEnv *env, jclass cls, jstring str)
{
    char buf[255]; //fun fact: an email can't be more than 254 characters long. Allow for a null character.
    char* string_tmp;
    string_tmp=(*env)->GetStringUTFChars(env, str, 0);
    strcpy(buf, string_tmp);
    (*env)->ReleaseStringUTFChars(env, str, string_tmp);
    
    if (text_entry==TEXT_ENTRY_ALIAS)
    {
        JNIEnv* env = Android_JNI_GetEnv();
        if (!env)
        {
            return;
        }
        jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/GameActivity");
        jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "setUserAlias", "(Ljava/lang/String;)V");
        if (!mid)
        {
            return ;
        }
        (*env)->CallStaticVoidMethod(env, mActivityClass, mid, str);
    }
    else if (text_entry==TEXT_ENTRY_EMAIL)
    {
        JNIEnv* env = Android_JNI_GetEnv();
        if (!env)
        {
            return;
        }
        jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/GameActivity");
        jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "setUserEmail", "(Ljava/lang/String;)V");
        if (!mid)
        {
            return ;
        }
        (*env)->CallStaticVoidMethod(env, mActivityClass, mid, str);
    }
    text_entry=TEXT_ENTRY_NONE;
}

JNIEXPORT void JNICALL JNI(ScoreActivity_nativeUpdateUserInfo)(JNIEnv *env, jclass cls, jstring alias)
{
    char* string_tmp;
    
    if (update_info)
        return;
    
    if (alias)
    {
        string_tmp=(*env)->GetStringUTFChars(env, alias, 0);
        sprintf(alias_buf, "Change Alias (%s)", string_tmp);
        (*env)->ReleaseStringUTFChars(env, alias, string_tmp);
    }
    else
    {
        strcpy(alias_buf, "Set Alias");
    }
    update_info=True;
}
#endif
