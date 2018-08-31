package com.moonlite.tuxracer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.moonlite.tuxracer.ScoreActivity;

/**
 * GameActivity implements full version on top of ScoreActivity
 */
public class GameActivity extends ScoreActivity {
	private static final String TAG = "Tuxracer";
	
	private static GameActivity mSingleton;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mSingleton=this;
	}

	protected void onDestroy() {
		super.onDestroy();
		mSingleton=null;
	}	
}
