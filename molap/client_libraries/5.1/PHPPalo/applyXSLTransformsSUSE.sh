#!/bin/sh

Xalan -o wrap.h.h ../Common/Documentation.xml MakeWrapperDecl.xslt
Xalan -o php_jedox_palo.c.h ../Common/Documentation.xml MakeWrapperEntries.xslt
Xalan -o wrap.cpp.h ../Common/Documentation.xml MakeWrappers.xslt
