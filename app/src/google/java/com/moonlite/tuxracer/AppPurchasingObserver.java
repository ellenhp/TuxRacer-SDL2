package com.moonlite.tuxracer;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.StringTokenizer;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.AsyncTask;
import android.util.Log;

import com.amazon.inapp.purchasing.BasePurchasingObserver;
import com.amazon.inapp.purchasing.GetUserIdResponse;
import com.amazon.inapp.purchasing.GetUserIdResponse.GetUserIdRequestStatus;
import com.amazon.inapp.purchasing.ItemDataResponse;
import com.amazon.inapp.purchasing.ItemDataResponse.ItemDataRequestStatus;
import com.amazon.inapp.purchasing.Offset;
import com.amazon.inapp.purchasing.PurchaseResponse;
import com.amazon.inapp.purchasing.PurchaseResponse.PurchaseRequestStatus;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse.PurchaseUpdatesRequestStatus;
import com.amazon.inapp.purchasing.PurchasingManager;
import com.amazon.inapp.purchasing.Receipt;

/**
 * Implementation of {@link BasePurchasingObserver} that implements each of the
 * callbacks and calls the methods on the {@link AppPurchasingObserverListener}
 * to notify the listener. In this sample app, the {@link MainActivity} is the
 * listener.
 */
public class AppPurchasingObserver extends BasePurchasingObserver {

	private static final String TAG = "Tuxracer";

	private PurchaseDataStorage purchaseDataStorage;

	// Note: change below to a list if you want more than one listener
	private AppPurchasingObserverListener listener;

	public AppPurchasingObserver(Activity activity,
			PurchaseDataStorage purchaseDataStorage) {
		super(activity);
		this.purchaseDataStorage = purchaseDataStorage;
	}

	/**
	 * Set listener which will get notified through callbacks. This example code
	 * only uses 1 listener, which is the {@link MainActivity}. If instead
	 * multiple listeners are needed, collect listeners with an addListener
	 * method and notify all listeners through the callbacks.
	 * 
	 * @param listener
	 */
	public void setListener(AppPurchasingObserverListener listener) {
		this.listener = listener;
	}

	/**
	 * This callback indicates that the SDK is initialized and whether the app
	 * is live or in sandbox (test) mode
	 * 
	 * @param isSandboxMode
	 */
	@Override
	public void onSdkAvailable(boolean isSandboxMode) {
		Log.i(TAG, "onSdkAvailable: isSandboxMode: " + isSandboxMode);
	}

	/**
	 * This is callback for {@link PurchasingManager#initiateGetUserIdRequest}.
	 * For successful case, save the current user from {@link GetUserIdResponse}
	 * , and notify listener through
	 * {@link AppPurchasingObserverListener#onGetUserIdResponseSuccess} method
	 * and initiate a purchase updates request. For failed case, notify listener
	 * through {@link AppPurchasingObserverListener#onGetUserIdResponseFailed}
	 * method.
	 * 
	 * @param response
	 */
	@Override
	public void onGetUserIdResponse(GetUserIdResponse response) {
		Log.i(TAG, "onGetUserIdResponse: requestId (" + response.getRequestId()
				+ ") userIdRequestStatus: " + response.getUserIdRequestStatus()
				+ ")");

		GetUserIdRequestStatus status = response.getUserIdRequestStatus();
		switch (status) {
		case SUCCESSFUL:
			String userId = response.getUserId();
			Log.i(TAG, "onGetUserIdResponse: save userId (" + userId
					+ ") as current user");
			boolean userChanged = saveCurrentUser(userId);

			Log.i(TAG,
					"onGetUserIdResponse: call onGetUserIdResponseSuccess for userId ("
							+ userId + ") userChanged (" + userChanged + ")");
			listener.onGetUserIdResponseSuccessful(userId, userChanged);

			Offset offset = purchaseDataStorage.getPurchaseUpdatesOffset();

			Log.i(TAG,
					"onGetUserIdResponse: call initiatePurchaseUpdatesRequest from offset ("
							+ offset + ")");
			PurchasingManager.initiatePurchaseUpdatesRequest(offset);
			break;

		case FAILED:
			Log.i(TAG, "onGetUserIdResponse: FAILED");
			listener.onGetUserIdResponseFailed(response.getRequestId());
			break;
		}
	}

	private boolean saveCurrentUser(String userId) {
		return purchaseDataStorage.saveCurrentUser(userId);
	}

