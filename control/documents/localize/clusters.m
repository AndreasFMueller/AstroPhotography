#
# clusters.m -- 
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
n = 50;
N = 700 + 3 * n;

l = 400;

results = zeros(N, 2);
results(1:700,1) = l * (rand(700, 1) - 0.5);
results(1:700,2) = l * (rand(700, 1) - 0.5);

sigma = 8;
results(701:700 + n,1) = normrnd(120,sigma,n,1);
results(701:700 + n,2) = normrnd( 50,sigma,n,1);

sigma = 5;
results(701 + n:700 + 2 * n,1) = normrnd(-10,sigma,n,1);
results(701 + n:700 + 2 * n,2) = normrnd( 30,sigma,n,1);

sigma = 12;
results(701 + 2 * n:700 + 3 * n,1) = normrnd( 50,sigma,n,1);
results(701 + 2 * n:700 + 3 * n,2) = normrnd(-20,sigma,n,1);

results

fid = fopen("noise.mp", "w");
fprintf(fid, "verbatimtex\n");
fprintf(fid, "\\documentclass{article}\n");
fprintf(fid, "\\usepackage{times}\n");
fprintf(fid, "\\usepackage{amsmath}\n");
fprintf(fid, "\\usepackage{amssymb}\n");
fprintf(fid, "\\usepackage{amsfonts}\n");
fprintf(fid, "\\usepackage{txfonts}\n");
fprintf(fid, "\\begin{document}\n");
fprintf(fid, "etex;\n");

fprintf(fid, "def circle(expr z, r) =\n");
fprintf(fid, "\t((0,r)\n");
fprintf(fid, "\tfor a = 10 step 10 until 360:\n");
fprintf(fid, "\t\t..((0,r) rotated a)\n");
fprintf(fid, "\tendfor\n");
fprintf(fid, "\t\t..cycle) shifted z\n");
fprintf(fid, "enddef;\n");

fprintf(fid, "beginfig(1)\n");
fprintf(fid, "pickup pencircle scaled 1pt;\n");
fprintf(fid, "path q;\n");
fprintf(fid, "q := circle((120,50), 8);\n");
fprintf(fid, "draw q withcolor red;\n");
fprintf(fid, "q := circle((-10,30), 5);\n");
fprintf(fid, "draw q withcolor red;\n");
fprintf(fid, "q := circle((50,-20), 12);\n");
fprintf(fid, "draw q withcolor red;\n");
fprintf(fid, "pickup pencircle scaled 1.5pt;\n");
for i = (1:N)
	fprintf(fid, "draw (%.2f, %.2f);\n", results(i, 1), results(i, 2));
endfor
fprintf(fid, "path p;\n");
fprintf(fid, "p = (-70,-100)--(180,-100)--(180,100)--(-70,100)--cycle;\n");
fprintf(fid, "clip currentpicture to p;\n");
fprintf(fid, "pickup pencircle scaled 1pt;\n");
fprintf(fid, "draw p;\n");
fprintf(fid, "endfig;\n");
fprintf(fid, "end\n");
fclose(fid);
