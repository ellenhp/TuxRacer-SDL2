package com.moonlite.tuxracer;

import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.text.ParseException;
import java.util.Arrays;

import android.app.*;
import android.content.*;
import android.view.*;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.AbsoluteLayout;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.Toast;
import android.net.Uri;
import android.os.*;
import android.util.Base64;
import android.util.Log;
import android.graphics.*;
import android.media.*;
import android.hardware.*;

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

import com.amazon.inapp.purchasing.Item;
import com.amazon.inapp.purchasing.PurchasingManager;
import com.moonlite.tuxracer.AppPurchasingObserver.PurchaseData;
import com.moonlite.tuxracer.AppPurchasingObserver.PurchaseDataStorage;
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

/**
 * SDL Activity
 */
public class SDLActivity extends Activity implements
		AppPurchasingObserverListener {
	private static final String TAG = "Tuxracer";

	// Keep track of the paused state
	public static boolean mIsPaused = false, mIsSurfaceReady = false,
			mHasFocus = true;
	public static boolean mExitCalledFromJava;

	public static float scaleFactor = 1.0f;

	// Main components
	protected static SDLActivity mSingleton;
	protected static SDLSurface mSurface;
	protected static View mTextEdit;
	protected static ViewGroup mLayout;
	protected static SDLJoystickHandler mJoystickHandler;

	// This is what SDL runs in. It invokes SDL_main(), eventually
	protected static Thread mSDLThread;

	// Audio
	protected static Thread mAudioThread;
	protected static AudioTrack mAudioTrack;

	private static SDLActivity myActivity;

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
								SDLActivity.this,
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
						new AlertDialog.Builder(SDLActivity.this)
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
								.handleError(SDLActivity.this, errorCode,
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
		purchaseRequest.put("testing", "true"); // This value is only needed for
												// testing, not setting it
												// results in a live purchase
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
			FileOutputStream fos;
			try {
				fos = mSingleton.openFileOutput("receipts", Context.MODE_PRIVATE);
				OutputStreamWriter osw = new OutputStreamWriter(fos);
				osw.append(receiptResponse);
				osw.close();
				fos.close();
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
						
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
			OuyaEncryptionHelper helper = new OuyaEncryptionHelper();
			List<Receipt> receipts;
			try {
				FileInputStream fis = mSingleton.openFileInput("receipts");
				InputStreamReader isr = new InputStreamReader(fis);
				BufferedReader bufferedReader = new BufferedReader(isr);
				StringBuilder sb = new StringBuilder();
				String line;
				while ((line = bufferedReader.readLine()) != null) {
					sb.append(line);
				}
				
				JSONObject response = new JSONObject(sb.toString());
				receipts = helper.decryptReceiptResponse(response, mPublicKey);
			} catch (ParseException e) {
				throw new RuntimeException(e);
			} catch (JSONException e) {
				throw new RuntimeException(e);
			} catch (GeneralSecurityException e) {
				throw new RuntimeException(e);
			} catch (IOException e) {
				//no big deal, the file probably just doesn't exist
				nativeCoursePrice(2.99f);
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

	static class ShowTextInputTask implements Runnable {
		/*
		 * This is used to regulate the pan&scan method to have some offset from
		 * the bottom edge of the input region and the top edge of an input
		 * method (soft keyboard)
		 */
		static final int HEIGHT_PADDING = 15;

		public int x, y, w, h;

		public ShowTextInputTask(int x, int y, int w, int h) {
			this.x = x;
			this.y = y;
			this.w = w;
			this.h = h;
		}

		@Override
		public void run() {
			AbsoluteLayout.LayoutParams params = new AbsoluteLayout.LayoutParams(
					w, h + HEIGHT_PADDING, x, y);

			if (mTextEdit == null) {
				mTextEdit = new DummyEdit(getContext());

				mLayout.addView(mTextEdit, params);
			} else {
				mTextEdit.setLayoutParams(params);
			}

			mTextEdit.setVisibility(View.VISIBLE);
			mTextEdit.requestFocus();

			InputMethodManager imm = (InputMethodManager) getContext()
					.getSystemService(Context.INPUT_METHOD_SERVICE);
			imm.showSoftInput(mTextEdit, 0);
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

		// So we can call stuff from static callbacks
		mSingleton = this;

		// Set up the surface
		mSurface = new SDLSurface(getApplication());

	    // Make sure this variable is initialized here!
        mExitCalledFromJava = false;
        
        if(Build.VERSION.SDK_INT >= 12) {
            mJoystickHandler = new SDLJoystickHandler_API12();
        }
        else {
            mJoystickHandler = new SDLJoystickHandler();
        }
		
		Point size = new Point();
		getWindowManager().getDefaultDisplay().getSize(size);

		size.x *= scaleFactor;
		size.y *= scaleFactor;

		mSurface.getHolder().setFixedSize(size.x, size.y);

		mLayout = new FrameLayout(this);
		mLayout.addView(mSurface);

		setContentView(mLayout);

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		myActivity = this;

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

	// Events
	@Override
	protected void onPause() {
		Log.v("SDL", "onPause()");
		super.onPause();
		SDLActivity.handlePause();
	}

	@Override
	protected void onResume() {
		Log.v("SDL", "onResume()");
		super.onResume();
		SDLActivity.handleResume();
		if (!onOUYA)
			AmazonRequestIAP();
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		Log.v("SDL", "onWindowFocusChanged(): " + hasFocus);

		SDLActivity.mHasFocus = hasFocus;
		if (hasFocus) {
			SDLActivity.handleResume();
		}
	}

	@Override
	public void onLowMemory() {
		Log.v("SDL", "onLowMemory()");
		super.onLowMemory();
		SDLActivity.nativeLowMemory();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		Log.v("SDL", "onDestroy()");
		// Send a quit message to the application
		SDLActivity.nativeQuit();

		// Now wait for the SDL thread to quit
		if (mSDLThread != null) {
			try {
				mSDLThread.join();
			} catch (Exception e) {
				Log.v("SDL", "Problem stopping thread: " + e);
			}
			mSDLThread = null;

			// Log.v("SDL", "Finished waiting for SDL thread");
		}
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		int keyCode = event.getKeyCode();
		// Ignore certain special keys so they're handled by Android
		if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN
				|| keyCode == KeyEvent.KEYCODE_VOLUME_UP
				|| keyCode == KeyEvent.KEYCODE_CAMERA
				|| keyCode == 168 // API 11: KeyEvent.KEYCODE_ZOOM_IN
				|| keyCode == 169 // API 11: KeyEvent.KEYCODE_ZOOM_OUT
		) {
			return false;
		}
		return super.dispatchKeyEvent(event);
	}

	/**
	 * Called by onPause or surfaceDestroyed. Even if surfaceDestroyed is the
	 * first to be called, mIsSurfaceReady should still be set to 'true' during
	 * the call to onPause (in a usual scenario).
	 */
	public static void handlePause() {
		if (!SDLActivity.mIsPaused && SDLActivity.mIsSurfaceReady) {
			SDLActivity.mIsPaused = true;
			SDLActivity.nativePause();
			mSurface.enableSensor(Sensor.TYPE_ACCELEROMETER, false);
		}
	}

	/**
	 * Called by onResume or surfaceCreated. An actual resume should be done
	 * only when the surface is ready. Note: Some Android variants may send
	 * multiple surfaceChanged events, so we don't need to resume every time we
	 * get one of those events, only if it comes after surfaceDestroyed
	 */
	public static void handleResume() {
		if (SDLActivity.mIsPaused && SDLActivity.mIsSurfaceReady
				&& SDLActivity.mHasFocus) {
			SDLActivity.mIsPaused = false;
			SDLActivity.nativeResume();
			mSurface.enableSensor(Sensor.TYPE_ACCELEROMETER, true);
		}
	}

	// Messages from the SDLMain thread
	static final int COMMAND_CHANGE_TITLE = 1;
	static final int COMMAND_UNUSED = 2;
	static final int COMMAND_TEXTEDIT_HIDE = 3;

	protected static final int COMMAND_USER = 0x8000;

	/**
	 * This method is called by SDL if SDL did not handle a message itself. This
	 * happens if a received message contains an unsupported command. Method can
	 * be overwritten to handle Messages in a different class.
	 * 
	 * @param command
	 *            the command of the message.
	 * @param param
	 *            the parameter of the message. May be null.
	 * @return if the message was handled in overridden method.
	 */
	protected boolean onUnhandledMessage(int command, Object param) {
		return false;
	}

	/**
	 * A Handler class for Messages from native SDL applications. It uses
	 * current Activities as target (e.g. for the title). static to prevent
	 * implicit references to enclosing object.
	 */
	protected static class SDLCommandHandler extends Handler {
		@Override
		public void handleMessage(Message msg) {
			Context context = getContext();
			if (context == null) {
				Log.e(TAG, "error handling message, getContext() returned null");
				return;
			}
			switch (msg.arg1) {
			case COMMAND_CHANGE_TITLE:
				if (context instanceof Activity) {
					((Activity) context).setTitle((String) msg.obj);
				} else {
					Log.e(TAG,
							"error handling message, getContext() returned no Activity");
				}
				break;
			case COMMAND_TEXTEDIT_HIDE:
				if (mTextEdit != null) {
					mTextEdit.setVisibility(View.GONE);

					InputMethodManager imm = (InputMethodManager) context
							.getSystemService(Context.INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(mTextEdit.getWindowToken(), 0);
				}
				break;
			default:
				if ((context instanceof SDLActivity)
						&& !((SDLActivity) context).onUnhandledMessage(
								msg.arg1, msg.obj)) {
					Log.e(TAG, "error handling message, command is " + msg.arg1);
				}
			}
		}
	}

	// Handler for the messages
	Handler commandHandler = new SDLCommandHandler();

	// Send a message from the SDLMain thread
	boolean sendCommand(int command, Object data) {
		Message msg = commandHandler.obtainMessage();
		msg.arg1 = command;
		msg.obj = data;
		return commandHandler.sendMessage(msg);
	}

	// C functions we call
	public static native void nativeInit();
	public static native void nativeLowMemory();
	public static native void nativeQuit();
	public static native void nativePause();
	public static native void nativeResume();
	public static native void onNativeResize(int x, int y, int format);
	public static native int onNativePadDown(int device_id, int keycode);
	public static native int onNativePadUp(int device_id, int keycode);
	public static native void onNativeJoy(int device_id, int axis, float value);
	public static native void onNativeKeyDown(int keycode);
	public static native void onNativeKeyUp(int keycode);
	public static native void onNativeKeyboardFocusLost();
	public static native void onNativeTouch(int touchDevId,
			int pointerFingerId, int action, float x, float y, float p);
	public static native void onNativeAccel(float x, float y, float z);
	public static native void onNativeSurfaceChanged();
	public static native void onNativeSurfaceDestroyed();
	public static native void nativeFlipBuffers();
	public static native int nativeAddJoystick(int device_id, String name,
			int is_accelerometer, int nbuttons, int naxes, int nhats, int nballs);
	public static native int nativeRemoveJoystick(int device_id);
	public static native void nativeSetPlayerData(String playerName,
			boolean isOnOuya);
	public static native void nativeScoreloopGotScores(int scoreMode,
			Object[] scoreStrings);
	public static native void nativeTextCallback(String string);
	public static native void nativeDisableAliasPrompt();
	public static native void nativeUpdateUserInfo(String alias);
	public static native void nativeCoursePrice(float price);
	
	public static void flipBuffers() {
		SDLActivity.nativeFlipBuffers();
	}

	public static boolean setActivityTitle(String title) {
		// Called from SDLMain() thread and can't directly affect the view
		return mSingleton.sendCommand(COMMAND_CHANGE_TITLE, title);
	}

	public static boolean sendMessage(int command, int param) {
		return mSingleton.sendCommand(command, Integer.valueOf(param));
	}

	public static Context getContext() {
		return mSingleton;
	}

	/**
	 * @return result of getSystemService(name) but executed on UI thread.
	 */
	public Object getSystemServiceFromUiThread(final String name) {
		final Object lock = new Object();
		final Object[] results = new Object[2]; // array for writable variables
		synchronized (lock) {
			runOnUiThread(new Runnable() {
				@Override
				public void run() {
					synchronized (lock) {
						results[0] = getSystemService(name);
						results[1] = Boolean.TRUE;
						lock.notify();
					}
				}
			});
			if (results[1] == null) {
				try {
					lock.wait();
				} catch (InterruptedException ex) {
					ex.printStackTrace();
				}
			}
		}
		return results[0];
	}

	public static boolean showTextInput(int x, int y, int w, int h) {
		// Transfer the task to the main thread as a Runnable
		return mSingleton.commandHandler
				.post(new ShowTextInputTask(x, y, w, h));
	}

	public static Surface getNativeSurface() {
		return SDLActivity.mSurface.getNativeSurface();
	}

	// Audio
	public static int audioInit(int sampleRate, boolean is16Bit,
			boolean isStereo, int desiredFrames) {
		int channelConfig = isStereo ? AudioFormat.CHANNEL_CONFIGURATION_STEREO
				: AudioFormat.CHANNEL_CONFIGURATION_MONO;
		int audioFormat = is16Bit ? AudioFormat.ENCODING_PCM_16BIT
				: AudioFormat.ENCODING_PCM_8BIT;
		int frameSize = (isStereo ? 2 : 1) * (is16Bit ? 2 : 1);

		Log.v("SDL", "SDL audio: wanted " + (isStereo ? "stereo" : "mono")
				+ " " + (is16Bit ? "16-bit" : "8-bit") + " "
				+ (sampleRate / 1000f) + "kHz, " + desiredFrames
				+ " frames buffer");

		// Let the user pick a larger buffer if they really want -- but ye
		// gods they probably shouldn't, the minimums are horrifyingly high
		// latency already
		desiredFrames = Math.max(
				desiredFrames,
				(AudioTrack.getMinBufferSize(sampleRate, channelConfig,
						audioFormat) + frameSize - 1)
						/ frameSize);

		if (mAudioTrack == null) {
			mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
					channelConfig, audioFormat, desiredFrames * frameSize,
					AudioTrack.MODE_STREAM);

			// Instantiating AudioTrack can "succeed" without an exception and
			// the track may still be invalid
			// Ref:
			// https://android.googlesource.com/platform/frameworks/base/+/refs/heads/master/media/java/android/media/AudioTrack.java
			// Ref:
			// http://developer.android.com/reference/android/media/AudioTrack.html#getState()

			if (mAudioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
				Log.e("SDL", "Failed during initialization of Audio Track");
				mAudioTrack = null;
				return -1;
			}

			mAudioTrack.play();
		}

		Log.v("SDL",
				"SDL audio: got "
						+ ((mAudioTrack.getChannelCount() >= 2) ? "stereo"
								: "mono")
						+ " "
						+ ((mAudioTrack.getAudioFormat() == AudioFormat.ENCODING_PCM_16BIT) ? "16-bit"
								: "8-bit") + " "
						+ (mAudioTrack.getSampleRate() / 1000f) + "kHz, "
						+ desiredFrames + " frames buffer");

		return 0;
	}

	public static void audioWriteShortBuffer(short[] buffer) {
		for (int i = 0; i < buffer.length;) {
			int result = mAudioTrack.write(buffer, i, buffer.length - i);
			if (result > 0) {
				i += result;
			} else if (result == 0) {
				try {
					Thread.sleep(1);
				} catch (InterruptedException e) {
					// Nom nom
				}
			} else {
				Log.w("SDL", "SDL audio: error return from write(short)");
				return;
			}
		}
	}

	public static void audioWriteByteBuffer(byte[] buffer) {
		for (int i = 0; i < buffer.length;) {
			int result = mAudioTrack.write(buffer, i, buffer.length - i);
			if (result > 0) {
				i += result;
			} else if (result == 0) {
				try {
					Thread.sleep(1);
				} catch (InterruptedException e) {
					// Nom nom
				}
			} else {
				Log.w("SDL", "SDL audio: error return from write(byte)");
				return;
			}
		}
	}

	public static void audioQuit() {
		if (mAudioTrack != null) {
			mAudioTrack.stop();
			mAudioTrack = null;
		}
	}

	// Input

	/**
	 * @return an array which may be empty but is never null.
	 */
	public static int[] inputGetInputDeviceIds(int sources) {
		int[] ids = InputDevice.getDeviceIds();
		int[] filtered = new int[ids.length];
		int used = 0;
		for (int i = 0; i < ids.length; ++i) {
			InputDevice device = InputDevice.getDevice(ids[i]);
			if ((device != null) && ((device.getSources() & sources) != 0)) {
				filtered[used++] = device.getId();
			}
		}
		return Arrays.copyOf(filtered, used);
	}

	// Joystick glue code, just a series of stubs that redirect to the
	// SDLJoystickHandler instance
	public static boolean handleJoystickMotionEvent(MotionEvent event) {
		return mJoystickHandler.handleMotionEvent(event);
	}

	public static void pollInputDevices() {
		if (SDLActivity.mSDLThread != null) {
			mJoystickHandler.pollInputDevices();
		}
	}

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
					input.setText(SDLActivity.mScoreloopUsername);
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

/**
 * Simple nativeInit() runnable
 */
class SDLMain implements Runnable {
	@Override
	public void run() {
		// Runs SDL_main()
		SDLActivity.nativeInit();

		// Log.v("SDL", "SDL thread terminated");
	}
}

/**
 * SDLSurface. This is what we draw on, so we need to know when it's created in
 * order to do anything useful.
 * 
 * Because of this, that's where we set up the SDL thread
 */
class SDLSurface extends SurfaceView implements SurfaceHolder.Callback,
		View.OnKeyListener, View.OnTouchListener, SensorEventListener {

	// Sensors
	protected static SensorManager mSensorManager;
	protected static Display mDisplay;

	// Keep track of the surface size to normalize touch events
	protected static float mWidth, mHeight;

	// Startup
	public SDLSurface(Context context) {
		super(context);
		getHolder().addCallback(this);

		setFocusable(true);
		setFocusableInTouchMode(true);
		requestFocus();
		setOnKeyListener(this);
		setOnTouchListener(this);

		mDisplay = ((WindowManager) context
				.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		mSensorManager = (SensorManager) context
				.getSystemService(Context.SENSOR_SERVICE);
        
        if(Build.VERSION.SDK_INT >= 12) {
            setOnGenericMotionListener(new SDLGenericMotionListener_API12());
        }

		// Some arbitrary defaults to avoid a potential division by zero
		mWidth = 1.0f;
		mHeight = 1.0f;
	}

	public Surface getNativeSurface() {
		return getHolder().getSurface();
	}

	// Called when we have a valid drawing surface
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.v("SDL", "surfaceCreated()");
		holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
	}

	// Called when we lose the surface
	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.v("SDL", "surfaceDestroyed()");
		// Call this *before* setting mIsSurfaceReady to 'false'
		SDLActivity.handlePause();
		SDLActivity.mIsSurfaceReady = false;
		SDLActivity.onNativeSurfaceDestroyed();
	}

	// Called when the surface is resized
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.v("SDL", "surfaceChanged()");

		int sdlFormat = 0x15151002; // SDL_PIXELFORMAT_RGB565 by default
		switch (format) {
		case PixelFormat.A_8:
			Log.v("SDL", "pixel format A_8");
			break;
		case PixelFormat.LA_88:
			Log.v("SDL", "pixel format LA_88");
			break;
		case PixelFormat.L_8:
			Log.v("SDL", "pixel format L_8");
			break;
		case PixelFormat.RGBA_4444:
			Log.v("SDL", "pixel format RGBA_4444");
			sdlFormat = 0x15421002; // SDL_PIXELFORMAT_RGBA4444
			break;
		case PixelFormat.RGBA_5551:
			Log.v("SDL", "pixel format RGBA_5551");
			sdlFormat = 0x15441002; // SDL_PIXELFORMAT_RGBA5551
			break;
		case PixelFormat.RGBA_8888:
			Log.v("SDL", "pixel format RGBA_8888");
			sdlFormat = 0x16462004; // SDL_PIXELFORMAT_RGBA8888
			break;
		case PixelFormat.RGBX_8888:
			Log.v("SDL", "pixel format RGBX_8888");
			sdlFormat = 0x16261804; // SDL_PIXELFORMAT_RGBX8888
			break;
		case PixelFormat.RGB_332:
			Log.v("SDL", "pixel format RGB_332");
			sdlFormat = 0x14110801; // SDL_PIXELFORMAT_RGB332
			break;
		case PixelFormat.RGB_565:
			Log.v("SDL", "pixel format RGB_565");
			sdlFormat = 0x15151002; // SDL_PIXELFORMAT_RGB565
			break;
		case PixelFormat.RGB_888:
			Log.v("SDL", "pixel format RGB_888");
			// Not sure this is right, maybe SDL_PIXELFORMAT_RGB24 instead?
			sdlFormat = 0x16161804; // SDL_PIXELFORMAT_RGB888
			break;
		default:
			Log.v("SDL", "pixel format unknown " + format);
			break;
		}

		mWidth = width;
		mHeight = height;
		SDLActivity.onNativeResize(width, height, sdlFormat);
		Log.v("SDL", "Window size:" + width + "x" + height);

		// Set mIsSurfaceReady to 'true' *before* making a call to handleResume
		SDLActivity.mIsSurfaceReady = true;
		SDLActivity.onNativeSurfaceChanged();

		if (SDLActivity.mSDLThread == null) {
			// This is the entry point to the C app.
			// Start up the C app thread and enable sensor input for the first
			// time

			SDLActivity.mSDLThread = new Thread(new SDLMain(), "SDLThread");
			enableSensor(Sensor.TYPE_ACCELEROMETER, true);
			SDLActivity.mSDLThread.start();
		}
	}

	// unused
	@Override
	public void onDraw(Canvas canvas) {
	}

    // Key events
    @Override
    public boolean onKey(View  v, int keyCode, KeyEvent event) {
        // Dispatch the different events depending on where they come from
        // Some SOURCE_DPAD or SOURCE_GAMEPAD are also SOURCE_KEYBOARD
        // So, we try to process them as DPAD or GAMEPAD events first, if that fails we try them as KEYBOARD
        
        if ( (event.getSource() & 0x00000401) != 0 || /* API 12: SOURCE_GAMEPAD */
                   (event.getSource() & InputDevice.SOURCE_DPAD) != 0 ) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                if (SDLActivity.onNativePadDown(event.getDeviceId(), keyCode) == 0) {
                    return true;
                }
            } else if (event.getAction() == KeyEvent.ACTION_UP) {
                if (SDLActivity.onNativePadUp(event.getDeviceId(), keyCode) == 0) {
                    return true;
                }
            }
        }
        
        if( (event.getSource() & InputDevice.SOURCE_KEYBOARD) != 0) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                //Log.v("SDL", "key down: " + keyCode);
                SDLActivity.onNativeKeyDown(keyCode);
                return true;
            }
            else if (event.getAction() == KeyEvent.ACTION_UP) {
                //Log.v("SDL", "key up: " + keyCode);
                SDLActivity.onNativeKeyUp(keyCode);
                return true;
            }
        }
        
        return false;
    }

	// Touch events
	@Override
	public boolean onTouch(View v, MotionEvent event) {
		float screenWidth = mWidth / SDLActivity.scaleFactor;
		float screenHeight = mHeight / SDLActivity.scaleFactor;

		final int touchDevId = event.getDeviceId();
		final int pointerCount = event.getPointerCount();
		// touchId, pointerId, action, x, y, pressure
		int actionPointerIndex = (event.getAction() & MotionEvent.ACTION_POINTER_ID_MASK) >> MotionEvent.ACTION_POINTER_ID_SHIFT;
		/*API 8: event.getActionIndex();*/
		int pointerFingerId = event.getPointerId(actionPointerIndex);
		int action = (event.getAction() & MotionEvent.ACTION_MASK);
		/*API 8: event.getActionMasked();*/
		float x = event.getX(actionPointerIndex) / screenWidth;
		float y = event.getY(actionPointerIndex) / screenHeight;
		float p = event.getPressure(actionPointerIndex);

		if (action == MotionEvent.ACTION_MOVE && pointerCount > 1) {
			// TODO send motion to every pointer if its position has
			// changed since prev event.
			for (int i = 0; i < pointerCount; i++) {
				pointerFingerId = event.getPointerId(i);
				x = event.getX(i) / screenWidth;
				y = event.getY(i) / screenHeight;
				p = event.getPressure(i);
				SDLActivity.onNativeTouch(touchDevId, pointerFingerId, action,
						x, y, p);
			}
		} else {
			SDLActivity.onNativeTouch(touchDevId, pointerFingerId, action, x,
					y, p);
		}
		return true;
	}

	// Sensor events
	public void enableSensor(int sensortype, boolean enabled) {
		// TODO: This uses getDefaultSensor - what if we have >1 accels?
		if (enabled) {
			mSensorManager.registerListener(this,
					mSensorManager.getDefaultSensor(sensortype),
					SensorManager.SENSOR_DELAY_GAME, null);
		} else {
			mSensorManager.unregisterListener(this,
					mSensorManager.getDefaultSensor(sensortype));
		}
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            float[] adjustedValues = new float[3];

            final int axisSwap[][] = {
            {  1,  -1,  0,  1  },     // ROTATION_0 
            {-1,  -1,  1,  0  },     // ROTATION_90 
            {-1,    1,  0,  1  },     // ROTATION_180 
            {  1,    1,  1,  0  }  }; // ROTATION_270 

            final int[] as = axisSwap[mDisplay.getRotation()];
            adjustedValues[0]  =  (float)as[0] * event.values[ as[2] ]; 
            adjustedValues[1]  =  (float)as[1] * event.values[ as[3] ]; 
            adjustedValues[2]  =  event.values[2];
            SDLActivity.onNativeAccel(adjustedValues[0] / SensorManager.GRAVITY_EARTH,
            		adjustedValues[1] / SensorManager.GRAVITY_EARTH,
            		adjustedValues[2] / SensorManager.GRAVITY_EARTH);
        }
	}

}

