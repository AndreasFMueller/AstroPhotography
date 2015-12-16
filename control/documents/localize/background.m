#
# background.m -- 
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
N = 2100;

l = 500;

results = zeros(N, 3);
results(:,1) = l * (rand(N, 1) - 0.5);
results(:,2) = l * (rand(N, 1) - 0.5);

xmin = 1;
alpha = 2.5;

for i = (1:N)
	r = rand(1);
	x = xmin * (1-r)^(1/(1-alpha));
	mag = 7 - x;
	if (mag < 0) 
		mag = 0;
	endif
	if (mag > 6)
		mag = 6;
	endif
	results(i, 3) = mag;
endfor;

results

fid = fopen("stars.mp", "w");
fprintf(fid, "def starbackground(expr a, mlimit) =\n");
for i = (1:N)
	fprintf(fid, "stern((%.2f, %.2f), %.1f, a, mlimit);\n", results(i, 1), results(i, 2), results(i, 3));
endfor
fprintf(fid, "enddef;\n");
fclose(fid);


