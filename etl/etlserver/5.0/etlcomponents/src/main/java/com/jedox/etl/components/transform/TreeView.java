package com.jedox.etl.components.transform;

import com.jedox.etl.components.config.transform.TreeViewConfigurator;
import com.jedox.etl.components.extract.DimensionExtract;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.treecompat.PaloTreeManager;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.palojlib.interfaces.IDimension;

public class TreeView extends DimensionExtract implements ITransform {
	
	private ITreeSource source;
	
	public TreeView() {
		setConfigurator(new TreeViewConfigurator());
	}

	public TreeViewConfigurator getConfigurator() {
		return (TreeViewConfigurator)super.getConfigurator();
	}
	
	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}
	
	public IOLAPConnection getConnection() throws RuntimeException {
		return null; //no connection needed.
	}
	
	protected IDimension getDimensionObj() throws RuntimeException{
		return new TreeManagerNG(source.generate());
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new SourceManager());
			source = getConfigurator().getSource();
			getSourceManager().add(source);
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
