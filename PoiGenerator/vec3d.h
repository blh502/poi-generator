#ifndef VEC3D_H_
#define VEC3D_H_

#include <cmath>

class Vec3d
{
public:
	double x;
	double y;
	double z;

	Vec3d()
	: x(0.0), y(0.0), z(0.0)
	{ }

	Vec3d(double set_x, double set_y, double set_z)
	: x(set_x), y(set_y), z(set_z)
	{ }

	~Vec3d() {
	}

	Vec3d operator-() const
	{
		return (*this) * -1.0;
	}

	Vec3d operator+(const Vec3d & rhs) const
	{
		return Vec3d(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Vec3d operator-(const Vec3d & rhs) const
	{
		return (*this) + (-rhs);
	}

	Vec3d operator*(const double scalar) const
	{
		return Vec3d(x * scalar, y * scalar, z * scalar);
	}

	Vec3d operator/(const double scalar) const
	{
		return (*this) * (1.0 / scalar);
	}

	double length()
	{
		return sqrt((x * x) + (y * y) + (z * z));
	}

	double distance(const Vec3d & other)
	{
		return (other - (*this)).length();
	}

	Vec3d normalise()
	{
		return (*this) / length();
	}
};

#endif // VEC3D_H_