rem ##########################################################################################################################
rem dependencies with "compile" scope 
rem these are required for compilation
call mvn install:install-file -Dfile=.\palojlib-5.1.X.jar -DgroupId=com.jedox -DartifactId=palojlib -Dversion=5.1.X -Dpackaging=jar
call mvn install:install-file -Dfile=.\JRI-0.5.5.jar -DgroupId=org.rosuda -DartifactId=JRI -Dversion=0.5.5 -Dpackaging=jar
call mvn install:install-file -Dfile=.\olap4j-1.1.0.jar -DgroupId=olap4j -DartifactId=olap4j -Dversion=1.1.0 -Dpackaging=jar
call mvn install:install-file -Dfile=.\olap4j-xmla-1.1.0.jar -DgroupId=olap4j -DartifactId=olap4j-xmla -Dversion=1.1.0 -Dpackaging=jar
call mvn install:install-file -Dfile=.\jgraphx-1.11.0.0_patched.jar -DgroupId=org.tinyjee.jgraphx -DartifactId=jgraphx -Dversion=1.11.0.0_patched -Dpackaging=jar
call mvn install:install-file -Dfile=.\UCanAccess-2.0.0.jar -DgroupId=UCanAccess -DartifactId=UCanAccess -Dversion=2.0.0 -Dpackaging=jar
call mvn install:install-file -Dfile=.\wsc-23.jar -DgroupId=salesforce -DartifactId=wsc -Dversion=23 -Dpackaging=jar
call mvn install:install-file -Dfile=.\generatedClient-30.0.jar -DgroupId=salesforce -DartifactId=generatedClient -Dversion=30.0 -Dpackaging=jar
rem ##########################################################################################################################
rem dependencies with "runtime" scope
rem the code will compile even if they were removed from the POMs, nevertheless, some functionality may not work then
call mvn install:install-file -Dfile=.\groovy-all-2.1.6.jar -DgroupId=org.codehaus.groovy -DartifactId=groovy-all -Dversion=2.1.6 -Dpackaging=jar
call mvn install:install-file -Dfile=.\javascript-engine-1.7R2.jar -DgroupId=rhino -DartifactId=js -Dversion=1.7R2 -Dpackaging=jar
call mvn install:install-file -Dfile=.\sapdb-7.6.00.30.5567.jar -DgroupId=com.sap -DartifactId=sapdb -Dversion=7.6.00.30.5567 -Dpackaging=jar
call mvn install:install-file -Dfile=.\sqljdbc4-4.0.jar -DgroupId=com.microsoft -DartifactId=sqljdbc4 -Dversion=4.0 -Dpackaging=jar
call mvn install:install-file -Dfile=.\SqlserverIntegratedAuthentication-4.0.jar -DgroupId=com.microsoft -DartifactId=SqlserverIntegratedAuthentication -Dversion=4.0 -Dpackaging=jar
call mvn install:install-file -Dfile=.\ojdbc-11.2.0.3.jar -DgroupId=ojdbc -DartifactId=ojdbc -Dversion=11.2.0.3 -Dpackaging=jar
call mvn install:install-file -Dfile=.\db2jcc-9.5.jar -DgroupId=com.ibm -DartifactId=db2jcc -Dversion=9.5 -Dpackaging=jar
call mvn install:install-file -Dfile=.\db2jcc_license-9.5.jar -DgroupId=com.ibm -DartifactId=db2jcc_license -Dversion=9.5 -Dpackaging=jar