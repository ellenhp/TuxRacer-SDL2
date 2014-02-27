package com.moonlite.tuxracer;

import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import com.amazon.inapp.purchasing.GetUserIdResponse;
import com.amazon.inapp.purchasing.GetUserIdResponse.GetUserIdRequestStatus;
import com.amazon.inapp.purchasing.Item;
import com.amazon.inapp.purchasing.ItemDataResponse.ItemDataRequestStatus;
import com.amazon.inapp.purchasing.PurchaseResponse.PurchaseRequestStatus;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse.PurchaseUpdatesRequestStatus;
import com.amazon.inapp.purchasing.PurchasingManager;

import com.moonlite.tuxracer.AppPurchasingObserver.PurchaseData;
import com.moonlite.tuxracer.AppPurchasingObserver.PurchaseDataStorage;

import com.moonlite.tuxracer.ScoreActivity;

/**
 * GameActivity implements Amazon In-App Purchase on top of ScoreActivity
 */
public class GameActivity extends ScoreActivity implements
		AppPurchasingObserverListener {
	private static final String TAG = "Tuxracer";
	
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
	static public void BuyItem(int id) {
		String requestId = PurchasingManager
				.initiatePurchaseRequest(MySKU.COURSE_PACK.getSku());
		PurchaseData purchaseData = purchaseDataStorage
				.newPurchaseData(requestId);
		Log.i(TAG, "AmazonBuyItem: requestId (" + requestId
				+ ") requestState (" + purchaseData.getRequestState() + ")");
	}

	// Setup
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// Log.v("SDL", "onCreate()");
		super.onCreate(savedInstanceState);

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		nativeSetPlayerData("", false);

		// Initialize appropriate In-App Purchase system
		AmazonInitIAP();
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
       	AmazonRequestIAP();
    }


	public static native void nativeSetPlayerData(String playerName, boolean isOnOuya);
	public static native void nativeCoursePrice(float price);
}
