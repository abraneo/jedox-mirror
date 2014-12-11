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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.node;

public class RelationalNode extends ColumnNode {

	/**
	 * Enumerates the available update modes, when data is written to a persistence back-end. The update modes can be defined on a per column basis.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum UpdateModes {
		/**
		 * columns identified by mode key are used like primary keys to find fitting data vectors to be updated.
		 */
		key,
		/**
		 * the data of columns identified by mode keep is simple kept. The new value is dropped.
		 */
		first,
		/**
		 * the data of columns identified by mode put is written to the persistence and replaces old data.
		 */
		last,
		/**
		 * the data of column identified by mode add is added to the old value found in the persistence. The new value there is the sum of the old and the new data.
		 */
		sum,
		/**
		 * new value is the minimum value.
		 */
		min,
		/**
		 * new value is the maximum value
		 */
		max,
		/**
		 * new value is the number of values.
		 */
		count,
		/**
		 * new value is the average value
		 */
		avg,
		/**
		 * data serving as primary key
		 */
		primary,
		/**
		 * none defined.
		 */
		none
	}
	
	private UpdateModes role;
	private int scale=0;
	
	protected RelationalNode() {
		super();
	}
	
	protected RelationalNode(String name) {
		super(name);
	}

	public void setRole(UpdateModes role) {
		this.role = role;
	}

	public UpdateModes getRole() {
		return (role != null) ? role : UpdateModes.none;
	}
	
	public int getScale() {
		return scale;
	}
	
	public void setScale(int scale) {
		this.scale=scale;	
	}
	
	public void mimic(IColumn source) {
		super.mimic(source);
		if (source instanceof RelationalNode) {
			RelationalNode cn = (RelationalNode)source;			
			// this.fallback = cn.fallback;
			this.role = cn.role;
			this.scale = cn.scale;
		}
	}

}
