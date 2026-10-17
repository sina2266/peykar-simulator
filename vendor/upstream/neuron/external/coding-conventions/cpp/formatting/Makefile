# Targets:
#
#   README.md: rebuild the Markdown documentation (default target)
#
#   all: alias for README.md
#
#   clean: remove top generated files
#
#   distclean: clean + remove intermediate files
#

PANDOC ?= pandoc

SNIPPETS = $(wildcard snippets/*.cpp)

all: README.md

clean:
	$(RM) README.md README.html

distclean: clean
	$(RM) .clang-format

# README.html
.md.html: $<
	$(PANDOC) -s -c github-pandoc.css $< -o $@
README.html: github-pandoc.css

# README.md
README.md: README.md.jinja formatting.py $(SNIPPETS)
	../../bin/format --lang c++
	./formatting.py $< $@

.SUFFIXES: .md .html
