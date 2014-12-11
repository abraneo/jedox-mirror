Palo Suite
Package: Palo ETL - Standalone Client  
Version: 3.1
---------------------------------

Palo BI Suite - ETL Server is a tool for the Extraction, Transformation and
Loading of mass data from heterogeneous sources into Palo OLAP Server.

This package features a full Command Line Based Version of ETL Server to execute
ETL processes without a Palo ETL Server Service 

------------
REQUIREMENTS
------------
- Windows XP, Vista, Server 2003/2008, or a Linux distribution
- Java Runtime Environment 1.6 (JRE 6) or higher 
  available for free at http://java.sun.com/javase/

------------
INSTALLATION
------------
1. Unzip the zip-File to an arbitrary directory
2. Make sure that the environment variables JAVA_HOME and PATH are set with the
   correct path to the JRE.
or use the Installer for Windows systems available at http://www.jedox.com

---
RUN
---
Windows:
1. Start the command line interpreter
2. Switch to the installation directory
3. Run "etlstandalone.bat"

Linux: 
1. Start the command line interpreter
2. Switch to the installation directory
3. Run "etlstandalone.sh"

A list of valid parameters is available with switch '-h'

--------
EXAMPLES
--------
This package contains some sample ETL-Projects.  To execute a demo for the
import from a Relational Database to Palo OLAP Server, execute:

Windows:
  etlstandalone -p samples\importRelDB
Linux:
  etlstandalone.sh -p samples\importRelDB

-------------
DOCUMENTATION
-------------
A user manual is available at http://www.jedox.com

-------
LICENCE
-------
Palo BI Suite - ETL Server is released under the GPL licence.

-------
CONTACT
-------
Please contact us directly for further information:
  Jedox Homepage: http://www.jedox.com 
  Palo Forum:     http://forum.palo.net/board.php?boardid=22
