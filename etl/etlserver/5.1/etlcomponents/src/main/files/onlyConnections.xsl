<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">			
<xsl:template match="/">
	<xsl:for-each select="project/connections">
		<connections><!-- This is needed when
		1. "connections" Element itself should also be included
		2. When the resulting tree should have one root 
		-->
	 		<xsl:copy-of select="@*|node()" />
	 	</connections> 
	</xsl:for-each>
</xsl:template>
</xsl:stylesheet> 