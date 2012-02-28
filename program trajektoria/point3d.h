#pragma once

/**
 * Punkt w przestrzeni.
 */
class Point3D
{
public:
	Point3D(): x(-1), y(-1), z(-1){}
	Point3D(float xx, float yy, float zz): x(xx), y(yy), z(zz){}
	
	float x, y, z;
};
