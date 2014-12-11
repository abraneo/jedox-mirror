<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:variable name="vLower" select="'abcdefghijklmnopqrstuvwxyz'" />
	<xsl:variable name="vUpper" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />


	<xsl:template match="usedby|uses">
		<tr>
			<td valign="top">
				<b>
					<xsl:choose>
						<xsl:when test="name(.)='usedby'">
							Used By
						</xsl:when>
						<xsl:otherwise>
							Uses
						</xsl:otherwise>
					</xsl:choose>
				</b>
			</td>
			<td>
				<xsl:apply-templates select="connection" mode="main" />
				<xsl:apply-templates select="extract" mode="main" />
				<xsl:apply-templates select="transform" mode="main" />
				<xsl:apply-templates select="function" mode="main" />
				<xsl:apply-templates select="load" mode="main" />
				<xsl:apply-templates select="job" mode="main" />
			</td>
		</tr>
	</xsl:template>

	<xsl:template match="@name">
		<a name="{name(..)}{.}">
			<h3>
				<xsl:copy-of
					select="concat(translate(substring(name(..),1,1), $vLower, $vUpper),substring(name(..), 2))" />
				<xsl:value-of select="' '" />
				<xsl:text>"</xsl:text><xsl:value-of select="." /><xsl:text>"</xsl:text>
			</h3>
		</a>
	</xsl:template>
	
	<xsl:template match="function" mode="function_name">
		<a name="{name(.)}{@name}{usedby/transform[1]}">
			<h3>
				<xsl:copy-of
					select="concat(translate(substring(name(.),1,1), $vLower, $vUpper),substring(name(.), 2))" />
				<xsl:value-of select="' '" />
				<xsl:text>"</xsl:text><xsl:value-of select="@name" /><xsl:text>" in transform "</xsl:text>
				<xsl:value-of select="usedby/transform[1]" />"
			</h3>
		</a>
	</xsl:template>

	<xsl:template match="project">
		<xsl:param name="header" />
		<br />
		<table width="100%" border="0">
			<tr>
				<td align="left">
					<p class="listOf">
						<xsl:copy-of select="$header" />
					</p>
				</td>
				<td align="right">
					<a href="#top" class="toplink">TOP</a>
				</td>
			</tr>
		</table>
		<hr />
	</xsl:template>

	<xsl:template match="@type">
		<tr>
			<td>
				<b>Type</b>
			</td>
			<td>
				<xsl:value-of select="." />
			</td>
		</tr>
	</xsl:template>

	<xsl:template match="description">
		<tr>
			<td valign="top">
				<b>Description</b>
			</td>
			<td>
				<xsl:value-of select="." />
			</td>
		</tr>
	</xsl:template>

	<xsl:template match="connection|extract|transform|function|load|job"
		mode="main">
		<xsl:if test="position()=1">
			<xsl:value-of select="name(.)" />
			<xsl:if test="position()!=last()">
			
			<xsl:text>s</xsl:text>
			
			</xsl:if><xsl:text>:</xsl:text>
		</xsl:if>
		<xsl:text>"</xsl:text>
		<xsl:choose>
		<xsl:when test="name(.)!='function'">
		<a href="#{name(.)}{.}">
			<xsl:value-of select="." />
		</a>
				</xsl:when>
		<xsl:otherwise>
		<a href="#{name(.)}{.}{../../@name}">
			<xsl:value-of select="." />
		</a>
		</xsl:otherwise>
		</xsl:choose>
		<xsl:text>"</xsl:text>
		<xsl:choose>
			<xsl:when test="not(position()=last())">
				<xsl:text>,</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<br />
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="connection|extract|transform|function|load|job"
		mode="top">
		<tr>
			<td>
			<xsl:choose>
			<xsl:when test="name(.)!='function'">
				<a href="#{name(.)}{@name}">
					<xsl:value-of select="@name" />
				</a>
				</xsl:when>
				<xsl:otherwise>
				<a href="#{name(.)}{@name}{usedby/transform[1]}">
					<xsl:value-of select="@name" />
				</a>
			</xsl:otherwise>
		</xsl:choose>
			</td>
			<td>
				<xsl:value-of select="@type" />
			</td>
		</tr>
	</xsl:template>

	<xsl:template match="/">
		<html>
			<head>
				<link rel="stylesheet" type="text/css" href="my_style.css" />
			</head>
			<body>
				<a name="top" />
				<h5>Jedox ETL Project Documentation</h5>
				<h3>
					<xsl:text>Project "</xsl:text><xsl:value-of select="project/@name" /><xsl:text>"</xsl:text>
				</h3>
				
				<xsl:if test="project/invalid != ''">
				<p class="error">
					Status:
					INVALID
				</p>
				</xsl:if>
				
				<p class="main">
					<xsl:text>Description:</xsl:text>
					<xsl:value-of select="project/description" />
				</p>
				<p class="main">
					<xsl:text>ETL Server Version:</xsl:text>
					<xsl:value-of select="project/version" />
					<br />
					<xsl:text>Documentation created on:</xsl:text>
					<xsl:value-of select="project/date" />
				</p>
				<hr style="margin-top: 20px;margin-bottom: 20px;" />
				<!-- add the tables -->
				<table class="toptable">
					<tr>
						<th>Variable</th>
						<th>Default Value</th>
					</tr>
					<xsl:for-each select="project/variables/*">
						<tr>
							<td>
								<xsl:value-of select="name(.)" />
							</td>
							<td>
								<xsl:value-of select="." />
							</td>
						</tr>
					</xsl:for-each>
				</table>

				<br />
				<table class="toptable">
					<tr>
						<th>Connection</th>
						<th>Type</th>
					</tr>
					<xsl:apply-templates select="project/connections/connection"
						mode="top" />
				</table>

				<br />
				<table class="toptable">
					<tr>
						<th>Extract</th>
						<th>Type</th>
					</tr>
					<xsl:apply-templates select="project/extracts/extract"
						mode="top" />
				</table>

				<br />
				<table class="toptable">
					<tr>
						<th>Transform</th>
						<th>Type</th>
					</tr>
					<xsl:apply-templates select="project/transforms/transform"
						mode="top" />
				</table>
				
				<br />
				<table class="toptable">
					<tr>
						<th>Function</th>
						<th>Type</th>
					</tr>
					<xsl:apply-templates select="project/functions/function"
						mode="top" />
				</table>

				<br />
				<table class="toptable">
					<tr>
						<th>Load</th>
						<th>Type</th>
					</tr>
					<xsl:apply-templates select="project/loads/load"
						mode="top" />
				</table>

				<br />
				<table class="toptable">
					<tr>
						<th>Job</th>
						<th>Type</th>
					</tr>
					<xsl:apply-templates select="project/jobs/job"
						mode="top" />
				</table>

				<!-- Connections part -->
				<xsl:apply-templates select="/project">
					<xsl:with-param name="header">
						List of Connections
					</xsl:with-param>
				</xsl:apply-templates>
				<xsl:for-each select="project/connections/connection">
					<xsl:apply-templates select="@name" />
					<table class="maintable">
						<col class="header" span="1" />
						<xsl:apply-templates select="@type" />
						<xsl:apply-templates select="description" />
						<xsl:apply-templates select="usedby" />
						<xsl:apply-templates select="uses" />
					</table>
				</xsl:for-each>

				<!-- extracts part -->
				<xsl:apply-templates select="/project">
					<xsl:with-param name="header">
						List of Extracts
					</xsl:with-param>
				</xsl:apply-templates>
				<xsl:for-each select="project/extracts/extract">
					<xsl:apply-templates select="@name" />
					<table class="maintable">
						<col class="header" span="1" />
						<xsl:apply-templates select="@type" />
						<xsl:apply-templates select="description" />
						<xsl:apply-templates select="usedby" />
						<xsl:apply-templates select="uses" />
					</table>
				</xsl:for-each>

				<!-- transforms part -->
				<xsl:apply-templates select="/project">
					<xsl:with-param name="header">
						List of Transforms
					</xsl:with-param>
				</xsl:apply-templates>
				<xsl:for-each select="project/transforms/transform">
					<xsl:apply-templates select="@name" />
					<table class="maintable">
						<col class="header" span="1" />
						<xsl:apply-templates select="@type" />
						<xsl:apply-templates select="description" />
						<xsl:apply-templates select="usedby" />
						<xsl:apply-templates select="uses" />
					</table>
				</xsl:for-each>
				
				<!-- functions part -->
				<xsl:apply-templates select="/project">
					<xsl:with-param name="header">
						List of Functions
					</xsl:with-param>
				</xsl:apply-templates>
				<xsl:for-each select="project/functions/function">
					<xsl:apply-templates select="." mode="function_name"/>
					<table class="maintable">
						<col class="header" span="1" />
						<xsl:apply-templates select="@type" />
						<xsl:apply-templates select="description" />
						<xsl:apply-templates select="usedby" />
						<xsl:apply-templates select="uses" />
					</table>
				</xsl:for-each>

				<!-- loads part -->
				<xsl:apply-templates select="/project">
					<xsl:with-param name="header">
						List of Loads
					</xsl:with-param>
				</xsl:apply-templates>
				<xsl:for-each select="project/loads/load">
					<xsl:apply-templates select="@name" />
					<table class="maintable">
						<col class="header" span="1" />
						<xsl:apply-templates select="@type" />
						<xsl:apply-templates select="description" />
						<xsl:apply-templates select="usedby" />
						<xsl:apply-templates select="uses" />
					</table>
				</xsl:for-each>

				<!-- Jobs part -->
				<xsl:apply-templates select="/project">
					<xsl:with-param name="header">
						List of Jobs
					</xsl:with-param>
				</xsl:apply-templates>
				<xsl:for-each select="project/jobs/job">
					<xsl:apply-templates select="@name" />
					<table class="maintable">
						<col class="header" span="1" />
						<xsl:apply-templates select="@type" />
						<xsl:apply-templates select="description" />
						<xsl:apply-templates select="usedby" />
						<xsl:apply-templates select="uses" />
					</table>
				</xsl:for-each>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet> 