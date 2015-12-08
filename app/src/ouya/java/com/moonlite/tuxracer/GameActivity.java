package com.moonlite.tuxracer;

import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.text.ParseException;
import java.util.Arrays;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.ListView;
import android.widget.Toast;

import tv.ouya.console.api.*;
import static tv.ouya.console.api.OuyaController.BUTTON_O;

import com.moonlite.tuxracer.ScoreActivity;

/**
 * GameActivity implements OUYA In-App Purchase on top of ScoreActivity
 */
public class GameActivity extends ScoreActivity {

	private static final String TAG = "Tuxracer";
	
	/************************************************************************/
	/*																		*/
	/*                  O U Y A   S P E C I F I C   C O D E                 */
	/*																		*/
	/************************************************************************/

	/**
	 * Log onto the developer website (you should have received a URL, a
	 * username and a password in email) and get your developer ID. Plug it in
	 * here. Use your developer ID, not your developer UUID.
	 * <p/>
	 * This the value for Tux Racer.
	 */
	public static final String DEVELOPER_ID = "6972b601-4164-4f94-9738-f241c2cb2459";

	/**
	 * The application key. This is used to decrypt encrypted receipt responses.
	 * This is the application key for Tux Racer
	 */

	private static final byte[] APPLICATION_KEY = { (byte) 0x30, (byte) 0x81,
			(byte) 0x9f, (byte) 0x30, (byte) 0x0d, (byte) 0x06, (byte) 0x09,
			(byte) 0x2a, (byte) 0x86, (byte) 0x48, (byte) 0x86, (byte) 0xf7,
			(byte) 0x0d, (byte) 0x01, (byte) 0x01, (byte) 0x01, (byte) 0x05,
			(byte) 0x00, (byte) 0x03, (byte) 0x81, (byte) 0x8d, (byte) 0x00,
			(byte) 0x30, (byte) 0x81, (byte) 0x89, (byte) 0x02, (byte) 0x81,
			(byte) 0x81, (byte) 0x00, (byte) 0xe1, (byte) 0x6f, (byte) 0xec,
			(byte) 0x6f, (byte) 0xac, (byte) 0x7a, (byte) 0x42, (byte) 0x14,
			(byte) 0xf8, (byte) 0x98, (byte) 0xf5, (byte) 0x2e, (byte) 0x74,
			(byte) 0x91, (byte) 0x3e, (byte) 0x1c, (byte) 0xbb, (byte) 0x5a,
			(byte) 0x69, (byte) 0xe3, (byte) 0x23, (byte) 0x9b, (byte) 0xd0,
			(byte) 0x2c, (byte) 0x71, (byte) 0xd3, (byte) 0x19, (byte) 0xf4,
			(byte) 0xf2, (byte) 0xd3, (byte) 0x78, (byte) 0x36, (byte) 0xb8,
			(byte) 0xfc, (byte) 0xba, (byte) 0xa2, (byte) 0x91, (byte) 0xb7,
			(byte) 0x49, (byte) 0xd7, (byte) 0x13, (byte) 0x63, (byte) 0xa6,
			(byte) 0x38, (byte) 0x75, (byte) 0xb2, (byte) 0x0e, (byte) 0xd7,
			(byte) 0xb1, (byte) 0xb7, (byte) 0xe0, (byte) 0x02, (byte) 0x60,
			(byte) 0x8c, (byte) 0x66, (byte) 0xcf, (byte) 0x9c, (byte) 0x88,
			(byte) 0x8c, (byte) 0x39, (byte) 0xb5, (byte) 0x1d, (byte) 0x1f,
			(byte) 0x13, (byte) 0x22, (byte) 0xe9, (byte) 0xdd, (byte) 0x62,
			(byte) 0x44, (byte) 0x47, (byte) 0xd1, (byte) 0x74, (byte) 0x67,
			(byte) 0x54, (byte) 0xf8, (byte) 0x36, (byte) 0x8b, (byte) 0xab,
			(byte) 0x89, (byte) 0x2f, (byte) 0x0f, (byte) 0xc6, (byte) 0xc9,
			(byte) 0xad, (byte) 0x1d, (byte) 0x3c, (byte) 0xf0, (byte) 0x10,
			(byte) 0x7b, (byte) 0x14, (byte) 0xf8, (byte) 0x19, (byte) 0xe2,
			(byte) 0xf2, (byte) 0x92, (byte) 0x04, (byte) 0x76, (byte) 0xc2,
			(byte) 0x89, (byte) 0xa1, (byte) 0x2f, (byte) 0x3c, (byte) 0x89,
			(byte) 0x0d, (byte) 0x0d, (byte) 0x72, (byte) 0x91, (byte) 0x18,
			(byte) 0x42, (byte) 0xb7, (byte) 0x85, (byte) 0x8f, (byte) 0xaa,
			(byte) 0x93, (byte) 0x43, (byte) 0x40, (byte) 0x59, (byte) 0x31,
			(byte) 0xd5, (byte) 0xe2, (byte) 0xe5, (byte) 0x45, (byte) 0x42,
			(byte) 0x6c, (byte) 0x2d, (byte) 0x01, (byte) 0x65, (byte) 0xcb,
			(byte) 0x02, (byte) 0x03, (byte) 0x01, (byte) 0x00, (byte) 0x01, };

