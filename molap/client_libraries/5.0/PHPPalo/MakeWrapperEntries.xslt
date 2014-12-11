<?xml version="1.0" encoding="utf-8"?>
<stylesheet version="1.0" xmlns="http://www.w3.org/1999/XSL/Transform"
                          xmlns:doc="http://tempuri.org/Palo/SpreadsheetFuncs/Documentation.xsd">
  <output method="text" indent="no"/>

  <template match="doc:Function">
    <if test="./doc:PHPSpecific">
      <text disable-output-escaping="yes">
      ZEND_FE( </text>
      <value-of select="@c_name"/>
      <if test="not(./doc:PHPSpecific/@pass_by_ref)">
        <text disable-output-escaping="yes">, NULL)</text>
      </if>
      <if test="./doc:PHPSpecific/@pass_by_ref">
        <text disable-output-escaping="yes">, pass_all_by_reference)</text>
      </if>
    </if>
  </template>
</stylesheet>
