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
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.UnsupportedEncodingException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jasypt.util.text.BasicTextEncryptor;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;
import java.io.File;

/**
 * Persistence backend for the {@link ConfigManager} to persist all registered configurations to disk.
 * A file is created in the projects directory, which is defined in the config.xml
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class FileConfigPersistence implements IConfigPersistence {

	private static final Log log = LogFactory.getLog(FileConfigPersistence.class);
	private static FileConfigPersistence instance;
	protected static String repositoryFileName = "repository.xml";
	protected static String olapPersistenceRepositoryFileName = "etl_fallback_repository.xml";

	/**
	 * gets the instance of this persistence backend.
	 * @return this persistence backend
	 */
	public synchronized static FileConfigPersistence getInstance() {
		if(instance == null){
			instance = new FileConfigPersistence();
		}
		return instance;
	}

	/**
	 * loads the repository file from the projects directory.
	 * @return the loaded repository document or a new repository document if none exists.
	 */
	public void load() throws Exception {
		Document document = null;
		String projectDir = Settings.getInstance().getProjectDir();
		XMLReader reader = new XMLReader();
		try {
			log.debug("Loading project repository.");
			String filename=projectDir+File.separator+repositoryFileName;
			if (!new File(filename).exists()) {
				log.debug("No project repository found, starting with new one");
				return;
			}			
			if(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("repositoryEncryption").equals("true")){
				// the file with the encrypted text is read ==> decrypted ==> parsed as xml
				File ef = new File(filename);
				InputStreamReader in = new InputStreamReader(new FileInputStream(ef),"UTF-8");
				char [] content = new char[(int) ef.length()];
				in.read(content);
				in.close();
				document =  dycryptToXML(new String(content));

			}else {
				if (new File(filename).exists())
				document = reader.readDocument(filename);
			}
			ConfigConverter converter = new ConfigConverter();
			//so that older versions (before 4.1)could also be read
			if(document.getRootElement().getName().equals("projects")){
				for(Object project:document.getRootElement().getChildren()){
					Element projectElement = (Element) project;
					converter.convert(projectElement);
					ConfigPersistor.getInstance().addProject(projectElement.getAttributeValue("name"), projectElement);	
				}

			}else{
				for(Object project:document.getRootElement().getChild("projects").getChildren()){
					Element projectElement = (Element) project;
					converter.convert(projectElement);					
					ConfigPersistor.getInstance().addProject(projectElement.getAttributeValue("name"), projectElement);	
				}
				for(Object group:document.getRootElement().getChild("groups").getChildren()){
					Element groupElement = (Element) group;
					ConfigPersistor.getInstance().addGroup(groupElement.getAttributeValue("name"), groupElement);	
				}
			}
		}
		catch (Exception e) {
			throw new Exception("Error while reading ETL repository: " + e.getMessage());
		}
	}

	
	private Document getCurrentDocument() {
		Document document = new Document();
		Element configsElement = new Element("configs");
		Element groupsElement = new Element("groups");
		Element projectsElement = new Element("projects");
		configsElement.addContent(groupsElement);
		configsElement.addContent(projectsElement);
		document.setRootElement(configsElement);
		//document.getRootElement().setAttribute("version", String.valueOf(ConfigConverter.currentVersion));
		for(Element proj : ConfigPersistor.getInstance().getProjects().values()){
			document.getRootElement().getChild("projects").addContent(proj.detach());
		}
		for(Element group : ConfigPersistor.getInstance().getGroups().values()){
			document.getRootElement().getChild("groups").addContent(group.detach());
		}
		return document;
	}
	
	
	/**
	 * saves the a a project to disk to a file called repositoryFileName in the projects directory.
	 * @param document the document to save
	 * @throws Exception 
	 */
	public void save(Locator loc, Element config) throws Exception {
		//update Maps
		ConfigPersistor.getInstance().updateMaps(loc,config);
		
		if (!loc.getManager().equals(NamingUtil.group_manager)) {
			// remove the project from the groups as well			
			ConfigPersistor.getInstance().updateProjectInGroup(loc,(config==null?null:config.getAttributeValue("name")));
		}	
		saveToFile();
	}

	public void backupToFile() throws Exception {
		switchToFallBack();
		saveToFile();
	}

	/**
	 * 
	 */
	public void switchToFallBack() {
		if(!repositoryFileName.equals(olapPersistenceRepositoryFileName)){
			String projectDir = Settings.getInstance().getProjectDir();
			File ef = new File(projectDir+File.separator+repositoryFileName);
			ef.deleteOnExit();
			repositoryFileName = olapPersistenceRepositoryFileName;
		}
	}
	
	
	/**
	 * @param document
	 * @throws IOException
	 * @throws UnsupportedEncodingException
	 * @throws FileNotFoundException
	 */
	private void saveToFile() throws Exception {
		
		Document document=getCurrentDocument();

		String projectDir = Settings.getInstance().getProjectDir();
		// Try to create project directory if not existing
		File projectDirFile = new File(projectDir);
		if (!projectDirFile.exists()) {
			log.debug("Creating project directory "+projectDir);			
			projectDirFile.mkdirs();
		}
		
		//backup old status of the reporsitory
		File file = new File(projectDir+ File.separator +repositoryFileName);
		if(file.exists()){
			File backupFile = new File(projectDir+ File.separator +repositoryFileName.replace(".xml", ".backup"));
			FileUtil.copyFile(file, backupFile);
		}
		
		if(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("repositoryEncryption").equals("true")){

			String encrypted = encrypt(document);
			FileWriter fw = new FileWriter(projectDir+ File.separator +repositoryFileName);
			// Write strings to the file
			fw.write(encrypted);
			fw.close();
		}else{
			OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream(projectDir + File.separator +repositoryFileName),"UTF8");
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

	
	public boolean needMigration() {
		return false;
	}
	
	public void migrate (IConfigPersistence oldPersistence) throws Exception {
		// No migration to File persistence
		return;
	}
	
}