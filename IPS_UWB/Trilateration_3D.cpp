#include "Trilateration_3D.h"
#include <math.h>
#include <stdio.h>
double square(float x)
{
	return x*x;
}
ywPos calcTagPosition(CpQubeSize QubeSize, CpRealDistance rd)
{
	ywPos tag = { 0, 0, 0 };

	//float Cx2 = Anchor2.x - Anchor1.x;
	//float Cy3 = Anchor3.y - Anchor1.y;
	
	float Cx2 = QubeSize.width;
	float Cy3 = QubeSize.length;
	float zZ = 0.0f;

	tag.x = (square(rd.Anchor[1]) - square(rd.Anchor[2]) + square(Cx2)) / (2 * Cx2);
	tag.y = (square(rd.Anchor[1]) - square(rd.Anchor[3]) + square(Cy3)) / (2 * Cy3);
	zZ = square(rd.Anchor[1]) - square(tag.x) - square(tag.y);
	
	if (zZ < 0) tag.z = 1.0f;
	else tag.z = sqrtf(zZ);


	//sprintf(debugStr, "r1 = %f, square(r1) = %f, tag.x = %f", r1, square(r1), tag.x);
	//sprintf(debugStr, "Tag's Position - x:%6.1f, y:%6.1f, z:%6.1f ///  r1^2 = %6.1f, x^2 = %.1f, y^2 = %.1f zZ = %.1f", tag.x, tag.y, tag.z, square(r1), square(tag.x), square(tag.y), zZ);


	tag.x /= 10.0f; tag.y /= 10.0f; tag.z /= 10.f;
	

	return tag;
}