	/**
	 * Before this app will run, you must define some purchasable items on the
	 * developer website. Once you have defined those items, put their Product
	 * IDs in the List below.
	 * <p/>
	 * The Product IDs below are those in our developer account. You should
	 * change them.
	 */
	public static final List<Purchasable> PRODUCT_IDENTIFIER_LIST = Arrays
			.asList(new Purchasable("com_moonlite_tuxracer-course_pack"));

	/**
	 * The saved instance state key for products
	 */

	private static final String PRODUCTS_INSTANCE_STATE_KEY = "Products";

	/**
	 * The saved instance state key for receipts
	 */

	private static final String RECEIPTS_INSTANCE_STATE_KEY = "Receipts";

	/**
	 * The ID used to track the activity started by an authentication intent
	 * during a purchase.
	 */

	private static final int PURCHASE_AUTHENTICATION_ACTIVITY_ID = 1;

	/**
	 * The ID used to track the activity started by an authentication intent
	 * during a request for the gamers UUID.
	 */

	private static final int GAMER_UUID_AUTHENTICATION_ACTIVITY_ID = 2;

	/**
	 * The receipt adapter will display a previously-purchased item in a cell in
	 * a ListView. It's not part of the in-app purchase API. Neither is the
	 * ListView itself.
	 */
	private ListView receiptListView;

	/**
	 * Your game talks to the OuyaFacade, which hides all the mechanics of doing
	 * an in-app purchase.
	 */
	private static OuyaFacade ouyaFacade;
	private static Product mProduct;

	/**
	 * The outstanding purchase request UUIDs.
	 */

	private final static Map<String, Product> mOutstandingPurchaseRequests = new HashMap<String, Product>();

	/**
	 * Broadcast listener to handle re-requesting the receipts when a user has
	 * re-authenticated
	 */

