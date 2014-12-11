////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////

using System;
using System.Xml.Xsl;

namespace XMLTransform
{
    class Program
    {
        /* usage: XMLTransform transform.xslt data.xml output_file */
        static int Main(string[] args)
        {
            try
            {
                // Create the XslTransform.
                XslCompiledTransform xslt = new XslCompiledTransform();

                // Load the style sheet.
                xslt.Load(args[0]);
                
                // Transform the file.
                xslt.Transform(args[1], args[2]);

                return 0;
            } 
            catch (Exception ex)
            {
                Console.Error.WriteLine("XMLTransform: error code0000: Failed to transform document!\n" + ex.Message);

                return -1;
            }
        }
    }
}
