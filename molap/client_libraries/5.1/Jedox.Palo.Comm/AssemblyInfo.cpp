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

using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;

// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly:AssemblyTitleAttribute("Palo .NET Library")];
[assembly:AssemblyDescriptionAttribute("Palo Engine .NET Library")];
[assembly:AssemblyConfigurationAttribute("")];
[assembly:AssemblyCompanyAttribute("Jedox AG")];
[assembly:AssemblyProductAttribute("Palo Engine")];
[assembly:AssemblyCopyrightAttribute("copyright (C) 2004-2014 Jedox AG")];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("")];

// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version 
//      Build Number
//      Revision
//
// You can specify all the value or you can default the Revision and Build Numbers 
// by using the '*' as shown below:
[assembly:AssemblyVersionAttribute("5.1.0.0")];

// In order to sign your assembly you must specify a key to use. Refer to the 
// Microsoft .NET Framework documentation for more information on assembly signing.
//
// Use the attributes below to control which key is used for signing. 
//
// Notes: 
//   (*) If no key is specified - the assembly cannot be signed.
//   (*) KeyName refers to a key that has been installed in the Crypto Service
//       Provider (CSP) on your machine. 
//   (*) If the key file and a key name attributes are both specified, the 
//       following processing occurs:
//       (1) If the KeyName can be found in the CSP - that key is used.
//       (2) If the KeyName does not exist and the KeyFile does exist, the key 
//           in the file is installed into the CSP and used.
//   (*) To generate a keyFile, you can use the program sn.exe (Strong Name).
//       If KeyFile is given , then the path of KeyFile must be relative to 
//       project output directory :
//       %Project Directory%\obj\<configuration>. If KeyFile is located e.g.
//       in projectdir you specify the AssemblyKeyFile-Attribut 
//       as follows: [assembly: AssemblyKeyFile("..\\..\\mykey.snk")]
//   (*) Delay Signing is an advanced option - see the Microsoft .NET Framework
//       documentation for more information on this.
//
[assembly:AssemblyKeyNameAttribute("")];
