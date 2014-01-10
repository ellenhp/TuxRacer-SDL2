package com.moonlite.tuxracer;

import java.util.Map;
import java.util.Set;

import com.amazon.inapp.purchasing.GetUserIdResponse;
import com.amazon.inapp.purchasing.GetUserIdResponse.GetUserIdRequestStatus;
import com.amazon.inapp.purchasing.Item;
import com.amazon.inapp.purchasing.ItemDataResponse.ItemDataRequestStatus;
import com.amazon.inapp.purchasing.PurchaseResponse.PurchaseRequestStatus;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse.PurchaseUpdatesRequestStatus;
import com.amazon.inapp.purchasing.PurchasingManager;

public interface AppPurchasingObserverListener {

	/**
	 * Callback for a successful get user id response {@link GetUserIdResponseStatus#SUCCESSFUL}. 
	 * 
	 * @param userId returned from {@link GetUserIdResponse#getUserId()}.
	 * @param userChanged - whether user changed from previously stored user.
	 */
	public void onGetUserIdResponseSuccessful(String userId, boolean userChanged);

	/**
	 * Callback for a failed get user id response {@link GetUserIdRequestStatus#FAILED}
	 * 
	 * @param requestId returned from {@link GetUserIdResponsee#getRequestId()} that can be used
	 * to correlate with original request sent with {@link PurchasingManager#initiateGetUserIdRequest()}.
	 */
	public void onGetUserIdResponseFailed(String requestId);

	
	/**
	 * Callback for successful item data response with unavailable SKUs {@link ItemDataRequestStatus#SUCCESSFUL_WITH_UNAVAILABLE_SKUS}. 
	 * This means that these unavailable SKUs are NOT accessible in developer portal.
	 *  
	 * @param unavailableSkus - skus that are not valid in developer portal
	 */
	public void onItemDataResponseSuccessfulWithUnavailableSkus(
			Set<String> unavailableSkus);
  
	/**
	 * Callback for successful item data response {@link ItemDataRequestStatus#SUCCESSFUL} with item data 
	 * 
	 * @param itemData - map of valid SKU->Items
	 */
	public void onItemDataResponseSuccessful(Map<String, Item> itemData);

	/**
	 * Callback for failed item data response {@link ItemDataRequestStatus#FAILED}.
	 * 
	 * @param requestId
	 */
	public void onItemDataResponseFailed(String requestId);


	/**
	 * Callback on successful purchase response {@link PurchaseRequestStatus#SUCCESSFUL}. 
	 * @param sku
	 */
	public void onPurchaseResponseSuccess(String userId, String sku,
			String purchaseToken);

	/**
	 * Callback when user is already entitled {@link PurchaseRequestStatus#ALREADY_ENTITLED} 
   * to sku passed into initiatePurchaseRequest.
	 * 
	 * @param userId
	 * @param sku
	 */
	public void onPurchaseResponseAlreadyEntitled(String userId, String sku);

	/**
	 * Callback when sku passed into {@link PurchasingManager#initiatePurchaseRequest} is not valid
	 * {@link PurchaseRequestStatus#INVALID_SKU}.
	 * 
	 * @param userId
	 * @param sku
	 */
	public void onPurchaseResponseInvalidSKU(String requestId, String sku);

	/**
	 * Callback on failed purchase response {@link PurchaseRequestStatus#FAILED}.
	 * 
	 * @param requestId
	 * @param sku
	 */
	public void onPurchaseResponseFailed(String requestId, String sku);
	
	/**
	 * Callback on successful purchase updates response {@link PurchaseUpdatesRequestStatus#SUCCESSFUL} 
	 * for each receipt. 
	 * 
	 * @param userId
	 * @param sku
	 * @param purchaseToken
	 */
	public void onPurchaseUpdatesResponseSuccess(String userId, String sku, String purchaseToken);
	
	/**
	 * Callback on successful purchase updates response {@link PurchaseUpdatesRequestStatus#SUCCESSFUL} 
	 * for revoked SKU. 
	 * 
	 * @param userId
	 * @param revokedSKU
	 */
	public void onPurchaseUpdatesResponseSuccessRevokedSku(String userId, String revokedSku);

	/**
	 * Callback on failed purchase updates response {@link PurchaseUpdatesRequestStatus#FAILED}
	 * 
	 * @param requestId
	 */
	public void onPurchaseUpdatesResponseFailed(String requestId);

}
