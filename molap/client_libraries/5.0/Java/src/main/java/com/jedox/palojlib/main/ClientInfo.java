/**
 * 
 */
package com.jedox.palojlib.main;

/**
 * @author khaddadin
 *
 */
public class ClientInfo {
	
	private final String client;
	private final String clientDesc;
	
	public ClientInfo(String client,String clientDesc){
		this.client = client;
		this.clientDesc = clientDesc;
	}

	public String getClient() {
		return client;
	}

	public String getClientDesc() {
		return clientDesc;
	}
	

}