	private BroadcastReceiver mAuthChangeReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			requestReceipts();
		}
	};

	/**
	 * The cryptographic key for this application
	 */

	private static PublicKey mPublicKey;
	
	boolean onOUYA;

	protected void OuyaInitIAP() {

		ouyaFacade = OuyaFacade.getInstance();
		ouyaFacade.init(this, DEVELOPER_ID);

		// Uncomment this line to test against the server using "fake" credits.
		// This will also switch over to a separate "test" purchase history.
		// ouyaFacade.setTestMode();

		// Request the product list if it could not be restored from the
		// savedInstanceState Bundle
		requestProducts();

		requestReceipts();

		// Create a PublicKey object from the key data downloaded from the
		// developer portal.
		try {
			X509EncodedKeySpec keySpec = new X509EncodedKeySpec(APPLICATION_KEY);
			KeyFactory keyFactory = KeyFactory.getInstance("RSA");
			mPublicKey = keyFactory.generatePublic(keySpec);
		} catch (Exception e) {
			Log.e(TAG, "Unable to create encryption key", e);
		}
	}

	/**
	 * Get the list of products the user can purchase from the server.
	 */
	private void requestProducts() {
		ouyaFacade.requestProductList(PRODUCT_IDENTIFIER_LIST,
				new CancelIgnoringOuyaResponseListener<ArrayList<Product>>() {
					@Override
					public void onSuccess(final ArrayList<Product> products) {
						for (Product product : products) {
							mProduct = product;
							String price = "$" + Double.toString(product.getLocalPrice());
							Log.i(TAG, "Product " + product.getName()
									+ " price=" + price);
							nativeCoursePrice(price);
						}
					}

					@Override
					public void onFailure(int errorCode, String errorMessage,
							Bundle optionalData) {
						// Your app probably wants to do something more
						// sophisticated than popping a Toast. This is
						// here to tell you that your app needs to handle this
						// case: if your app doesn't display
						// something, the user won't know of the failure.
						Toast.makeText(
								GameActivity.this,
								"Could not fetch product information (error "
										+ errorCode + ": " + errorMessage + ")",
								Toast.LENGTH_LONG).show();
					}
				});
	}

	private void fetchGamerInfo() {
		ouyaFacade
				.requestGamerInfo(new CancelIgnoringOuyaResponseListener<GamerInfo>() {
					@Override
					public void onSuccess(GamerInfo result) {
						new AlertDialog.Builder(GameActivity.this)
								.setTitle(getString(R.string.alert_title))
								.setMessage(
										getResources().getString(
												R.string.userinfo,
												result.getUsername(),
												result.getUuid()))
								.setPositiveButton(R.string.ok, null).show();
					}

					@Override
					public void onFailure(int errorCode, String errorMessage,
							Bundle optionalData) {
						Log.w(TAG, "fetch gamer UUID error (code " + errorCode
								+ ": " + errorMessage + ")");
						boolean wasHandledByAuthHelper = OuyaAuthenticationHelper
								.handleError(GameActivity.this, errorCode,
										errorMessage, optionalData,
										GAMER_UUID_AUTHENTICATION_ACTIVITY_ID,
										new OuyaResponseListener<Void>() {
											@Override
											public void onSuccess(Void result) {
												fetchGamerInfo(); // Retry the
																	// fetch if
																	// the error
																	// was
																	// handled.
											}

											@Override
											public void onFailure(
													int errorCode,
													String errorMessage,
													Bundle optionalData) {
												showError("Unable to fetch gamer UUID (error "
														+ errorCode
														+ ": "
														+ errorMessage + ")");
											}

											@Override
											public void onCancel() {
												showError("Unable to fetch gamer UUID");
											}
										});

						if (!wasHandledByAuthHelper) {
							showError("Unable to fetch gamer UUID (error "
									+ errorCode + ": " + errorMessage + ")");
						}
					}
				});
	}

	/**
	 * Request the receipts from the users previous purchases from the server.
	 */

	private static void requestReceipts() {
		ouyaFacade.requestReceipts(new ReceiptListener());
	}

	/*
	 * This will be called when the user clicks on an item in the ListView.
	 */
	public static void requestPurchase(final Product product)
			throws GeneralSecurityException, UnsupportedEncodingException,
			JSONException {
		SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");

		// This is an ID that allows you to associate a successful purchase with
		// it's original request. The server does nothing with this string
		// except
		// pass it back to you, so it only needs to be unique within this
		// instance
		// of your app to allow you to pair responses with requests.
		String uniqueId = Long.toHexString(sr.nextLong());

		JSONObject purchaseRequest = new JSONObject();
		purchaseRequest.put("uuid", uniqueId);
		purchaseRequest.put("identifier", product.getIdentifier());
		// This value is only needed for testing, not setting it results in a live purchase
		purchaseRequest.put("testing", "true");
		String purchaseRequestJson = purchaseRequest.toString();

		byte[] keyBytes = new byte[16];
		sr.nextBytes(keyBytes);
		SecretKey key = new SecretKeySpec(keyBytes, "AES");

		byte[] ivBytes = new byte[16];
		sr.nextBytes(ivBytes);
		IvParameterSpec iv = new IvParameterSpec(ivBytes);

		Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding", "BC");
		cipher.init(Cipher.ENCRYPT_MODE, key, iv);
		byte[] payload = cipher.doFinal(purchaseRequestJson.getBytes("UTF-8"));

		cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding", "BC");
		cipher.init(Cipher.ENCRYPT_MODE, mPublicKey);
		byte[] encryptedKey = cipher.doFinal(keyBytes);

		Purchasable purchasable = new Purchasable(product.getIdentifier(),
				Base64.encodeToString(encryptedKey, Base64.NO_WRAP),
				Base64.encodeToString(ivBytes, Base64.NO_WRAP),
				Base64.encodeToString(payload, Base64.NO_WRAP));

		synchronized (mOutstandingPurchaseRequests) {
			mOutstandingPurchaseRequests.put(uniqueId, product);
		}
		ouyaFacade.requestPurchase(purchasable, new PurchaseListener(product));
	}

	/**
	 * OnClickListener to handle purchase requests.
	 */

	public class RequestPurchaseClickListener implements View.OnClickListener {
		@Override
		public void onClick(View v) {
			try {
				requestPurchase((Product) v.getTag());
			} catch (Exception ex) {
				Log.e(TAG, "Error requesting purchase", ex);
				showError(ex.getMessage());
			}
		}
	}

	/**
	 * Display an error to the user. We're using a toast for simplicity.
	 */

	private static void showError(final String errorMessage) {
		Toast.makeText(getContext(), errorMessage, Toast.LENGTH_LONG).show();
	}

	/**
	 * The callback for when the list of user receipts has been requested.
	 */
	private static class ReceiptListener implements
			OuyaResponseListener<String> {
		/**
		 * Handle the successful fetching of the data for the receipts from the
		 * server.
		 * 
		 * @param receiptResponse
		 *            The response from the server.
		 */
		@Override
		public void onSuccess(String receiptResponse) {
			OuyaFacade.getInstance().putGameData("receipts", receiptResponse);
			Log.d(TAG, "storing receipts");
			
			OuyaEncryptionHelper helper = new OuyaEncryptionHelper();
			List<Receipt> receipts;
			try {
				JSONObject response = new JSONObject(receiptResponse);
				receipts = helper.decryptReceiptResponse(response, mPublicKey);
			} catch (ParseException e) {
				throw new RuntimeException(e);
			} catch (JSONException e) {
				throw new RuntimeException(e);
			} catch (GeneralSecurityException e) {
				throw new RuntimeException(e);
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
			
			// Iterate through receipts
			for (Receipt receipt : receipts) {
				String item = receipt.getIdentifier();
				String gamer = receipt.getGamer();
				Date date = receipt.getPurchaseDate();
				Log.i(TAG, gamer + " bought " + item + " on " + date);
				nativeCoursePrice(null);
			}
		}

		/**
		 * Handle a failure. Because displaying the receipts is not critical to
		 * the application we just show an error message rather than asking the
		 * user to authenticate themselves just to start the application up.
		 * 
		 * @param errorCode
		 *            An HTTP error code between 0 and 999, if there was one.
		 *            Otherwise, an internal error code from the Ouya server,
		 *            documented in the {@link OuyaErrorCodes} class.
		 * 
		 * @param errorMessage
		 *            Empty for HTTP error codes. Otherwise, a brief,
		 *            non-localized, explanation of the error.
		 * 
		 * @param optionalData
		 *            A Map of optional key/value pairs which provide additional
		 *            information.
		 */

		@Override
		public void onFailure(int errorCode, String errorMessage, Bundle optionalData) {
			Log.e(TAG, "getting receipts failed");
			
			OuyaEncryptionHelper helper = new OuyaEncryptionHelper();
			List<Receipt> receipts;
			try {
				String receiptResponse=OuyaFacade.getInstance().getGameData("receipts");
				
				if (receiptResponse==null)
					return;
				
				Log.e(TAG, "using stored receipts");
				
				JSONObject response = new JSONObject(receiptResponse);
				receipts = helper.decryptReceiptResponse(response, mPublicKey);
			} catch (ParseException e) {
				throw new RuntimeException(e);
			} catch (JSONException e) {
				throw new RuntimeException(e);
			} catch (GeneralSecurityException e) {
				throw new RuntimeException(e);
			} catch (IOException e) {
				return;
			}
			
			// Iterate through receipts
			for (Receipt receipt : receipts) {
				String item = receipt.getIdentifier();
				String gamer = receipt.getGamer();
				Date date = receipt.getPurchaseDate();
				Log.i(TAG, gamer + " bought " + item + " on " + date);
				nativeCoursePrice(null);
			}
		}

		/*
		 * Handle user canceling
		 */
		@Override
		public void onCancel() {
			showError("User cancelled getting receipts");
		}

	}

	/**
	 * The callback for when the user attempts to purchase something. We're not
	 * worried about the user cancelling the purchase so we extend
	 * CancelIgnoringOuyaResponseListener, if you want to handle cancelations
	 * differently you should extend OuyaResponseListener and implement an
	 * onCancel method.
	 * 
	 * @see tv.ouya.console.api.CancelIgnoringOuyaResponseListener
	 * @see tv.ouya.console.api.OuyaResponseListener#onCancel()
	 */
	private static class PurchaseListener implements
			OuyaResponseListener<String> {
		/**
		 * The ID of the product the user is trying to purchase. This is used in
		 * the onFailure method to start a re-purchase if they user wishes to do
		 * so.
		 */

		private Product mProduct;

		/**
		 * Constructor. Store the ID of the product being purchased.
		 */

		PurchaseListener(final Product product) {
			mProduct = product;
		}

		/**
		 * Handle a successful purchase.
		 * 
		 * @param result
		 *            The response from the server.
		 */
		@Override
		public void onSuccess(String result) {
//			Product product;
			String id;
			try {
				OuyaEncryptionHelper helper = new OuyaEncryptionHelper();

				JSONObject response = new JSONObject(result);
				id = helper.decryptPurchaseResponse(response, mPublicKey);
				Product storedProduct;
				synchronized (mOutstandingPurchaseRequests) {
					storedProduct = mOutstandingPurchaseRequests.remove(id);
				}
				if (storedProduct == null
						|| !storedProduct.getIdentifier().equals(
								mProduct.getIdentifier())) {
					onFailure(
							OuyaErrorCodes.THROW_DURING_ON_SUCCESS,
							"Purchased product is not the same as purchase request product",
							Bundle.EMPTY);
					return;
				}
			} catch (ParseException e) {
				onFailure(OuyaErrorCodes.THROW_DURING_ON_SUCCESS,
						e.getMessage(), Bundle.EMPTY);
			} catch (JSONException e) {
				onFailure(OuyaErrorCodes.THROW_DURING_ON_SUCCESS,
						e.getMessage(), Bundle.EMPTY);
				return;
			} catch (IOException e) {
				onFailure(OuyaErrorCodes.THROW_DURING_ON_SUCCESS,
						e.getMessage(), Bundle.EMPTY);
				return;
			} catch (GeneralSecurityException e) {
				onFailure(OuyaErrorCodes.THROW_DURING_ON_SUCCESS,
						e.getMessage(), Bundle.EMPTY);
				return;
			}

			requestReceipts();

		}

		@Override
		public void onFailure(int errorCode, String errorMessage,
				Bundle optionalData) {
		}

		/*
		 * Handling the user canceling
		 */
		@Override
		public void onCancel() {
			showError("User cancelled purchase");
		}
	}

	/**
	 * JNI Callback called when user clicks button to buy the course_pack
	 * entitlement. This method calls
	 * {@link PurchasingManager#initiatePurchaseRequest(String)} with the SKU
	 * for the course_pack entitlement.
	 */
	static public void BuyItem(int id) {
		try {
			requestPurchase(mProduct);
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (GeneralSecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	// Setup
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// Log.v("SDL", "onCreate()");
		super.onCreate(savedInstanceState);

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		onOUYA = OuyaFacade.getInstance().isRunningOnOUYAHardware();
		nativeSetPlayerData("", onOUYA);

		// Initialize appropriate In-App Purchase system
		OuyaInitIAP();
	}

    // Events
    @Override
    protected void onPause() {
        Log.v(TAG, "onPause()");
        super.onPause();
    }

    @Override
    protected void onResume() {
        Log.v(TAG, "onResume()");
        super.onResume();
    }


	public static native void nativeSetPlayerData(String playerName, boolean isOnOuya);
	public static native void nativeCoursePrice(String price);
}
