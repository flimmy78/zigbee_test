
#include <stdio.h>


int main(int argc, char* argv[])
{
	//test de double/float

	double d1,d2,d3;
	float f1,f2,f3;

	d1 = 15.67789;
	d2 = 59.8465;
	d3 = d1 * d2;

	fprintf(stdout, "d3 = %g\n", d3);

	f1 = 5.268;
	f2 = 8.547;
	f3 = f1 / f2;

	fprintf(stdout, "f3 = %g\n", f3);

	return 0;
}


