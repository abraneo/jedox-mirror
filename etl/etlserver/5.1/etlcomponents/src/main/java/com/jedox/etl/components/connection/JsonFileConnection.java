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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.components.connection;

import java.io.File;
import java.io.InputStreamReader;
import java.util.Properties;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.IJsonConnection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.Recoder;
import com.jedox.etl.core.util.SSLUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;


public class JsonFileConnection extends Connection implements IJsonConnection{

	//private static final Log log = LogFactory.getLog(JsonFileConnection.class);
	private String filename;
	private SSLModes sslMode;
	
	protected void setFilename(String filename) {
		this.filename = filename;
	}

	protected String getFilename() {
		return filename;
	}
	
	public String getDatabase() {
		String database = super.getDatabase();
		if (getHost() == null && FileUtil.isRelativ(database))
			return FileUtil.convertPath(Settings.getInstance().getDataDir() + "/" + database);
		else	
			return database;
	}
	
	protected String getDatabaseString() {
		return super.getDatabase();
	}

	/**
	 * initialize the connection by setting the file name
	 */
	public void init() throws InitializationException {
		try {
			super.init();
			setFilename(getDatabase()); //Relative();
			sslMode = SSLModes.valueOf(getParameter("ssl",SSLUtil.SSLModes.verify.toString()));			
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
	
	public String getEncoding() {
		//Data is internally stored in utf8 database no matter what the original file encoding was.
		return Recoder.internalCoding;
	}
	
	public String getFileEncoding() {
		return super.getEncoding();
	}


	/**
	 * open a connection to the XML file and parse it using DOM
	 * @return parsed XML document
	 */
	public InputStreamReader open() throws RuntimeException{
		
		try {
			return new InputStreamReader(FileUtil.getInputStream(getFilename(), true, sslMode),getFileEncoding());
			//return FileUtil.getInputStream(getFilename(), true, sslMode);
		} catch (Exception e) {
			String filePath = (FileUtil.isURL(getFilename())?filePath = getFilename():new File(getFilename()).getAbsolutePath());
			throw new RuntimeException("Failed to read file "+ filePath +": "+e.getMessage());
		}

	}

	@Override
	public void close() {
		filename=null;
	}
	
	public String getMetadata(Properties properties) throws RuntimeException {
		throw new RuntimeException("Not implemented in "+this.getClass().getCanonicalName());
	}

	public MetadataCriteria[] getMetadataCriterias() {
		return null;
	}

}

