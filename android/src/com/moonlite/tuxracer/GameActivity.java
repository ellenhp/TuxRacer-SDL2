package com.moonlite.tuxracer;

import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.text.ParseException;
import java.util.Arrays;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.lang.*;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeSet;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.amazon.inapp.purchasing.Item;
import com.amazon.inapp.purchasing.PurchasingManager;
import com.moonlite.tuxracer.AppPurchasingObserver.PurchaseData;
import com.moonlite.tuxracer.AppPurchasingObserver.PurchaseDataStorage;

// Scoreloop interface
import com.scoreloop.client.android.core.controller.RequestController;
import com.scoreloop.client.android.core.controller.RequestControllerObserver;
import com.scoreloop.client.android.core.controller.ScoreController;
import com.scoreloop.client.android.core.controller.ScoresController;
import com.scoreloop.client.android.core.controller.TermsOfServiceController;
import com.scoreloop.client.android.core.controller.TermsOfServiceControllerObserver;
import com.scoreloop.client.android.core.controller.UserController;
import com.scoreloop.client.android.core.model.Client;
import com.scoreloop.client.android.core.model.Score;
import com.scoreloop.client.android.core.model.ScoreFormatter;
import com.scoreloop.client.android.core.model.SearchList;
import com.scoreloop.client.android.core.model.Session;

import tv.ouya.console.api.*;
import static tv.ouya.console.api.OuyaController.BUTTON_O;

import org.libsdl.app.SDLActivity;

/**
 * GameActivity implements (both Amazon and OUYA) In-App Purchase on top of SDLActivity
 */
