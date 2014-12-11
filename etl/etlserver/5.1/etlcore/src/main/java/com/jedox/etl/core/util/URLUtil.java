/**
*   @brief <Description of Class>
*
*   @file
*
*   Copyright (C) 2008-2013 Jedox AG
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License (Version 2) as published
*   by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
*
*   This program is distributed in the hope that it will be useful, but WITHOUT
*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
*   more details.
*
*   You should have received a copy of the GNU General Public License along with
*   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
*   Place, Suite 330, Boston, MA 02111-1307 USA
*
*   If you are developing and distributing open source applications under the
*   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
*   ISVs, and VARs who distribute Palo with their products, and do not license
*   and distribute their source code under the GPL, Jedox provides a flexible
*   OEM Commercial License.
**
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util;

import java.io.IOException;
import java.io.InputStream;
import java.net.ConnectException;
import java.net.CookieHandler;
import java.net.CookieManager;
import java.net.CookiePolicy;
import java.net.HttpCookie;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLConnection;
import java.net.UnknownHostException;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.http.NameValuePair;
import org.apache.http.client.utils.DateUtils;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;



public class URLUtil {
	
	private class LogoutUrl extends Thread {
		String logoutUrlStr = null;
		LogoutUrl(String url) {
			this.logoutUrlStr = url;
	    }

	    public void run() {
	    	try {
	    	URL logoutUrl = new URL(this.logoutUrlStr);	
			URLConnection logoutUrlc = logoutUrl.openConnection();
			InputStream logoutis = logoutUrlc.getInputStream();
			logoutis.close();
			log.debug("JedoxWeb logout url is executed correctly.");
	    	} catch (Exception e) {
				log.debug("Error while trying to logout from possible jedox web connection " + logoutUrlStr + " : "+ e.getMessage());
			}
	    
	    }
	}
	
	private static URLUtil instance = null;
	private static CookieManager manager;
	private static final Log log = LogFactory.getLog(URLUtil.class);
	
	private URLUtil(){
	}
	
	public synchronized static final URLUtil getInstance()  {
		if (instance == null) {
			instance = new URLUtil();			
			// Note: If Cookie Manager is created here, a file of Jedox File Manager can be read
			// independent of user credentials after one successful access. Only in Client-Server-Mode  
			
//			manager = new CookieManager();
//			manager.setCookiePolicy(CookiePolicy.ACCEPT_ALL);
//			CookieHandler.setDefault(manager);			
		}
		return instance;
	}
	
	public List<NameValuePair> getParameters(String urlStr, String charset) throws ConfigurationException{
		
		try {
			return org.apache.http.client.utils.URLEncodedUtils.parse(new URI(urlStr),"UTF-8");
		} catch (URISyntaxException e) {
			throw new ConfigurationException(e.getMessage());
		}
		
	}

	public long getResponseLastModified(String urlStr) throws IOException{
		URL url = new URL(urlStr);
		URLConnection conn = url.openConnection();
		java.util.Date d = DateUtils.parseDate(conn.getHeaderField("Last-Modified"));
		return d.getTime();
	}

	public InputStream getInputStream(String urlstring, boolean errorIfExcel, SSLModes sslMode) throws IOException, RuntimeException {	
		URL url = new URL(urlstring);							
	
		if(url.getProtocol().equals("https") && sslMode.equals(SSLModes.trust)){
			try {
				SSLUtil.getInstance().addCertToKeyStore(url);
			}
			catch (Exception e) {
				if (e instanceof UnknownHostException)
					throw new RuntimeException("Host "+url+" is unknown.");
				if (e instanceof ConnectException)
					throw new RuntimeException("Could not connect to host "+url+" : "+e.getMessage());
				if (e instanceof MalformedURLException) 
					throw new RuntimeException("Filename "+url+" is not a legal URL. SSL trust mode is only available for URLs.");
				throw new RuntimeException(e);						
			}
		}
		
		// Create Cookie Manager with default implementations of CookieHandler
		// Necessary for access to Files on Jedox File Manager with user credentials in the URL
		manager = new CookieManager();
		manager.setCookiePolicy(CookiePolicy.ACCEPT_ALL);
		CookieHandler.setDefault(manager);	
		// get input stream		
		URLConnection urlc = url.openConnection();
		InputStream is = urlc.getInputStream();
		try {
			List<HttpCookie> cookies = manager.getCookieStore().getCookies();
			for(HttpCookie cookie:cookies){
				if(cookie.getName().equals("JDX_SID")){
				
						String prefix = urlstring.substring(0,urlstring.indexOf("/ui/lnk/"));
						String logoutUrlStr = prefix + "/ui/login/?out";
						/*URL logoutUrl = new URL(logoutUrlStr);	
						URLConnection logoutUrlc = logoutUrl.openConnection();
						InputStream logoutis = logoutUrlc.getInputStream();
						logoutis.close();*/
						// since with https, getInputStream method hangs, 
						// probably some resources are locked from the original URL
						// the logout will happen in a different thread
						// so that it will not block the main Thread
						// After testing i saw that the logout will complete
						// successfully when the extract is finished
						new LogoutUrl(logoutUrlStr).start();
				}
			}
		} catch (Exception e) {
			log.debug("Error while trying to logout from possible jedox web connection " + urlstring + " : "+ e.getMessage());
		}
		if(errorIfExcel) 
			FileUtil.checkNotExcel(getFilenameFromUrl(urlc));
		
		manager=null;  // reset Cookie Manager		
		return is;			
	}
	
	public String getFilenameFromUrl(URLConnection urlc) {
		try{
			String headerField = urlc.getHeaderFields().get("Content-Disposition").get(0);
			return (headerField.split(";")[1]).split("=")[1].trim();
		}catch(Exception e){
			// do nothing, not URL may have this
			return "";
		}		
	}
	

}
