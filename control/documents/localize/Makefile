#
# Makefile -- build localize images
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
localize_FILES = localize-1.pdf
localize: $(localize_FILES)
$(localize_FILES):	localize.mp stars.mp
	TEX=latex mpost localize.mp
	for f in localize.[0-9]*; \
	do \
		mptopdf $${f}; \
	done

stars.mp:	background.m
	octave background.m
