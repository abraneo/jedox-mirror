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
package com.jedox.etl.components.transform;

import java.util.List;

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
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.node.ColumnManager;


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
		if (persist) {
			log.debug("Using Persistent Join");			
			PersistentJoin join = new PersistentJoin(joinDefinition, getSourceManager(),getInternalConnection().getIdentifierQuote());
			return getInternalConnection().getProcessor(join.getJoinQuery(), false, size);
		}
		else {
			log.debug("Using Lookup Join");			
			return new LookupJoin(joinDefinition, getSourceManager()).getProcessor(size);
		}
	}

	public Row getOutputDescription() throws RuntimeException {
		ColumnManager columns = new ColumnManager();
		for (ISource s : getSourceManager().getAll()) {
			columns.addCoordinates(s.getOutputDescription());
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
			setCaching(false); //Dont persist the join. Underlying sources are persisted in case persist = true
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
