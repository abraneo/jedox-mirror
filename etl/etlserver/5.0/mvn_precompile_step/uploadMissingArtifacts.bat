call mvn install:install-file -Dfile=.\palojlib-5.0.36.jar -DgroupId=com.jedox -DartifactId=palojlib -Dversion=5.0.36 -Dpackaging=jar
call mvn install:install-file -Dfile=.\groovy-all-1.8.6.jar -DgroupId=org.codehaus.groovy -DartifactId=groovy-all -Dversion=1.8.6 -Dpackaging=jar
call mvn install:install-file -Dfile=.\jython-engine-2.2.1.jar -DgroupId=ScriptEngine -DartifactId=jython-engine -Dversion=2.2.1 -Dpackaging=jar
call mvn install:install-file -Dfile=.\javascript-engine-1.6R7.jar -DgroupId=ScriptEngine -DartifactId=javascript-engine -Dversion=1.6R7 -Dpackaging=jar
call mvn install:install-file -Dfile=.\sapdb-7.6.00.30.5567.jar -DgroupId=com.sap -DartifactId=sapdb -Dversion=7.6.00.30.5567 -Dpackaging=jar
call mvn install:install-file -Dfile=.\sqljdbc4-4.0.jar -DgroupId=com.microsoft -DartifactId=sqljdbc4 -Dversion=4.0 -Dpackaging=jar
call mvn install:install-file -Dfile=.\SqlserverIntegratedAuthentication-4.0.jar -DgroupId=com.microsoft -DartifactId=SqlserverIntegratedAuthentication -Dversion=4.0 -Dpackaging=jar
call mvn install:install-file -Dfile=.\ojdbc-11.2.0.3.jar -DgroupId=ojdbc -DartifactId=ojdbc -Dversion=11.2.0.3 -Dpackaging=jar
call mvn install:install-file -Dfile=.\db2jcc-3.1.57.jar -DgroupId=com.ibm -DartifactId=db2jcc -Dversion=3.1.57 -Dpackaging=jar
call mvn install:install-file -Dfile=.\db2jcc_license-3.1.57.jar -DgroupId=com.ibm -DartifactId=db2jcc_license -Dversion=3.1.57 -Dpackaging=jar
pause