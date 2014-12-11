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
using System.Xml;
using System.Xml.Schema;

namespace XMLValidate
{
    class Program
    {
        /* usage: XMLValidate file.xml */
        static int Main(string[] args)
        {
            try {
                XmlTextReader r = new XmlTextReader(args[0]);
                XmlReaderSettings s = new XmlReaderSettings();
                s.ValidationType = ValidationType.Schema;
                s.ValidationEventHandler += new ValidationEventHandler(MyValidationEventHandler);
                XmlReader xr = XmlReader.Create(r, s);
                while (xr.Read())
                {
                    // Can add code here to process the content.
                }
                xr.Close();

                // Check whether the document is valid or invalid.
                if (isValid)
                {
                    Console.WriteLine("Document is valid.");
                } 
                else
                {
                    Console.WriteLine("Document is invalid!");
                    throw new Exception();
                }

                return 0;
            } catch (Exception exp)
            {
                Console.Error.WriteLine("XMLValidate: error code0000: Failed to parse document!\n" + exp.Message);

                return -1;
            }
        }

        public static void MyValidationEventHandler(object sender, ValidationEventArgs args)
        {
            isValid = false;
            Console.WriteLine("Validation event\n" + args.Message);
        }

        private static bool isValid = true;
    }
}