	/**
	 * This is callback for {@link PurchasingManager#initiateItemDataRequest}.
	 * For unavailable SKUs, notify listener through
	 * {@link AppPurchasingObserverListener#onItemDataResponseUnavailableSkus}
	 * method.
	 */
	@Override
	public void onItemDataResponse(ItemDataResponse response) {
		final ItemDataRequestStatus status = response
				.getItemDataRequestStatus();
		Log.i(TAG, "onItemDataResponse: itemDataRequestStatus (" + status + ")");

		switch (status) {
		case SUCCESSFUL_WITH_UNAVAILABLE_SKUS:
			Set<String> unavailableSkus = response.getUnavailableSkus();
			Log.i(TAG, "onItemDataResponse: " + unavailableSkus.size()
					+ " unavailable skus");
			if (!unavailableSkus.isEmpty()) {
				Log.i(TAG,
						"onItemDataResponse: call onItemDataResponseUnavailableSkus");
				listener.onItemDataResponseSuccessfulWithUnavailableSkus(unavailableSkus);
			}
		case SUCCESSFUL:
			Log.d(TAG,
					"onItemDataResponse: successful.  The item data map in this response includes the valid SKUs");
			listener.onItemDataResponseSuccessful(response.getItemData());
			break;
		case FAILED:
			Log.d(TAG, "onItemDataResponse: failed, should retry request");
			listener.onItemDataResponseFailed(response.getRequestId());
			break;
		}
	}

	/**
	 * This is callback for
	 * {@link PurchasingManager#initiatePurchaseUpdatesRequest}. Check that
	 * UserId from {@link PurchaseUpdatesResponse} is same as current user. Fire
	 * off async task to update fulfillments from receipts and revoke
	 * fulfillments for revoked skus. If there are more updates, call
	 * {@link PurchasingManager#initiatePurchaseUpdatesRequest} with last
	 * offset. For a failed response, notify listener through
	 * {@link AppPurchasingObserverListener#onPurchaseUpdatesResponseFailed}
	 */
	@Override
	public void onPurchaseUpdatesResponse(PurchaseUpdatesResponse response) {
		final String userId = response.getUserId();
		final PurchaseUpdatesRequestStatus status = response
				.getPurchaseUpdatesRequestStatus();

		Log.i(TAG, "onPurchaseUpdatesResponse: userId (" + userId
				+ ") purchaseUpdatesRequestStatus (" + status + ")");
		if (!purchaseDataStorage.isSameAsCurrentUser(userId)) {
			// In most cases UserId in PurchaseUpdatesResponse should be the
			// same as UserId from GetUserIdResponse
			Log.i(TAG, "onPurchaseUpdatesResponse: userId (" + userId
					+ ") in response is NOT the same as current user!");
			return;
		}

		switch (status) {
		case SUCCESSFUL:
			// Update fulfillments for receipts
			// Handle receipts before revoked skus
			Set<Receipt> receipts = response.getReceipts();
			Set<String> revokedSkus = response.getRevokedSkus();
			Log.i(TAG, "onPurchaseUpdatesResponse: (" + receipts.size()
					+ ") receipts and (" + revokedSkus.size()
					+ ") revoked SKUs");
			if (!receipts.isEmpty() || !revokedSkus.isEmpty()) {
				PurchaseUpdatesData purchaseUpdatesResponseData = new PurchaseUpdatesData(
						response.getUserId(), receipts, revokedSkus);
				new PurchaseUpdatesAsyncTask()
						.execute(purchaseUpdatesResponseData);
			}

			Offset offset = response.getOffset();
			// If more updates, send another request with current offset
			if (response.isMore()) {
				Log.i(TAG,
						"onPurchaseUpdatesResponse: more updates, call initiatePurchaseUpdatesRequest with offset: "
								+ offset);
				PurchasingManager.initiatePurchaseUpdatesRequest(offset);
			}
			purchaseDataStorage.savePurchaseUpdatesOffset(offset);
			break;
		case FAILED:
			Log.i(TAG, "onPurchaseUpdatesResponse: FAILED: response: "
					+ response);
			listener.onPurchaseUpdatesResponseFailed(response.getRequestId());
			// May want to retry request
			break;
		}
	}

