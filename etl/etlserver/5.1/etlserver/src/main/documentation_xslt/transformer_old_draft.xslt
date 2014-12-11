<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:template match="/">
		<html>
			<body>
				<h2 style="text-align:center;">Jedox ETL Project Documentation</h2>
				<h3>
					Project
					<xsl:value-of select="project/@name" />
				</h3>
				<p>
					Description:
					<xsl:value-of select="project/description" />
				</p>
				<p>
					Version:
					<xsl:value-of select="project/version" />
					<br />
					Documentation created on:
					<xsl:value-of select="project/date" />
				</p>
				<!-- Variables part -->
				<h3>List of Variables</h3>
				<hr />
				<br />
				<xsl:for-each select="project/variables/variable">
					<xsl:value-of select="@name" />
					(default:
					<xsl:value-of select="title" />
					<br />
				</xsl:for-each>
				<!-- Connections part -->
				<h3>List of Connections</h3>
				<hr />
				<xsl:for-each select="project/connections/connection">
					<h3>
						Connection
						<xsl:value-of select="@name" />
					</h3>
					Type:
					<xsl:value-of select="@type" />
					<br />
					Description:
					<xsl:value-of select="description" />
					<!-- usedby part -->
					<xsl:if test="usedby != ''">
						<br />
						<br />
						Used in components:
						<xsl:if test="usedby/extract != ''">
							<br />
							<xsl:for-each select="usedby/extract">
								extract "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<xsl:if test="usedby/transform != ''">
							<br />
							<xsl:for-each select="usedby/transform">
								transform "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<xsl:if test="usedby/load != ''">
							<br />
							<xsl:for-each select="usedby/load">
								load "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<br />
					</xsl:if>
				</xsl:for-each>


				<!-- extracts part -->
				<h3>List of Extracts</h3>
				<hr />
				<xsl:for-each select="project/extracts/extract">
					<h3>
						Extract
						<xsl:value-of select="@name" />
					</h3>
					Type:
					<xsl:value-of select="@type" />
					<br />
					Description:
					<xsl:value-of select="description" />
					<!-- usedby part -->
					<xsl:if test="usedby != ''">
						<br />
						<br />
						Used by components:
						<xsl:if test="usedby/transform != ''">
							<br />
							<xsl:for-each select="usedby/transform">
								transform "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<xsl:if test="usedby/load != ''">
							<br />
							<xsl:for-each select="usedby/load">
								load "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<br />
					</xsl:if>
				</xsl:for-each>

				<!-- transform part -->
				<h3>List of Transforms</h3>
				<hr />
				<xsl:for-each select="project/transforms/transform">
					<h3>
						Transfom
						<xsl:value-of select="@name" />
					</h3>
					Type:
					<xsl:value-of select="@type" />
					<br />
					Description:
					<xsl:value-of select="description" />

					<!-- uses part -->
					<xsl:if test="uses != ''">
						<br />
						<br />
						Uses components:
						<xsl:if test="uses/extract != ''">
							<br />
							<xsl:for-each select="uses/extract">
								extract "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<xsl:if test="uses/transform != ''">
							<br />
							<xsl:for-each select="uses/transform">
								transform "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<xsl:if test="uses/function != ''">
							<br />
							<xsl:for-each select="uses/function">
								function "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
					</xsl:if>

					<!-- usedby part -->
					<xsl:if test="usedby != ''">
						<br />
						<br />
						Used by components:
						<xsl:if test="usedby/transform != ''">
							<br />
							<xsl:for-each select="usedby/transform">
								transform "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
						<xsl:if test="usedby/load != ''">
							<br />
							<xsl:for-each select="usedby/load">
								load "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
					</xsl:if>
				</xsl:for-each>


				<!-- loads part -->
				<h3>List of Loads</h3>
				<hr />
				<xsl:for-each select="project/loads/load">
					<h3>
						Load
						<xsl:value-of select="@name" />
					</h3>
					Type:
					<xsl:value-of select="@type" />
					<br />
					Description:
					<xsl:value-of select="description" />

					<!-- uses part -->
					<br />
					<br />
					Uses components:
					<br />
					<xsl:if test="uses/extract != ''">
						extract "<xsl:value-of select="uses/extract" />"
					</xsl:if>

					<xsl:if test="uses/transform != ''">
						transform "<xsl:value-of select="uses/transform" />"
					</xsl:if>

					<!-- usedby part -->
					<xsl:if test="usedby != ''">
						<br />
						<br />
						Used by components:
						<br />
						connection "<xsl:value-of select="usedby/connection" />"
						<br />
						<xsl:for-each select="usedby/jobs">
							job "<xsl:value-of select="." />"
							<xsl:if test="not(position()=last())">
								,
							</xsl:if>
						</xsl:for-each>
					</xsl:if>
				</xsl:for-each>

				<!-- loads part -->
				<h3>List of Jobs</h3>
				<hr />
				<xsl:for-each select="project/jobs/job">
					<h3>
						Job
						<xsl:value-of select="@name" />
					</h3>
					Type:
					<xsl:value-of select="@type" />
					<br />
					Description:
					<xsl:value-of select="description" />

					<!-- uses part -->
					<br />
					<br />
					Uses components:
					<xsl:if test="uses/load != ''">
						<br />
						<xsl:for-each select="uses/load">
							load "<xsl:value-of select="." />"
							<xsl:if test="not(position()=last())">
								,
							</xsl:if>
						</xsl:for-each>
					</xsl:if>
					<xsl:if test="uses/job != ''">
						<br />
						<xsl:for-each select="uses/job">
							job "<xsl:value-of select="." />"
							<xsl:if test="not(position()=last())">
								,
							</xsl:if>
						</xsl:for-each>
					</xsl:if>

					<!-- usedby part -->
					<xsl:if test="usedby != ''">
						<br />
						<br />
						Used by components:
						<xsl:if test="usedby/job != ''">
							<br />
							<xsl:for-each select="usedby/job">
								job "<xsl:value-of select="." />"
								<xsl:if test="not(position()=last())">
									,
								</xsl:if>
							</xsl:for-each>
						</xsl:if>
					</xsl:if>
				</xsl:for-each>
				<h3>ANNEX:</h3>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet> 