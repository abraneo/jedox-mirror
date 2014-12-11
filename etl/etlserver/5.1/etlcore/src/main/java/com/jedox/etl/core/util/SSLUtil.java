package com.jedox.etl.core.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;

import java.util.Properties;


public class SSLUtil {

	public static enum SSLModes {
		verify, trust, off
	}
	
	private String storeLocation;
	private String storePassword;
	private KeyStore ks;
	private static final Log log = LogFactory.getLog(SSLUtil.class);
	private static SSLUtil instance = null;
	
	public static SSLUtil getInstance() throws RuntimeException{
		if(instance==null)
			instance = new SSLUtil();
		
		return instance;
	}
	 
	private SSLUtil() throws RuntimeException{
		try {
			String storeConfigPath = Settings.getConfigDir()+File.separator+"ssl.properties";
			Properties storeProperties = new Properties();
			storeProperties.load(new FileInputStream(storeConfigPath));
			storeLocation = Settings.getRootDir()+File.separator+storeProperties.getProperty("javax.net.ssl.keyStore");
			storePassword = storeProperties.getProperty("javax.net.ssl.keyStorePassword");
			ks = getKeystore(storeLocation,storePassword);
		}
		catch (Exception e) {
			throw new RuntimeException("Cannot access etl keystore: "+e.getMessage());
		}
	}
	
	public synchronized void addCertToKeyStore(URL url) throws Exception {
		 URLConnection c = url.openConnection();
		 if (c instanceof HttpsURLConnection) {
			 //use costum trust manager to be able to connect without the certs already in store
			 
			 TrustManager trm = new X509TrustManager() {
				 public X509Certificate[] getAcceptedIssuers() {
					 return null;
				 }

				 public void checkClientTrusted(X509Certificate[] certs, String authType) {
				 }

				 public void checkServerTrusted(X509Certificate[] certs, String authType) {
				 }
				 
			 };
			 
	
			 SSLContext sc = SSLContext.getInstance("SSL");
			 sc.init(null, new TrustManager[] { trm }, null);
			 SSLSocketFactory factory =sc.getSocketFactory();
			 int port = url.getPort();
			 if (port == -1) port = url.getDefaultPort();
			 java.security.cert.Certificate[] certs;
			 try {                              
				 //HttpsURLConnection.setDefaultHostnameVerifier(new NoHostnameVerifier());
				 HttpsURLConnection connection = (HttpsURLConnection)url.openConnection();
				 connection.setSSLSocketFactory(factory);
				 connection.connect();
				 certs = connection.getServerCertificates();
			 }
			 catch (Exception e) {
				 SSLSocket socket =(SSLSocket)factory.createSocket(url.getHost(), port);
				 socket.startHandshake();
				 SSLSession session = socket.getSession();
				 //get Certificates and add them into keystore.
				 certs = session.getPeerCertificates();
			 }
			 
			 int i=0;
			 for (Certificate cert : certs) {
				 ks.setCertificateEntry(url.getHost()+i, cert);
				 i++;
			 }
			 saveKeystore();
			 //reinit Factories with modified keystore to be able to access online modification
			 TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
			 tmf.init(ks);
			 KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
			 kmf.init(ks, storePassword.toCharArray());
			 //reinit the SSL Context with the updated factories
			 sc.init(kmf.getKeyManagers(), tmf.getTrustManagers(), null);
			 //assign the updated SSL Context to all future HttsULRConnections
			 HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
			 //HttpsURLConnection.setDefaultHostnameVerifier(new NoHostnameVerifier());
		 }
		 else {
			 log.warn("SSL mode is set to "+SSLModes.trust.toString()+" but url does not support ssl: "+url.toExternalForm());
		 }
		 
	}
	
	private KeyStore getKeystore(String storeLocation, String storePassword) throws Exception {
		KeyStore ks = KeyStore.getInstance("JKS");  
		/* 
		 * LOAD THE STORE 
		 * The first time you're doing this (i.e. the keystore does not 
		 * yet exist - you're creating it), you HAVE to load the keystore 
		 * from a null source with null password. Before any methods can 
		 * be called on your keystore you HAVE to load it first. Loading 
		 * it from a null source and null password simply creates an empty 
		 * keystore. At a later time, when you want to verify the keystore 
		 * or get certificates (or whatever) you can load it from the 
		 * file with your password. 
		 */  
		if (new File(storeLocation).exists())
			ks.load( new FileInputStream(storeLocation), storePassword.toCharArray() ); 
		else ks.load(null,null);
		return ks;
	}
	
	
	public void saveKeystore() throws Exception {
		//SAVE THE KEYSTORE TO A FILE  
		/* 
		 * After this is saved, I believe you can just do setCertificateEntry 
		 * to add entries and then not call store. I believe it will update 
		 * the existing store you load it from and not just in memory. 
		 */ 
		FileOutputStream os  = new FileOutputStream( storeLocation);
		ks.store(os , storePassword.toCharArray() );
		os.flush();
		os.close();
	}
	
}
