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
package com.jedox.etl.components.function;

import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.NamingUtil;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
 
public class Concatenation extends Function {
	private Vector<TemplateColumn> columns = new Vector<TemplateColumn>();
	private String template;
	private String delimiter;
	
	/**
	 * Internal helper / wrapper class for parsing a single column from a template string.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	protected class TemplateColumn {
		private int start;
		private int len;
		private IColumn column;
		
		public TemplateColumn(IColumn column, int start, int columnlength) {
			this.start = start;
			this.column = column;
			len = columnlength + 3;
		}
		
		public int getStart() {
			return start;
		}
		
		public int getLength() {
			return len;
		}
		
		public IColumn getColumn() {
			return column;
		}
	}
	
	/**
	 * Parses all single columns used by the template
	 * @param columns an empty vector
	 * @param template the template string
	 * @return the vector of {@link TemplateColumn} objects
	 * @throws Exception 
	 */
	protected Vector<TemplateColumn> parseColumns(Vector<TemplateColumn> columns, String template){
		if ((columns.isEmpty()) && (template != null)) {
			String regex = "#\\{[.*[^#$\\{\\}]]+\\}";
			Matcher m = Pattern.compile(regex).matcher(template);
			while (m.find()) {
				String group = m.group();
				boolean errorInput = false;
				int index = 0;
				String groupsub = group.substring(2,group.length()-1);
				
				IColumn col =  null;
				if(groupsub.startsWith("_input")){
					try{
						index = Integer.parseInt(groupsub.substring(6))-1;
						col =  getInputs().getColumns().get(index);

					}catch(Exception e){
						errorInput = true;
					}
					
				}else{
					col =  getInputs().getColumn(groupsub);
				}
				
				if(col != null){
					TemplateColumn c = new TemplateColumn(col,m.start(),groupsub.length());
					columns.add(c);
				}
				else{
					if(!errorInput){
						getLog().warn("Unexpected parameter in Function "+getName()+": Column " + groupsub + " not found in inputs list.");
					}else{
						getLog().warn("Unexpected parameter in Function "+getName()+": Input " + groupsub + " may exceed the number of given inputs or is not in the correct format.");
					}
				}
			}
		}
		return columns;
	}
	
	/**
	 * Calculates a value based on the template specification and the current ResultSet state.
	 * @param columns the list of columns to listen on
	 * @param value the template to process.
	 * @return the processed value
	 */
	protected String process(Vector<TemplateColumn> columns, String template) {
		StringBuffer buf = new StringBuffer();
		int start = 0;
		for (int i=0; i<columns.size();i++) {
			TemplateColumn c = columns.get(i);
			buf.append(template.substring(start,c.getStart()));
			start = c.getStart() + c.getLength();
			String	colres = c.getColumn().getValueAsString();
			//return default value if a needed column result is null
			if (colres.isEmpty()) return getDefaultValue();
			buf.append(colres);	
		}
		buf.append(template.substring(start));
		return buf.toString();
	}
	
	protected String process(Row values, String delimiter) {
		StringBuffer sb = new StringBuffer();
		for (int i=0;i<values.size()-1;i++) {
			sb.append(values.getColumn(i).getValueAsString());
			sb.append(delimiter);
		}
		sb.append(values.getColumn(values.size()-1).getValueAsString());
		return sb.toString();
	}
	
	/**
	 * Gets the resolved value according to the template and the actual input.
	 * @return the resolved value 
	 * @throws Exception 
	 */	
	protected Object transform(Row values){
		if (template != null) {
			return process(parseColumns(columns,template),template);
		}
		//fallback to delimiter separeted values
		return process(values, delimiter);
	}
	
	/**
	 * Gets the template of the value of a column node. Columns are indicated by #\w#. All other parts are considered as fixed strings. 
	 * @param value: the template for the value to resolve. Example template: #a3#.#b4#: The dot seperates values from columns with name a3 and b4 
	 */
	
	public void addInput(IColumn input) {
		super.addInput(input);
		columns.clear();
	}
	
	public String getValueType() {
		return "java.lang.String";
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,true);
	}
		
	protected void validateParameters() throws ConfigurationException {
		template = getParameter("template",null);
		if(template != null)
			template = template.replaceAll(NamingUtil.spaceValue(), " ");
		
		delimiter = getParameter("delimiter","").replaceAll(NamingUtil.spaceValue(), " ");
		
		if(template != null && !delimiter.equals("")){
			getLog().warn("For function " + getName() + " either delimiter or template may be given. In this case only template will be evaluated.");
		}
		
		if((template == null || template.isEmpty()) && delimiter.equals("")){
			template = "";
			for(int i=0;i<getInputs().size();i++){
				template = template.concat("#{_input" + (i+1) + "}");
			}
		}
	}

}