/*
 * This is a fake invisible editor view that receives the input and defines the
 * pan&scan region
 */
class DummyEdit extends View implements View.OnKeyListener {
	InputConnection ic;

	public DummyEdit(Context context) {
		super(context);
		setFocusableInTouchMode(true);
		setFocusable(true);
		setOnKeyListener(this);
	}

	@Override
	public boolean onCheckIsTextEditor() {
		return true;
	}

	@Override
	public boolean onKey(View v, int keyCode, KeyEvent event) {

		// This handles the hardware keyboard input
		if (event.isPrintingKey()) {
			if (event.getAction() == KeyEvent.ACTION_DOWN) {
				ic.commitText(String.valueOf((char) event.getUnicodeChar()), 1);
			}
			return true;
		}

		if (event.getAction() == KeyEvent.ACTION_DOWN) {
			SDLActivity.onNativeKeyDown(keyCode);
			return true;
		} else if (event.getAction() == KeyEvent.ACTION_UP) {
			SDLActivity.onNativeKeyUp(keyCode);
			return true;
		}

		return false;
	}

	//
	@Override
	public boolean onKeyPreIme(int keyCode, KeyEvent event) {
		// As seen on StackOverflow:
		// http://stackoverflow.com/questions/7634346/keyboard-hide-event
		// FIXME: Discussion at http://bugzilla.libsdl.org/show_bug.cgi?id=1639
		// FIXME: This is not a 100% effective solution to the problem of
		// detecting if the keyboard is showing or not
		// FIXME: A more effective solution would be to change our Layout from
		// AbsoluteLayout to Relative or Linear
		// FIXME: And determine the keyboard presence doing this:
		// http://stackoverflow.com/questions/2150078/how-to-check-visibility-of-software-keyboard-in-android
		// FIXME: An even more effective way would be if Android provided this
		// out of the box, but where would the fun be in that :)
		if (event.getAction() == KeyEvent.ACTION_UP
				&& keyCode == KeyEvent.KEYCODE_BACK) {
			if (SDLActivity.mTextEdit != null
					&& SDLActivity.mTextEdit.getVisibility() == View.VISIBLE) {
				SDLActivity.onNativeKeyboardFocusLost();
			}
		}
		return super.onKeyPreIme(keyCode, event);
	}

