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
package com.jedox.etl.core.config;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Reader;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jasypt.util.text.BasicTextEncryptor;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.util.XMLUtil;

import java.io.File;

/**
 * Persistence backend for the {@link ConfigManager} to persist all registered configurations to disk.
 * A file called "repository.xml" is created in the projects directory, which is defined in the config.xml
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ConfigPersistence {

	private static final ConfigPersistence instance = new ConfigPersistence();
	private static final Log log = LogFactory.getLog(ConfigPersistence.class);

	ConfigPersistence() {}

	/**
	 * gets the instance of this persistence backend.
	 * @return this persistence backend
	 */
	public synchronized static ConfigPersistence getInstance() {
		return instance;
	}

	/**
	 * creates a new document for the storage of configurations
	 * @return a new repository document
	 */
	public Document create() {
		Document document = new Document();
		document.setRootElement(new Element("projects"));
		document.getRootElement().setAttribute("version", String.valueOf(ConfigConverter.currentVersion));
		return document;
	}

	/**
	 * loads the repository.xml from the projects directory.
	 * @return the loaded repository document or a new repository document if none exists.
	 */
	public Document load() {
		Document document = null;
		String projectDir = Settings.getInstance().getProjectDir();
		XMLReader reader = new XMLReader();
		try {
			log.debug("Loading project repository.");

			if(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("repositoryEncryption").equals("true")){
				// the file with the encrypted text is read ==> decrypted ==> parsed as xml
				File ef = 	new File(projectDir+File.separator+"repository.xml");
				InputStreamReader in = new InputStreamReader(new FileInputStream(ef),"UTF-8");
				char [] content = new char[(int) ef.length()];
				in.read(content);
				in.close();
			    document =  dycryptToXML(new String(content));

			}else{
				document = reader.readDocument(projectDir+File.separator+"repository.xml");
			}
			return new ConfigConverter().convert(document);
		}
		catch (Exception e) {
			log.debug("No project repository found, starting with new one:" + e.getMessage());
			document = create();
		}
		return document;
	}

	/**
	 * saves the a repository document to disk to a file called "repository.xml" in the projects directory.
	 * @param document the document to save
	 */
	public void save(Document document) throws IOException {
		String projectDir = Settings.getInstance().getProjectDir();
		// Try to create project directory if not existing
		File projectDirFile = new File(projectDir);
		if (!projectDirFile.exists()) {
			log.debug("Creating project directory "+projectDir);			
			projectDirFile.mkdirs();
		}
		if(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("repositoryEncryption").equals("true")){

			String encrypted = encrypt(document);
			FileWriter fw = new FileWriter(projectDir+"/repository.xml");
		    // Write strings to the file
		     fw.write(encrypted);
		     fw.close();
		}else{
			OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream(projectDir+"/repository.xml"),"UTF8");
			XMLOutputter outputter = new XMLOutputter(Format.getPrettyFormat());
			outputter.output(document, writer);
			writer.flush();
			writer.close();
		}
	}

	private Document dycryptToXML(String xmlAsText) throws ConfigurationException{
		BasicTextEncryptor crypt = new BasicTextEncryptor();
		crypt.setPassword(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("password"));
		String decryptedRepository = crypt.decrypt(xmlAsText);
		Reader r = new BufferedReader(new InputStreamReader((new ByteArrayInputStream(decryptedRepository.getBytes()))));
		XMLReader reader = new XMLReader();
		return reader.readDocument(r);
	}

	private String encrypt(Document doc) throws IOException{
		BasicTextEncryptor crypt = new BasicTextEncryptor();
		crypt.setPassword(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("password"));
		return crypt.encrypt(XMLUtil.jdomToString(doc.getRootElement()));
	}

	/**
	 * saves the a repository document to disk to a file called "repository.xml" in the projects directory only if the autosave property is activated in the config.xml.
	 * @param document the document to autosave
	 */
	public void autoSave(Document document) throws IOException {
		// boolean autosave = Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("autosave","true").equalsIgnoreCase("true");
		if (Settings.getInstance().getAutoSave())
			save(document);
	}


}