	/**
	 * This is callback for {@link PurchasingManager#initiatePurchaseRequest}.
	 * Save purchase response including receipt and then fire off async task to
	 * fulfill purchase. For the other response cases: already entitled, invalid
	 * sku and failed, notify {@link AppPurchasingObserverListener} through
	 * appropriate method.
	 */
	@Override
	public void onPurchaseResponse(PurchaseResponse response) {
		String requestId = response.getRequestId();
		String userId = response.getUserId();
		PurchaseRequestStatus status = response.getPurchaseRequestStatus();
		Log.i(TAG, "onPurchaseResponse: requestId (" + requestId + ") userId ("
				+ userId + ") purchaseRequestStatus (" + status + ")");
		if (!purchaseDataStorage.isSameAsCurrentUser(userId)) {
			// In most cases UserId in PurchaseResponse should be the
			// same as UserId from GetUserIdResponse
			Log.i(TAG, "onPurchaseResponse: userId (" + userId
					+ ") in response is NOT the same as current user!");
			return;
		}

		PurchaseData purchaseDataForRequestId = null;
		String sku = null;
		
		switch (status) {
		case SUCCESSFUL:
			Receipt receipt = response.getReceipt();
			Log.i(TAG,
					"onPurchaseResponse: receipt itemType ("
							+ receipt.getItemType() + ") SKU ("
							+ receipt.getSku() + ") purchaseToken ("
							+ receipt.getPurchaseToken() + ")");

			Log.i(TAG,
					"onPurchaseResponse: call savePurchaseReceipt for requestId ("
							+ response.getRequestId() + ")");
			PurchaseData purchaseData = purchaseDataStorage
					.savePurchaseResponse(response);
			if (purchaseData == null) {
				Log.i(TAG,
						"onPurchaseResponse: could not save purchase receipt for requestId ("
								+ response.getRequestId()
								+ "), skipping fulfillment");
				break;
			}

			Log.i(TAG, "onPurchaseResponse: fulfill purchase with AsyncTask");
			new PurchaseResponseSuccessAsyncTask().execute(purchaseData);
			break;
		case ALREADY_ENTITLED:
			Log.i(TAG,
					"onPurchaseResponse: already entitled so remove purchase request from local storage");
			purchaseDataForRequestId = purchaseDataStorage.getPurchaseData(requestId);
			purchaseDataStorage.removePurchaseData(requestId);
		    if (purchaseDataForRequestId!=null) 
		        sku = purchaseDataForRequestId.getSKU();
			listener.onPurchaseResponseAlreadyEntitled(userId, sku);
			break;
		case INVALID_SKU:
			Log.i(TAG,
					"onPurchaseResponse: invalid SKU! Should never get here, onItemDataResponse should have disabled buy button already.");
			// We should never get here because onItemDataResponse should have
			// taken care of invalid skus already and disabled the buy button
			purchaseDataForRequestId = purchaseDataStorage.getPurchaseData(requestId);
			purchaseDataStorage.removePurchaseData(requestId);
		    if (purchaseDataForRequestId!=null) 
		        sku = purchaseDataForRequestId.getSKU();
		    listener.onPurchaseResponseInvalidSKU(userId, sku);
			break;
		case FAILED:
			Log.i(TAG,
					"onPurchaseResponse: failed so remove purchase request from local storage");
			purchaseDataForRequestId = purchaseDataStorage.getPurchaseData(requestId);
			purchaseDataStorage.removePurchaseData(requestId);
		    if (purchaseDataForRequestId!=null) 
		        sku = purchaseDataForRequestId.getSKU();
			listener.onPurchaseResponseFailed(requestId, sku);
			// May want to retry request
			break;
		}

	}

	/**
	 * AsyncTask to fulfill purchase which is kicked off from
	 * onPurchaseResponse. Notify listener through
	 * {@link AppPurchasingObserverListener#onPurchaseResponseSuccess} method.
	 * Save the fact that purchase token and requestId are fulfilled.
	 */
	private class PurchaseResponseSuccessAsyncTask extends
			AsyncTask<PurchaseData, Void, Boolean> {
		@Override
		protected Boolean doInBackground(PurchaseData... args) {
			PurchaseData purchaseData = args[0];

			String requestId = purchaseData.getRequestId();

			String userId = purchaseData.getUserId();
			String sku = purchaseData.getSKU();
			String purchaseToken = purchaseData.getPurchaseToken();

			Log.i(TAG,
					"PurchaseResponseSuccessAsyncTask.doInBackground: call listener's onPurchaseResponseSucccess for sku ("
							+ sku + ")");
			listener.onPurchaseResponseSuccess(userId, sku, purchaseToken);

			Log.d(TAG,
					"PurchaseResponseSuccessAsyncTask.doInBackground: fulfilled SKU ("
							+ sku + ") purchaseToken (" + purchaseToken + ")");
			purchaseDataStorage.setPurchaseTokenFulfilled(purchaseToken);

			purchaseDataStorage.setRequestStateFulfilled(requestId);
			Log.d(TAG,
					"PurchaseResponseSuccessAsyncTask.doInBackground: completed for requestId ("
							+ requestId + ")");
			return true;
		}
	}

