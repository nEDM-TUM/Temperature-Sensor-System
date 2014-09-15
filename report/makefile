main = short
# put the names of figure files here.  include the .eps
figures = smiley.eps logo.eps

# if no references, run "touch main.bbl; touch referencefile.bib"
# or delete dependency in .dvi and tarball sections
referencefile = refs

TEX = latex

# support subdirectories
VPATH = Figs

# reruns latex if needed.  to get rid of this capability, delete the
# three lines after the rule.  Delete .bbl dependency if not using
# BibTeX references.
# idea from http://ctan.unsw.edu.au/help/uk-tex-faq/Makefile
$(main).dvi : $(figures) $(main).tex $(main).bbl
	while ($(TEX) $(main) ; \
	grep -q "Rerun to get cross" $(main).log ) do true ; \
	done


# keep .eps files in the same directory as the .fig files
%.eps : %.fig
	fig2dev -L eps $< > $(dir $< )$@

$(main).pdf : $(main).ps
	ps2pdf $(main).ps $(main).pdf

pdf : $(main).pdf

$(main).ps : $(main).dvi
	dvips $(main)

ps : $(main).ps 

# make can't know all the sourcefiles.  some file may not have
# sourcefiles, e.g. eps's taken from other documents. 
$(main).tar.gz : $(figures) $(main).tex $(referencefile).bib
	tar -czvf $(main).tar.gz $^

tarball: $(main).tar.gz
