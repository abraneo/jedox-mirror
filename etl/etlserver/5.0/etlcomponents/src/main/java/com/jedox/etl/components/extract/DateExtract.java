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
package com.jedox.etl.components.extract;

import java.text.DateFormatSymbols;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.treecompat.TreeManager;
import com.jedox.etl.core.node.treecompat.PaloTreeManager;
import com.jedox.etl.core.node.treecompat.TreeNode;
import com.jedox.etl.core.source.TreeSource;

/**
 * Generator Class for date / time Tree Structures 
 * Facilitates the creation of date / time dimensions.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class DateExtract extends TreeSource implements IExtract {

	private DateFormatSymbols symbols;
	private final String caption = "caption";
	private int startYear;
	private int endYear;
	private String format = "Y";
	
	private boolean years;
	private boolean quarters;
	private boolean months;
	private boolean weeks;
	private boolean days;
	private boolean hours;
	private boolean minutes;
	private boolean seconds;
	private TreeManager internalTree;
	
	public DateExtract() {
		symbols = new DateFormatSymbols();
		GregorianCalendar calendar = new GregorianCalendar();
		calendar.setTime(new Date());
		startYear = calendar.get(Calendar.YEAR)-1;
		endYear = calendar.get(Calendar.YEAR);
		format = "Y";
	}
	
	protected String getDateFormat() {
		return format;
	}
	
	protected String getDelimiter(String mode) {
		String options ="YQMWDHNS";
		String f = getDateFormat().toUpperCase();
		int pos = f.indexOf(mode.toUpperCase());
		//if mode is part of format and preceeding character is not in possible options interpret this character as delimiter.
		if ((pos > 0) && (options.indexOf(f.charAt(pos-1)) == -1))
			return String.valueOf(f.charAt(pos-1)); 
		return "";
	}
	
	protected TreeManager buildInternalTree() throws RuntimeException {
		internalTree = new TreeManager(getName(),"#!__ROOT__!#"+getName());
		return generate(startYear,endYear,format);
	}
	
	protected void clearInternalTree() {
		internalTree = null;
	}
	
	protected ITreeManager buildTree() throws RuntimeException {
		TreeManagerNG manager = new TreeManagerNG(new PaloTreeManager(buildInternalTree()));
		clearInternalTree();
		setTreeManager(manager);
		return manager;
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			startYear = Integer.parseInt(getParameter("start",String.valueOf(startYear)));
			endYear = Integer.parseInt(getParameter("end",String.valueOf(endYear)));
			format = getParameter("format",format);
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
	
	/**
	 * Generates a tree with the given format. If the format string specifys year generation the current and the previous year are used.
	 * The format String may contain contain the following letters and arbitrary delimiters between them
	 * Y..generates years
	 * Q..generates quarters
	 * M..generates months
	 * W..generates the weeks
	 * D..generates days
	 * The lower level elements are fitted for the higher level elements. 
	 * Example: "W.D" generates 54 weeks with 7 days each. "M.D" generates 12 months with 31 days each.
	 * @param format
	 * @return the date tree
	 */
	public TreeManager generate(String format) {
		return generate(startYear, endYear,format);
	}
	
	/**
	 * Generates a tree with the given format. Year generation (if requested in the format string) starts with the startYear and end with the current year.
	 * See {@link #generate(String)}
	 * @param startYear the year to start year generation 
	 * @param format the format string
	 * @return the date tree
	 */
	public TreeManager generate(int startYear, String format) {
		return generate(startYear,endYear,format);
	}
	
	/**
	 * Generates a tree with the given format. Year generation (if requested in the format string) starts with the startYear and end with endYear.
	 * See {@link #generate(String)}
	 * @param startYear the start year
	 * @param endYear the end year
	 * @param format the format string
	 * @return the date tree
	 */
	public TreeManager generate(int startYear, int endYear, String format) {
		years = format.toUpperCase().contains("Y");
		quarters = format.toUpperCase().contains("Q");
		months = format.toUpperCase().contains("M");
		weeks = format.toUpperCase().contains("W");
		days = format.toUpperCase().contains("D");
		hours = format.toUpperCase().contains("H");
		minutes = format.toUpperCase().contains("N");
		seconds = format.toUpperCase().contains("S");
		if (years) 
			return years(startYear, endYear);
		else if (quarters) 
			return quarters(internalTree.getRootNode()); 
		else if (months) 
			return months(internalTree.getRootNode(), 1, 12);
		else if (weeks)
			return weeks(internalTree.getRootNode(), 1, 54);
		else if (days)
			return days(internalTree.getRootNode(), 1, 31);
		else if (hours)
			return hours(internalTree.getRootNode());
		else if (minutes)
			return minutes(internalTree.getRootNode());
		else if (seconds)
			return seconds(internalTree.getRootNode());
		return internalTree;
	}
	
	private TreeManager years(int start, int end) {
		TreeNode parent = internalTree.getRootNode();
		for (int i=start; i<=end; i++) {
			String name = Integer.toString(i);
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			//n.addAttribute(caption, Integer.toString(i));
			if (quarters) quarters(n);
			else if (months) months(n,1,12);
			else if (weeks) weeks(n, 1, 54);
			else if (days) days(n,1, 366);
		}
		return internalTree;
	}
	
	private TreeManager quarters(TreeNode parent) {
		for (int i=1; i<=4; i++) {
			String name = Integer.toString(i);
			if (!internalTree.isRoot(parent.getName())) {
				name = parent.getName() + getDelimiter("Q") + name;
			}
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			internalTree.addAttribute(n.getName(),caption, i+". Quartal", IColumn.ColumnTypes.alias);
			if (months) months(n,1+3*(i-1),3*i);
			else if (weeks) weeks(n,1, 14);
			else if (days) days(n,1,92);
		}
		return internalTree;
	}
	
	private String getNumber(int i, int end, String mode) {
		int maxLength = String.valueOf(end).length();
		String s = String.valueOf(i);
		//take as is if mode is in upper case or number does not need padding.
		if ((s.length() == maxLength) || (getDateFormat().contains(mode.toUpperCase())))
			return s;
		//else pad with a leading zero.
		StringBuffer buf = new StringBuffer(s);
		for (int j=0; j < maxLength-s.length(); j++)
			buf.insert(0, "0");
		return buf.toString();
	}
	
	private TreeManager months(TreeNode parent, int start, int end) {
		for (int i=start; i<=end; i++) {
			String name = getNumber(i,end,"M");
			if (!internalTree.isRoot(parent.getName())) {
				name = parent.getName() + getDelimiter("M") + name;
			}
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			internalTree.addAttribute(n.getName(),caption, symbols.getMonths()[i-1], IColumn.ColumnTypes.alias);
			if (weeks) weeks(n,1,5);
			else if (days) days(n,1, 31);
		}
		return internalTree;
	}
	
	private TreeManager weeks(TreeNode parent, int start, int end) {
		for (int i=start; i<=end; i++) {
			String name = getNumber(i,end,"W");
			if (!internalTree.isRoot(parent.getName())) {
				name = parent.getName() + getDelimiter("W") + name;
			}
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			internalTree.addAttribute(n.getName(),caption, i+". Woche", IColumn.ColumnTypes.alias);
			if (days) days(n,1, 7);
		}
		return internalTree;
	}
	
	private TreeManager days(TreeNode parent, int start, int end) {
		for (int i=start; i<=end; i++) {
			String name = getNumber(i,end,"D");
			if (!internalTree.isRoot(parent.getName())) {
				name = parent.getName() + getDelimiter("D") + name;
			}
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			if ((start == 1) && (end == 7)) //innerhalb Woche
				internalTree.addAttribute(n.getName(),caption, symbols.getWeekdays()[i], IColumn.ColumnTypes.alias);
			else
				internalTree.addAttribute(n.getName(),caption, i+". Tag", IColumn.ColumnTypes.alias);
			if (hours) hours(n);
		}
		return internalTree;
	}
	
	private TreeManager hours(TreeNode parent) {
		int start = 0;
		int end = 23;
		for (int i=start; i<=end; i++) {
			String name = getNumber(i,end,"H");
			if (!internalTree.isRoot(parent.getName())) {
				name = parent.getName() + getDelimiter("H") + name;
			}
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			internalTree.addAttribute(n.getName(),caption, i+"h", IColumn.ColumnTypes.alias);
			if (minutes) minutes(n);
		}
		return internalTree;
	}
	
	private TreeManager minutes(TreeNode parent) {
		int start = 0;
		int end = 59;
		for (int i=start; i<=end; i++) {
			String name = getNumber(i,end,"N");
			if (!internalTree.isRoot(parent.getName())) {
				name = parent.getName() + getDelimiter("N") + name;
			}
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			internalTree.addAttribute(n.getName(),caption, internalTree.getAttribute(parent, internalTree.getAttributeDefinition().getColumn(caption)).getValueAsString()+ " "+ i +"m", IColumn.ColumnTypes.alias);
			if (seconds) seconds(n);
		}
		return internalTree;
	}
	
	private TreeManager seconds(TreeNode parent) {
		int start = 0;
		int end = 59;
		for (int i=start; i<=end; i++) {
			String name = getNumber(i,end,"S");
			if (!internalTree.isRoot(parent.getName())) {
				name = parent.getName() + getDelimiter("S") + name;
			}
			TreeNode n = internalTree.createNode(name, parent.getName(),1);
			internalTree.addAttribute(n.getName(),caption, internalTree.getAttribute(parent, internalTree.getAttributeDefinition().getColumn(caption)).getValueAsString()+ " "+ i+"s", IColumn.ColumnTypes.alias);
		}
		return internalTree;
	}
}