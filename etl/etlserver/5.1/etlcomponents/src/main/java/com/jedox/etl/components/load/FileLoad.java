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

import java.io.FileNotFoundException;
import java.io.UnsupportedEncodingException;
import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.load.FileConfigurator;
import com.jedox.etl.components.connection.FileConnection;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IFileConnection;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.writer.IFileWriter;

public class FileLoad extends Load {

	private static final Log log = LogFactory.getLog(FileLoad.class);
	private boolean ignoreEmpty;
	private int skipLines;

	public FileLoad() {
		setConfigurator(new FileConfigurator());
	}

	public FileConfigurator getConfigurator() {
		return (FileConfigurator)super.getConfigurator();
	}

	//get connection for writing.
	public IFileConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IFileConnection)) {
			IFileConnection c = (IFileConnection) connection;
			return c;
		}
		throw new RuntimeException("File connection is needed for load "+getName()+".");
	}

	private void writeFromProcessor(IFileWriter writer, boolean deleteEmpty) throws RuntimeException {
		writer.writeEmptyLines(skipLines);		
		writer.write(getProcessor());
		if (deleteEmpty && writer.getLinesOut()==0 ) {
			FileUtil.delete(getConnection().getDatabase());					
			log.info("File "+getConnection().getDatabase()+" is not created. Source is empty.");
		}
		else {			
			log.info("Lines written to file "+getConnection().getDatabase()+": "+writer.getLinesOut());
		}	
	}

	private void writeInExistingFile(boolean insert) throws RuntimeException, FileNotFoundException, UnsupportedEncodingException {
		IFileWriter writer =  null;
		try {
			log.info((insert?"Inserting":"Updating")+" data in file "+getConnection().getDatabase()+" at line "+skipLines);
			String tmpfile = getConnection().getDatabase()+"_temp"+new Date().hashCode();
			log.debug("Creating temporary file "+tmpfile);
			FileUtil.rename(getConnection().getDatabase(),tmpfile);		

			writer = getConnection().getWriter(true);
			writer.setAutoClose(false);
			String encoding = ((FileConnection)getConnection()).getFileEncoding();
			// Write first part of existing file
			int lines = writer.writeFromReader(FileUtil.getBufferedReader(tmpfile,encoding),1,skipLines);
			// Add empty rows if necessary
			if (lines<=skipLines)
				writer.writeEmptyLines(skipLines-lines+1);
			// Write data from processor
			writer.write(getProcessor());
			log.info("Lines written to file "+getConnection().getDatabase()+": "+writer.getLinesOut());
			// Append remaining part of existing file
			if (insert) {
				writer.writeFromReader(FileUtil.getBufferedReader(tmpfile,encoding),skipLines+1,Integer.MAX_VALUE);
			}		
			FileUtil.delete(tmpfile);
			log.debug("Deleted temporary file "+tmpfile);
		}finally{
			if(writer!=null)
				writer.close();
		}
	}
		
	@Override
	public void executeLoad() {
		IFileWriter writer = null;
		try {				
			log.info("Starting File load "+getName());
			boolean deleteEmpty = ignoreEmpty && !FileUtil.fileExists(getConnection().getDatabase());
			switch (getMode()) {
			case CREATE: {
				writer = getConnection().getWriter(false);
				writeFromProcessor(writer,deleteEmpty);
				break;
			}
			case ADD : {
				writer = getConnection().getWriter(true);
				writeFromProcessor(writer,deleteEmpty);
				break;
			}
			case INSERT : {
				if (FileUtil.fileExists(getConnection().getDatabase()))
					writeInExistingFile(true);
				else{
					writer = getConnection().getWriter(false);
					writeFromProcessor(writer,deleteEmpty);
				}
				break;
			}
			case UPDATE : {
				if (FileUtil.fileExists(getConnection().getDatabase()))
					writeInExistingFile(false);
				else{
					writer = getConnection().getWriter(false);
					writeFromProcessor(writer,deleteEmpty);
				}
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
		}finally{
			if(writer!=null)
				writer.close();
		}
		log.info("Finished load "+getName());
	}
	
	public void test() throws RuntimeException {
		super.test();
		if (hasConnection())
			getConnection().test();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			ignoreEmpty=getConfigurator().getIgnoreEmpty();			
			skipLines = getConfigurator().getSkipLines();
		} catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
