/**
*   @brief <Description of Class>
*  
*   @file
*  
*   Copyright (C) 2008-2009 Jedox AG
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
*   <a href="http://www.jedox.com/license_palo_bi_suite.txt">
*     http://www.jedox.com/license_palo_bi_suite.txt
*   </a>
*  
*   If you are developing and distributing open source applications under the
*   GPL License, then you are free to use Palo under the GPL License.  For OEMs, 
*   ISVs, and VARs who distribute Palo with their products, and do not license
*   and distribute their source code under the GPL, Jedox provides a flexible  
*   OEM Commercial License.
*  
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.palojlib.main;

import java.util.Date;

/**
 * abstract class which manages the cache logic
 * @author khaddadin
 *
 */
public abstract class CachedComponent {
	
	protected int cacheTrustExpiry = -1;
	private Date cacheExpiryTime = new Date();
	
	/**
	 * end the trust period, this does not delete the cache though
	 */
	protected void endTrustTime() {
		cacheExpiryTime.setTime((new Date()).getTime()-1000);
	}
	
	/**
	 * checks whether the current time is in the trust period
	 * @return
	 */
	protected boolean inTrustTime(){
		return cacheExpiryTime.after(new Date());
	}
	
	/**
	 * set the trust period ending at current time + seconds parameter
	 * @param seconds length of trust period in seconds
	 */
	public void setCacheTrustExpiry(int seconds) {
		cacheTrustExpiry = seconds;
		cacheExpiryTime.setTime((new Date()).getTime()+(cacheTrustExpiry*1000));	
	}

}
