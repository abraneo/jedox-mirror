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
package com.jedox.etl.core.source.filter;

import java.util.HashSet;
/*
import org.palo.api.Dimension;
import org.palo.api.Element;
import org.palo.api.subsets.*;
*/

/**
 * Evaluates if an object is in a set of elements held by this evaluator
 * @author Kais Haddadin
 *
 */
public class SubsetEvaluator implements IEvaluator {

/* Subsets currently not supported as not yet available in palojlib */
	
	private HashSet<String> subsetElementsNamesSet = new HashSet<String>();
	private String subsetName ="";

	public SubsetEvaluator(String subsetName) {
		this.subsetName = subsetName;
	}

/*	
	public void setElements(Dimension dimension) {

		SubsetHandler sh = dimension.getSubsetHandler();
		 Subset2[] subsets= sh.getSubsets();
		  for(Subset2 s: subsets){
			  if(s.getName().equals(subsetName)){
				  Element[] subsetElements = s.getElements();
				  for(Element e:subsetElements){
					  subsetElementsNamesSet.add(e.getName());
				  }
			  }
		  }
	}
*/
	public boolean evaluate(Object element) {
/*		
		String elementStr = element.toString();
			if(subsetElementsNamesSet.contains(elementStr))
				return true;
*/	
		return false;
	}


}
