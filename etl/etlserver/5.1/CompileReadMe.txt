Requirements: JRE 7, Maven 3

Follow the following steps to build Jedox ETL Server

1. Check out directory to a folder <etl>

2. Apache Tomcat
- Download Apache Tomcat 7.0.? from http://tomcat.apache.org/download-70.cgi as zip-File
- Unzip this file under <etl>/etlserver/src/main 
- Rename the folder ./apache-tomcat-7.0.? to ./tomcat
- You may delete everything in the folder ./tomcat/webapps

3. Apache Axis2
- Download Apache Axis2 Version 1.6.2 from http://ws.apache.org/axis2/download/1_6_2/download.cgi as zip-File
- Unzip folder axis2-1.6.2/webapp/axis2-web under <etl>/etlserver/src/main/webapp/axis2-web
- Unzip folder axis2-1.6.2/conf under <etl>/etlserver/src/main/webapp/WEB-INF/conf
- Unzip folder axis2-1.6.2/webapp/WEB-INF/classes under <etl>/etlserver/src/main/resources

4. since we can not upload the following jars specified in the mvn_precompile_step\uploadMissingArtifacts.bat. The jars should be manually downloaded from 
its' sources, and then copyed in the mvn_precompile_step folder.

5. run uploadMissingArtifacts.bat, this uploads the missing dependencies to your local maven repository.

6. Build using Maven:
- In folder <etl>: 
mvn clean install
- In folder <etl>/etlserver:
mvn assembly:assembly

This project built under maven can be imported into java development environment (e.g. Eclipse).

Enjoy