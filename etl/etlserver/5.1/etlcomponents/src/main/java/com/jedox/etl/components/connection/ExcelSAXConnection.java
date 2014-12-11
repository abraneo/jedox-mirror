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

import java.io.InputStream;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.poi.openxml4j.opc.OPCPackage;
import org.apache.poi.poifs.crypt.Decryptor;
import org.apache.poi.poifs.crypt.EncryptionInfo;
import org.apache.poi.poifs.filesystem.NPOIFSFileSystem;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.util.FileUtil;

public class ExcelSAXConnection extends ExcelConnection {
	
	
	private boolean memoryOptimised=true;	
	private static final Log log = LogFactory.getLog(ExcelSAXConnection.class);

	@Override
	public Object open() throws RuntimeException {
		if (!memoryOptimised) {
			return super.open();
		}
		try {
			if (pkg == null) {
				try{
					return  OPCPackage.open(getDatabase());
				}
				catch (Exception e) {
					try {
		
						InputStream inp = FileUtil.getInputStream(getDatabase(), false, sslMode);
						String password = getParameter("password","");
						if(!password.isEmpty()){
							NPOIFSFileSystem fs = new NPOIFSFileSystem(inp);
							EncryptionInfo info = new EncryptionInfo(fs);
							Decryptor d = Decryptor.getInstance(info);
							if (!d.verifyPassword(password)) {
					            throw new RuntimeException("password is incorrect.");
					        }
							inp = d.getDataStream(fs);
						}
						return OPCPackage.open(inp);
					} catch (Exception e1) {
						throw new RuntimeException("Failed to open workbook: "+e1.getMessage());
					}
				}
		}
	
			
			return pkg;
		} catch (Exception e) {
			// Try to open it with the super class
			Object conn = super.open();
			log.info("MemoryOptimised=true can only be used with *.xslx excel format. The connection will be opened with MemoryOptimised=false.");
			memoryOptimised = false;
			return conn;
		}
	}
	
	public boolean isMemoryOptimised() {
		return memoryOptimised;
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			memoryOptimised = getParameter("memoryOptimised","true").equalsIgnoreCase("true");
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
