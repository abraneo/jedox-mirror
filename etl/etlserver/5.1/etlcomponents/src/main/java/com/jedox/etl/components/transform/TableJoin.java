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

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.transform.TableJoinConfigurator;
import com.jedox.etl.components.config.transform.TableJoinConfigurator.Conditions;
import com.jedox.etl.components.config.transform.TableJoinConfigurator.JoinDefinition;
import com.jedox.etl.components.config.transform.TableJoinConfigurator.Match;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.ITransform;


public class TableJoin extends TableSource implements ITransform {
	
	private JoinDefinition joinDefinition;
	private boolean persist;
	private static final Log log = LogFactory.getLog(TableJoin.class);

	public TableJoin() {
		setConfigurator(new TableJoinConfigurator());
	}

	public TableJoinConfigurator getConfigurator() {
		return (TableJoinConfigurator)super.getConfigurator();
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}

	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		CacheTypes myCacheType = (persist ? CacheTypes.disk : CacheTypes.none);
		if (isOptimizePersistence()) {
			Set<CacheTypes> sourceTypeSet = new HashSet<CacheTypes>();
			for (ISource s : getSourceManager().getAll()) {
				if (s instanceof IView) {
					sourceTypeSet.add(((IView)s).getBaseSource().getCacheType());
				} else {
					sourceTypeSet.add(s.getCacheType());
				}
			}
			if (!sourceTypeSet.contains(CacheTypes.none) && sourceTypeSet.size() == 1) {
				CacheTypes uniqueType = sourceTypeSet.iterator().next();
				if (!uniqueType.equals(myCacheType)) {
					log.info("Overriding persistence setting with "+uniqueType.toString()+" persistence because it is already available.");
					myCacheType = uniqueType;
				}
			}
		}
		if (myCacheType.equals(CacheTypes.disk) || myCacheType.equals(CacheTypes.memory)) {
			log.debug("Using Persistent Join");			
			IProcessor processor = initProcessor(new PersistentJoin(this,joinDefinition, getSourceManager(),getInternalConnection(),myCacheType,size),Facets.OUTPUT);
			return processor;
		}
		else {
			log.debug("Using Lookup Join");			
			IProcessor p = initProcessor(new LookupJoin(this,joinDefinition, getSourceManager()),Facets.OUTPUT);
			p.setLastRow(size);
			return p;
		}
	}

	public Row getOutputDescription() throws RuntimeException {
		Row columns = new Row();
		for (ISource s : getSourceManager().getAll()) {
			columns.addColumns(s.getOutputDescription());
		}
		return columns;
	}
	
	private boolean getPersistence(List<JoinDefinition> definitions) {
		boolean result = getConfigurator().persist();
		if (!result)
			for (JoinDefinition d : definitions) {
				for (Match m : d.getMatches()) {
					if (!m.getCondition().equals(Conditions.EQ)) {
						log.warn("Join Condition "+m.getCondition().toString()+" in transform "+getName()+" needs persistence. Switching to persistent join.");
						return true;
					}
				}
			}
		return result;
	}
	

	public void init() throws InitializationException {
		super.init();
		try {
			List<JoinDefinition> definitions = getConfigurator().getJoins();
			persist = getPersistence(definitions);
			joinDefinition = definitions.get(0);
			SourceManager manager = new SourceManager();
			manager.addAll(getConfigurator().getSources());
			addManager(manager);
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}
}
