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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.load;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.load.DimensionConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.processor.IProcessor;

public class DimensionLoad extends AbstractDimensionLoad {

	private static final Log log = LogFactory.getLog(DimensionLoad.class);
	private Modes elementsMode;
	private Modes hierarchyMode;
	private Modes attributesMode;
	private ElementLoad elementLoad;
	private ConsolidationLoad consolidationLoad;
	private AttributeLoad attributeLoad;

	public DimensionLoad() {
		setConfigurator(new DimensionConfigurator());
	}

	/**
	 * Export the serialized Tree Structure as a dimension of a cube
	 */
	public void execute() {
		if (isExecutable()) {
			try {
				log.info("Starting load "+getName()+" of Dimension "+getDimensionName());
				ITreeManager manager = getView().renderTree(Views.EA);
				if (isExecutable() && elementLoad != null) {
					log.debug("Exporting Elements...");
					elementLoad.setMode(elementsMode);
					elementLoad.exportElements(manager);
				}
				if (isExecutable() && consolidationLoad != null) {
					log.debug("Exporting Consolidations...");
					consolidationLoad.setMode(hierarchyMode);
					if (!getView().isTreeBased()) {
						log.warn("Source "+getProcessor().getName()+" of load "+getName()+" has to be a tree for the loading of consolidations. Dimension will be flat.");
					} else {
						consolidationLoad.exportConsolidations(manager);
					}

				}
				if (isExecutable() && attributeLoad != null) {
					log.debug("Exporting Attributes...");
					attributeLoad.setMode(attributesMode);
					attributeLoad.exportAttributes(manager);
				}
				//getDatabase().save();
			}
			catch (Exception e) {
				log.error("Cannot load Dimension "+getDimensionName()+": "+e.getMessage());
				log.debug(e);
			}
			log.info("Finished load "+getName()+" of Dimension "+getDimensionName());
		}
	}

	public void init() throws InitializationException {
		try {
			super.init();
			elementsMode = getConfigurator().getElementsMode();
			if (elementsMode != null)
				elementLoad = new ElementLoad(getConfigurator());;
			hierarchyMode = getConfigurator().getHierarchyMode();
			if (hierarchyMode != null)
				consolidationLoad = new ConsolidationLoad(getConfigurator());
			attributesMode = getConfigurator().getAttributesMode();
			if (attributesMode != null)
				attributeLoad = new AttributeLoad(getConfigurator());
			//check case of table source, which can not be used for elements / attributes and consolidations at the same time
//			if (!getView().isTreeBased()) {
//				if (hierarchyMode != null)
//					throw new InitializationException("Source "+getProcessor().getName()+" of load "+getName()+" is not valid for loading of consolidations. The source has to be a tree.");
//			}
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
