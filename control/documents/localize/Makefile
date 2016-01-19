#
# Makefile -- build localize images
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
localize_FILES = localize-1.pdf localize-2.pdf localize-3.pdf localize-4.pdf
noise_FILES = noise-1.pdf noise-2.pdf

all:	$(noise_FILES) $(localize_FILES)

localize: $(localize_FILES)
$(localize_FILES):	localize.mp stars.mp
	TEX=latex mpost localize.mp
	for f in localize.[0-9]*; \
	do \
		mptopdf $${f}; \
	done

stars.mp:	background.m
	octave background.m

noise.mp:	clusters.m
	octave clusters.m

noise:	$(noise_FILES)
$(noise_FILES):	noise.mp
	TEX=latex mpost noise.mp
	for f in noise.[0-9]*; \
	do \
		mptopdf $${f}; \
	done
