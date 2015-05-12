#pragma annotation "author" "JohnJohn"
#pragma annotation "version" "1.0"

surface types(
	float f = 1;
	string s = "s";
	color c = color( 1, 2, 3 );
	point p = point( 0, 1, 2 );
	vector v = vector( -1, 0, 1 );
	normal n = normal( -2, -1, 0 );
	matrix m = matrix( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 );
	float fa[2] = { 1, 2 };
	string sa[2] = { "one", "two" };
	color ca[2] = { 1, 2 };
	point pa[2] = { 2, 3 };
	vector va[2] = { 2, 3 };
	normal na[2] = { 2, 3 };
	matrix ma[2] = { matrix( 1 ), matrix( 1 ) };
	float fav[] = {};
	string sav[] = {};
	color cav[] = {};
	point pav[] = {};
	vector vav[] = {};
	normal nav[] = {};
	matrix mav[] = {};
	float f3[3] = { 1, 2, 3 };
	shader sh = null;
	shader sha[] = {};
	shader sha3[3] = { null, null, null };
)
{
	Ci = 1;
	Oi = 1;
}
