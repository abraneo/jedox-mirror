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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.service;

import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Enumeration;

import javax.servlet.*;  

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class ETLContextListener implements ServletContextListener  
{  
	private static Log log = LogFactory.getLog(ETLContextListener.class);
	
	
	/**
	 * Unregister all JDBC drivers to prevent Tomcat log messages on shutdown indicating a memory leak
	 */
	private void unregisterDrivers() {
		Enumeration<Driver> drivers = DriverManager.getDrivers();
        while (drivers.hasMoreElements()) {
            Driver driver = drivers.nextElement();
            try {
                DriverManager.deregisterDriver(driver);
                log.info("Deregistering jdbc driver: "+driver);
            } catch (SQLException e) {
                log.error("Error deregistering jdbc driver "+driver);
            }
        }    
	}
	
    public void contextInitialized(ServletContextEvent event)  
    {  
        // do nothing  
    }  
   
    public void contextDestroyed(ServletContextEvent event)  
    {  
        unregisterDrivers();
    }
}    