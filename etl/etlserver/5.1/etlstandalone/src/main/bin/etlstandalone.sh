#!/bin/sh

PROGDIR=`dirname "$0"`
TMPJ=/tmp/j$$
# here we have PROGNAME and PROGDIR which should be correct in most cases

OLDDIR=`pwd`
cd "$PROGDIR"

if [ "lib" ] ; then 
  LIB="lib"
fi

ARGS=""
while [ "$1" != "" ] ; do
  ARGS="$ARGS $1"
  shift
done

if [ -z "$ARGS" ] ; then
  ARGS="-h"
fi

JCP=""
for jar in "$LIB"/*.jar ; do
  JCP="$JCP${JCP:+:}$jar"
done
for jar in config/lib/*.jar ; do
  JCP="$JCP${JCP:+:}$jar"
done

##################################################
# Check for JAVA_HOME
##################################################
if [ -z "$JAVA_HOME" ]
then
    # If a java runtime is not defined, search the following
    # directories for a JVM and sort by version. Use the highest
    # version number.

    # Java search path
    JAVA_LOCATIONS="\
        /usr/java \
        /usr/bin \
        /usr/local/bin \
        /usr/local/java \
        /usr/local/jdk \
        /usr/local/jre \
        /opt/java \
        /opt/jdk \
        /opt/jre \
            "
    JAVA_NAMES="java jdk jre"
    for N in $JAVA_NAMES ; do
        for L in $JAVA_LOCATIONS ; do
            [ -d $L ] || continue
            find $L -name "$N" ! -type d | grep -v threads | while read J ; do
                [ -x $J ] || continue
                VERSION=`eval $J -version 2>&1`
                [ $? = 0 ] || continue
                VERSION=`expr "$VERSION" : '.*"\(1.[0-9\.]*\)["_]'`
                [ "$VERSION" = "" ] && continue
                expr $VERSION \< 1.2 >/dev/null && continue
                echo $VERSION:$J
            done
        done
    done | sort | tail -1 > $TMPJ
    JAVA=`cat $TMPJ | cut -d: -f2`
    JVERSION=`cat $TMPJ | cut -d: -f1`

    JAVA_HOME=`dirname $JAVA`
    while [ ! -z "$JAVA_HOME" -a "$JAVA_HOME" != "/" -a ! -f "$JAVA_HOME/lib/tools.jar" ] ; do
        JAVA_HOME=`dirname $JAVA_HOME`
          done
    [ "$JAVA_HOME" = "" ] && JAVA_HOME=
    [ "$JAVA_HOME" = "/" ] && JAVA_HOME=

    echo "Found JAVA=$JAVA in JAVA_HOME=$JAVA_HOME"
fi

if [ -z "$JAVACMD" ] ; then
  if [ -n "$JAVA_HOME"  ] ; then
    if [ -x "$JAVA_HOME/jre/sh/java" ] ; then
      # IBM's JDK on AIX uses strange locations for the executables
      JAVACMD="$JAVA_HOME/jre/sh/java"
    else
      JAVACMD="$JAVA_HOME/bin/java"
    fi
  else
    JAVACMD=$JAVA
  fi
fi

if [ ! -x "$JAVACMD" ] ; then
  echo "Error: JAVA_HOME is not defined correctly."
  echo "  We cannot execute $JAVACMD"
  exit 1
fi

if [ -z "$JAVA_HOME" ] ; then
  echo "Warning: JAVA_HOME environment variable is not set."
fi

exec "$JAVACMD" \
  -Xmx1024m -cp $JCP:${artifactId}-${version}.jar \
  com.jedox.etl.core.run.StandaloneClient \
  $ARGS
  
cd $OLDDIR