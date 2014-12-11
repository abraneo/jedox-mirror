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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.transform;

import java.util.ArrayList;
import java.util.List;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.SourceFactory;
import com.jedox.etl.core.source.ViewSource;

public class TreeJoinConfigurator extends TreeSourceConfigurator {
	
	public class Hook {
		public String node;
		public double weight;
		public IView source;
	}
	
	public ArrayList<Hook> getHooks() throws ConfigurationException {
		ArrayList<Hook> hooks = new ArrayList<Hook>();
		List<?> sources = getChildren(getXML(),"source");
		for (int j=0; j<sources.size(); j++) {
			Element source = (Element) sources.get(j);
			try {
				ISource viewSource = SourceFactory.getInstance().newSource(ViewSource.getViewDescriptor(), this, getContext(), source);
				String root = source.getAttributeValue("root","");
				Hook hook = new Hook();
				hook.node = root;
				hook.source = (IView)viewSource;
				hook.weight = Double.parseDouble(source.getAttributeValue("weight", "1"));
				hooks.add(hook);
			}
			catch (CreationException ex) {
				throw new ConfigurationException(ex.getMessage());
			}
		}
		return hooks;
	}

}
