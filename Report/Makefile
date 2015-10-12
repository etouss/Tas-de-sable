
SRC= abstract.tex \
     intro.tex \
     section1.tex \
     section2.tex \
     section3.tex \
		 section4.tex \
		 section5.tex \
		 section6.tex \
		 section7.tex \
		 section8.tex \
     conclusion.tex \
		 appendix.tex \
     # bilio.bib

all: main.pdf

main.bbl: main.tex 
	pdflatex -file-line-error main.tex
	# bibtex main.aux

main.pdf: $(SRC) main.bbl
	pdflatex -file-line-error main.tex

clean:
	rm *.syn* *.aux *.log *~ *.bbl *.bst *.out
