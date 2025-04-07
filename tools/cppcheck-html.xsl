<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" encoding="UTF-8"/>
  <xsl:template match="/">
    <html>
      <head>
        <title>Cppcheck Analysis Report</title>
        <style>
          body { font-family: Arial, sans-serif; margin: 20px; }
          h1 { color: #333; }
          .error { color: #d00; }
          .warning { color: #f80; }
          .style { color: #00a; }
          .performance { color: #080; }
          .information { color: #888; }
          table { border-collapse: collapse; width: 100%; }
          th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }
          tr:hover { background-color: #f5f5f5; }
          th { background-color: #4CAF50; color: white; }
        </style>
      </head>
      <body>
        <h1>Cppcheck Analysis Report</h1>
        <table>
          <tr>
            <th>Severity</th>
            <th>File</th>
            <th>Line</th>
            <th>Message</th>
          </tr>
          <xsl:for-each select="//error">
            <tr>
              <td class="{@severity}"><xsl:value-of select="@severity"/></td>
              <td><xsl:value-of select="location/@file"/></td>
              <td><xsl:value-of select="location/@line"/></td>
              <td><xsl:value-of select="@msg"/></td>
            </tr>
          </xsl:for-each>
        </table>
      </body>
    </html>
  </xsl:template>
</xsl:stylesheet>
