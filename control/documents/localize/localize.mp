verbatimtex
\documentclass{article}
\usepackage{times}
\usepackage{amsmath}
\usepackage{amssymb}
\usepackage{amsfonts}
\usepackage{txfonts}
\begin{document}
etex;

input TEX;

TEXPRE("%&latex" & char(10) & "\documentclass[12pt]{article}\usepackage{times}\usepackage{txfonts}\begin{document}");
TEXPOST("\end{document}");

color starcolor;
starcolor = (0.7,0.7,0.7);

numeric triangleoffset;
triangleoffset = 5;

numeric l;
l = 120;

numeric rotation;
rotation = 0;

numeric featurearrow;
featurearrow := 0;

numeric positionarrow;
positionarrow := 0;

def stern(expr z, m, a, mlimit) =
	if m < mlimit:
		pickup pencircle scaled (1 + (7-m)/2);
		draw z rotated a withcolor starcolor;
	fi;
enddef;

input stars.mp;

def circle(expr r) =
	(0,r)
	for winkel = 10 step 10 until 360:
		..((0,r) rotated winkel)
	endfor
	..cycle
enddef;

def centerpoint =
	((30,20) rotated (rotation))
enddef;

def drawfeature(expr sa, sb, sc, a, c, name) =
	pickup pencircle scaled 1pt;
	draw ((sa--sb--sc--cycle) rotated a) withcolor c;
	if 1 = featurearrow:
		drawarrow ((sa--(0.33333 * (sa + sb + sc))) rotated a) withcolor red;
	fi;
	path guard;
	guard := (circle(4)) shifted centerpoint;
	%fill guard withcolor (1,1,0);
	pickup pencircle scaled 1pt;
	if 1 = positionarrow:
		drawarrow ((sa rotated a)--
			guard intersectionpoint
				((sa rotated a)--centerpoint))
					withcolor blue;
	fi;
	pickup pencircle scaled 4.5pt;
	draw (sa rotated a) withcolor white;
	pickup pencircle scaled 3.5pt;
	draw (sa rotated a);
	pickup pencircle scaled 4.0pt;
	draw (sb rotated a) withcolor white;
	pickup pencircle scaled 3.0pt;
	draw (sb rotated a);
	pickup pencircle scaled 3.5pt;
	draw (sc rotated a) withcolor white;
	pickup pencircle scaled 2.5pt;
	draw (sc rotated a);
	label(TEX(name), ((0.33333 * (sa + sb + sc)) rotated a) shifted (0,8))
		withcolor c;
enddef;

def features(expr i, a, c, name) =
	if i = 1:
		drawfeature((-40,-80), (-20,  0), (-80,80), a, c, name);
	elseif i = 2:
		drawfeature((-40,-80), ( 60,-20), (-20, 0), a, c, name);
	elseif i = 3:
		drawfeature((-20,  0), (60,-20), (20, 60), a, c, name);
	elseif i = 4:
		drawfeature((-20,  0), (20,60), (-80, 80), a, c, name);
	elseif i = 5:
		drawfeature((60, -20), (100,20), (20, 60), a, c, name);

	elseif i = 6:
		drawfeature((-40,-80), (-20,  0), (-80,80), a + 70, c, name);
	elseif i = 7:
		drawfeature((-40,-80), ( 60,-20), (-20, 0), a - 70, c, name);
	elseif i = 8:
		drawfeature((-20,  0), (60,-20), (20, 60), a + 40, c, name);
	elseif i = 9:
		drawfeature((-20,  0), (20,60), (-80, 80), a - 45, c, name);
	elseif i = 10:
		drawfeature((60, -20), (100,20), (20, 60), a, c, name);
	fi;
enddef;

def cross(expr z, a) =
	pickup pencircle scaled 1pt;
	draw ((z + (-10,0))--(z + (10,0))) rotated a withcolor blue;
	draw ((z + (0,-10))--(z + (0,10))) rotated a withcolor blue;
	pickup pencircle scaled 4pt;
	draw z rotated a withcolor blue;
	pickup pencircle scaled 3pt;
	draw z rotated a withcolor white;
enddef;

%
%
%
beginfig(1)

starbackground(0, 6);
path border;
border := (-l,-l)--(l,-l)--(l,l)--(-l,l)--cycle;
clip currentpicture to border;
pickup pencircle scaled 1pt;
draw border;

%rotation := -20;
%cross((30,20), -20);

color featurecolor;
featurecolor := (1,0.8,0);

features(triangleoffset + 1, 0, featurecolor, "1");
features(triangleoffset + 2, 0, featurecolor, "2");
features(triangleoffset + 3, 0, featurecolor, "3");
features(triangleoffset + 4, 0, featurecolor, "4");
features(triangleoffset + 5, 0, featurecolor, "5");

endfig;

beginfig(2)

starbackground(0, 6);
path border;
border := (-l,-l)--(l,-l)--(l,l)--(-l,l)--cycle;
clip currentpicture to border;
pickup pencircle scaled 1pt;
draw border;

rotation := -20;
cross((30,20), -20);

color featurecolor;
featurecolor := (1,0.8,0);

positionarrow := 0;
features(triangleoffset + 1, 0, featurecolor, "1");
features(triangleoffset + 2, 0, featurecolor, "2");
features(triangleoffset + 4, 0, featurecolor, "4");
positionarrow := 1;
features(triangleoffset + 3, 0, featurecolor, "3");
features(triangleoffset + 5, 0, featurecolor, "5");

endfig;

color darkgreen;
color brightgreen;
darkgreen = (0,0.6,0);
brightgreen = (0,0.8,0.2);

%
%
%
beginfig(3)

rotation := 0;

starbackground(20, 5.1);
path border;
border = (-60,-40)--(120,-40)--(120,80)--(-60,80)--cycle;
clip currentpicture to border;
pickup pencircle scaled 1pt;
draw border;

positionarrow := 0;

features(triangleoffset + 3, 20, darkgreen, "3");
features(triangleoffset + 5, 20, brightgreen, "5");

endfig;

%
%
%
beginfig(4)

rotation := 0;

starbackground(20, 5.1);
path border;
border = (-60,-40)--(120,-40)--(120,80)--(-60,80)--cycle;
clip currentpicture to border;
pickup pencircle scaled 1pt;
draw border;

cross((30,20), 0);

positionarrow := 1;
features(triangleoffset + 3, 20, darkgreen, "3");
features(triangleoffset + 5, 20, brightgreen, "5");

endfig;

end