	@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
		ic = new SDLInputConnection(this, true);

		outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI | 33554432;  //API 11: EditorInfo.IME_FLAG_NO_FULLSCREEN

		return ic;
	}
}

class SDLInputConnection extends BaseInputConnection {

	public SDLInputConnection(View targetView, boolean fullEditor) {
		super(targetView, fullEditor);

	}

	@Override
	public boolean sendKeyEvent(KeyEvent event) {

		/*
		 * This handles the keycodes from soft keyboard (and IME-translated
		 * input from hardkeyboard)
		 */
		int keyCode = event.getKeyCode();
		if (event.getAction() == KeyEvent.ACTION_DOWN) {
			if (event.isPrintingKey()) {
				commitText(String.valueOf((char) event.getUnicodeChar()), 1);
			}
			SDLActivity.onNativeKeyDown(keyCode);
			return true;
		} else if (event.getAction() == KeyEvent.ACTION_UP) {

			SDLActivity.onNativeKeyUp(keyCode);
			return true;
		}
		return super.sendKeyEvent(event);
	}

	@Override
	public boolean commitText(CharSequence text, int newCursorPosition) {

		nativeCommitText(text.toString(), newCursorPosition);

		return super.commitText(text, newCursorPosition);
	}

	@Override
	public boolean setComposingText(CharSequence text, int newCursorPosition) {

		nativeSetComposingText(text.toString(), newCursorPosition);

		return super.setComposingText(text, newCursorPosition);
	}

