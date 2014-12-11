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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Christian Schwarzinger, proclos OG, Wien, Austria
 *   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 *  
 *  */
package com.jedox.etl.components.config.job;

import java.util.List;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceFactory;
import com.jedox.etl.core.source.ViewSource;


/**
 * @author Kais Haddadin
 *
 */
public class LoopJobConfigurator extends ParallelJobConfigurator{

	private ISource loopsource;
	
	public ISource getLoopSource() throws ConfigurationException {
		if (loopsource == null) {
			
			Element datasources = getXML().getChild("loops");
			List<?> ds = datasources.getChildren("loop");
			if(ds.size()!=1){
				throw new ConfigurationException("Only one loop source is allowed.");
			}
			for (int j=0; j<ds.size(); j++) {
				Element e = (Element) ds.get(j);
				try {
					ISource viewSource = SourceFactory.getInstance().newSource(ViewSource.getViewDescriptor(), getContext(), getContext(), e);
					loopsource =  viewSource;
				}
				catch (CreationException ex) {
					throw new ConfigurationException(ex.getMessage());
				}
			}
		}
		return loopsource;
	}
	
	public int getBulkSize() throws ConfigurationException{		
		Element bulkSize = getXML().getChild("bulksize");			
		if(bulkSize!=null){
			if(getParallelLocators().size()==0){
				throw new ConfigurationException("BulkSize can only be given if parallel job/load is used in the loop.");
			}
			return Integer.parseInt(bulkSize.getValue());
		}		
		return Integer.MAX_VALUE;
	}

}
