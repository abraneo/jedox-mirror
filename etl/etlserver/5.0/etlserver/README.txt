Palo Suite
Package: Palo ETL - Server  
Version: 3.1
---------------------------------

------------
REQUIREMENTS
------------
- Windows XP, Vista, Server 2003/2008, or a Linux distribution
- Java Runtime Environment 1.6 (JRE 6) or higher 
  available for free at http://java.sun.com/javase/
- A Web Browser for the use of the ETL Web-Client.

-------------------------------------
INSTALLATION JAVA RUNTIME ENVIRONMENT
-------------------------------------
If not yet installed, the Java Runtime Environment (JRE) is available for
free at http://java.sun.com/javase/. 

Please follow the installation instructions given at this site. Especially, the
JAVA environment variables have to be set correctly. For Windows, they can be
configured at Windows Control Panel -> System -> Advanced Environment Variables:

- Set the JAVA_HOME environment variable to the root location of the JRE or JDK installation directory 
(e.g. C:\Programme\Java\jre1.6.0_04)

- Set the PATH environment variable to include the JRE or JDK .\bin installation directory
(e.g. C:\Programme\Java\jre1.6.0_04\bin)

----------------------------
INSTALLATION PALO ETL SERVER 
----------------------------

WINDOWS:
-------

Start the Windows Installer for Palo ETL Server and follow the instructions.

LINUX:
-----

1. Create an arbitrary directory for the ETL installation

2. Unzip files "etlserver-3.1-tomcat.zip" and "etlclient-3.1-bin.zip" 
into ETL installation directory

3. Copy files "etlserver.war" and "web-etl.war" into directory ./webapps

4. Execute "startup.sh" in directory ./bin to start the service

-------------
START PROGRAM
-------------

1. To start the ETL Web Client:
Open the following URL in the Webbrowser: http://localhost:7775/web-etl

For an initial English GUI open the following URL: 
http://localhost:7775/web-etl/com.jedox.etl.webclient.application.Login/Login.html?locale=en

2. To start the ETL Command Line Client:
- Start the command line interpreter
- Switch to directory .\client under the ETL installation directory
- Run "etlclient.bat" to see all options 


-------
REMARKS
-------

- To install Palo ETL Server 3.1 it is not necessary to deinstall an existing
Palo ETL Server 1.X installation. 

- As part of the installation, an Apache Tomcat Servlet Container is installed. 
It is also possible to deploy Palo ETL Server in an existing Apache Tomcat 6 Installation:
1. Stop Apache Tomcat Service
2. Clear directories .\temp and .\work
3. Copy files "etlserver.war" and "web-etl.war" into directory .\webapps
4. Start Apache Tomcat Service

- The Palo ETL Server service (Apache Tomcat) is installed by default on port 7775.
To change the port you have to edit file .\conf\server.xml and for the Web-Client
the file .\webapps\web-etl\WEB-INF\classes\webetl.properties

- The Web-Client of Palo ETL Server has been tested with the following 
Webbrowsers: Internet Explorer 7.0 and higher, Firefox 3.0 and higher

- After an update of your Palo ETL Server 3.1 installation, in some cases you have to 
initially clear your browser cache once for the correct use of the Web-Client
 
- Several ETL-samples can be found in directory .\client\samples. They can be uploaded
to Palo ETL Server, either with the Web-Client or the Command Line Client.