	/**
	 * AsyncTask to update fulfillment for purchase called from
	 * onPurchaseUpdatesResponse. For each receipt, check whether we should
	 * fulfill and if so, notify listener through
	 * {@link AppPurchasingObserverListener#onPurchaseUpdatesResponseSuccess}
	 * method. For each revoked SKU, notify listener through
	 * {@link AppPurchasingObserverListener#onPurchaseUpdatesResponseSuccessRevokedSku}
	 * method
	 */
	private class PurchaseUpdatesAsyncTask extends
			AsyncTask<PurchaseUpdatesData, Void, Boolean> {
		@Override
		protected Boolean doInBackground(PurchaseUpdatesData... args) {
			PurchaseUpdatesData purchaseUpdatesData = args[0];
			String userId = purchaseUpdatesData.getUserId();
			Set<Receipt> receipts = purchaseUpdatesData.getReceipts();
			for (Receipt receipt : receipts) {
				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: receipt itemType ("
								+ receipt.getItemType() + ") SKU ("
								+ receipt.getSku() + ") purchaseToken ("
								+ receipt.getPurchaseToken() + ")");
				String sku = receipt.getSku();

				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: call onPurchaseUpdatesResponseSuccessSku for sku ("
								+ sku + ")");
				listener.onPurchaseUpdatesResponseSuccess(userId, sku,
						receipt.getPurchaseToken());

				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: completed for receipt with purchaseToken ("
								+ receipt.getPurchaseToken() + ")");
			}

			Set<String> revokedSkus = purchaseUpdatesData.getRevokedSkus();
			for (String revokedSku : revokedSkus) {
				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: call onPurchaseUpdatesResponseSuccessRevokedSku for revoked sku ("
								+ revokedSku + ")");
				listener.onPurchaseUpdatesResponseSuccessRevokedSku(userId,
						revokedSku);

				purchaseDataStorage.skuFulfilledCountDown(revokedSku);
			}
			return true;
		}
	}

	/**
	 * Used to store current user, purchase request state and purchase receipts
	 * related information into SharedPreferences specific to each user.
	 */
	protected static class PurchaseDataStorage {

		private Activity activity;

		private String currentUser;

		private SharedPreferences savedUserRequestsAndPurchaseReceipts;

		public PurchaseDataStorage(Activity activity) {
			this.activity = activity;
		}

		/**
		 * Save current user and switch to SharedPreferences specific to the
		 * user.
		 * 
		 * @return whether user changed from previously stored user
		 */
		public boolean saveCurrentUser(String userId) {
			boolean userChanged = ((this.currentUser == null) ? true
					: (!this.currentUser.equals(userId)));

			this.currentUser = userId;
			Log.d(TAG, "saveCurrentUser: " + userId);

			resetSavedUserRequestsAndPurchaseReceipts();
			return userChanged;
		}

		/**
		 * Checks whether userId passed in is same as the currently stored user.
		 */
		public boolean isSameAsCurrentUser(String userId) {
			return this.currentUser.equals(userId);
		}

		/**
		 * Adds requestId to a list of saved request IDs
		 */
		private void addRequestId(String requestId) {
			Set<String> requestIds = getStringSet("REQUEST_IDS");
			requestIds.add(requestId);
			putStringSet("REQUEST_IDS", requestIds);
		}

		/**
		 * Removes requestId from the list of saved request IDs
		 */
		private void removeRequestId(String requestId) {
			Set<String> requestIds = getStringSet("REQUEST_IDS");
			requestIds.remove(requestId);
			putStringSet("REQUEST_IDS", requestIds);
		}

		/**
		 * Get all saved request IDs
		 */
		public Set<String> getAllRequestIds() {
			return getStringSet("REQUEST_IDS");
		}

		/**
		 * Save userrId and receipt information from PurchaseResponse into
		 * SharedPreferences, mark requestId state as having received the
		 * purchase response and increment sku fulfill count.
		 */
		public PurchaseData savePurchaseResponse(PurchaseResponse response) {
			String requestId = response.getRequestId();
			String userId = response.getUserId();
			Receipt receipt = response.getReceipt();

			// RequestId should match a previously sent requestId
			if (!doesRequestIdMatchSentRequestId(requestId)) {
				Log.i(TAG, "savePurchaseReceipt: requestId (" + requestId
						+ ") does NOT match any requestId sent before!");
				return null;
			}

			String purchaseToken = receipt.getPurchaseToken();
			String sku = receipt.getSku();

			PurchaseData purchaseData = getPurchaseData(requestId);
			purchaseData.setUserId(userId);
			purchaseData.setRequestState(RequestState.RECEIVED);
			purchaseData.setPurchaseToken(purchaseToken);
			purchaseData.setSKU(sku);

			Log.d(TAG,
					"savePurchaseResponse: saving purchaseToken ("
							+ purchaseToken + ") sku (" + sku
							+ ") and request state as ("
							+ purchaseData.getRequestState() + ")");
			savePurchaseData(purchaseData);

			skuFulfilledCountUp(sku);

			return purchaseData;
		}

