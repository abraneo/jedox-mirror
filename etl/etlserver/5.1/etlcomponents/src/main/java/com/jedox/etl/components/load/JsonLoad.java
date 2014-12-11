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

package com.jedox.etl.components.load;


import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.Date;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.load.JsonConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.writer.JsonWriter;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IJsonConnection;
import com.jedox.etl.core.load.Load;

public class JsonLoad extends Load {

	private static final Log log = LogFactory.getLog(JsonLoad.class);
	private String customRoot = null;

	public JsonLoad() {
		setConfigurator(new JsonConfigurator());
	}

	public JsonConfigurator getConfigurator() {
		return (JsonConfigurator)super.getConfigurator();
	}

	protected String getEncoding() throws RuntimeException {
		if (getConnection() != null) {
			return getConnection().getEncoding();
		}
		return null;
	}

	//get connection for writing.
	public IJsonConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IJsonConnection)) {
			IJsonConnection c = (IJsonConnection) connection;
			return c;
		}
		throw new RuntimeException("Json File connection is needed for load "+getName()+".");
	}


	@Override
	public void executeLoad() {
		
		try {
			log.info("Starting Json load "+getName());
			switch (getMode()) {
			case CREATE: {
				writeFromProcessorAsString(false);
				break;
			}
			case ADD : {
				writeFromProcessorAsString(true);	
				break;
			}

			default: {
				log.error("Unsupported mode "+getMode().toString()+" in load "+getName()+" of type File.");
			}
			}
		}
		catch (Exception e) {
			log.error("Failed to write to file: "+e.getMessage());
			log.debug("",e);
		}
		log.info("Finished load "+getName());
	
	}

	private void writeFromProcessorAsString(boolean insert) throws RuntimeException, IOException {
		File file = new File(getConnection().getDatabase());				
		JsonWriter writer = null;
		
		if(!insert || !file.exists()){	
			FileOutputStream fWriter= new FileOutputStream(file);
			BufferedWriter  bufferedWriter = new BufferedWriter(new OutputStreamWriter(fWriter,getConnection().getFileEncoding()));
			writer = new JsonWriter(bufferedWriter);
			writer.setCustomRoot(this.customRoot);
			writer.write(getProcessor());
		}else{
			String tmpfile = getConnection().getDatabase()+"_temp"+new Date().hashCode();
			log.debug("Creating temporary file "+tmpfile);
			FileUtil.rename(getConnection().getDatabase(),tmpfile);	
			FileOutputStream fWriter= new FileOutputStream(file);
			BufferedWriter  bufferedWriter = new BufferedWriter(new OutputStreamWriter(fWriter,getConnection().getFileEncoding()));
			writer = new JsonWriter(bufferedWriter,tmpfile);
			writer.setCustomRoot(this.customRoot);
			try {
				writer.write(getProcessor());
			} catch (Exception e) {
				file.delete();
				FileUtil.rename(tmpfile,getConnection().getDatabase());	
				throw new RuntimeException(e.getMessage());
			}finally{
				FileUtil.delete(tmpfile);
				log.debug("Deleted temporary file "+tmpfile);
			}
		}
		if (writer.getLinesOut()==0)		
			log.info("No lines are written to file "+getConnection().getDatabase()+". Source is empty.");
		else
			log.info("Lines written to file "+getConnection().getDatabase()+": "+writer.getLinesOut());		
	}



	public void test() throws RuntimeException {
		super.test();
		if (hasConnection())
			getConnection().test();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			customRoot = getConfigurator().getRootName();
		} catch (ConfigurationException e) {
			throw new InitializationException(e);
		}
	}

}
