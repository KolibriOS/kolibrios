Web Proxy for WebView Browser (/programs/cmm/browser)
=====================================================

How to use it?
==============

1. Put this proxy.php to your WebServer which:
- supports PHP
- has curl binary which can be called in shell

2. Edit UserAgent in $your_useragent variable in proxy.php if needed

3. Change content of $your_local_page_address:
- you should write path from HTTP-server root to your file (if proxy.php (or any other name) is in /srv/http/dir1/proxy.php on your disk, then use '/dir1/proxy.php')
- if you renamed proxy.php as index.php:
  - if your browser calls something like http://yoursite.domain/dir1/index.php?site=..., then nothing should be changed
  - if your browser calls something like http://yoursite.domain/dir1/?site=..., then remove 'index.php' at path ending (example: '/dir1/')

4. Change a proxy address in WebView source code (line where is something like http://somename.domain/?site=) to your address with '?site=' at the end. Recompile the browser.
