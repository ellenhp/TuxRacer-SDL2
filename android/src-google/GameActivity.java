package com.moonlite.tuxracer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.moonlite.tuxracer.ScoreActivity;
import com.moonlite.tuxracer.googleplayiap.*;

/**
 * GameActivity implements (both Amazon and OUYA) In-App Purchase on top of
 * ScoreActivity
 */
public class GameActivity extends ScoreActivity {
	private static final String TAG = "Tuxracer";
	protected static final String SKU_PREMIUM = "full_version";
    protected static final int RC_REQUEST = 10001;

	private IabHelper mHelper;
	
	private static GameActivity mSingleton;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		InitGoogleIAP();
		mSingleton=this;
	}

	protected void onDestroy() {
		super.onDestroy();
		mSingleton=null;
	}
	
	private String read_key() {
		try {
			InputStream is = getResources().getAssets().open("play_key.txt");
			BufferedReader reader=new BufferedReader(new InputStreamReader(is, "UTF-8"));
			return reader.readLine().trim();
		} catch (IOException e) {
			Log.e(TAG, "Unable to read IAP key");
			return "";
		}
	}

	private void InitGoogleIAP() {
		String base64EncodedPublicKey = xor_secret(read_key());

		mHelper = new IabHelper(this, base64EncodedPublicKey);
		
		mHelper.enableDebugLogging(true);

		// Start setup. This is asynchronous and the specified listener
		// will be called once setup completes.
		Log.d(TAG, "Starting setup.");
		mHelper.startSetup(new IabHelper.OnIabSetupFinishedListener() {
			IabHelper.QueryInventoryFinishedListener mGotInventoryListener = new IabHelper.QueryInventoryFinishedListener() {
				public void onQueryInventoryFinished(IabResult result,
						Inventory inventory) {
					Log.d(TAG, "Query inventory finished.");

					// Have we been disposed of in the meantime? If so, quit.
					if (mHelper == null)
						return;

					// Is it a failure?
					if (result.isFailure()) {
						Log.e(TAG, "Failed to query inventory: " + result);
						return;
					}

					Log.d(TAG, "Query inventory was successful.");

					/*
					 * Check for items we own. Notice that for each purchase, we
					 * check the developer payload to see if it's correct! See
					 * verifyDeveloperPayload().
					 */

					// Do we have the premium upgrade?
					Purchase premiumPurchase = inventory.getPurchase(SKU_PREMIUM);

					Log.d(TAG, "premium: "+(premiumPurchase!=null));
					
					if (premiumPurchase!=null) {
						nativeCoursePrice(null);
					}
				}
			};

			public void onIabSetupFinished(IabResult result) {
				Log.d(TAG, "Setup finished.");

				if (!result.isSuccess()) {
					Log.e(TAG, "Problem setting up in-app billing: " + result);
					return;
				}

				// Have we been disposed of in the meantime? If so, quit.
				if (mHelper == null) {
					return;
				}

				// IAB is fully set up. Now, let's get an inventory of stuff we
				// own.
				Log.d(TAG, "Setup successful. Querying inventory.");
				mHelper.queryInventoryAsync(mGotInventoryListener);
				Log.d(TAG, "Started async inventory query.");
			}
		});
	}
	
	static public void BuyItem(int id) {
		Log.d(TAG, "purchasing");
		mSingleton.doPurchase();
	}
	
	private void doPurchase() {
		if (mSingleton!=null) {
	        Log.d(TAG, "Buying full version.");

	        String payload = "";

	        mHelper.launchPurchaseFlow(this, SKU_PREMIUM, RC_REQUEST, mPurchaseFinishedListener, payload);
		}
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		Log.d(TAG, "onActivityResult(" + requestCode + "," + resultCode + "," + data);
		if (!mHelper.handleActivityResult(requestCode, resultCode, data)) {
			super.onActivityResult(requestCode, resultCode, data);
		} else {
			Log.d(TAG, "onActivityResult handled by IABUtil.");
		}
	}
	
    IabHelper.OnIabPurchaseFinishedListener mPurchaseFinishedListener = new IabHelper.OnIabPurchaseFinishedListener() {
        public void onIabPurchaseFinished(IabResult result, Purchase purchase) {
            Log.d(TAG, "Purchase finished: " + result + ", purchase: " + purchase);

            // if we were disposed of in the meantime, quit.
            if (mHelper == null) return;

            if (result.isFailure()) {
                return;
            }

            Log.d(TAG, "Purchase successful.");

            if (purchase.getSku().equals(SKU_PREMIUM)) {
            	nativeCoursePrice(null);
            }
        }
    };

	public static native void nativeSetPlayerData(String playerName,
			boolean isOnOuya);

	public static native void nativeCoursePrice(String price);
}
