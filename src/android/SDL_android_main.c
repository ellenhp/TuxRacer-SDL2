
#include "SDL_config.h"

#ifdef __ANDROID__

/* Include the SDL main definition header */
#include "SDL_main.h"
#include <assert.h>

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/
#include <jni.h>

#ifndef SDL_PREFIX
	#define SDL_PREFIX org_libsdl_app_SDLActivity
#endif

#define CONCAT1(p,f)	CONCAT2(p,f)
#define CONCAT2(p,f)	Java_ ## p ## _ ## f

#define JNI(f)			CONCAT1(SDL_PREFIX,f)

// Called before SDL_main() to initialize JNI bindings in SDL library
extern void SDL_Android_Init(JNIEnv* env, jclass cls);

// Initialize ScoreLoop C SDK
void JNI(nativeInit)(JNIEnv* env, jclass cls, jobject obj)
{
    /* This interface could expand with ABI negotiation, calbacks, etc. */
    SDL_Android_Init(env, cls);

    SDL_SetMainReady();

    /* Run the application code! */
    int status;
    char *argv[2];
    argv[0] = SDL_strdup("SDL_app");
    argv[1] = NULL;
    status = SDL_main(1, argv);

    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    //exit(status);
}

#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
