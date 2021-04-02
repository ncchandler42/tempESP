all:
	pandoc --filter pandoc-citeproc notes.md -o notes.pdf

open:
	xdg-open notes.pdf
	
clean:
	rm -f notes.pdf