	public native void nativeCommitText(String text, int newCursorPosition);

	public native void nativeSetComposingText(String text, int newCursorPosition);

	@Override
	public boolean deleteSurroundingText(int beforeLength, int afterLength) {
		// Workaround to capture backspace key. Ref:
		// http://stackoverflow.com/questions/14560344/android-backspace-in-webview-baseinputconnection
		if (beforeLength == 1 && afterLength == 0) {
			// backspace
			return super.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN,
					KeyEvent.KEYCODE_DEL))
					&& super.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_UP,
							KeyEvent.KEYCODE_DEL));
		}

		return super.deleteSurroundingText(beforeLength, afterLength);
	}
}

/*
 * A null joystick handler for API level < 12 devices (the accelerometer is
 * handled separately)
 */
class SDLJoystickHandler {

	public boolean handleMotionEvent(MotionEvent event) {
		return false;
	}

	public void pollInputDevices() {
	}
}

/* Actual joystick functionality available for API >= 12 devices */
class SDLJoystickHandler_API12 extends SDLJoystickHandler {

	class SDLJoystick {
		public int device_id;
		public String name;
		public ArrayList<InputDevice.MotionRange> axes;
	}

	private ArrayList<SDLJoystick> mJoysticks;

	public SDLJoystickHandler_API12() {

		mJoysticks = new ArrayList<SDLJoystick>();
	}

