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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.service;

import java.io.IOException;
import java.util.ArrayList;
import java.util.StringTokenizer;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * @author khaddadin
 *
 */
public class ETLFilter implements Filter {
	
	private ArrayList<String> urlList;
	private static Log log = LogFactory.getLog(ETLFilter.class);

	/* filter coming requests not starting with the allowed list Strings
	 * @see javax.servlet.Filter#doFilter(javax.servlet.ServletRequest, javax.servlet.ServletResponse, javax.servlet.FilterChain)
	 */
	@Override
	public void doFilter(ServletRequest req, ServletResponse res,
			FilterChain chain) throws IOException, ServletException {

        HttpServletRequest request = (HttpServletRequest) req;
        HttpServletResponse response = (HttpServletResponse) res;
        //request.getSession(false);
        String url = request.getRequestURI().toLowerCase();
        //log.info("performing doFilter with url" + url);
        boolean isAllowed = false;
        
        for(int i=0;i<urlList.size();i++){
        	if(url.startsWith(urlList.get(i))){
        		isAllowed = true;
        		break;
        	}
        }
 
        if(!isAllowed) {
        	log.info("Url " + url + " will be redirected.");
        	response.sendRedirect("/etlserver/info/index.html");
        }else{
        	chain.doFilter(req, res);
		}   
	}

	/*  read the allow list urls
	 * @see javax.servlet.Filter#init(javax.servlet.FilterConfig)
	 */
	@Override
	public void init(FilterConfig config) throws ServletException {
        String urls = config.getInitParameter("allow-urls-starting-with");
        StringTokenizer token = new StringTokenizer(urls, ",");
 
        urlList = new ArrayList<String>();
 
        while (token.hasMoreTokens()) {
            urlList.add(token.nextToken().toLowerCase());
 
        }

	}

	@Override
	public void destroy() {
		urlList.clear();	
	}

}
