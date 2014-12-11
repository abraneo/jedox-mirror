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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.config.Settings;

public class CustomClassLoader extends URLClassLoader {
	
	private static Log log = LogFactory.getLog(CustomClassLoader.class);
	
	private static CustomClassLoader instance;

	private CustomClassLoader(URL[] urls) {
		super(urls, CustomClassLoader.class.getClassLoader());
	}
	
	public static CustomClassLoader getInstance() {
		if (instance == null) {
			ArrayList<URL> urls = new ArrayList<URL>();
			File f = new File(Settings.getInstance().getCustomlibDir());
			if (f.isDirectory()) {
				for (File jarfile: f.listFiles()) {
					if (jarfile.getAbsolutePath().endsWith(".jar")) {
						try {
							urls.add(jarfile.toURI().toURL());
							log.debug("Found custom extension: " + jarfile.getAbsolutePath());
						} catch (MalformedURLException e) {
							log.debug("Malformed url: " + e.getMessage());
						}
					}
				}
			} else {
				log.debug(f.getAbsolutePath() + " is not a directory.");
			}
			instance = new CustomClassLoader(urls.toArray(new URL[urls.size()]));
		}
		return instance;
	}

}
