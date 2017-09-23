http.html: http.adoc
	asciidoctor -a toc=left http.adoc

http.epub: http.xml
	pandoc -f docbook http.xml -o http.epub

http.xml: http.adoc
	asciidoctor -b docbook5 http.adoc
