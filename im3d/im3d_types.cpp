#include "im3d_types.h"
#include "im3d_math.h"

namespace Im3d
{

Color::Color(const Vec4 &_rgba)
{
    v = (U32)(_rgba.x * 255.0f) << 24;
    v |= (U32)(_rgba.y * 255.0f) << 16;
    v |= (U32)(_rgba.z * 255.0f) << 8;
    v |= (U32)(_rgba.w * 255.0f);
}
Color::Color(const Vec3 &_rgb, float _alpha)
{
    v = (U32)(_rgb.x * 255.0f) << 24;
    v |= (U32)(_rgb.y * 255.0f) << 16;
    v |= (U32)(_rgb.z * 255.0f) << 8;
    v |= (U32)(_alpha * 255.0f);
}
Color::Color(float _r, float _g, float _b, float _a)
{
    v = (U32)(_r * 255.0f) << 24;
    v |= (U32)(_g * 255.0f) << 16;
    v |= (U32)(_b * 255.0f) << 8;
    v |= (U32)(_a * 255.0f);
}

/******************************************************************************

                                 im3d_math

******************************************************************************/

// Vec3
Vec3::Vec3(const Vec4& _v)
	: x(_v.x)
	, y(_v.y)
	, z(_v.z)
{
}

// Vec4
Vec4::Vec4(Color _rgba)
	: x(_rgba.getR())
	, y(_rgba.getG())
	, z(_rgba.getB())
	, w(_rgba.getA())
{
}

// Mat3
Mat3::Mat3(float _diagonal)
{
	(*this)(0, 0) = _diagonal; (*this)(0, 1) = 0.0f;      (*this)(0, 2) = 0.0f;
	(*this)(1, 0) = 0.0f;      (*this)(1, 1) = _diagonal; (*this)(1, 2) = 0.0f;
	(*this)(2, 0) = 0.0f;      (*this)(2, 1) = 0.0f;      (*this)(2, 2) = _diagonal;
}
Mat3::Mat3(
	float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22
	)
{
	(*this)(0, 0) = m00; (*this)(0, 1) = m01; (*this)(0, 2) = m02;
	(*this)(1, 0) = m10; (*this)(1, 1) = m11; (*this)(1, 2) = m12;
	(*this)(2, 0) = m20; (*this)(2, 1) = m21; (*this)(2, 2) = m22;
}
Mat3::Mat3(const Vec3& _colX, const Vec3& _colY, const Vec3& _colZ)
{
	(*this)(0, 0) = _colX.x; (*this)(0, 1) = _colY.x; (*this)(0, 2) = _colZ.x;
	(*this)(1, 0) = _colX.y; (*this)(1, 1) = _colY.y; (*this)(1, 2) = _colZ.y;
	(*this)(2, 0) = _colX.z; (*this)(2, 1) = _colY.z; (*this)(2, 2) = _colZ.z;
}
Mat3::Mat3(const Mat4& _mat4)
{
	(*this)(0, 0) = _mat4(0, 0); (*this)(0, 1) = _mat4(0, 1); (*this)(0, 2) = _mat4(0, 2);
	(*this)(1, 0) = _mat4(1, 0); (*this)(1, 1) = _mat4(1, 1); (*this)(1, 2) = _mat4(1, 2);
	(*this)(2, 0) = _mat4(2, 0); (*this)(2, 1) = _mat4(2, 1); (*this)(2, 2) = _mat4(2, 2);
}
Vec3 Mat3::getCol(int _i) const
{
	return Vec3((*this)(0, _i), (*this)(1, _i), (*this)(2, _i));
}
Vec3 Mat3::getRow(int _i) const
{
	return Vec3((*this)(_i, 0), (*this)(_i, 1), (*this)(_i, 2));
}
void Mat3::setCol(int _i, const Vec3& _v)
{
	(*this)(0, _i) = _v.x;
	(*this)(1, _i) = _v.y;
	(*this)(2, _i) = _v.z;
}
void Mat3::setRow(int _i, const Vec3& _v)
{
	(*this)(_i, 0) = _v.x;
	(*this)(_i, 1) = _v.y;
	(*this)(_i, 2) = _v.z;
}
Vec3 Mat3::getScale() const
{
	return Vec3(Length(getCol(0)), Length(getCol(1)), Length(getCol(2)));
}
void Mat3::setScale(const Vec3& _scale)
{
	Vec3 scale = _scale / getScale();
	setCol(0, getCol(0) * scale.x);
	setCol(1, getCol(1) * scale.y);
	setCol(2, getCol(2) * scale.z);
}

} // namespace Im3d