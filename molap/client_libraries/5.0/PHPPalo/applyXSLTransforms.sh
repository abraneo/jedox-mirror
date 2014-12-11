#!/bin/sh

xalan -in ../Common/Documentation.xml -xsl MakeWrapperDecl.xslt -out wrap.h.h
xalan -in ../Common/Documentation.xml -xsl MakeWrapperEntries.xslt -out php_jedox_palo.c.h
xalan -in ../Common/Documentation.xml -xsl MakeWrappers.xslt -out wrap.cpp.h
