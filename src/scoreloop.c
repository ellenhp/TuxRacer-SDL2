

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
#include "fog.h"
#include "viewfrustum.h"
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

#ifdef __ANDROID__
#include "jni.h"
#define JNI_NULL_CHAR "\xC0\x80"
#define JNI(f)	Java_com_moonlite_tuxracer_ ## f
#endif

widget_t* enter_nickname_btn=NULL;
widget_t* enter_email_btn=NULL;

char alias_buf[100]="";
bool_t update_info=False;

typedef enum TEXT_ENTRY_MODE
{
    TEXT_ENTRY_NONE,
    TEXT_ENTRY_ALIAS,
    TEXT_ENTRY_EMAIL,
} TEXT_ENTRY_MODE;

TEXT_ENTRY_MODE text_entry=TEXT_ENTRY_NONE;

void start_dialog(char* title, char* message)
{
#ifdef __ANDROID__
    JNIEnv* env = Android_JNI_GetEnv();
    jstring j_title=(*env)->NewStringUTF(env, title);
    jstring j_message=(*env)->NewStringUTF(env, message);
    if (!env)
    {
        return;
    }
    jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/SDLActivity");
    jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "displayTextInputDialog", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (!mid)
    {
        return ;
    }
    (*env)->CallStaticVoidMethod(env, mActivityClass, mid, j_title, j_message);
    (*env)->DeleteLocalRef(env, j_title);
    (*env)->DeleteLocalRef(env, j_message);
#endif
}

void nickname_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    start_dialog("Enter alias" JNI_NULL_CHAR, "Enter the alias you want to appear next to your scores. Note that you will not be able to reuse this on other devices unless you also enter an email." JNI_NULL_CHAR);
    text_entry=TEXT_ENTRY_ALIAS;
}

void email_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    start_dialog("Enter email" JNI_NULL_CHAR, "Enter an email address. This is entirely optional, but recommended if you like your alias and might want to use it again on another device." JNI_NULL_CHAR);
    text_entry=TEXT_ENTRY_EMAIL;
}

#ifdef __ANDROID__
JNIEXPORT void JNICALL JNI(SDLActivity_nativeTextCallback)(JNIEnv *env, jclass cls, jstring str)
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
        jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/SDLActivity");
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
        jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/SDLActivity");
        jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "setUserEmail", "(Ljava/lang/String;)V");
        if (!mid)
        {
            return ;
        }
        (*env)->CallStaticVoidMethod(env, mActivityClass, mid, str);
    }
    text_entry=TEXT_ENTRY_NONE;
}

JNIEXPORT void JNICALL JNI(SDLActivity_nativeUpdateUserInfo)(JNIEnv *env, jclass cls, jstring alias)
{
    char* string_tmp;
    
    if (update_info)
        return;
    
    if (alias)
    {
        string_tmp=(*env)->GetStringUTFChars(env, alias, 0);
        sprintf(alias_buf, "Change Alias (using: %s)", string_tmp);
        (*env)->ReleaseStringUTFChars(env, alias, string_tmp);
    }
    else
    {
        strcpy(alias_buf, "Set Alias");
    }
    update_info=True;
}
#endif

static void scoreloop_init(void)
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( GameMenu_mouse_func );
    winsys_set_motion_func( GameMenu_motion_func );
    winsys_set_passive_motion_func( GameMenu_motion_func );
	winsys_set_joystick_func( GameMenu_joystick_func );
	winsys_set_joystick_button_func( GameMenu_joystick_button_func );

	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_B, SDLK_ESCAPE);

	GameMenu_init();
	setup_gui();
    
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

	gui_balance_lines(0);
    
    play_music( "start_screen" );
}

static void scoreloop_loop( scalar_t time_step )
{
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();
    
    check_gl_error();
    
    update_audio();
    
    clear_rendering_context();
    
    set_gl_options( GUI );
    
    ui_setup_display();
    
    if (getparam_ui_snow()) {
        update_ui_snow( time_step, False );
        draw_ui_snow();
    }

	ui_draw_menu_decorations(True);
    
    if (update_info)
    {
        button_set_text(enter_nickname_btn, alias_buf);
        update_info=False;
    }
    
	GameMenu_draw();

    ui_draw_cursor();
    
    reshape( width, height );
    
    winsys_swap_buffers();
} 

START_KEYBOARD_CB( scoreloop_key_cb )
{
	if (release) return;

	switch (key)
	{
	case SDLK_ESCAPE:
	case SDLK_q:
	case SDLK_AC_BACK:
        set_game_mode( GAME_TYPE_SELECT );
		break;
	default:
	    GameMenu_keypress(key);
	}

}
END_KEYBOARD_CB

void scoreloop_register()
{
    int status = 0;
    
    status |= add_keymap_entry( SCORELOOP, 
                               DEFAULT_CALLBACK, 
                               NULL, NULL, scoreloop_key_cb );
    
    check_assertion( status == 0, "out of keymap entries" );
    
    register_loop_funcs( SCORELOOP, scoreloop_init, scoreloop_loop, NULL );
}