		/**
		 * Checks if requestId matches requestId we sent out previously
		 */
		private boolean doesRequestIdMatchSentRequestId(String requestId) {
			PurchaseData purchaseData = getPurchaseData(requestId);
			return purchaseData != null;
		}

		/**
		 * Increment SKU fulfilled count and save. Use this fulfilled count to
		 * decide whether to fulfill entitlement.
		 */
		public void skuFulfilledCountUp(String sku) {
			SKUData skuData = getOrCreateSKUData(sku);
			skuData.fulfilledCountUp();
			Log.i(TAG,
					"skuFulfilledCountUp: fulfilledCountUp to ("
							+ skuData.getFulfilledCount() + ") for sku (" + sku
							+ "), save SKU data");
			saveSKUData(skuData);
		}

		/**
		 * Decrement SKU fulfilled count for revoked SKU and save.
		 */
		protected void skuFulfilledCountDown(String revokedSku) {
			SKUData skuData = getSKUData(revokedSku);
			if (skuData == null)
				return;
			skuData.fulfilledCountDown();
			Log.i(TAG, "skuFulfilledCountDown: fulfilledCountDown to ("
					+ skuData.getFulfilledCount() + ") for revoked sku ("
					+ revokedSku + "), save SKU data");
			saveSKUData(skuData);
		}

		/**
		 * Checks whether we should fulfill for this SKU by checking SKU's
		 * fulfill count
		 */
		public boolean shouldFulfillSKU(String sku) {
			SKUData skuData = getSKUData(sku);
			if (skuData == null)
				return false;
			return skuData.getFulfilledCount() > 0;
		}

		/**
		 * Mark requestId state as fulfilled
		 */
		public void setRequestStateFulfilled(String requestId) {
			PurchaseData purchaseData = getPurchaseData(requestId);
			purchaseData.setRequestState(RequestState.FULFILLED);
			savePurchaseData(purchaseData);
			Log.i(TAG,
					"setRequestStateFulfilled: requestId (" + requestId
							+ ") setting requestState to ("
							+ purchaseData.getRequestState() + ")");
		}
		
		/**
		 * Check if requestId state is SENT
		 */
		public boolean isRequestStateSent(String requestId) {
			PurchaseData purchaseData = getPurchaseData(requestId);
			if (purchaseData==null)
				return false;
			return RequestState.SENT == purchaseData.getRequestState();
		}

		/**
		 * Mark purchase token as fulfilled
		 */
		public void setPurchaseTokenFulfilled(String purchaseToken) {
			PurchaseData purchaseData = getPurchaseDataByPurchaseToken(purchaseToken);
			purchaseData.setPurchaseTokenFulfilled();
			Log.i(TAG, "setPurchaseTokenFulfilled: set purchaseToken ("
					+ purchaseToken + ") as fulfilled");
			savePurchaseData(purchaseData);
		}

		/**
		 * Checks if purchase for purchase token has been fulfilled
		 */
		public boolean isPurchaseTokenFulfilled(String purchaseToken) {
			PurchaseData purchaseData = getPurchaseDataByPurchaseToken(purchaseToken);
			if (purchaseData == null)
				return false;
			return purchaseData.isPurchaseTokenFulfilled();
		}

		/**
		 * Adds requestId to list of saved requestIds, creates new
		 * {@link PurchaseData} for requestId and sets request state to SENT
		 */
		public PurchaseData newPurchaseData(String requestId) {
			addRequestId(requestId);

			PurchaseData purchaseData = new PurchaseData(requestId);
			purchaseData.setRequestState(RequestState.SENT);

			savePurchaseData(purchaseData);

			Log.d(TAG, "newPurchaseData: adding requestId (" + requestId
					+ ") to saved list and setting request state to ("
					+ purchaseData.getRequestState() + ")");
			return purchaseData;
		}

		/**
		 * Saves PurchaseResponse data into SharedPreferences keyed off of both
		 * requestId and purchaseToken.
		 */
		public void savePurchaseData(PurchaseData purchaseData) {
			String json = PurchaseDataJSON.toJSON(purchaseData);

			Log.d(TAG, "savePurchaseData: saving for requestId ("
					+ purchaseData.getRequestId() + ") json: " + json);
			String requestId = purchaseData.getRequestId();
			putString(requestId, json);

			String purchaseToken = purchaseData.getPurchaseToken();
			if (purchaseToken != null) {
				Log.d(TAG, "savePurchaseData: saving for purchaseToken ("
						+ purchaseToken + ") json: " + json);
				putString(purchaseToken, json);
			}
		}

		/**
		 * Get PurchaseResponse data by requestId
		 */
		public PurchaseData getPurchaseData(String requestId) {
			String json = getString(requestId);
			if (json == null)
				return null;
			return PurchaseDataJSON.fromJSON(json);
		}

