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
package com.jedox.etl.core.extract;

import org.jdom.Element;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ILocatable;
import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.source.SourceFactory;

/**
 * Factory class for the creation of {@link IExtract Extracts}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ExtractFactory extends SourceFactory {
	
private static final ExtractFactory instance = new ExtractFactory();
	
	public ExtractFactory() {
	}
	
	/**
	 * gets the static instance
	 * @return the singleton Factory instance
	 */
	public static ExtractFactory getInstance() {
		return instance;
	}
	
	
	/**
	 * creates a new {@link IExtract Extract} from a config
	 * @param descriptor the descriptor holding static component information from a component.xml 
	 * @param parent the parent object of this connection, usually a {@link ExtractManager}
	 * @param context the context to use for extract creation
	 * @param config the XML configuration of the extract
	 * @return a newly created extract
	 * @throws CreationException, if anything goes wrong in this process
	 */
	public IExtract newExtract(ComponentDescriptor descriptor, ILocatable parent, IContext context, Element config) throws CreationException{
		//add here some extra things when needed. for now it is just a source
		return (IExtract) newSource(descriptor, parent, context, config);
	}

}