public class GameActivity extends SDLActivity implements
		AppPurchasingObserverListener {
	private static final String TAG = "Tuxracer";

	private static boolean mUserCanSubmitScores = false;
	private static Runnable mScoreToSubmit = null;

	private static String mOuyaUsername = null;
	private static String mScoreloopUsername = "";
	
	// Load the .so
	static {
		System.loadLibrary("SDL2");
		System.loadLibrary("SDL2_image");
		System.loadLibrary("SDL2_mixer");
		// System.loadLibrary("SDL2_net");
		// System.loadLibrary("SDL2_ttf");
		System.loadLibrary("tcl");
		// Step 1 Load libscoreloopcore.so before loading your game's native
		// library.
		// System.loadLibrary("scoreloopcore");
		System.loadLibrary("main");
	}

	private String xor_secret(String secret) {
		int i;
		char[] buf = new char[secret.length()];
		char[] secretBuf = secret.toCharArray();
		int len = secret.length();
		for (i = 0; i < len; ++i) {
			buf[i] = (char) (secretBuf[i] ^ (i & 15) + 1);
		}
		return new String(buf);
	}

	/************************************************************************/
	/*																		*/
	/*                A M A Z O N   I N - A P P   P U R C H A S E           */
	/*																		*/
	/************************************************************************/

	private static PurchaseDataStorage purchaseDataStorage;

	void AmazonInitIAP() {
		/**
		 * Setup for IAP SDK called from onCreate. Sets up
		 * {@link PurchaseDataStorage} for storing purchase receipt data,
		 * {@link AppPurchasingObserver} for listening to IAP API callbacks and
		 * sets up this activity as a {@link AppPurchasingObserverListener} to
		 * listen for callbacks from the {@link AppPurchasingObserver}.
		 */
		purchaseDataStorage = new PurchaseDataStorage(this);

		AppPurchasingObserver purchasingObserver = new AppPurchasingObserver(
				this, purchaseDataStorage);
		purchasingObserver.setListener(this);

		Log.i(TAG, "onCreate: registering AppPurchasingObserver");
		PurchasingManager.registerObserver(purchasingObserver);
	}

	/**
	 * Enable entitlement for SKU
	 * 
	 * @param sku
	 */
	private void enableEntitlementForSKU(String sku) {
		if (MySKU.COURSE_PACK.getSku().equals(sku)) {
			nativeCoursePrice(0);
		}
	}

	/**
	 * Callback for a successful get user id response
	 * {@link GetUserIdResponseStatus#SUCCESSFUL}.
	 * 
	 * In this sample app, if the user changed from the previously stored user,
	 * this method updates the display based on purchase data stored for the
	 * user in SharedPreferences. The entitlement is fulfilled if a stored
	 * purchase token was found to NOT be fulfilled or if the SKU should be
	 * fulfilled.
	 * 
	 * @param userId
	 *            returned from {@link GetUserIdResponse#getUserId()}.
	 * @param userChanged
	 *            - whether user changed from previously stored user.
	 */
	@Override
	public void onGetUserIdResponseSuccessful(String userId, boolean userChanged) {
		Log.i(TAG, "onGetUserIdResponseSuccessful: update display if userId ("
				+ userId + ") user changed from previous stored user ("
				+ userChanged + ")");

		if (!userChanged)
			return;

		// Reset to original setting where entitlement is disabled
		// disableEntitlementInView();

		Set<String> requestIds = purchaseDataStorage.getAllRequestIds();
		Log.i(TAG, "onGetUserIdResponseSuccessful: (" + requestIds.size()
				+ ") saved requestIds");
		for (String requestId : requestIds) {
			PurchaseData purchaseData = purchaseDataStorage
					.getPurchaseData(requestId);
			if (purchaseData == null) {
				Log.i(TAG,
						"onGetUserIdResponseSuccessful: could NOT find purchaseData for requestId ("
								+ requestId + "), skipping");
				continue;
			}
			if (purchaseDataStorage.isRequestStateSent(requestId)) {
				Log.i(TAG,
						"onGetUserIdResponseSuccessful: have not received purchase response for requestId still in SENT status: requestId ("
								+ requestId + "), skipping");
				continue;
			}

			Log.d(TAG, "onGetUserIdResponseSuccessful: requestId (" + requestId
					+ ") " + purchaseData);

			String purchaseToken = purchaseData.getPurchaseToken();
			String sku = purchaseData.getSKU();
			if (!purchaseData.isPurchaseTokenFulfilled()) {
				Log.i(TAG, "onGetUserIdResponseSuccess: requestId ("
						+ requestId + ") userId (" + userId + ") sku (" + sku
						+ ") purchaseToken (" + purchaseToken
						+ ") was NOT fulfilled, fulfilling purchase now");
				onPurchaseResponseSuccess(userId, sku, purchaseToken);

				purchaseDataStorage.setPurchaseTokenFulfilled(purchaseToken);
				purchaseDataStorage.setRequestStateFulfilled(requestId);
			} else {
				boolean shouldFulfillSKU = purchaseDataStorage
						.shouldFulfillSKU(sku);
				if (shouldFulfillSKU) {
					Log.i(TAG,
							"onGetUserIdResponseSuccess: should fulfill sku ("
									+ sku
									+ ") is true, so fulfilling purchasing now");
					onPurchaseUpdatesResponseSuccess(userId, sku, purchaseToken);
				}
			}
		}
	}

	/**
	 * Callback for a failed get user id response
	 * {@link GetUserIdRequestStatus#FAILED}
	 * 
	 * @param requestId
	 *            returned from {@link GetUserIdResponsee#getRequestId()} that
	 *            can be used to correlate with original request sent with
	 *            {@link PurchasingManager#initiateGetUserIdRequest()}.
	 */
	@Override
	public void onGetUserIdResponseFailed(String requestId) {
		Log.i(TAG, "onGetUserIdResponseFailed for requestId (" + requestId
				+ ")");
	}

	/**
	 * Callback for successful item data response with unavailable SKUs
	 * {@link ItemDataRequestStatus#SUCCESSFUL_WITH_UNAVAILABLE_SKUS}. This
	 * means that these unavailable SKUs are NOT accessible in developer portal.
	 * 
	 * @param unavailableSkus
	 *            - skus that are not valid in developer portal
	 */
	@Override
	public void onItemDataResponseSuccessfulWithUnavailableSkus(
			Set<String> unavailableSkus) {
		Log.i(TAG, "onItemDataResponseSuccessfulWithUnavailableSkus: for ("
				+ unavailableSkus.size() + ") unavailableSkus");
	}

	/**
	 * Callback for successful item data response
	 * {@link ItemDataRequestStatus#SUCCESSFUL} with item data
	 * 
	 * @param itemData
	 *            - map of valid SKU->Items
	 */
	@Override
	public void onItemDataResponseSuccessful(Map<String, Item> itemData) {
		for (Entry<String, Item> entry : itemData.entrySet()) {
			String sku = entry.getKey();
			Item item = entry.getValue();
			Log.i(TAG, "onItemDataResponseSuccessful: sku (" + sku + ") item ("
					+ item + ")");
			if (MySKU.COURSE_PACK.getSku().equals(sku)) {
				float price = Float.parseFloat(item.getPrice());
				nativeCoursePrice(price);
			}
		}
	}

	/**
	 * Callback for failed item data response
	 * {@link ItemDataRequestStatus#FAILED}.
	 * 
	 * @param requestId
	 */
	public void onItemDataResponseFailed(String requestId) {
		Log.i(TAG, "onItemDataResponseFailed: for requestId (" + requestId
				+ ")");
	}

	/**
	 * Callback on successful purchase response
	 * {@link PurchaseRequestStatus#SUCCESSFUL}.
	 * 
	 * @param sku
	 */
	@Override
	public void onPurchaseResponseSuccess(String userId, String sku,
			String purchaseToken) {
		Log.i(TAG, "onPurchaseResponseSuccess: for userId (" + userId
				+ ") sku (" + sku + ") purchaseToken (" + purchaseToken + ")");
		enableEntitlementForSKU(sku);
	}

	/**
	 * Callback when user is already entitled
	 * {@link PurchaseRequestStatus#ALREADY_ENTITLED} to sku passed into
	 * initiatePurchaseRequest.
	 * 
	 * @param userId
	 */
	@Override
	public void onPurchaseResponseAlreadyEntitled(String userId, String sku) {
		Log.i(TAG, "onPurchaseResponseAlreadyEntitled: for userId (" + userId
				+ ") sku (" + sku + ")");
		// For entitlements, even if already entitled, make sure to enable.
		enableEntitlementForSKU(sku);
	}

	/**
	 * Callback when sku passed into
	 * {@link PurchasingManager#initiatePurchaseRequest} is not valid
	 * {@link PurchaseRequestStatus#INVALID_SKU}.
	 * 
	 * @param userId
	 * @param sku
	 */
	@Override
	public void onPurchaseResponseInvalidSKU(String userId, String sku) {
		Log.i(TAG, "onPurchaseResponseInvalidSKU: for userId (" + userId
				+ ") sku (" + sku + ")");
	}

	/**
	 * Callback on failed purchase response {@link PurchaseRequestStatus#FAILED}
	 * .
	 * 
	 * @param requestId
	 * @param sku
	 */
	@Override
	public void onPurchaseResponseFailed(String requestId, String sku) {
		Log.i(TAG, "onPurchaseResponseFailed: for requestId (" + requestId
				+ ") sku (" + sku + ")");
	}

	/**
	 * Callback on successful purchase updates response
	 * {@link PurchaseUpdatesRequestStatus#SUCCESSFUL} for each receipt.
	 * 
	 * @param userId
	 * @param sku
	 * @param purchaseToken
	 */
	@Override
	public void onPurchaseUpdatesResponseSuccess(String userId, String sku,
			String purchaseToken) {
		Log.i(TAG, "onPurchaseUpdatesResponseSuccess: for userId (" + userId
				+ ") sku (" + sku + ") purchaseToken (" + purchaseToken + ")");
		enableEntitlementForSKU(sku);
	}

	/**
	 * Callback on successful purchase updates response
	 * {@link PurchaseUpdatesRequestStatus#SUCCESSFUL} for revoked SKU.
	 * 
	 * @param userId
	 * @param revokedSKU
	 */
	@Override
	public void onPurchaseUpdatesResponseSuccessRevokedSku(String userId,
			String revokedSku) {
		Log.i(TAG, "onPurchaseUpdatesResponseSuccessRevokedSku: for userId ("
				+ userId + ") revokedSku (" + revokedSku + ")");
		if (!MySKU.COURSE_PACK.getSku().equals(revokedSku))
			return;

		Log.i(TAG,
				"onPurchaseUpdatesResponseSuccessRevokedSku: disabling entitlement");
		// disableEntitlementInView();
	}

	/**
	 * Callback on failed purchase updates response
	 * {@link PurchaseUpdatesRequestStatus#FAILED}
	 * 
	 * @param requestId
	 */
	public void onPurchaseUpdatesResponseFailed(String requestId) {
		Log.i(TAG, "onPurchaseUpdatesResponseFailed: for requestId ("
				+ requestId + ")");
	}

	protected void AmazonRequestIAP() {
		Log.i(TAG, "onResume: call initiateGetUserIdRequest");
		PurchasingManager.initiateGetUserIdRequest();

		Log.i(TAG,
				"onResume: call initiateItemDataRequest for skus: "
						+ MySKU.getAll());
		PurchasingManager.initiateItemDataRequest(MySKU.getAll());
	}

	/**
	 * JNI Callback called when user clicks button to buy the course_pack
	 * entitlement. This method calls
	 * {@link PurchasingManager#initiatePurchaseRequest(String)} with the SKU
	 * for the course_pack entitlement.
	 */
	static public void AmazonBuyItem(int id) {
		String requestId = PurchasingManager
				.initiatePurchaseRequest(MySKU.COURSE_PACK.getSku());
		PurchaseData purchaseData = purchaseDataStorage
				.newPurchaseData(requestId);
		Log.i(TAG, "AmazonBuyItem: requestId (" + requestId
				+ ") requestState (" + purchaseData.getRequestState() + ")");
	}

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
							float price = (float) product.getLocalPrice();
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

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		if (keyCode == BUTTON_O) {
			View focusedButton = getCurrentFocus();
			focusedButton.performClick();
			return true;
		}
		return super.onKeyUp(keyCode, event);
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
				nativeCoursePrice(0);
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
				nativeCoursePrice(0);
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
			Product product;
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

	private static void updateUserInfo() {
		UserController userController = new UserController(
				new RequestControllerObserver() {
					@Override
					public void requestControllerDidReceiveResponse(
							RequestController arg0) {
						mUserCanSubmitScores = !((UserController) arg0)
								.getUser().isAnonymous();
						if (mUserCanSubmitScores) {
							mScoreloopUsername=((UserController) arg0)
									.getUser().getLogin();
							nativeUpdateUserInfo(mScoreloopUsername);
						} else {
							nativeUpdateUserInfo(null);
						}
					}

					@Override
					public void requestControllerDidFail(
							RequestController arg0, Exception arg1) {
						// something's up, not sure what to do.
						nativeUpdateUserInfo("[error]");
					}
				});
		userController.loadUser();
	}

	/**
	 * JNI Callback called when user clicks button to buy the course_pack
	 * entitlement. This method calls
	 * {@link PurchasingManager#initiatePurchaseRequest(String)} with the SKU
	 * for the course_pack entitlement.
	 */
	static public void OUYABuyItem(int id) {
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

		Client.init(this.getApplicationContext(),
				xor_secret("S)TEPoEmjI_]" + ((char) 127)
						+ "Zf]1C(}INbRP:n>O\\7C1uo|okrl@=BjUXX#8M;bQG:5"), null);

		final TermsOfServiceController controller = new TermsOfServiceController(
				new TermsOfServiceControllerObserver() {
					@Override
					public void termsOfServiceControllerDidFinish(
							final TermsOfServiceController controller,
							final Boolean accepted) {
						if (accepted != null) {
							// TODO react to this
							if (accepted) {
								updateUserInfo();
							} else {
							}
						}
					}
				});
		controller.query(this);
		// Initialize appropriate In-App Purchase system
		if (onOUYA)
			OuyaInitIAP();
		else
			AmazonInitIAP();
	}

	public static native void nativeSetPlayerData(String playerName, boolean isOnOuya);
	public static native void nativeScoreloopGotScores(int scoreMode, Object[] scoreStrings);
	public static native void nativeTextCallback(String string);
	public static native void nativeDisableAliasPrompt();
	public static native void nativeUpdateUserInfo(String alias);
	public static native void nativeCoursePrice(float price);

	/************************************************************************/
	/*																		*/
	/*                  S C O R E L O O P   A C H E I V E M E N T           */
	/*																		*/
	/************************************************************************/

	public static void setUserEmail(String email) {
		class EmailSetter implements Runnable {
			String email;

			public EmailSetter(String email) {
				this.email = email;
			}

			@Override
			public void run() {
				UserController controller = new UserController(
						new RequestControllerObserver() {
							@Override
							public void requestControllerDidFail(
									RequestController arg0, Exception arg1) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Email in use")
										.setMessage(
												"Check your email to confirm this email is yours. Look for a message from Scoreloop. Once you merge the devices, restart Tux Racer to use the alias associated with the new email. Open browser now?")
										.setCancelable(false)
										.setPositiveButton(
												"Yes",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
														Uri uri = Uri
																.parse("http://www.google.com");
														Intent intent = new Intent(
																Intent.ACTION_VIEW,
																uri);
														mSingleton
																.startActivity(intent);
													}
												})
										.setNegativeButton(
												"No",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
							}

							@Override
							public void requestControllerDidReceiveResponse(
									RequestController arg0) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Success")
										.setMessage(
												"Email address was set successfully. Now you can reuse your alias or use one you set previously on another device.")
										.setCancelable(false)
										.setPositiveButton(
												"OK",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
								updateUserInfo();
							}
						});
				controller.setUser(Session.getCurrentSession().getUser());
				controller.getUser().setEmailAddress(email);
				controller.submitUser();
			}
		}
		if (android.util.Patterns.EMAIL_ADDRESS.matcher(email).matches()) {
			EmailSetter setter = new EmailSetter(email);
			mSingleton.runOnUiThread(setter);
		} else {
			mSingleton.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
							mSingleton);

					alertDialogBuilder
							.setTitle("Invalid Email")
							.setMessage("That email seems to be invalid.")
							.setCancelable(false)
							.setPositiveButton("OK",
									new DialogInterface.OnClickListener() {
										public void onClick(
												DialogInterface dialog, int id) {
										}
									}).create().show();
				}
			});
		}
	}

	public static void setUserAlias(String alias) {
		class AliasSetter implements Runnable {
			String alias;

			public AliasSetter(String alias) {
				this.alias = alias;
			}

			@Override
			public void run() {
				UserController controller = new UserController(
						new RequestControllerObserver() {
							@Override
							public void requestControllerDidFail(
									RequestController arg0, Exception arg1) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Alias in use")
										.setMessage(
												"Try again or (if you are using this alias on another device) set your email here and then check for a message from Scoreloop asking if you want to merge the devices")
										.setPositiveButton(
												"OK",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
							}

							@Override
							public void requestControllerDidReceiveResponse(
									RequestController arg0) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Success")
										.setMessage("Now go set a high score!")
										.setPositiveButton(
												"OK",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
								updateUserInfo();
							}
						});
				controller.setUser(Session.getCurrentSession().getUser());
				controller.getUser().setLogin(alias);
				controller.submitUser();
				Log.d("scoreloop", "setting username to " + alias);
			}
		}
		AliasSetter aliasSetter = new AliasSetter(alias);
		mSingleton.runOnUiThread(aliasSetter);
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
					input.setText(GameActivity.mScoreloopUsername);
					input.setSelection(mScoreloopUsername.length());
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
		class ScoreRequester implements Runnable {
			public int scoreMode;

			@Override
			public void run() {
				class ScoreboardObserver implements RequestControllerObserver {
					int scoreMode;
					int expectedCallbacks;
					int callbacks = 0;
					HashSet<Score> scores = new HashSet<Score>();

					public ScoreboardObserver(int expectedCallbacks,
							int scoreMode) {
						this.expectedCallbacks = expectedCallbacks;
						this.scoreMode = scoreMode;
					}

					@Override
					public void requestControllerDidFail(
							RequestController controller, Exception ex) {
						ex.printStackTrace();
						callbacks++;
						if (callbacks == expectedCallbacks) {
							process();
						}
					}

					@Override
					public void requestControllerDidReceiveResponse(
							RequestController controller) {
						callbacks++;
						scores.addAll(((ScoresController) controller)
								.getScores());
						if (callbacks == expectedCallbacks) {
							process();
						}
					}

					private void process() {
						ArrayList<Score> scoreList = new ArrayList<Score>(
								scores);
						Collections.sort(scoreList, new Comparator<Score>() {
							public int compare(Score a, Score b) {
								return a.getRank() - b.getRank();
							}
						});
						// if there are >10 unique elements, the user isn't in
						// the top 10 so we remove the 10th element (index 9)
						// and allow the user (previously position 11/index 10)
						// to take his or her spot.
						if (scoreList.size() > 10) {
							scoreList.remove(9);
						}
						ArrayList<String> scoreStrings = new ArrayList<String>();
						for (Score score : scoreList) {
							scoreStrings.add(score.getRank().toString() + "\t"
									+ score.getUser().getDisplayName() + "\t"
									+ score.getResult().intValue());
						}
						nativeScoreloopGotScores(scoreMode,
								scoreStrings.toArray());
					}
				}
				;
				Log.d("scores", "Starting score acquisition.");
				ScoreboardObserver observer = new ScoreboardObserver(2,
						scoreMode);
				ScoresController scoresController = new ScoresController(
						Session.getCurrentSession(), observer);
				scoresController.setMode(scoreMode);
				scoresController.setRangeLength(10);
				scoresController.loadRangeAtRank(1);
				scoresController = new ScoresController(
						Session.getCurrentSession(), observer);
				scoresController.setMode(scoreMode);
				scoresController.setRangeLength(1);
				scoresController.loadRangeForUser(Session.getCurrentSession()
						.getUser());
			}
		}
		;
		ScoreRequester requester = new ScoreRequester();
		requester.scoreMode = scoreMode;
		mSingleton.runOnUiThread(requester);
	}

	// this is a really ugly method, but I need to chain several callbacks
	// together and I don't see a way to do that without duplicating code
	public static void promptForAlias(int scoreMode, int scoreValue) {
		class AliasPromptAndSubmit implements Runnable {
			int scoreMode;
			int scoreValue;
			String title;
			String message;

			public AliasPromptAndSubmit(int scoreMode, int scoreValue,
					String title, String message) {
				this.scoreMode = scoreMode;
				this.scoreValue = scoreValue;
				this.title = title;
				this.message = message;
			}

			@Override
			public void run() {
				AlertDialog.Builder alert = new AlertDialog.Builder(mSingleton);
				alert.setTitle("Submit Score?");
				alert.setMessage("Would you like to submit your score? You will need to set your alias first, which will publicly appear next to your scores. You can do this at any time in the Settings menu.");
				alert.setPositiveButton("OK",
						new DialogInterface.OnClickListener() {
							private void setAlias() {
								AlertDialog.Builder alert = new AlertDialog.Builder(
										mSingleton);
								alert.setTitle(title);
								alert.setMessage(message);
								final EditText input = new EditText(mSingleton);
								alert.setView(input);
								alert.setPositiveButton("OK",
										new DialogInterface.OnClickListener() {
											public void onClick(
													DialogInterface dialog,
													int whichButton) {
												UserController controller = new UserController(
														new RequestControllerObserver() {
															@Override
															public void requestControllerDidFail(
																	RequestController arg0,
																	Exception arg1) {
																AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
																		mSingleton);
																alertDialogBuilder
																		.setTitle(
																				"Alias in use")
																		.setMessage(
																				"If you are using this alias on another device you will need to set your email address in the Settings menu. Otherwise, you can try again with a different alias.")
																		.setPositiveButton(
																				"Try Again",
																				new DialogInterface.OnClickListener() {
																					public void onClick(
																							DialogInterface dialog,
																							int id) {
																						setAlias(); // do
																									// this
																									// recursively
																					}
																				})
																		.setNegativeButton(
																				"Cancel",
																				new DialogInterface.OnClickListener() {
																					public void onClick(
																							DialogInterface dialog,
																							int id) {
																					}
																				})
																		.create()
																		.show();
															}

															@Override
															public void requestControllerDidReceiveResponse(
																	RequestController arg0) {
																AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
																		mSingleton);
																alertDialogBuilder
																		.setTitle(
																				"Success!")
																		.setMessage(
																				"Submitting your score.")
																		.setPositiveButton(
																				"OK",
																				new DialogInterface.OnClickListener() {
																					public void onClick(
																							DialogInterface dialog,
																							int id) {
																					}
																				})
																		.create()
																		.show();
																mUserCanSubmitScores = true;
																updateUserInfo();
																submitScore(
																		scoreMode,
																		scoreValue);
															}
														});
												controller.setUser(Session
														.getCurrentSession()
														.getUser());
												controller.getUser().setLogin(
														input.getText()
																.toString());
												controller.submitUser();
											}
										});

								alert.setNegativeButton("Cancel",
										new DialogInterface.OnClickListener() {
											public void onClick(
													DialogInterface dialog,
													int whichButton) {
											}
										});
								alert.show();
							}

							public void onClick(DialogInterface dialog,
									int whichButton) {
								setAlias();
							}
						});

				alert.setNegativeButton("Cancel",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
							}
						});

				alert.setNeutralButton("Never Prompt",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
								nativeDisableAliasPrompt();
							}
						}).create().show();
			}
		}
		AliasPromptAndSubmit submitter = new AliasPromptAndSubmit(
				scoreMode,
				scoreValue,
				"Enter Alias",
				"Enter the alias you want to appear next to your scores. Note that you will not be able to reuse this on other devices unless you also enter an email in the Settings menu.");
		mSingleton.runOnUiThread(submitter);
	}

	public static boolean submitScore(int scoreMode, int scoreValue) {
		class ScoreSubmitter implements Runnable {
			int mode;
			double scoreValue;

			@Override
			public void run() {
				final Score score = new Score(scoreValue, null);
				score.setMode(mode);
				final ScoreController scoreController = new ScoreController(
						new RequestControllerObserver() {
							@Override
							public void requestControllerDidFail(
									RequestController arg0, Exception arg1) {
								Log.d("scores", "score submission failed");
							}

							@Override
							public void requestControllerDidReceiveResponse(
									RequestController arg0) {
								Log.d("scores", "score submission succeeded");
							}
						});
				scoreController.submitScore(score);
			}
		}
		ScoreSubmitter submitter = new ScoreSubmitter();
		submitter.mode = scoreMode;
		submitter.scoreValue = scoreValue;
		if (mUserCanSubmitScores) {
			mSingleton.runOnUiThread(submitter);
			return true;
		} else {
			return false;
		}
	}
}