		/**
		 * Get PurchaseResponse data by purchaseToken
		 */
		public PurchaseData getPurchaseDataByPurchaseToken(String purchaseToken) {
			String json = getString(purchaseToken);
			if (json == null)
				return null;
			return PurchaseDataJSON.fromJSON(json);
		}

		/**
		 * Creates new {@link SKUData} which is used to keep track of fulfilled
		 * count
		 */
		public SKUData newSKUData(String sku) {
			Log.d(TAG, "newSKUData: creating new SKUData for sku (" + sku + ")");
			return new SKUData(sku);
		}

		/**
		 * Saves {@link SKUData} to SharedPreferences
		 */
		public void saveSKUData(SKUData skuData) {
			String json = SKUDataJSON.toJSON(skuData);
			Log.d(TAG, "saveSKUData: saving for sku (" + skuData.getSKU()
					+ ") json: " + json);
			putString(skuData.getSKU(), json);
		}

		/**
		 * Gets {@link SKUData} from SharedPreferences by sku
		 */
		public SKUData getSKUData(String sku) {
			String json = getString(sku);
			if (json == null)
				return null;
			return SKUDataJSON.fromJSON(json);
		}

		/**
		 * Gets or Creates new {@link SKUData} from SharedPreferences by sku
		 */
		public SKUData getOrCreateSKUData(String sku) {
			SKUData skuData = getSKUData(sku);
			if (skuData == null) {
				skuData = newSKUData(sku);
			}
			return skuData;
		}

		/**
		 * Saves Offset from Purchase Updates response
		 */
		public void savePurchaseUpdatesOffset(Offset offset) {
			putString("PURCHASE_UPDATES_OFFSET", offset.toString());
		}

		/**
		 * Get Offset saved from last Purchase Updates response
		 */
		public Offset getPurchaseUpdatesOffset() {
			String offsetString = getString("PURCHASE_UPDATES_OFFSET");
			if (offsetString == null) {
				Log.i(TAG,
						"getPurchaseUpdatesOffset: no previous offset saved, use Offset.BEGINNING");
				return Offset.BEGINNING;
			}
			return Offset.fromString(offsetString);
		}

		/**
		 * Remove purchase data for requestId
		 */
		public void removePurchaseData(String requestId) {
			remove(requestId);
			removeRequestId(requestId);
		}

		/**
		 * Reset SharedPreferences to switch to new SharedPreferences for
		 * different user
		 */
		private void resetSavedUserRequestsAndPurchaseReceipts() {
			this.savedUserRequestsAndPurchaseReceipts = null;
		}

		/**
		 * Gets user specific SharedPreferences where we save requestId state
		 * and purchase receipt data into.
		 */
		private SharedPreferences getSavedUserRequestsAndPurchaseReceipts() {
			if (savedUserRequestsAndPurchaseReceipts != null)
				return savedUserRequestsAndPurchaseReceipts;
			savedUserRequestsAndPurchaseReceipts = activity
					.getSharedPreferences(currentUser, Activity.MODE_PRIVATE);
			return savedUserRequestsAndPurchaseReceipts;
		}

		/**
		 * Convenience method to getStringSet for key from SharedPreferences
		 */
		protected Set<String> getStringSet(String key) {
			Set<String> emptySet = new HashSet<String>();
			return getStringSet(key, emptySet);
		}

		/**
		 * Convenience method to getStringSet for key from SharedPreferences,
		 * specifying default values if null.
		 */
		protected Set<String> getStringSet(String key, Set<String> defValues) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			// If you're only targeting devices with Android API Level 11 or above
			// you can just use the getStringSet method
			String pipeDelimitedValues = getString(key);
			return convertPipeDelimitedToList(pipeDelimitedValues);
		}
		
		private Set<String> convertPipeDelimitedToList(String pipeDelimitedValues) {
			Set<String> result = new HashSet<String>();
			if (pipeDelimitedValues==null || "".equals(pipeDelimitedValues))
				return result;
			
			StringTokenizer stk = new StringTokenizer(pipeDelimitedValues, "|");
			while (stk.hasMoreTokens()) {
				String token = stk.nextToken();
				result.add(token);
			}
			return result;
		}

		/**
		 * Convenience method to save string set by key into SharedPreferences
		 */
		protected void putStringSet(String key, Set<String> valuesSet) {
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			// If you're only targeting devices with Android API Level 11 or above
	        // you can just use the putStringSet method
			String pipeDelimitedValues = convertListToPipeDelimited(valuesSet);
			editor.putString(key, pipeDelimitedValues);
			editor.apply();
		}
		
		private String convertListToPipeDelimited(Set<String> values) {
			if (values==null || values.isEmpty())
				return "";
			StringBuilder result = new StringBuilder();
			for (Iterator<String> iter = values.iterator(); iter.hasNext();) {
				result.append(iter.next());
				if (iter.hasNext()) {
					result.append("|");
				}
			}
			return result.toString();
		}

