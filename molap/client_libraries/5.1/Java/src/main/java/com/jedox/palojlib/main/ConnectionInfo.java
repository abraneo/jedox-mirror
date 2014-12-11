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

import com.jedox.palojlib.interfaces.IConnectionInfo;

public class ConnectionInfo extends ComponentInfo implements Cloneable,IConnectionInfo {

	private final String majorVersion;
	private final String minorVersion;
	private final String bugfixVersion;
	private final String buildNumber;
	private final EncryptionType encryptionType;
	private final int httpsPort;
	
    protected Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

	protected ConnectionInfo(String majorVersion, String minorVersion,
			String bugfixVersion, String buildNumber, String encryptionType,
			String httpsPort) {
		this.majorVersion = majorVersion;
		this.minorVersion = minorVersion;
		this.bugfixVersion = bugfixVersion;
		this.buildNumber = buildNumber;

		if (encryptionType.equals("0"))
			this.encryptionType = EncryptionType.ENCRYPTION_NONE;
		else if (encryptionType.equals("1"))
			this.encryptionType = EncryptionType.ENCRYPTION_OPTIONAL;
		else
			this.encryptionType = EncryptionType.ENCRYPTION_REQUIRED;

		this.httpsPort = Integer.parseInt(httpsPort);

	}

	public String getMajorVersion() {
		return majorVersion;
	}

	public String getMinorVersion() {
		return minorVersion;
	}

	public String getBugfixVersion() {
		return bugfixVersion;
	}

	public String getBuildNumber() {
		return buildNumber;
	}

	public EncryptionType getEncryptionType() {
		return encryptionType;
	}

	public int getHttpsPort() {
		return httpsPort;
	}

}
