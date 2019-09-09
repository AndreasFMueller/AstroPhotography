#! /bin/sh
#
# mkconstellations.sh -- turn the ConstellationLinesAll2002.csv into
#                        a C data structure for plotting constellation
#                        lines
#
# (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
#

cat <<\EOF
/*
 * constellations.h -- constellation data
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
typedef struct {
	const char	*name;
	double	ra;
	double	dec;
	double	mag;
} constellation_point;

constellation_point	constellation_points[] = {
EOF

LANG=C
export LANG
awk -F, '{
	if ($2 == "") {
		printf("    {  NULL,   0.000000,   0.0000,  0.00 },\n")
	} else {
		printf("    { \"%s\",%11.6f,%9.4f,%6.2f },\n", $1, $3, $4, $5);
	}
}
END {
	printf("    {  NULL,   0.000000,   0.0000,  0.00 }\n")
}' ConstellationLinesAll2002.csv

cat <<\EOF
};
static const int	constellation_size = sizeof(constellation_points) / sizeof(constellation_point);
EOF
