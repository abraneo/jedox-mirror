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
*  
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right 
*   (commercial copyright) has Jedox AG, Freiburg.
*  
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.channels.*;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class FileUtil {

	private static final Log log = LogFactory.getLog(FileUtil.class);	
	
	public static boolean isURL(String filename) {
		try {
			new URL(filename);
		}	
		catch (Exception e) {
			return false;
		}
		return true;
	}	
		
	public static boolean isRelativ(String filename) {
		if (filename != null) {			
			if (isURL(filename)) {
				return false;
			}	
			else {
				File f = new File(filename);
				return  ! f.isAbsolute();
			}
		}
		return true;
		// Does not handle Network drives on Windows.
		/*if (filename != null && (filename.startsWith(File.separator) || filename.charAt(1) == ':')) {
		}
			return false;
		}
		return true;*/
	}
	
	public static boolean isAbsolute(String filename) {
		return !isRelativ(filename); 
	}
		
	public static boolean fileExists(String filename) {
		try {
			new URL(filename).openStream();
			return true;
		}
		catch (Exception e) {
			return new File(filename).exists();
		}
	}
	
	public static void test(String filename, boolean warnIfNotExist) throws RuntimeException {
		if (isURL(filename)) {
			try {
				URL url = new URL(filename);				
				url.openStream();
			}	
			catch (Exception e) {				
				throw new RuntimeException ("Could not connect to URL "+filename);
			}
		}
		else {
			if (!(new File(filename).exists())) {
				if (warnIfNotExist)
					log.warn("File " + filename + " does not exist. If this file is created in a Load, the warning can be ignored.");
				else
					throw new RuntimeException("File does not exist: "+filename);				
			}
		}
	}	
		
	public static InputStream getInputStream(String filename) throws MalformedURLException, IOException {
		InputStream s = null;
		if (isURL(filename)) {
			URLConnection urlc = new URL(filename).openConnection();
			s = urlc.getInputStream();			
		}	
		else
			s = new File(filename).toURI().toURL().openStream();
		return s;
	}
/*	
	public static URL stringToUrl(String locator) {
		URL url = null;
		try {
			url = new URL(locator);
		}
		catch (Exception e) {
			log.error("Failed to build url from String: "+locator+": "+e.getMessage());
			log.debug(e);
		}
		return url;
	}
*/	
	
	public static String readStreamAsString(InputStream in) throws java.io.IOException {
		StringBuffer fileData = new StringBuffer(1000);
		BufferedReader reader = new BufferedReader(new InputStreamReader(in,"UTF8"));
		char[] buf = new char[1024];
		int numRead = 0;
		while ((numRead = reader.read(buf)) != -1) {
			String readData = String.valueOf(buf, 0, numRead);
			fileData.append(readData);
			buf = new char[1024];
		}
		reader.close();
		return fileData.toString();
	}
	
	 public static void copyFile(File in, File out) throws IOException {
	     FileChannel inChannel = new
	         FileInputStream(in).getChannel();
	     FileChannel outChannel = new
	         FileOutputStream(out).getChannel();
	     try {
	         inChannel.transferTo(0, inChannel.size(),
	                 outChannel);
	     } 
	     catch (IOException e) {
	         throw e;
	     }
	     finally {
	         if (inChannel != null) inChannel.close();
	         if (outChannel != null) outChannel.close();
	     }
	 }
	 
	 static public boolean deleteDirectory(File path) {
	    if( path.exists() ) {
	      File[] files = path.listFiles();
	      for(int i=0; i<files.length; i++) {
	         if(files[i].isDirectory()) {
	           deleteDirectory(files[i]);
	         }
	         else {
	           files[i].delete();
	         }
	      }
	    }
	    return( path.delete() );
	 }
	 
	 static public boolean deleteInDirectory(File path, String filesExp) {
		    if( path.exists() ) {
		      File[] files = path.listFiles();
		      for(int i=0; i<files.length; i++) {
		    	  if(files[i].getName().matches(filesExp)){
		    		 if(files[i].isDirectory()) {
		        	 deleteDirectory(files[i]);
		         	}
		         	else {
		        	 files[i].delete();
		         	}
		    	  }
		      }
		    }
		    return true;
		 }
	 
	 public static void rename (String oldFilename, String newFilename) {
		File file = new File(oldFilename);
		file.renameTo(new File(newFilename));	 
	 }

	 public static void delete (String filename) {
		new File(filename).delete();
	 }
	 
	 public static BufferedReader getBufferedReader (String filename,String encoding) throws FileNotFoundException, UnsupportedEncodingException {
		 return new BufferedReader(new InputStreamReader(new FileInputStream(filename), encoding));		 
	 }
	 
}
