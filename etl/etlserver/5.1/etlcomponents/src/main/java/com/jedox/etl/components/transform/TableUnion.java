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
package com.jedox.etl.components.transform;

import java.util.List;

import org.jdom.Element;
import com.jedox.etl.components.config.transform.TableUnionConfigurator;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceFactory;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.ViewSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.LoopProcessor;
import com.jedox.etl.core.source.processor.UnionProcessor;
import com.jedox.etl.core.transform.ITransform;

public class TableUnion extends TableSource implements ITransform {
	
	protected SourceManager baseManager = new SourceManager();
	protected SourceManager loopManager = new SourceManager();

	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor bases = null;
		if (!loopManager.isEmpty()) { //there are loops
			IProcessor loops = initProcessor(UnionProcessor.getInstance(loopManager.getProcessors()),Facets.HIDDEN);
			IProcessor loopProcesor = initProcessor(new LoopProcessor(loops,baseManager),Facets.INPUT); 
			bases = initProcessor(loopProcesor,Facets.OUTPUT);
		}
		else { //no loops, just a union
			IProcessor unionProcessor = initProcessor(UnionProcessor.getInstance(baseManager.getProcessors()),Facets.INPUT);
			bases = initProcessor(unionProcessor,Facets.OUTPUT);
		}
		bases.setLastRow(size);
		return bases;
	}

	public TableUnion() {
		setConfigurator(new TableUnionConfigurator());
	}

	public TableUnionConfigurator getConfigurator() {
		return (TableUnionConfigurator)super.getConfigurator();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			baseManager.setAllowDuplicates(true);
			loopManager.setAllowDuplicates(true);
			//put all sources in a source manager for dependency management but additionally use different managers for base-sources and loop-sources
			SourceManager manager = new SourceManager();
			addManager(manager);
			List<IComponent> bases = getConfigurator().getSources();
			manager.addAll(bases);
			baseManager.addAll(bases);
			for (Element l : getConfigurator().getLoopElements()) {
				ISource loop = SourceFactory.getInstance().newSource(ViewSource.getViewDescriptor(), this, getContext(), l);
				loopManager.add(loop);
				manager.add(loop);
			}
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
