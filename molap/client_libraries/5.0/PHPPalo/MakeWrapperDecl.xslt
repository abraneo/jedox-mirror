<?xml version="1.0" encoding="utf-8"?>
<stylesheet version="1.0" xmlns="http://www.w3.org/1999/XSL/Transform"
                          xmlns:doc="http://tempuri.org/Palo/SpreadsheetFuncs/Documentation.xsd">
  <output method="text" indent="no"/>

  <template match="doc:Function">
    <if test="./doc:PHPSpecific">
      <text disable-output-escaping="yes">
      ZEND_FUNCTION( </text>
      <value-of select="@c_name"/>
      <text disable-output-escaping="yes"> );</text>
    </if>
  </template>
  
</stylesheet>