		/**
		 * Convenience method to save string by key into SharedPreferences
		 */
		protected void putString(String key, String value) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.putString(key, value);
			editor.apply();
		}

		/**
		 * Convenience method to get int by key from SharedPreferences
		 */
		protected int getInt(String key, int defValue) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			return savedUserRequestsAndPurchaseReceipts.getInt(key, defValue);
		}

		/**
		 * Convenience method to save int by key into SharedPreferences
		 */
		protected void putInt(String key, int value) {
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.putInt(key, value);
			editor.apply();
		}

		/**
		 * Convenience method to get string by key from SharedPreferences
		 */
		protected String getString(String key) {
			return getString(key, null);
		}

		/**
		 * Convenience method to get string by key from SharedPreferences
		 * returning default value if null
		 */
		protected String getString(String key, String defValue) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			return savedUserRequestsAndPurchaseReceipts
					.getString(key, defValue);
		}

		/**
		 * Convenience method to get boolean by key from SharedPreferences
		 * returning false as default value if null
		 */
		protected boolean getBoolean(String key) {
			return getBoolean(key, false);
		}

		/**
		 * Convenience method to get boolean by key from SharedPreferences
		 * returning default value if null
		 */
		protected boolean getBoolean(String key, boolean defValue) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			return savedUserRequestsAndPurchaseReceipts.getBoolean(key,
					defValue);
		}

		/**
		 * Convenience method to save boolean by key into SharedPreferences
		 */
		protected void putBoolean(String key, boolean value) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.putBoolean(key, value);
			editor.apply();
		}

		/**
		 * Convenience method to remove data in SharedPreferences by key
		 */
		protected void remove(String key) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.remove(key);
			editor.apply();
		}

	}

	/**
	 * Represents {@link PurchaseResponse} related data such as requestIds,
	 * purchase tokens and sku. This data is saved into SharedPreferences in
	 * serialized form.
	 */
	protected static class PurchaseData {

		private String requestId;
		private String userId;
		private RequestState requestState;
		private String purchaseToken;
		private String sku;
		private boolean purchaseTokenFulfilled;

		public PurchaseData(String requestId) {
			this.requestId = requestId;
		}

		public void setUserId(String userId) {
			this.userId = userId;
		}

		public void setRequestState(RequestState requestState) {
			this.requestState = requestState;
		}

		public String getRequestId() {
			return requestId;
		}

		public String getUserId() {
			return userId;
		}

		public RequestState getRequestState() {
			return requestState;
		}

		public int getRequestStateAsInt() {
			return requestState.getState();
		}

		public void setPurchaseToken(String purchaseToken) {
			this.purchaseToken = purchaseToken;
		}

		public void setSKU(String sku) {
			this.sku = sku;
		}

		public String getPurchaseToken() {
			return purchaseToken;
		}

		public String getSKU() {
			return sku;
		}

		public void setPurchaseTokenFulfilled() {
			this.purchaseTokenFulfilled = true;
		}

		public boolean isPurchaseTokenFulfilled() {
			return purchaseTokenFulfilled;
		}

		@Override
		public String toString() {
			return "PurchaseData [requestId=" + requestId + ", userId="
					+ userId + ", requestState=" + requestState
					+ ", purchaseToken=" + purchaseToken + ", sku=" + sku
					+ ", purchaseTokenFulfilled=" + purchaseTokenFulfilled
					+ "]";
		}

	}

	/**
	 * Used to serialize {@link PurchaseData} into JSON and back again.
	 */
	protected static class PurchaseDataJSON {

		private static final String REQUEST_ID = "requestId";
		private static final String REQUEST_STATE = "requestState";
		private static final String PURCHASE_TOKEN = "purchaseToken";
		private static final String SKU = "sku";
		private static final String PURCHASE_TOKEN_FULFILLED = "purchaseTokenFulfilled";

		/**
		 * Serializes PurchaseData objects into JSON string.
		 */
		public static String toJSON(PurchaseData data) {
			JSONObject obj = new JSONObject();
			try {
				obj.put(REQUEST_ID, data.getRequestId());
				obj.put(REQUEST_STATE, data.getRequestStateAsInt());
				if (data.getPurchaseToken() != null)
					obj.put(PURCHASE_TOKEN, data.getPurchaseToken());
				if (data.getSKU() != null)
					obj.put(SKU, data.getSKU());
				if (data.isPurchaseTokenFulfilled())
					obj.put(PURCHASE_TOKEN_FULFILLED, true);
			} catch (JSONException e) {
				Log.e(TAG, "toJSON: ERROR serializing: " + data);
			}

			return obj.toString();
		}

		/**
		 * Deserializes JSON string back into PurchaseData object.
		 */
		public static PurchaseData fromJSON(String json) {
			if (json == null)
				return null;
			JSONObject obj = null;
			try {
				obj = new JSONObject(json);
				String requestId = obj.getString(REQUEST_ID);
				int requestState = obj.getInt(REQUEST_STATE);
				String purchaseToken = obj.optString(PURCHASE_TOKEN);
				String sku = obj.optString(SKU);
				boolean purchaseTokenFulfilled = obj
						.optBoolean(PURCHASE_TOKEN_FULFILLED);

				PurchaseData result = new PurchaseData(requestId);
				result.setRequestState(RequestState.valueOf(requestState));
				result.setPurchaseToken(purchaseToken);
				result.setSKU(sku);
				if (purchaseTokenFulfilled) {
					result.setPurchaseTokenFulfilled();
				}
				return result;
			} catch (JSONException e) {
				e.printStackTrace();
			}
			return null;
		}

	}

	/**
	 * Represents SKU and fulfilled count. Saved in SharedPreferences in
	 * serialized form.
	 */
	protected static class SKUData {
		private String sku;
		private int fulfilledCount;

		public SKUData(String sku) {
			this.sku = sku;
			this.fulfilledCount = 0;
		}

		public void fulfilledCountUp() {
			this.fulfilledCount++;
		}

		public void fulfilledCountDown() {
			this.fulfilledCount--;
		}

		public String getSKU() {
			return sku;
		}

		public int getFulfilledCount() {
			return fulfilledCount;
		}

		public void setFulfilledCount(int fulfilledCount) {
			this.fulfilledCount = fulfilledCount;
		}

		@Override
		public String toString() {
			return "SKUData [sku=" + sku + ", fulfilledCount=" + fulfilledCount
					+ "]";
		}

	}

	/**
	 * Used to serialize SKUData into JSON and back again.
	 */
	protected static class SKUDataJSON {

		private static final String SKU = "sku";
		private static final String FULFILLED_COUNT = "fulfilledCount";

		/**
		 * Serializes SKUData into JSON string.
		 */
		public static String toJSON(SKUData data) {
			if (data == null)
				return null;
			JSONObject obj = new JSONObject();
			try {
				obj.put(SKU, data.getSKU());
				obj.put(FULFILLED_COUNT, data.getFulfilledCount());
			} catch (JSONException e) {
				Log.e(TAG, "toJSON: ERROR serializing: " + data);
			}

			// Log.i(TAG, "toJSON: "+obj.toString());
			return obj.toString();
		}

		/**
		 * Deserializes JSON string back into SKUData object.
		 */
		public static SKUData fromJSON(String json) {
			if (json == null)
				return null;
			JSONObject obj = null;
			try {
				obj = new JSONObject(json);
				String sku = obj.getString(SKU);
				int fulfilledCount = obj.getInt(FULFILLED_COUNT);
				SKUData result = new SKUData(sku);
				result.setFulfilledCount(fulfilledCount);
				// Log.i(TAG, "fromJSON: " + result);
				return result;
			} catch (JSONException e) {
				e.printStackTrace();
			}
			return null;
		}

	}

	/**
	 * Represents {@link PurchaseUpdatesResponse} related data such as userId,
	 * receipts and revoked skus.
	 */
	protected static class PurchaseUpdatesData {
		private final String userId;
		private final Set<Receipt> receipts;
		private final Set<String> revokedSkus;

		public PurchaseUpdatesData(String userId, Set<Receipt> receipts,
				Set<String> revokedSkus) {
			this.userId = userId;
			this.receipts = receipts;
			this.revokedSkus = revokedSkus;
		}

		public Set<Receipt> getReceipts() {
			return receipts;
		}

		public Set<String> getRevokedSkus() {
			return revokedSkus;
		}

		public String getUserId() {
			return userId;
		}

		@Override
		public String toString() {
			return "PurchaseUpdatesData [userId=" + userId + ", receipts="
					+ receipts + ", revokedSkus=" + revokedSkus + "]";
		}

	}

	/**
	 * Represents the state of a request which goes from SENT, RECEIVED to
	 * FULFILLED.
	 */
	public static enum RequestState {

		NOT_AVAILABLE(0), //
		SENT(1), //
		RECEIVED(2), //
		FULFILLED(3);

		private int state;

		private RequestState(int state) {
			this.state = state;
		}

		public int getState() {
			return state;
		}

		/**
		 * Gets RequestState enum by int state.
		 */
		public static RequestState valueOf(int state) {
			for (RequestState requestState : values()) {
				if (requestState.getState() == state) {
					return requestState;
				}
			}
			return null;
		}
	}

}
