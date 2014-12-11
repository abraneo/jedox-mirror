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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;

import java.io.File;
import java.io.FileFilter;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.UnionProcessor;

public class DirectoryConnection extends FileConnection {

	//private boolean consume;
	private String regexp;
	private static final Log log = LogFactory.getLog(DirectoryConnection.class);
	
	private class Filter implements FileFilter {

		private Pattern pattern;

		public Filter(String regexp) {
			pattern = Pattern.compile(regexp);
		}

		public boolean accept(File pathname) {
			Matcher m = pattern.matcher(pathname.getName());
			return m.find();
		}
	}

	public void close() {
		super.close();
/*		File dir = new File(getDatabase());
		if (consume && dir.exists()) {
			File[] files = dir.listFiles(new Filter(regexp));
			for (File f: files)
				f.delete();
		}
*/	}

	private File[] getFiles() throws RuntimeException {
		File[] files;
		File dir = new File(getDatabase());
		if (dir.exists()) {
			files = dir.listFiles(new Filter(regexp));
			if (files==null || files.length == 0) {
				log.warn("No matching files for pattern '"+regexp+"' exist in directory "+getFilename());
				return new File[0];
			}	
			Arrays.sort(files, new Comparator<File>(){
			    public int compare(File f1, File f2)
			    {
			        return f1.getName().compareTo(f2.getName());
			    } });
		}
		else {
			throw new RuntimeException("Directory does not exist: "+getFilename());
		}
		return files;
	}
		
	protected boolean isCached() throws RuntimeException {
		/*		File[] files = getFiles();
		if (files.length == 0)
			return false;
		for (File f : files) {
			if (f.lastModified() > getTimestamp())
				return false;
		}
		return true;
*/
		return false;
	}

	protected String getFilename() {
		return getDatabase();
	}

	protected IProcessor getFileProcessor(IAliasMap aliasMap, int size) throws RuntimeException {
		File[] files = getFiles();
		log.info("Extracting "+files.length+" files in directory "+getDatabase());
		ArrayList<IProcessor> processors = new ArrayList<IProcessor>();
		size = (size == 0) ? getLastRow() : size;
		for (File f : files) {
			IProcessor processor = new FileProcessor(f.getAbsolutePath(),getFileEncoding(),aliasMap);
			// Configuration of first and last row relates to individual files, not to the union of files 
			processor.setFirstRow(getFirstRow());
			processor.setLastRow(Math.min(getLastRow(),getFirstRow()+size));
			processor.current().setAliases(aliasMap);
			processors.add(processor);
		}
		IProcessor union = UnionProcessor.getInstance(processors);
		return union;
	}
	
	public void test() throws RuntimeException {
		getFiles();
	}
		
	public void init() throws InitializationException {
		super.init();
		try {
			regexp = getParameter("pattern",".");
			//consume = getParameter("consume","false").equalsIgnoreCase("true");
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

	private StringWriter getFilesWriter() throws RuntimeException{
		File[] files = getFiles();
		StringWriter out = new StringWriter();
		PrintWriter writer = new PrintWriter(out);
		try {
			writer.println("Filename");
			for (File file : files) {
				writer.println(file.getName());
			}
			close();			
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get files for "+getFilename()+": "+e.getMessage());
		}
		return out;
	}
		
	public String getMetadata(Properties properties) throws RuntimeException {
		String selector = properties.getProperty("selector");
		if (selector.equals("files"))
			return getFilesWriter().toString();
		else
			return super.getMetadata(properties);
	}

	public MetadataCriteria[] getMetadataCriterias() {
		MetadataCriteria[] crit = new MetadataCriteria[2];
		crit[0] = new MetadataCriteria("columns");
		crit[1] = new MetadataCriteria("files");
		return crit;
	}	
	
}
