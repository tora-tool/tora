<?xml version="1.0" ?>
<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:fn="http://www.w3.org/2005/xpath-functions"
		xmlns:wix="http://schemas.microsoft.com/wix/2006/wi">

  <!-- Copy all attributes and elements to the output. -->
  <xsl:template match="@*|*">
    <xsl:copy>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*" />
    </xsl:copy>
  </xsl:template>
  <xsl:output method="xml" indent="yes" />

  <!-- exclude all non-.dll non-.exe files -->
  <!-- <xsl:template match='wix:Wix/wix:Fragment/wix:ComponentGroup/wix:Component[not(contains(@Id,".dll")) and not(contains(@Id,".exe"))]'/> -->

</xsl:stylesheet>
