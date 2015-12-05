package com.moonlite.tuxracer;

import java.util.HashSet;
import java.util.Set;

public enum MySKU {

    COURSE_PACK("com.moonlite.tuxracer.course_pack");

	private String sku;

	private MySKU(String sku) {
		this.sku = sku;
	}

	public static MySKU valueForSKU(String sku) {
		if (COURSE_PACK.getSku().equals(sku)) {
			return COURSE_PACK;
		}
		return null;
	}

	public String getSku() {
		return sku;
	}

	private static Set<String> SKUS = new HashSet<String>();
	static {
		SKUS.add(COURSE_PACK.getSku());
	}

	public static Set<String> getAll() {
		return SKUS;
	}

}
