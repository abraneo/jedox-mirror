/**
 * 
 */
package com.jedox.etl.core.util;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLSession;

/**
 * @author khaddadin
 *
 */
public class NoHostnameVerifier implements HostnameVerifier {

	@Override
	public boolean verify(String arg0, SSLSession arg1) {
		return true;
	}
	 
 }
