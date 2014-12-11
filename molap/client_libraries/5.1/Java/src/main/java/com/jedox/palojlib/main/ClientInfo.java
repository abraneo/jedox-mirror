/**
 * 
 */
package com.jedox.palojlib.main;

/**
 * Information about the client connecting to olap
 * @author khaddadin
 *
 */
public class ClientInfo implements Cloneable{
	
	private final String client;
	private final String clientDesc;
	
	/**
	 * Constructor to build a ClientInfo object
	 * @param client client name
	 * @param clientDesc client descriptor
	 */
	public ClientInfo(String client,String clientDesc){
		this.client = client;
		this.clientDesc = clientDesc;
	}

	/**
	 * get the client name, e.g. ETL
	 * @return client name
	 */
	public String getClient() {
		return client;
	}

	/**
	 * get the client description, e.g. Jedox ETL server 
	 * @return client description
	 */
	public String getClientDesc() {
		return clientDesc;
	}
	
	protected Object clone() throws CloneNotSupportedException {
        return super.clone();
    }
	

}
