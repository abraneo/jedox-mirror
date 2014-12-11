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

package com.jedox.palojlib.http;

import java.io.IOException;
import java.io.InputStream;

/**
 * Fixed length inputstream, this uses the info response-length in palo header response to know when the stream ends.
 * @author khaddadin
 *
 */
public class FixedLengthInputStream extends InputStream {

	private final int maximumBytes;
	private int bytesRead;
	private boolean isClosed;
	private InputStream inStream = null;

	public FixedLengthInputStream(InputStream inStream, int maximumBytes) {
		this.maximumBytes = maximumBytes;
		this.inStream = inStream;
	}

	/**
	 * read one byte
	 */
	public final synchronized int read() throws IOException {
		if (isClosed)
			throw new IOException("InputStream is closed!");

		if (bytesRead >= maximumBytes)
			return -1;

		bytesRead++;
		return this.inStream.read();
	}

	/**
	 * read specific number of bytes
	 */
	public final int read(byte[] b) throws IOException {
		if (isClosed)
			throw new IOException("InputStream is closed!");

		if (bytesRead >= maximumBytes)
			return -1;

		int len = b.length;

		//check length
		if (bytesRead + len > maximumBytes) {
			len = (int) (maximumBytes - bytesRead);
		}
		int count = this.inStream.read(b, 0, len );
		bytesRead += count;
		return count;
	}

	/**
	 * closes the stream
	 */
	public final synchronized void close() throws IOException {
		if (!isClosed) {
			try {
				//we read until end:
				byte bytes[] = new byte[1024];
				while (this.read(bytes) >= 0  && !Thread.interrupted()) {
					;
				}
			} finally {
				isClosed = true;
			}
		}
	}

}
