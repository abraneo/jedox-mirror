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
package com.jedox.etl.core.component;

import javax.swing.event.EventListenerList;

import org.jdom.Element;

import com.jedox.etl.core.config.ConfigurationChangeEvent;
import com.jedox.etl.core.config.IConfigurationChangeListener;

/**
 * Abstract base class, which supports a notification mechanism, where all registered listener objects get notified on a change in configuration and may react properly.
 * Listeners have to implement the {@link com.jedox.etl.core.config.IConfigurationChangeListener} interface. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class Configurable implements IConfigurable {
	
	private EventListenerList listenerList;
	
	public void addConfigurationChangeListener(IConfigurationChangeListener listener) {
        if (listenerList == null) {
            listenerList = new EventListenerList();
        }
        //ensure that listeners are present only once
        listenerList.remove(IConfigurationChangeListener.class, listener);
        listenerList.add(IConfigurationChangeListener.class, listener);
    }
	
	public void addConfigurationChangeListeners(IConfigurationChangeListener[] listeners) {
		for (int i=0;i<listeners.length;i++) {
			addConfigurationChangeListener(listeners[i]);
		}
	}
	
	public void removeConfigurationChangeListener(IConfigurationChangeListener l) {
	    listenerList.remove(IConfigurationChangeListener.class, l);
	}
	
	public IConfigurationChangeListener[] getConfigurationChangeListeners() {
		return listenerList.getListeners(IConfigurationChangeListener.class);
	}
	
	/**
	 * notify all registered listeners, that a config change has occurred.
	 * @param locator the locator of the {@link ILocatable}, which has a changed config. 
	 * @param oldValue the old config
	 * @param newValue the new config
	 */
	protected void fireConfigChanged(Locator locator, Element oldValue, Element newValue) {
        if (listenerList != null) {
            ConfigurationChangeEvent configurationChangeEvent = new ConfigurationChangeEvent(this, locator, oldValue, newValue);
            Object[] listeners = listenerList.getListeners(IConfigurationChangeListener.class);
            for (int i = 0; i < listeners.length; i++) {
                   ((IConfigurationChangeListener)listeners[i]).configurationChanged(configurationChangeEvent);
            }
        }
    }

}
