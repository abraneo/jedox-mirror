<?xml version="1.0" encoding="utf-8"?>
<stylesheet version="1.0" xmlns="http://www.w3.org/1999/XSL/Transform"
                          xmlns:doc="http://tempuri.org/Palo/SpreadsheetFuncs/Documentation.xsd">
  <output method="text" indent="no"/>
  
  <template match="doc:Function">
    <if test="./doc:PHPSpecific">
      <text disable-output-escaping="yes">
      ZEND_FUNCTION( </text>
      <value-of select="@c_name"/>
      <text> ) {
        PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )-&gt;getContext(), &amp;PHPPaloSpreadsheetFuncs::</text>
      <value-of select="@internal_name"/>
      <text disable-output-escaping="yes"> )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
      }
      </text>
    </if>
  </template>
</stylesheet>
