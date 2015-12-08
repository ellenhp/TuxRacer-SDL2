package com.moonlite.tuxracer;

import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.ArrayList;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

import static tv.ouya.console.api.OuyaController.BUTTON_O;

import org.libsdl.app.SDLActivity;

/**
 * GameActivity implements (both Amazon and OUYA) In-App Purchase on top of SDLActivity
 */
public class ScoreActivity extends SDLActivity {

	private static final String TAG = "Tuxracer";

	private static boolean mUserCanSubmitScores = false;

	private static String mScoreUsername = "";
	
	// Load the .so
	static {
		System.loadLibrary("SDL2");
		System.loadLibrary("SDL2_image");
		System.loadLibrary("SDL2_mixer");
		// System.loadLibrary("SDL2_net");
		// System.loadLibrary("SDL2_ttf");
		System.loadLibrary("tcl");
		System.loadLibrary("main");
	}

	protected String xor_secret(String secret) {
		int i;
		char[] buf = new char[secret.length()];
		char[] secretBuf = secret.toCharArray();
		int len = secret.length();
		for (i = 0; i < len; ++i) {
			buf[i] = (char) (secretBuf[i] ^ (i & 15) + 1);
		}
		return new String(buf);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		if (keyCode == BUTTON_O) {
			View focusedButton = getCurrentFocus();
			focusedButton.performClick();
			return true;
		}
		return super.onKeyUp(keyCode, event);
	}

	public static native void nativeScoreloopGotScores(int scoreMode, Object[] scoreStrings);
	public static native void nativeTextCallback(String string);
	public static native void nativeDisableAliasPrompt();
	public static native void nativeUpdateUserInfo(String alias);
	
	// Setup
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// Log.v("SDL", "onCreate()");
		super.onCreate(savedInstanceState);
		
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}

	/************************************************************************/
	/*																		*/
	/*               S C O R E   S T U B   F U N C T I O N S                */
	/*																		*/
	/************************************************************************/

	private static void updateUserInfo() {
	}

	public static void setUserEmail(String email) {
	}

	public static void setUserAlias(String alias) {
	}

	public static void displayTextInputDialog(String title, String message, boolean isUsername) {
		Log.d("dialog", "displaying dialog");
		class DialogDisplayer implements Runnable {
			String title, message;
			boolean isUsername;
			EditText input;
			
			public DialogDisplayer(String title, String message, boolean isUsername) {
				this.title = title;
				this.message = message;
				this.isUsername = isUsername;
			}

			@Override
			public void run() {
				AlertDialog.Builder alert = new AlertDialog.Builder(mSingleton);
				alert.setTitle(title);
				alert.setMessage(message);
				input = new EditText(mSingleton);
				if (isUsername)
				{
					input.setText(mScoreUsername);
					input.setSelection(mScoreUsername.length());
				}
				alert.setView(input);

				alert.setPositiveButton("OK",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
								nativeTextCallback(input.getText().toString());
							}
						});

				alert.setNegativeButton("Cancel",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
							}
						});

				alert.show();
			}
		}
		DialogDisplayer displayer = new DialogDisplayer(title, message, isUsername);
		mSingleton.runOnUiThread(displayer);
	}

	public static void requestScores(int scoreMode) {
	}

	// this is a really ugly method, but I need to chain several callbacks
	// together and I don't see a way to do that without duplicating code
	public static void promptForAlias(int scoreMode, int scoreValue) {
	}

	public static boolean submitScore(int scoreMode, int scoreValue) {
		return mUserCanSubmitScores;
	}
}
