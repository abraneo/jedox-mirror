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
 *   You may obtain a copy of the License at
*
 *   If you are developing and distributing open source applications under the
 *   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 *   ISVs, and VARs who distribute Palo with their products, and do not license
 *   and distribute their source code under the GPL, Jedox provides a flexible
 *   OEM Commercial License.
 *
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.main;

import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;

/**
 * handler used by {@link Element} to make request on olap server
 * @author khaddadin
 *
 */

public final class ElementHandler{

	private static final String ELEMENT_INFO_REQUEST = "/element/info?";
	private static final String ELEMENT_RENAME_REQUEST = "/element/rename?";
	private static final String ELEMENT_MOVE_REQUEST = "/element/move?";
	private final String contextId;

	protected ElementHandler(String contextId) throws PaloException, PaloJException{
		this.contextId = contextId;
	}

	protected String getInfo(int databaseId, int dimensionId,int id) throws PaloException, PaloJException{

		StringBuilder ELEMENT_INFO_REQUEST_BUFFER = new StringBuilder(ELEMENT_INFO_REQUEST);
		StringBuilder currentRequest = ELEMENT_INFO_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(dimensionId).append("&element=").append(id);
		String [][]response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,true);
		return response[0][0];
	}

	protected void rename(int databaseId, int dimensionId, int id, String name) throws PaloException, PaloJException {
		StringBuilder ELEMENT_RENAME_REQUEST_BUFFER = new StringBuilder(ELEMENT_RENAME_REQUEST);
		StringBuilder currentRequest = ELEMENT_RENAME_REQUEST_BUFFER.append("dimension=").append(dimensionId).append("&database=").append(databaseId).append("&element=").append(id).append("&new_name=").append(name);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
	}

	public void move(int databaseId, int dimensionId, int id, int position) {
		StringBuilder ELEMENT_MOVE_REQUEST_BUFFER = new StringBuilder(ELEMENT_MOVE_REQUEST);
		StringBuilder currentRequest = ELEMENT_MOVE_REQUEST_BUFFER.append("dimension=").append(dimensionId).append("&database=").append(databaseId).append("&element=").append(id).append("&position=").append(position);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
	}


}
