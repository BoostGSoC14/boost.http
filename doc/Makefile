http.html: http.adoc
	asciidoctor -r asciidoctor-diagram -a toc=left http.adoc

http.epub: http.xml
	pandoc -f docbook http.xml -o http.epub

http.xml: http.adoc
	asciidoctor -r asciidoctor-diagram -b docbook5 http.adoc