	@Override
	public void pollInputDevices() {
		int[] deviceIds = InputDevice.getDeviceIds();
		// It helps processing the device ids in reverse order
		// For example, in the case of the XBox 360 wireless dongle,
		// so the first controller seen by SDL matches what the receiver
		// considers to be the first controller

		for (int i = deviceIds.length - 1; i > -1; i--) {
			SDLJoystick joystick = getJoystick(deviceIds[i]);
			if (joystick == null) {
				joystick = new SDLJoystick();
				InputDevice joystickDevice = InputDevice
						.getDevice(deviceIds[i]);
				if ((joystickDevice.getSources() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
					joystick.device_id = deviceIds[i];
					joystick.name = joystickDevice.getName();
					joystick.axes = new ArrayList<InputDevice.MotionRange>();

					for (InputDevice.MotionRange range : joystickDevice
							.getMotionRanges()) {
						if ((range.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
							joystick.axes.add(range);
						}
					}

					mJoysticks.add(joystick);
					SDLActivity.nativeAddJoystick(joystick.device_id,
							joystick.name, 0, -1, joystick.axes.size(), 0, 0);
				}
			}
		}

		/* Check removed devices */
		ArrayList<Integer> removedDevices = new ArrayList<Integer>();
		for (int i = 0; i < mJoysticks.size(); i++) {
			int device_id = mJoysticks.get(i).device_id;
			int j;
			for (j = 0; j < deviceIds.length; j++) {
				if (device_id == deviceIds[j])
					break;
			}
			if (j == deviceIds.length) {
				removedDevices.add(device_id);
			}
		}

		for (int i = 0; i < removedDevices.size(); i++) {
			int device_id = removedDevices.get(i);
			SDLActivity.nativeRemoveJoystick(device_id);
			for (int j = 0; j < mJoysticks.size(); j++) {
				if (mJoysticks.get(j).device_id == device_id) {
					mJoysticks.remove(j);
					break;
				}
			}
		}
	}

	protected SDLJoystick getJoystick(int device_id) {
		for (int i = 0; i < mJoysticks.size(); i++) {
			if (mJoysticks.get(i).device_id == device_id) {
				return mJoysticks.get(i);
			}
		}
		return null;
	}

	@Override
    public boolean handleMotionEvent(MotionEvent event) {
        if ( (event.getSource() & InputDevice.SOURCE_JOYSTICK) != 0) {
            int actionPointerIndex = event.getActionIndex();
            int action = event.getActionMasked();
            switch(action) {
                case MotionEvent.ACTION_MOVE:
                    SDLJoystick joystick = getJoystick(event.getDeviceId());
                    if ( joystick != null ) {
                        for (int i = 0; i < joystick.axes.size(); i++) {
                            InputDevice.MotionRange range = joystick.axes.get(i);
                            /* Normalize the value to -1...1 */
                            float value = ( event.getAxisValue( range.getAxis(), actionPointerIndex) - range.getMin() ) / range.getRange() * 2.0f - 1.0f;
                            SDLActivity.onNativeJoy(joystick.device_id, i, value );
                        }                       
                    }
                    break;
                default:
                    break;
            }
        }
        return true;
    }            
}

class SDLGenericMotionListener_API12 implements View.OnGenericMotionListener {
	// Generic Motion (mouse hover, joystick...) events go here
	// We only have joysticks yet
	@Override
	public boolean onGenericMotion(View v, MotionEvent event) {
		return SDLActivity.handleJoystickMotionEvent(event);
	}
}
