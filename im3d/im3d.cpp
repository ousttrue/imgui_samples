/*	CHANGE LOG
	==========
	2018-12-09 (v1.14) - Fixed memory leak in context dtor.
	2018-07-29 (v1.13) - Deprecated Draw(), instead use EndFrame() followed by GetDrawListCount() + GetDrawLists() to access draw data directly.
	2018-06-07 (v1.12) - Color_ constants are constexpr (fixed issues with static init).
	2018-03-20 (v1.11) - Thread-local context ptr (IM3D_THREAD_LOCAL_CONTEXT_PTR).
	                   - MergeContexts API.
	2018-01-27 (v1.10) - Added AppData::m_viewDirection (world space), fixed aligned gizmo fadeout in ortho views.
	                   - Gizmo snapping is absolute, not relative.
	2018-01-14 (v1.09) - Culling API.
	                   - Moved DrawPrimitiveSize to im3d.cpp, renamed as VertsPerDrawPrimitive.
	                   - Added traits + tag dispatch for math utilities (Min, Max, etc).
	2017-10-21 (v1.08) - Added DrawAlignedBoxFilled(), DrawSphereFilled(), fixed clamped ranges for auto LOD in high order primitives.
	2017-10-14 (v1.07) - Layers API.
	2017-07-03 (v1.06) - Rotation gizmo improvements; avoid selection failure at grazing angles + improved rotation behavior.
	2017-04-04 (v1.05) - GetActiveId() returns the gizmo id set by the app instead of an internal id. Added Gizmo* variants which take an Id directly.
	2017-03-24 (v1.04) - DrawArrow() interface changed (world space length/pixel thickness instead of head fraction).
	2017-03-01 (v1.02) - Configurable VertexData alignment (IM3D_VERTEX_ALIGNMENT).
	2017-02-23 (v1.01) - Removed AppData::m_tanHalfFov, replaced with AppData::m_projScaleY. Added AppData::m_projOrtho.
*/
#include "im3d.h"
#include "im3d_context.h"
#include "im3d_math.h"
#include "im3d_internal.h"

#include <cstdlib>
#include <cstring>
#include <cfloat>

#ifndef IM3D_CULL_PRIMITIVES
	#define IM3D_CULL_PRIMITIVES 0
#endif
#ifndef IM3D_CULL_GIZMOS
	#define IM3D_CULL_GIZMOS 0
#endif

// Compiler
#if defined(__GNUC__)
	#define IM3D_COMPILER_GNU
#elif defined(_MSC_VER)
	#define IM3D_COMPILER_MSVC
#else
	#error im3d: Compiler not defined
#endif

#if defined(IM3D_COMPILER_GNU)
	#define if_likely(e)   if ( __builtin_expect(!!(e), 1) )
	#define if_unlikely(e) if ( __builtin_expect(!!(e), 0) )
//#elif defined(IM3D_COMPILER_MSVC)
  // not defined for MSVC
#else
	#define if_likely(e)   if(!!(e))
	#define if_unlikely(e) if(!!(e))
#endif

// Internal config/debugging
#define IM3D_RELATIVE_SNAP 0          // snap relative to the gizmo stored position/rotation/scale (else snap is absolute)
#define IM3D_GIZMO_DEBUG   0          // draw debug bounds for gizmo intersections

namespace Im3d{

inline AppData& GetAppData()                                                 { return GetContext().getAppData();   }
inline void     NewFrame()                                                   { GetContext().reset();               }
inline void     EndFrame()                                                   { GetContext().endFrame();            }
inline void     Draw()                                                       { GetContext().draw();                }

inline const DrawList* GetDrawLists()                                        { return GetContext().getDrawLists();     }
inline U32             GetDrawListCount()                                    { return GetContext().getDrawListCount(); }

inline void  BeginPoints()                                                   { GetContext().begin(PrimitiveMode_Points);        }
inline void  BeginLines()                                                    { GetContext().begin(PrimitiveMode_Lines);         }
inline void  BeginLineLoop()                                                 { GetContext().begin(PrimitiveMode_LineLoop);      }
inline void  BeginLineStrip()                                                { GetContext().begin(PrimitiveMode_LineStrip);     }
inline void  BeginTriangles()                                                { GetContext().begin(PrimitiveMode_Triangles);     }
inline void  BeginTriangleStrip()                                            { GetContext().begin(PrimitiveMode_TriangleStrip); }
inline void  End()                                                           { GetContext().end(); }

inline void  Vertex(const Vec3& _position)                                   { GetContext().vertex(_position, GetContext().getSize(), GetContext().getColor()); }
inline void  Vertex(const Vec3& _position, Color _color)                     { GetContext().vertex(_position, GetContext().getSize(), _color); }
inline void  Vertex(const Vec3& _position, float _size)                      { GetContext().vertex(_position, _size, GetContext().getColor()); }
inline void  Vertex(const Vec3& _position, float _size, Color _color)        { GetContext().vertex(_position, _size, _color); }
inline void  Vertex(float _x, float _y, float _z)                            { Vertex(Vec3(_x, _y, _z)); }
inline void  Vertex(float _x, float _y, float _z, Color _color)              { Vertex(Vec3(_x, _y, _z), _color); }
inline void  Vertex(float _x, float _y, float _z, float _size)               { Vertex(Vec3(_x, _y, _z), _size); }
inline void  Vertex(float _x, float _y, float _z, float _size, Color _color) { Vertex(Vec3(_x, _y, _z), _size, _color); }

inline void  PushDrawState()                                                 { Context& ctx = GetContext(); ctx.pushColor(ctx.getColor()); ctx.pushAlpha(ctx.getAlpha()); ctx.pushSize(ctx.getSize()); ctx.pushEnableSorting(ctx.getEnableSorting()); }
inline void  PopDrawState()                                                  { Context& ctx = GetContext(); ctx.popColor(); ctx.popAlpha(); ctx.popSize(); ctx.popEnableSorting(); }

inline void  PushColor()                                                     { GetContext().pushColor(GetContext().getColor()); }
inline void  PushColor(Color _color)                                         { GetContext().pushColor(_color);                  }
inline void  PopColor()                                                      { GetContext().popColor();                         }
inline void  SetColor(Color _color)                                          { GetContext().setColor(_color);                   }
inline void  SetColor(float _r, float _g, float _b, float _a)                { GetContext().setColor(Color(_r, _g, _b, _a));    }
inline Color GetColor()                                                      { return GetContext().getColor();                  }

inline void  PushAlpha()                                                     { GetContext().pushAlpha(GetContext().getAlpha()); }
inline void  PushAlpha(float _alpha)                                         { GetContext().pushAlpha(_alpha);                  }
inline void  PopAlpha()                                                      { GetContext().popAlpha();                         }
inline void  SetAlpha(float _alpha)                                          { GetContext().setAlpha(_alpha);                   }
inline float GetAlpha()                                                      { return GetContext().getAlpha();                  }

inline void  PushSize()                                                      { GetContext().pushSize(GetContext().getAlpha());  }
inline void  PushSize(float _size)                                           { GetContext().pushSize(_size);                    }
inline void  PopSize()                                                       { GetContext().popSize();                          }
inline void  SetSize(float _size)                                            { GetContext().setSize(_size);                     }
inline float GetSize()                                                       { return GetContext().getSize();                   }

inline void  PushEnableSorting()                                             { GetContext().pushEnableSorting(GetContext().getEnableSorting()); }
inline void  PushEnableSorting(bool _enable)                                 { GetContext().pushEnableSorting(_enable); }
inline void  PopEnableSorting()                                              { GetContext().popEnableSorting();         }
inline void  EnableSorting(bool _enable)                                     { GetContext().setEnableSorting(_enable);  }

inline void  PushMatrix()                                                    { GetContext().pushMatrix(GetContext().getMatrix()); }
inline void  PushMatrix(const Mat4& _mat4)                                   { GetContext().pushMatrix(_mat4);                    }
inline void  PopMatrix()                                                     { GetContext().popMatrix();                          }
inline void  SetMatrix(const Mat4& _mat4)                                    { GetContext().setMatrix(_mat4);                     }
inline void  SetIdentity()                                                   { GetContext().setMatrix(Mat4(1.0f));                }

inline void  PushId()                                                        { GetContext().pushId(GetContext().getId()); }
inline void  PushId(Id _id)                                                  { GetContext().pushId(_id);                  }
inline void  PushId(const char* _str)                                        { GetContext().pushId(MakeId(_str));         }
inline void  PushId(const void* _ptr)                                        { GetContext().pushId(MakeId(_ptr));         }
inline void  PushId(int _i)                                                  { GetContext().pushId(MakeId(_i));           }
inline void  PopId()                                                         { GetContext().popId();                      }
inline Id    GetId()                                                         { return GetContext().getId();               }
inline Id    GetActiveId()                                                   { return GetContext().m_appActiveId;         }
inline Id    GetHotId()                                                      { return GetContext().m_appHotId;            }

inline void  PushLayerId()                                                   { GetContext().pushLayerId(GetContext().getLayerId()); }
inline void  PushLayerId(Id _layer)                                          { GetContext().pushLayerId(_layer); }
inline void  PushLayerId(const char* _str)                                   { PushLayerId(MakeId(_str));        }
inline void  PopLayerId()                                                    { GetContext().popLayerId();        }
inline Id    GetLayerId()                                                    { return GetContext().getLayerId(); }

inline bool GizmoTranslation(const char* _id, float _translation_[3], bool _local)                   { return GizmoTranslation(MakeId(_id), _translation_, _local);   }
inline bool GizmoRotation(const char* _id, float _rotation_[3*3], bool _local)                       { return GizmoRotation(MakeId(_id), _rotation_, _local);         }
inline bool GizmoScale(const char* _id, float _scale_[3])                                            { return GizmoScale(MakeId(_id), _scale_);                       }
inline bool Gizmo(const char* _id, float _translation_[3], float _rotation_[3*3], float _scale_[3])  { return Gizmo(MakeId(_id), _translation_, _rotation_, _scale_); }
inline bool Gizmo(const char* _id, float _transform_[4*4])                                           { return Gizmo(MakeId(_id), _transform_);                        }

inline bool IsVisible(const Vec3& _origin, float _radius)                    { return GetContext().isVisible(_origin, _radius); }
inline bool IsVisible(const Vec3& _min, const Vec3& _max)                    { return GetContext().isVisible(_min, _max);       }

inline Context& GetContext()                                                 { return *internal::g_CurrentContext; }
inline void     SetContext(Context& _ctx)                                    { internal::g_CurrentContext = &_ctx; }

inline void     MergeContexts(Context& _dst_, const Context& _src)           { _dst_.merge(_src); }
}

using namespace Im3d;


Color::Color(const Vec4& _rgba)
{
	v  = (U32)(_rgba.x * 255.0f) << 24;
	v |= (U32)(_rgba.y * 255.0f) << 16;
	v |= (U32)(_rgba.z * 255.0f) << 8;
	v |= (U32)(_rgba.w * 255.0f);
}
Color::Color(const Vec3& _rgb, float _alpha)
{
	v  = (U32)(_rgb.x * 255.0f) << 24;
	v |= (U32)(_rgb.y * 255.0f) << 16;
	v |= (U32)(_rgb.z * 255.0f) << 8;
	v |= (U32)(_alpha * 255.0f);
}
Color::Color(float _r, float _g, float _b, float _a)
{
	v  = (U32)(_r * 255.0f) << 24;
	v |= (U32)(_g * 255.0f) << 16;
	v |= (U32)(_b * 255.0f) << 8;
	v |= (U32)(_a * 255.0f);
}

void Im3d::MulMatrix(const Mat4& _mat4)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * _mat4);
}
void Im3d::Translate(float _x, float _y, float _z)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Translation(Vec3(_x, _y, _z)));
}
void Im3d::Translate(const Vec3& _vec3)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Translation(_vec3));
}
void Im3d::Rotate(const Vec3& _axis, float _angle)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Mat4(Rotation(_axis, _angle)));
}
void Im3d::Rotate(const Mat3& _rotation)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Mat4(_rotation));
}
void Im3d::Scale(float _x, float _y, float _z)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Mat4(Scale(Vec3(_x, _y, _z))));
}


void Im3d::DrawXyzAxes()
{
	Context& ctx = GetContext();
	ctx.pushColor(ctx.getColor());
	ctx.begin(PrimitiveMode_Lines);
		ctx.vertex(Vec3(0.0f, 0.0f, 0.0f), ctx.getSize(), Color_Red);
		ctx.vertex(Vec3(1.0f, 0.0f, 0.0f), ctx.getSize(), Color_Red);
		ctx.vertex(Vec3(0.0f, 0.0f, 0.0f), ctx.getSize(), Color_Green);
		ctx.vertex(Vec3(0.0f, 1.0f, 0.0f), ctx.getSize(), Color_Green);
		ctx.vertex(Vec3(0.0f, 0.0f, 0.0f), ctx.getSize(), Color_Blue);
		ctx.vertex(Vec3(0.0f, 0.0f, 1.0f), ctx.getSize(), Color_Blue);
	ctx.end();
	ctx.popColor();

}
void Im3d::DrawPoint(const Vec3& _position, float _size, Color _color)
{
	Context& ctx = GetContext();
	ctx.begin(PrimitiveMode_Points);
		ctx.vertex(_position, _size, _color);
	ctx.end();
}
void Im3d::DrawLine(const Vec3& _a, const Vec3& _b, float _size, Color _color)
{
	Context& ctx = GetContext();
	ctx.begin(PrimitiveMode_Lines);
		ctx.vertex(_a, _size, _color);
		ctx.vertex(_b, _size, _color);
	ctx.end();
}
void Im3d::DrawQuad(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d)
{
	Context& ctx = GetContext();
	ctx.begin(PrimitiveMode_LineLoop);
		ctx.vertex(_a);
		ctx.vertex(_b);
		ctx.vertex(_c);
		ctx.vertex(_d);
	ctx.end();
}
void Im3d::DrawQuad(const Vec3& _origin, const Vec3& _normal, const Vec2& _size)
{
	Context& ctx = GetContext();
	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal, ctx.getAppData().m_worldUp));
	DrawQuad(
		Vec3(-_size.x,  _size.y, 0.0f),
		Vec3( _size.x,  _size.y, 0.0f),
		Vec3( _size.x, -_size.y, 0.0f),
		Vec3(-_size.x, -_size.y, 0.0f)
		);
	ctx.popMatrix();
}
void Im3d::DrawQuadFilled(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d)
{
	Context& ctx = GetContext();
	ctx.begin(PrimitiveMode_Triangles);
		ctx.vertex(_a);
		ctx.vertex(_b);
		ctx.vertex(_c);
		ctx.vertex(_a);
		ctx.vertex(_c);
		ctx.vertex(_d);
	ctx.end();
}
void Im3d::DrawQuadFilled(const Vec3& _origin, const Vec3& _normal, const Vec2& _size)
{
	Context& ctx = GetContext();
	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal, ctx.getAppData().m_worldUp));
	DrawQuadFilled(
		Vec3(-_size.x, -_size.y, 0.0f),
		Vec3( _size.x, -_size.y, 0.0f),
		Vec3( _size.x,  _size.y, 0.0f),
		Vec3(-_size.x,  _size.y, 0.0f)
		);
	ctx.popMatrix();
}
void Im3d::DrawCircle(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible(_origin, _radius)) {
			return;
		}
	#endif


	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(_origin, _radius, 8, 48);
	}
	_detail = Max(_detail, 3);

 	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius, sinf(rad) * _radius, 0.0f));
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawCircleFilled(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible(_origin, _radius)) {
			return;
		}
	#endif

	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(_origin, _radius, 8, 64);
	}
	_detail = Max(_detail, 3);

 	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_Triangles);
		float cp = _radius;
		float sp = 0.0f;
		for (int i = 1; i <= _detail; ++i) {
			ctx.vertex(Vec3(0.0f, 0.0f, 0.0f));
			ctx.vertex(Vec3(cp, sp, 0.0f));
			float rad = TwoPi * ((float)i / (float)_detail);
			float c = cosf(rad) * _radius;
			float s = sinf(rad) * _radius;
			ctx.vertex(Vec3(c, s, 0.0f));
			cp = c;
			sp = s;
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawSphere(const Vec3& _origin, float _radius, int _detail)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible(_origin, _radius)) {
			return;
		}
	#endif

	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(_origin, _radius, 8, 48);
	}
	_detail = Max(_detail, 3);

 // xy circle
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius + _origin.x, sinf(rad) * _radius + _origin.y, 0.0f + _origin.z));
		}
	ctx.end();
 // xz circle
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius + _origin.x, 0.0f + _origin.y, sinf(rad) * _radius + _origin.z));
		}
	ctx.end();
 // yz circle
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(0.0f + _origin.x, cosf(rad) * _radius + _origin.y, sinf(rad) * _radius + _origin.z));
		}
	ctx.end();
}
void Im3d::DrawSphereFilled(const Vec3& _origin, float _radius, int _detail)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible(_origin, _radius)) {
			return;
		}
	#endif

	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(_origin, _radius, 12, 32);
	}
	_detail = Max(_detail, 6);

	ctx.begin(PrimitiveMode_Triangles);
		float yp = -_radius;
		float rp = 0.0f;
		for (int i = 1; i <= _detail / 2; ++i) {
			float y = ((float)i / (float)(_detail / 2)) * 2.0f - 1.0f;
			float r = cosf(y * HalfPi) * _radius;
			y = sinf(y * HalfPi) * _radius;

			float xp = 1.0f;
			float zp = 0.0f;
			for (int j = 1; j <= _detail; ++j) {
				float x = ((float)j / (float)(_detail)) * TwoPi;
				float z = sinf(x);
				x = cosf(x);

				ctx.vertex(Vec3(xp * rp, yp, zp * rp));
				ctx.vertex(Vec3(xp * r,  y,  zp * r));
				ctx.vertex(Vec3(x  * r,  y,  z  * r));

				ctx.vertex(Vec3(xp * rp, yp, zp * rp));
				ctx.vertex(Vec3(x  * r,  y,  z  * r));
				ctx.vertex(Vec3(x  * rp, yp, z  * rp));

				xp = x;
				zp = z;
			}

			yp = y;
			rp = r;
		}
	ctx.end();
}
void Im3d::DrawAlignedBox(const Vec3& _min, const Vec3& _max)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible(_min, _max)) {
			return;
		}
	#endif
	ctx.begin(PrimitiveMode_LineLoop);
		ctx.vertex(Vec3(_min.x, _min.y, _min.z));
		ctx.vertex(Vec3(_max.x, _min.y, _min.z));
		ctx.vertex(Vec3(_max.x, _min.y, _max.z));
		ctx.vertex(Vec3(_min.x, _min.y, _max.z));
	ctx.end();
	ctx.begin(PrimitiveMode_LineLoop);
		ctx.vertex(Vec3(_min.x, _max.y, _min.z));
		ctx.vertex(Vec3(_max.x, _max.y, _min.z));
		ctx.vertex(Vec3(_max.x, _max.y, _max.z));
		ctx.vertex(Vec3(_min.x, _max.y, _max.z));
	ctx.end();
	ctx.begin(PrimitiveMode_Lines);
		ctx.vertex(Vec3(_min.x, _min.y, _min.z));
		ctx.vertex(Vec3(_min.x, _max.y, _min.z));
		ctx.vertex(Vec3(_max.x, _min.y, _min.z));
		ctx.vertex(Vec3(_max.x, _max.y, _min.z));
		ctx.vertex(Vec3(_min.x, _min.y, _max.z));
		ctx.vertex(Vec3(_min.x, _max.y, _max.z));
		ctx.vertex(Vec3(_max.x, _min.y, _max.z));
		ctx.vertex(Vec3(_max.x, _max.y, _max.z));
	ctx.end();
}
void Im3d::DrawAlignedBoxFilled(const Vec3& _min, const Vec3& _max)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible(_min, _max)) {
			return;
		}
	#endif

	ctx.pushEnableSorting(true);
 // x+
	DrawQuadFilled(
		Vec3(_max.x, _max.y, _min.z),
		Vec3(_max.x, _max.y, _max.z),
		Vec3(_max.x, _min.y, _max.z),
		Vec3(_max.x, _min.y, _min.z)
		);
 // x-
	DrawQuadFilled(
		Vec3(_min.x, _min.y, _min.z),
		Vec3(_min.x, _min.y, _max.z),
		Vec3(_min.x, _max.y, _max.z),
		Vec3(_min.x, _max.y, _min.z)
		);
 // y+
	DrawQuadFilled(
		Vec3(_min.x, _max.y, _min.z),
		Vec3(_min.x, _max.y, _max.z),
		Vec3(_max.x, _max.y, _max.z),
		Vec3(_max.x, _max.y, _min.z)
		);
 // y-
	DrawQuadFilled(
		Vec3(_max.x, _min.y, _min.z),
		Vec3(_max.x, _min.y, _max.z),
		Vec3(_min.x, _min.y, _max.z),
		Vec3(_min.x, _min.y, _min.z)
		);
 // z+
	DrawQuadFilled(
		Vec3(_max.x, _min.y, _max.z),
		Vec3(_max.x, _max.y, _max.z),
		Vec3(_min.x, _max.y, _max.z),
		Vec3(_min.x, _min.y, _max.z)
		);
 // z-
	DrawQuadFilled(
		Vec3(_min.x, _min.y, _min.z),
		Vec3(_min.x, _max.y, _min.z),
		Vec3(_max.x, _max.y, _min.z),
		Vec3(_max.x, _min.y, _min.z)
		);
	ctx.popEnableSorting();
}
void Im3d::DrawCylinder(const Vec3& _start, const Vec3& _end, float _radius, int _detail)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible((_start + _end) * 0.5f, Max(Length2(_start - _end), _radius))) {
			return;
		}
	#endif

	Vec3 org  = _start + (_end - _start) * 0.5f;
	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(org, _radius, 16, 24);
	}
	_detail = Max(_detail, 3);

	float ln  = Length(_end - _start) * 0.5f;
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i <= _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i <= _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_Lines);
		for (int i = 0; i <= 6; ++i) {
			float rad = TwoPi * ((float)i / 6.0f) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			ctx.vertex(Vec3(0.0f, 0.0f,  ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawCapsule(const Vec3& _start, const Vec3& _end, float _radius, int _detail)
{
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible((_start + _end) * 0.5f, Max(Length2(_start - _end), _radius))) {
			return;
		}
	#endif

	Vec3 org = _start + (_end - _start) * 0.5f;
	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(org, _radius, 6, 24);
	}
	_detail = Max(_detail, 3);

	float ln = Length(_end - _start) * 0.5f;
	int detail2 = _detail * 2; // force cap base detail to match ends
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
	 // yz silhoette + cap bases
		for (int i = 0; i <= detail2; ++i) {
			float rad = TwoPi * ((float)i / (float)detail2) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail) + Pi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(0.0f, cosf(rad), sinf(rad)) * _radius);
		}
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(0.0f, cosf(rad), sinf(rad)) * _radius);
		}
		for (int i = 0; i <= detail2; ++i) {
			float rad = TwoPi * ((float)i / (float)detail2) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_LineLoop);
	 // xz silhoette
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail) + Pi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), 0.0f, sinf(rad)) * _radius);
		}
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), 0.0f, sinf(rad)) * _radius);
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawPrism(const Vec3& _start, const Vec3& _end, float _radius, int _sides)
{
	_sides = Max(_sides, 2);
	Context& ctx = GetContext();
	#if IM3D_CULL_PRIMITIVES
		if (!ctx.isVisible((_start + _end) * 0.5f, Max(Length2(_start - _end), _radius))) {
			return;
		}
	#endif

	Vec3 org  = _start + (_end - _start) * 0.5f;
	float ln  = Length(_end - _start) * 0.5f;
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i <= _sides; ++i) {
			float rad = TwoPi * ((float)i / (float)_sides) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
		for (int i = 0; i <= _sides; ++i) {
			float rad = TwoPi * ((float)i / (float)_sides) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_Lines);
		for (int i = 0; i <= _sides; ++i) {
			float rad = TwoPi * ((float)i / (float)_sides) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			ctx.vertex(Vec3(0.0f, 0.0f,  ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawArrow(const Vec3& _start, const Vec3& _end, float _headLength, float _headThickness)
{
	Context& ctx = GetContext();

	if (_headThickness < 0.0f) {
		_headThickness = ctx.getSize() * 2.0f;
	}

	Vec3 dir = _end - _start;
	float dirlen = Length(dir);
	if (_headLength < 0.0f) {
		_headLength = Min(dirlen / 2.0f, ctx.pixelsToWorldSize(_end, _headThickness * 2.0f));
	}
	dir = dir / dirlen;

	Vec3 head = _end - dir * _headLength;
	ctx.begin(PrimitiveMode_Lines);
		ctx.vertex(_start);
		ctx.vertex(head);
		ctx.vertex(head, _headThickness, ctx.getColor());
		ctx.vertex(_end, 2.0f, ctx.getColor()); // \hack \todo 2.0f here compensates for the shader antialiasing (which reduces alpha when size < 2)
	ctx.end();
}


static constexpr U32 kFnv1aPrime32 = 0x01000193u;
static U32 Hash(const char* _buf, int _buflen, U32 _base)
{
	U32 ret = _base;
	const char* lim = _buf + _buflen;
	while (_buf < lim) {
		ret ^= (U32)*_buf++;
		ret *= kFnv1aPrime32;
	}
	return ret;
}
static U32 HashStr(const char* _str, U32 _base)
{
	U32 ret = _base;
	while (*_str) {
		ret ^= (U32)*_str++;
		ret *= kFnv1aPrime32;
	}
	return ret;
}
Im3d::Id Im3d::MakeId(const char* _str)
{
	return HashStr(_str, GetContext().getId());
}
Im3d::Id Im3d::MakeId(const void* _ptr)
{
	return Hash((const char*)&_ptr, sizeof(void*), GetContext().getId());
}
Im3d::Id Im3d::MakeId(int _i)
{
	return Hash((const char*)&_i, sizeof(int), GetContext().getId());
}




bool Im3d::GizmoTranslation(Id _id, float _translation_[3], bool _local)
{
	Context& ctx = GetContext();

	bool ret = false;
	Vec3* outVec3 = (Vec3*)_translation_;
	Vec3 drawAt = *outVec3;

	float worldHeight = ctx.pixelsToWorldSize(drawAt, ctx.m_gizmoHeightPixels);
	#if IM3D_CULL_GIZMOS
		if (!ctx.isVisible(drawAt, worldHeight)) {
			return false;
		}
	#endif

	ctx.pushId(_id);
	ctx.m_appId = _id;

	if (_local) {
		Mat4 localMatrix = ctx.getMatrix();
		localMatrix.setScale(Vec3(1.0f));
		ctx.pushMatrix(localMatrix);
	}

	float planeSize = worldHeight * (0.5f * 0.5f);
	float planeOffset = worldHeight * 0.5f;
	float worldSize = ctx.pixelsToWorldSize(drawAt, ctx.m_gizmoSizePixels);

	struct AxisG { Id m_id; Vec3 m_axis; Color m_color; };
	AxisG axes[] = {
		{ MakeId("axisX"), Vec3(1.0f, 0.0f, 0.0f), Color_Red   },
		{ MakeId("axisY"), Vec3(0.0f, 1.0f, 0.0f), Color_Green },
		{ MakeId("axisZ"), Vec3(0.0f, 0.0f, 1.0f), Color_Blue  }
	};
	struct PlaneG { Id m_id; Vec3 m_origin; };
	PlaneG planes[] = {
		{ MakeId("planeYZ"), Vec3(0.0f, planeOffset, planeOffset) },
		{ MakeId("planeXZ"), Vec3(planeOffset, 0.0f, planeOffset) },
		{ MakeId("planeXY"), Vec3(planeOffset, planeOffset, 0.0f) },
		{ MakeId("planeV"),  Vec3(0.0f, 0.0f, 0.0f) }
	};

 // invert axes if viewing from behind
	const AppData& appData = ctx.getAppData();
	/*Vec3 viewDir = appData.m_viewOrigin - *outVec3;
	for (int i = 0; i < 3; ++i) {
		if (Dot(axes[i].m_axis, viewDir) < 0.0f) {
			axes[i].m_axis = -axes[i].m_axis;
			for (int j = 0; j < 3; ++j) {
				planes[j].m_origin[i] = -planes[j].m_origin[i];
			}
		}
	}*/

 	Sphere boundingSphere(*outVec3, worldHeight * 1.5f); // expand the bs to catch the planar subgizmos
	Ray ray(appData.m_cursorRayOrigin, appData.m_cursorRayDirection);
	bool intersects = ctx.m_appHotId == ctx.m_appId || Intersects(ray, boundingSphere);

 // planes
 	ctx.pushEnableSorting(true);
	if (_local) {
	 // local planes need to be drawn with the pushed matrix for correct orientation
		for (int i = 0; i < 3; ++i) {
			const PlaneG& plane = planes[i];
			ctx.gizmoPlaneTranslation_Draw(plane.m_id, plane.m_origin, axes[i].m_axis, planeSize, Color_GizmoHighlight);
			axes[i].m_axis = Normalize(Vec3(ctx.getMatrix().getCol(i))); // if local, extract axes from the pushed matrix
			if (intersects) {
				ret |= ctx.gizmoPlaneTranslation_Behavior(plane.m_id, ctx.getMatrix() * plane.m_origin, axes[i].m_axis, appData.m_snapTranslation, planeSize, outVec3);
			}
		}

	} else {
		ctx.pushMatrix(Mat4(1.0f));
		for (int i = 0; i < 3; ++i) {
			const PlaneG& plane = planes[i];
			ctx.gizmoPlaneTranslation_Draw(plane.m_id, drawAt + plane.m_origin, axes[i].m_axis, planeSize, Color_GizmoHighlight);
			if (intersects) {
				ret |= ctx.gizmoPlaneTranslation_Behavior(plane.m_id, drawAt + plane.m_origin, axes[i].m_axis, appData.m_snapTranslation, planeSize, outVec3);
			}
		}
		ctx.popMatrix();
	}

	ctx.pushMatrix(Mat4(1.0f));

	if (intersects) {
	 // view plane (store the normal when the gizmo becomes active)
		Id currentId = ctx.m_activeId;
		Vec3& storedViewNormal= *((Vec3*)ctx.m_gizmoStateMat3.m);
		Vec3 viewNormal;
		if (planes[3].m_id == ctx.m_activeId) {
			viewNormal = storedViewNormal;
		} else {
			viewNormal = ctx.getAppData().m_viewDirection;
		}
		ret |= ctx.gizmoPlaneTranslation_Behavior(planes[3].m_id, drawAt, viewNormal, appData.m_snapTranslation, worldSize, outVec3);
		if (currentId != ctx.m_activeId) {
		 // gizmo became active, store the view normal
			storedViewNormal = viewNormal;
		}

	 // highlight axes if the corresponding plane is hot
		if (planes[0].m_id == ctx.m_hotId) { // YZ
			axes[1].m_color = axes[2].m_color = Color_GizmoHighlight;
		} else if (planes[1].m_id == ctx.m_hotId) { // XZ
			axes[0].m_color = axes[2].m_color = Color_GizmoHighlight;
		} else if (planes[2].m_id == ctx.m_hotId) { // XY
			axes[0].m_color = axes[1].m_color = Color_GizmoHighlight;
		} else if (planes[3].m_id == ctx.m_hotId) {
			axes[0].m_color = axes[1].m_color = axes[2].m_color = Color_GizmoHighlight;
		}
	}
 // draw the view plane handle
	ctx.begin(PrimitiveMode_Points);
		ctx.vertex(drawAt, ctx.m_gizmoSizePixels * 2.0f, planes[3].m_id == ctx.m_hotId ? Color_GizmoHighlight : Color_White);
	ctx.end();

 // axes
	for (int i = 0; i < 3; ++i) {
		AxisG& axis = axes[i];
		ctx.gizmoAxisTranslation_Draw(axis.m_id, drawAt, axis.m_axis, worldHeight, worldSize, axis.m_color);
		if (intersects) {
			ret |= ctx.gizmoAxisTranslation_Behavior(axis.m_id, drawAt, axis.m_axis, appData.m_snapTranslation, worldHeight, worldSize, outVec3);
		}
	}
	ctx.popMatrix();
	ctx.popEnableSorting();

	if (_local) {
		ctx.popMatrix();
	}

	ctx.popId();

	return ret;
}
bool Im3d::GizmoRotation(Id _id, float _rotation_[3*3], bool _local)
{
	Context& ctx = GetContext();

	Vec3 origin = ctx.getMatrix().getTranslation();
	float worldRadius = ctx.pixelsToWorldSize(origin, ctx.m_gizmoHeightPixels);
	#if IM3D_CULL_GIZMOS
		if (!ctx.isVisible(origin, worldRadius)) {
			return false;
		}
	#endif

	Id currentId = ctx.m_activeId; // store currentId to detect if the gizmo becomes active during this call
	ctx.pushId(_id);
	ctx.m_appId = _id;

	bool ret = false;
	Mat3& storedRotation = ctx.m_gizmoStateMat3;
	Mat3* outMat3 = (Mat3*)_rotation_;
	Vec3 euler = ToEulerXYZ(*outMat3);
	float worldSize = ctx.pixelsToWorldSize(origin, ctx.m_gizmoSizePixels);

	struct AxisG { Id m_id; Vec3 m_axis; Color m_color; };
	AxisG axes[] = {
		{ MakeId("axisX"), Vec3(1.0f, 0.0f, 0.0f), Color_Red   },
		{ MakeId("axisY"), Vec3(0.0f, 1.0f, 0.0f), Color_Green },
		{ MakeId("axisZ"), Vec3(0.0f, 0.0f, 1.0f), Color_Blue  }
	};
	Id viewId = MakeId("axisV");

	Sphere boundingSphere(origin, worldRadius);
	Ray ray(ctx.getAppData().m_cursorRayOrigin, ctx.getAppData().m_cursorRayDirection);
	bool intersects = ctx.m_appHotId == ctx.m_appId || Intersects(ray, boundingSphere);

	const AppData& appData = ctx.getAppData();

	if (_local) {
	 // extract axes from the pushed matrix
		for (int i = 0; i < 3; ++i) {
			if (ctx.m_activeId == axes[i].m_id) {
			 // use the stored matrix where the id is active, avoid rotating the axis frame during interaction (cause numerical instability)
				axes[i].m_axis = Normalize(Vec3(storedRotation.getCol(i)));
			} else {
				axes[i].m_axis = Normalize(Vec3(ctx.getMatrix().getCol(i)));
			}
		}
	}

	ctx.pushMatrix(Mat4(1.0f));
	for (int i = 0; i < 3; ++i) {
		if (i == 0 && (ctx.m_activeId == axes[1].m_id || ctx.m_activeId == axes[2].m_id || ctx.m_activeId == viewId)) {
			continue;
		}
		if (i == 1 && (ctx.m_activeId == axes[2].m_id || ctx.m_activeId == axes[0].m_id || ctx.m_activeId == viewId)) {
			continue;
		}
		if (i == 2 && (ctx.m_activeId == axes[0].m_id || ctx.m_activeId == axes[1].m_id || ctx.m_activeId == viewId)) {
			continue;
		}

		AxisG& axis = axes[i];
		ctx.gizmoAxislAngle_Draw(axis.m_id, origin, axis.m_axis, worldRadius * 0.9f, euler[i], axis.m_color);
		if (intersects && ctx.gizmoAxislAngle_Behavior(axis.m_id, origin, axis.m_axis, appData.m_snapRotation, worldRadius * 0.9f, worldSize, &euler[i])) {
			*outMat3 = Rotation(axis.m_axis, euler[i] - ctx.m_gizmoStateFloat) * storedRotation;
			ret = true;
		}
	}
	if (!(ctx.m_activeId == axes[0].m_id || ctx.m_activeId == axes[1].m_id || ctx.m_activeId == axes[2].m_id)) {
		Vec3 viewNormal = ctx.getAppData().m_viewDirection;
		float angle = 0.0f;
		if (intersects && ctx.gizmoAxislAngle_Behavior(viewId, origin, viewNormal, appData.m_snapRotation, worldRadius, worldSize, &angle)) {
			*outMat3 = Rotation(viewNormal, angle) * storedRotation;
			ret = true;
		}
		ctx.gizmoAxislAngle_Draw(viewId, origin, viewNormal, worldRadius, angle, viewId == ctx.m_activeId ? Color_GizmoHighlight : Color_White);
	}
	ctx.popMatrix();

	if (currentId != ctx.m_activeId) {
	 // gizmo became active, store rotation matrix
		storedRotation = *outMat3;
	}
	ctx.popId();
	return ret;
}
bool Im3d::GizmoScale(Id _id, float _scale_[3])
{
	Context& ctx = GetContext();

	Vec3 origin = ctx.getMatrix().getTranslation();
	float worldHeight = ctx.pixelsToWorldSize(origin, ctx.m_gizmoHeightPixels);
	#if IM3D_CULL_GIZMOS
		if (!ctx.isVisible(origin, worldHeight)) {
			return false;
		}
	#endif

	ctx.pushId(_id);
	ctx.m_appId = _id;

	bool ret = false;
	Vec3* outVec3 = (Vec3*)_scale_;

	float planeSize = worldHeight * (0.5f * 0.5f);
	float planeOffset = worldHeight * 0.5f;
	float worldSize = ctx.pixelsToWorldSize(origin, ctx.m_gizmoSizePixels);

	struct AxisG { Id m_id; Vec3 m_axis; Color m_color; };
	AxisG axes[] = {
		{ MakeId("axisX"), Normalize(ctx.getMatrix().getCol(0)), Color_Red   },
		{ MakeId("axisY"), Normalize(ctx.getMatrix().getCol(1)), Color_Green },
		{ MakeId("axisZ"), Normalize(ctx.getMatrix().getCol(2)), Color_Blue  }
	};

 // invert axes if viewing from behind
	const AppData& appData = ctx.getAppData();
	/*Vec3 viewDir = appData.m_viewOrigin - *outVec3;
	for (int i = 0; i < 3; ++i) {
		if (Dot(axes[i].m_axis, viewDir) < 0.0f) {
			axes[i].m_axis = -axes[i].m_axis;
		}
	}*/

	Sphere boundingSphere(origin, worldHeight);
	Ray ray(appData.m_cursorRayOrigin, appData.m_cursorRayDirection);
	bool intersects = ctx.m_appHotId == ctx.m_appId || Intersects(ray, boundingSphere);

 	ctx.pushEnableSorting(true);
	ctx.pushMatrix(Mat4(1.0f));
	{ // uniform scale
		Id uniformId = MakeId("uniform");

		if (intersects) {
			Sphere handle(origin, ctx.pixelsToWorldSize(origin, ctx.m_gizmoSizePixels * 4.0f));
			float t0, t1;
			bool intersects = Intersect(ray, handle, t0, t1);
			Vec3& storedScale = ctx.m_gizmoStateVec3;
			Vec3& storedPosition = *((Vec3*)ctx.m_gizmoStateMat3.m);
			if (uniformId == ctx.m_activeId) {
				if (ctx.isKeyDown(Action_Select)) {
					Plane plane(Normalize(origin - appData.m_viewOrigin), origin);
					Intersect(ray, plane, t0);
					Vec3 intersection = ray.m_origin + ray.m_direction * t0;
					float sign = Dot(intersection - origin, storedPosition - origin);
					float scale= copysignf(Length(intersection - origin), sign) / worldHeight;
					scale = Snap(scale, appData.m_snapScale);
					*outVec3 = storedScale * Vec3(Max(1.0f + copysignf(scale, sign), 1e-4f));
					ret = true;
				} else {
					ctx.makeActive(Id_Invalid);
				}

			} else if (uniformId == ctx.m_hotId) {
				if (intersects) {
					if (ctx.isKeyDown(Action_Select)) {
						ctx.makeActive(uniformId);
						storedScale = *outVec3;
						storedPosition = ray.m_origin + ray.m_direction * t0;
					}
				} else {
					ctx.resetId();
				}

			} else {
			 	float depth = Length2(origin - appData.m_viewOrigin);
				ctx.makeHot(uniformId, depth, intersects);
			}
		}

		bool activeOrHot = ctx.m_activeId == uniformId || ctx.m_hotId == uniformId;
		if (activeOrHot) {
			for (int i = 0; i < 3; ++i) {
				axes[i].m_color = Color_GizmoHighlight;
			}
			ctx.pushColor(Color_GizmoHighlight);
			ctx.pushAlpha(1.0f);
			ctx.pushSize(2.0f);
				DrawCircle(origin, Normalize(origin - appData.m_viewOrigin), worldSize * 2.0f);
			ctx.popSize();
			ctx.popAlpha();
			ctx.popColor();
		}
		ctx.pushAlpha(ctx.m_hotId == uniformId ? 1.0f : ctx.getAlpha());
		ctx.begin(PrimitiveMode_Points);
			ctx.vertex(origin, ctx.m_gizmoSizePixels * 2.0f, activeOrHot ? Color_GizmoHighlight : Color_White);
		ctx.end();
		ctx.popAlpha();
	}

	for (int i = 0; i < 3; ++i) {
		AxisG& axis = axes[i];
		ctx.gizmoAxisScale_Draw(axis.m_id, origin, axis.m_axis, worldHeight, worldSize, axis.m_color);
		if (intersects) {
			ret |= ctx.gizmoAxisScale_Behavior(axis.m_id, origin, axis.m_axis, appData.m_snapScale, worldHeight, worldSize, &(*outVec3)[i]);
		}
	}

	ctx.popMatrix();
	ctx.popEnableSorting();

	ctx.popId();
	return ret;
}
bool Im3d::Gizmo(Id _id, float _transform_[4*4])
{
	IM3D_ASSERT(_transform_);

	Context& ctx = GetContext();
 	Mat4* outMat4 = (Mat4*)_transform_;
	ctx.pushMatrix(*outMat4);

	bool ret = false;
	switch (ctx.m_gizmoMode) {
		case GizmoMode_Translation: {
			Vec3 translation = outMat4->getTranslation();
			if (GizmoTranslation(_id, translation, ctx.m_gizmoLocal)) {
				outMat4->setTranslation(translation);
				ret = true;
			}
			break;
		}
		case GizmoMode_Rotation: {
			Mat3 rotation = outMat4->getRotation();
			if (GizmoRotation(_id, rotation, ctx.m_gizmoLocal)) {
				outMat4->setRotation(rotation);
				ret = true;
			}
			break;
		}
		case GizmoMode_Scale: {
			Vec3 scale = outMat4->getScale();
			if (GizmoScale(_id, scale)) {
				outMat4->setScale(scale);
				ret = true;
			}
			break;
		}
		default:
			break;
	};

	ctx.popMatrix();

	return ret;
}

bool Im3d::Gizmo(Id _id, float _translation_[3], float _rotation_[3*3], float _scale_[3])
{
	Context& ctx = GetContext();

	Mat4 transform(
		_translation_ ? *((Vec3*)_translation_) : Vec3(0.0f),
		_rotation_    ? *((Mat3*)_rotation_)    : Mat3(1.0f),
		_scale_       ? *((Vec3*)_scale_)       : Vec3(1.0f)
		);
	ctx.pushMatrix(transform);

	bool ret = false;
	switch (ctx.m_gizmoMode) {
		case GizmoMode_Translation:
			if (_translation_) {
				if (GizmoTranslation(_id, _translation_, ctx.m_gizmoLocal)) {
					ret = true;
				}
			}
			break;
		case GizmoMode_Rotation:
			if (_rotation_) {
				if (GizmoRotation(_id, _rotation_, ctx.m_gizmoLocal)) {
					ret = true;
				}
			}
			break;
		case GizmoMode_Scale:
			if (_scale_) {
				if (GizmoScale(_id, _scale_)) {
					ret = true;
				}
			}
			break;
		default:
			break;
	};

	ctx.popMatrix();

	return ret;
}

void AppData::setCullFrustum(const Mat4& _viewProj, bool _ndcZNegativeOneToOne)
{
	m_cullFrustum[FrustumPlane_Top].x    = _viewProj(3, 0) - _viewProj(1, 0);
	m_cullFrustum[FrustumPlane_Top].y    = _viewProj(3, 1) - _viewProj(1, 1);
	m_cullFrustum[FrustumPlane_Top].z    = _viewProj(3, 2) - _viewProj(1, 2);
	m_cullFrustum[FrustumPlane_Top].w    = -(_viewProj(3, 3) - _viewProj(1, 3));

	m_cullFrustum[FrustumPlane_Bottom].x = _viewProj(3, 0) + _viewProj(1, 0);
	m_cullFrustum[FrustumPlane_Bottom].y = _viewProj(3, 1) + _viewProj(1, 1);
	m_cullFrustum[FrustumPlane_Bottom].z = _viewProj(3, 2) + _viewProj(1, 2);
	m_cullFrustum[FrustumPlane_Bottom].w = -(_viewProj(3, 3) + _viewProj(1, 3));

	m_cullFrustum[FrustumPlane_Right].x  = _viewProj(3, 0) - _viewProj(0, 0);
	m_cullFrustum[FrustumPlane_Right].y  = _viewProj(3, 1) - _viewProj(0, 1);
	m_cullFrustum[FrustumPlane_Right].z  = _viewProj(3, 2) - _viewProj(0, 2);
	m_cullFrustum[FrustumPlane_Right].w  = -(_viewProj(3, 3) - _viewProj(0, 3));

	m_cullFrustum[FrustumPlane_Left].x   = _viewProj(3, 0) + _viewProj(0, 0);
	m_cullFrustum[FrustumPlane_Left].y   = _viewProj(3, 1) + _viewProj(0, 1);
	m_cullFrustum[FrustumPlane_Left].z   = _viewProj(3, 2) + _viewProj(0, 2);
	m_cullFrustum[FrustumPlane_Left].w   = -(_viewProj(3, 3) + _viewProj(0, 3));

	m_cullFrustum[FrustumPlane_Far].x    = _viewProj(3, 0) - _viewProj(2, 0);
	m_cullFrustum[FrustumPlane_Far].y    = _viewProj(3, 1) - _viewProj(2, 1);
	m_cullFrustum[FrustumPlane_Far].z    = _viewProj(3, 2) - _viewProj(2, 2);
	m_cullFrustum[FrustumPlane_Far].w    = -(_viewProj(3, 3) - _viewProj(2, 3));

	if (_ndcZNegativeOneToOne) {
		m_cullFrustum[FrustumPlane_Near].x = _viewProj(3, 0) + _viewProj(2, 0);
		m_cullFrustum[FrustumPlane_Near].y = _viewProj(3, 1) + _viewProj(2, 1);
		m_cullFrustum[FrustumPlane_Near].z = _viewProj(3, 2) + _viewProj(2, 2);
		m_cullFrustum[FrustumPlane_Near].w = -(_viewProj(3, 3) + _viewProj(2, 3));
	} else {
		m_cullFrustum[FrustumPlane_Near].x = _viewProj(2, 0);
		m_cullFrustum[FrustumPlane_Near].y = _viewProj(2, 1);
		m_cullFrustum[FrustumPlane_Near].z = _viewProj(2, 2);
		m_cullFrustum[FrustumPlane_Near].w = -(_viewProj(2, 3));
	}

 // normalize
	for (int i = 0; i < FrustumPlane_Count; ++i) {
		float d = 1.0f / Length(Vec3(m_cullFrustum[i]));
		m_cullFrustum[i] = m_cullFrustum[i] * d;
	}
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
Vec3 Im3d::ToEulerXYZ(const Mat3& _m)
{
 // http://www.staff.city.ac.uk/~sbbh653/publications/euler.pdf
	Vec3 ret;
	if_likely (fabs(_m(2, 0)) < 1.0f) {
		ret.y = -asinf(_m(2, 0));
		float c = 1.0f / cosf(ret.y);
		ret.x = atan2f(_m(2, 1) * c, _m(2, 2) * c);
		ret.z = atan2f(_m(1, 0) * c, _m(0, 0) * c);
	} else {
		ret.z = 0.0f;
		if (!(_m(2, 0) > -1.0f)) {
			ret.x = ret.z + atan2f(_m(0, 1), _m(0, 2));
			ret.y = HalfPi;
		} else {
			ret.x = -ret.z + atan2f(-_m(0, 1), -_m(0, 2));
			ret.y = -HalfPi;
		}
	}
	return ret;
}
Mat3 Im3d::FromEulerXYZ(Vec3& _euler)
{
	float cx = cosf(_euler.x);
	float sx = sinf(_euler.x);
	float cy = cosf(_euler.y);
	float sy = cosf(_euler.y);
	float cz = cosf(_euler.z);
	float sz = cosf(_euler.z);
	return Mat3(
		cy * cz, sz * sy * cz - cx * sz, cx * sy * cz + sx * sz,
		cy * sz, sx * sy * sz + cx * cz, cx * sy * sz - sx * cz,
		    -sz,                sx * cy,                cx * cy
		);
}
Mat3 Im3d::Transpose(const Mat3& _m)
{
	return Mat3(
		_m(0, 0), _m(1, 0), _m(2, 0),
		_m(0, 1), _m(1, 1), _m(2, 1),
		_m(0, 2), _m(1, 2), _m(2, 2)
		);
}
Mat3 Im3d::Rotation(const Vec3& _axis, float _rads)
{
	float c  = cosf(_rads);
	float rc = 1.0f - c;
	float s  = sinf(_rads);
	return Mat3(
		_axis.x * _axis.x + (1.0f - _axis.x * _axis.x) * c, _axis.x * _axis.y * rc - _axis.z * s,                _axis.x * _axis.z * rc + _axis.y * s,
		_axis.x * _axis.y * rc + _axis.z * s,               _axis.y * _axis.y + (1.0f - _axis.y * _axis.y) * c,  _axis.y * _axis.z * rc - _axis.x * s,
		_axis.x * _axis.z * rc - _axis.y * s,               _axis.y * _axis.z * rc + _axis.x * s,                _axis.z * _axis.z + (1.0f - _axis.z * _axis.z) * c
		);
}
Mat3 Im3d::Scale(const Vec3& _s)
{
	return Mat3(
		_s.x,  0.0f,  0.0f,
		0.0f,  _s.y,  0.0f,
		0.0f,  0.0f,  _s.z
		);
}


// Mat4
Mat4::Mat4(float _diagonal)
{
	(*this)(0, 0) = _diagonal; (*this)(0, 1) = 0.0f;      (*this)(0, 2) = 0.0f;      (*this)(0, 3) = 0.0f;
	(*this)(1, 0) = 0.0f;      (*this)(1, 1) = _diagonal; (*this)(1, 2) = 0.0f;      (*this)(1, 3) = 0.0f;
	(*this)(2, 0) = 0.0f;      (*this)(2, 1) = 0.0f;      (*this)(2, 2) = _diagonal; (*this)(2, 3) = 0.0f;
	(*this)(3, 0) = 0.0f;      (*this)(3, 1) = 0.0f;      (*this)(3, 2) = 0.0f;      (*this)(3, 3) = _diagonal;
}
Mat4::Mat4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33
	)
{
	(*this)(0, 0) = m00; (*this)(0, 1) = m01; (*this)(0, 2) = m02; (*this)(0, 3) = m03;
	(*this)(1, 0) = m10; (*this)(1, 1) = m11; (*this)(1, 2) = m12; (*this)(1, 3) = m13;
	(*this)(2, 0) = m20; (*this)(2, 1) = m21; (*this)(2, 2) = m22; (*this)(2, 3) = m23;
	(*this)(3, 0) = m30; (*this)(3, 1) = m31; (*this)(3, 2) = m32; (*this)(3, 3) = m33;
}
Mat4::Mat4(const Mat3& _mat3)
{
	(*this)(0, 0) = _mat3(0, 0); (*this)(0, 1) = _mat3(0, 1); (*this)(0, 2) = _mat3(0, 2); (*this)(0, 3) = 0.0f;
	(*this)(1, 0) = _mat3(1, 0); (*this)(1, 1) = _mat3(1, 1); (*this)(1, 2) = _mat3(1, 2); (*this)(1, 3) = 0.0f;
	(*this)(2, 0) = _mat3(2, 0); (*this)(2, 1) = _mat3(2, 1); (*this)(2, 2) = _mat3(2, 2); (*this)(2, 3) = 0.0f;
	(*this)(3, 0) =        0.0f; (*this)(3, 1) =        0.0f; (*this)(3, 2) =        0.0f; (*this)(3, 3) = 1.0f;
}
Mat4::Mat4(const Vec3& _translation, const Mat3& _rotation, const Vec3& _scale)
{
	setCol(0, Vec4(_rotation.getCol(0) * _scale.x, 0.0f));
	setCol(1, Vec4(_rotation.getCol(1) * _scale.y, 0.0f));
	setCol(2, Vec4(_rotation.getCol(2) * _scale.z, 0.0f));
	setCol(3, Vec4(_translation, 1.0f));
}
Vec4 Mat4::getCol(int _i) const
{
	return Vec4((*this)(0, _i), (*this)(1, _i), (*this)(2, _i), (*this)(3, _i));
}
Vec4 Mat4::getRow(int _i) const
{
	return Vec4((*this)(_i, 0), (*this)(_i, 1), (*this)(_i, 2), (*this)(_i, 3));
}
void Mat4::setCol(int _i, const Vec4& _v)
{
	(*this)(0, _i) = _v.x;
	(*this)(1, _i) = _v.y;
	(*this)(2, _i) = _v.z;
	(*this)(3, _i) = _v.w;
}
void Mat4::setRow(int _i, const Vec4& _v)
{
	(*this)(_i, 0) = _v.x;
	(*this)(_i, 1) = _v.y;
	(*this)(_i, 2) = _v.z;
	(*this)(_i, 3) = _v.w;
}
Vec3 Mat4::getTranslation() const
{
	return Vec3((*this)(0, 3), (*this)(1, 3), (*this)(2, 3));
}
void Mat4::setTranslation(const Vec3& _translation)
{
	(*this)(0, 3) = _translation.x;
	(*this)(1, 3) = _translation.y;
	(*this)(2, 3) = _translation.z;
}
Mat3 Mat4::getRotation() const
{
	Mat3 ret(*this);
	ret.setCol(0, Normalize(ret.getCol(0)));
	ret.setCol(1, Normalize(ret.getCol(1)));
	ret.setCol(2, Normalize(ret.getCol(2)));
	return ret;
}
void Mat4::setRotation(const Mat3& _rotation)
{
	Vec3 scale = getScale();
	setCol(0, Vec4(_rotation.getCol(0) * scale.x, 0.0f));
	setCol(1, Vec4(_rotation.getCol(1) * scale.y, 0.0f));
	setCol(2, Vec4(_rotation.getCol(2) * scale.z, 0.0f));
}
Vec3 Mat4::getScale() const
{
	return Vec3(Length(getCol(0)), Length(getCol(1)), Length(getCol(2)));
}
void Mat4::setScale(const Vec3& _scale)
{
	Vec3 scale = _scale / getScale();
	setCol(0, getCol(0) * scale.x);
	setCol(1, getCol(1) * scale.y);
	setCol(2, getCol(2) * scale.z);
}
inline static float Determinant(const Mat4& _m)
{
	return
		_m(0, 3) * _m(1, 2) * _m(2, 1) * _m(3, 0) - _m(0, 2) * _m(1, 3) * _m(2, 1) * _m(3, 0) - _m(0, 3) * _m(1, 1) * _m(2, 2) * _m(3, 0) + _m(0, 1) * _m(1, 3) * _m(2, 2) * _m(3, 0) +
		_m(0, 2) * _m(1, 1) * _m(2, 3) * _m(3, 0) - _m(0, 1) * _m(1, 2) * _m(2, 3) * _m(3, 0) - _m(0, 3) * _m(1, 2) * _m(2, 0) * _m(3, 1) + _m(0, 2) * _m(1, 3) * _m(2, 0) * _m(3, 1) +
		_m(0, 3) * _m(1, 0) * _m(2, 2) * _m(3, 1) - _m(0, 0) * _m(1, 3) * _m(2, 2) * _m(3, 1) - _m(0, 2) * _m(1, 0) * _m(2, 3) * _m(3, 1) + _m(0, 0) * _m(1, 2) * _m(2, 3) * _m(3, 1) +
		_m(0, 3) * _m(1, 1) * _m(2, 0) * _m(3, 2) - _m(0, 1) * _m(1, 3) * _m(2, 0) * _m(3, 2) - _m(0, 3) * _m(1, 0) * _m(2, 1) * _m(3, 2) + _m(0, 0) * _m(1, 3) * _m(2, 1) * _m(3, 2) +
		_m(0, 1) * _m(1, 0) * _m(2, 3) * _m(3, 2) - _m(0, 0) * _m(1, 1) * _m(2, 3) * _m(3, 2) - _m(0, 2) * _m(1, 1) * _m(2, 0) * _m(3, 3) + _m(0, 1) * _m(1, 2) * _m(2, 0) * _m(3, 3) +
		_m(0, 2) * _m(1, 0) * _m(2, 1) * _m(3, 3) - _m(0, 0) * _m(1, 2) * _m(2, 1) * _m(3, 3) - _m(0, 1) * _m(1, 0) * _m(2, 2) * _m(3, 3) + _m(0, 0) * _m(1, 1) * _m(2, 2) * _m(3, 3)
		;
}
Mat4 Im3d::Inverse(const Mat4& _m)
{
	Mat4 ret;
	ret(0, 0) = _m(1, 2) * _m(2, 3) * _m(3, 1) - _m(1, 3) * _m(2, 2) * _m(3, 1) + _m(1, 3) * _m(2, 1) * _m(3, 2) - _m(1, 1) * _m(2, 3) * _m(3, 2) - _m(1, 2) * _m(2, 1) * _m(3, 3) + _m(1, 1) * _m(2, 2) * _m(3, 3);
	ret(0, 1) = _m(0, 3) * _m(2, 2) * _m(3, 1) - _m(0, 2) * _m(2, 3) * _m(3, 1) - _m(0, 3) * _m(2, 1) * _m(3, 2) + _m(0, 1) * _m(2, 3) * _m(3, 2) + _m(0, 2) * _m(2, 1) * _m(3, 3) - _m(0, 1) * _m(2, 2) * _m(3, 3);
	ret(0, 2) = _m(0, 2) * _m(1, 3) * _m(3, 1) - _m(0, 3) * _m(1, 2) * _m(3, 1) + _m(0, 3) * _m(1, 1) * _m(3, 2) - _m(0, 1) * _m(1, 3) * _m(3, 2) - _m(0, 2) * _m(1, 1) * _m(3, 3) + _m(0, 1) * _m(1, 2) * _m(3, 3);
	ret(0, 3) = _m(0, 3) * _m(1, 2) * _m(2, 1) - _m(0, 2) * _m(1, 3) * _m(2, 1) - _m(0, 3) * _m(1, 1) * _m(2, 2) + _m(0, 1) * _m(1, 3) * _m(2, 2) + _m(0, 2) * _m(1, 1) * _m(2, 3) - _m(0, 1) * _m(1, 2) * _m(2, 3);
	ret(1, 0) = _m(1, 3) * _m(2, 2) * _m(3, 0) - _m(1, 2) * _m(2, 3) * _m(3, 0) - _m(1, 3) * _m(2, 0) * _m(3, 2) + _m(1, 0) * _m(2, 3) * _m(3, 2) + _m(1, 2) * _m(2, 0) * _m(3, 3) - _m(1, 0) * _m(2, 2) * _m(3, 3);
	ret(1, 1) = _m(0, 2) * _m(2, 3) * _m(3, 0) - _m(0, 3) * _m(2, 2) * _m(3, 0) + _m(0, 3) * _m(2, 0) * _m(3, 2) - _m(0, 0) * _m(2, 3) * _m(3, 2) - _m(0, 2) * _m(2, 0) * _m(3, 3) + _m(0, 0) * _m(2, 2) * _m(3, 3);
	ret(1, 2) = _m(0, 3) * _m(1, 2) * _m(3, 0) - _m(0, 2) * _m(1, 3) * _m(3, 0) - _m(0, 3) * _m(1, 0) * _m(3, 2) + _m(0, 0) * _m(1, 3) * _m(3, 2) + _m(0, 2) * _m(1, 0) * _m(3, 3) - _m(0, 0) * _m(1, 2) * _m(3, 3);
	ret(1, 3) = _m(0, 2) * _m(1, 3) * _m(2, 0) - _m(0, 3) * _m(1, 2) * _m(2, 0) + _m(0, 3) * _m(1, 0) * _m(2, 2) - _m(0, 0) * _m(1, 3) * _m(2, 2) - _m(0, 2) * _m(1, 0) * _m(2, 3) + _m(0, 0) * _m(1, 2) * _m(2, 3);
	ret(2, 0) = _m(1, 1) * _m(2, 3) * _m(3, 0) - _m(1, 3) * _m(2, 1) * _m(3, 0) + _m(1, 3) * _m(2, 0) * _m(3, 1) - _m(1, 0) * _m(2, 3) * _m(3, 1) - _m(1, 1) * _m(2, 0) * _m(3, 3) + _m(1, 0) * _m(2, 1) * _m(3, 3);
	ret(2, 1) = _m(0, 3) * _m(2, 1) * _m(3, 0) - _m(0, 1) * _m(2, 3) * _m(3, 0) - _m(0, 3) * _m(2, 0) * _m(3, 1) + _m(0, 0) * _m(2, 3) * _m(3, 1) + _m(0, 1) * _m(2, 0) * _m(3, 3) - _m(0, 0) * _m(2, 1) * _m(3, 3);
	ret(2, 2) = _m(0, 1) * _m(1, 3) * _m(3, 0) - _m(0, 3) * _m(1, 1) * _m(3, 0) + _m(0, 3) * _m(1, 0) * _m(3, 1) - _m(0, 0) * _m(1, 3) * _m(3, 1) - _m(0, 1) * _m(1, 0) * _m(3, 3) + _m(0, 0) * _m(1, 1) * _m(3, 3);
	ret(2, 3) = _m(0, 3) * _m(1, 1) * _m(2, 0) - _m(0, 1) * _m(1, 3) * _m(2, 0) - _m(0, 3) * _m(1, 0) * _m(2, 1) + _m(0, 0) * _m(1, 3) * _m(2, 1) + _m(0, 1) * _m(1, 0) * _m(2, 3) - _m(0, 0) * _m(1, 1) * _m(2, 3);
	ret(3, 0) = _m(1, 2) * _m(2, 1) * _m(3, 0) - _m(1, 1) * _m(2, 2) * _m(3, 0) - _m(1, 2) * _m(2, 0) * _m(3, 1) + _m(1, 0) * _m(2, 2) * _m(3, 1) + _m(1, 1) * _m(2, 0) * _m(3, 2) - _m(1, 0) * _m(2, 1) * _m(3, 2);
	ret(3, 1) = _m(0, 1) * _m(2, 2) * _m(3, 0) - _m(0, 2) * _m(2, 1) * _m(3, 0) + _m(0, 2) * _m(2, 0) * _m(3, 1) - _m(0, 0) * _m(2, 2) * _m(3, 1) - _m(0, 1) * _m(2, 0) * _m(3, 2) + _m(0, 0) * _m(2, 1) * _m(3, 2);
	ret(3, 2) = _m(0, 2) * _m(1, 1) * _m(3, 0) - _m(0, 1) * _m(1, 2) * _m(3, 0) - _m(0, 2) * _m(1, 0) * _m(3, 1) + _m(0, 0) * _m(1, 2) * _m(3, 1) + _m(0, 1) * _m(1, 0) * _m(3, 2) - _m(0, 0) * _m(1, 1) * _m(3, 2);
	ret(3, 3) = _m(0, 1) * _m(1, 2) * _m(2, 0) - _m(0, 2) * _m(1, 1) * _m(2, 0) + _m(0, 2) * _m(1, 0) * _m(2, 1) - _m(0, 0) * _m(1, 2) * _m(2, 1) - _m(0, 1) * _m(1, 0) * _m(2, 2) + _m(0, 0) * _m(1, 1) * _m(2, 2);

	float det = 1.0f / Determinant(_m);
	for (int i = 0; i < 16; ++i) {
		ret[i] *= det;
	}
	return ret;
}
Mat4 Im3d::Transpose(const Mat4& _m)
{
	return Mat4(
		_m(0, 0), _m(1, 0), _m(2, 0), _m(3, 0),
		_m(0, 1), _m(1, 1), _m(2, 1), _m(3, 1),
		_m(0, 2), _m(1, 2), _m(2, 2), _m(3, 2),
		_m(0, 3), _m(1, 3), _m(2, 3), _m(3, 3)
		);
}
Mat4 Im3d::Translation(const Vec3& _t)
{
	return Mat4(
		1.0f, 0.0f, 0.0f, _t.x,
		0.0f, 1.0f, 0.0f, _t.y,
		0.0f, 0.0f, 1.0f, _t.z,
		0.0f, 0.0f, 0.0f, 1.0f
		);
}
Mat4 Im3d::AlignZ(const Vec3& _axis, const Vec3& _up)
{
	Vec3 x, y;
	y = _up - _axis * Dot(_up, _axis);
	float ylen = Length(y);
	if_unlikely (ylen < FLT_EPSILON) {
		Vec3 k = Vec3(1.0f, 0.0f, 0.0f);
		y = k - _axis * Dot(k, _axis);
		ylen = Length(y);
		if_unlikely (ylen < FLT_EPSILON) {
			k = Vec3(0.0f, 0.0f, 1.0f);
			y = k - _axis * Dot(k, _axis);
			ylen = Length(y);
		}
	}
	y = y / ylen;
	x = Cross(y, _axis);

	return Mat4(
		x.x,    y.x,    _axis.x,    0.0f,
		x.y,    y.y,    _axis.y,    0.0f,
		x.z,    y.z,    _axis.z,    0.0f
		);
}
Mat4 Im3d::LookAt(const Vec3& _from, const Vec3& _to, const Vec3& _up)
{
	Mat4 ret = AlignZ(Normalize(_to - _from), _up);
	ret.setCol(3, Vec4(_from, 1.0f)); // inject translation
	return ret;
}

// Geometry
Line::Line(const Vec3& _origin, const Vec3& _direction)
	: m_origin(_origin)
	, m_direction(_direction)
{
}
Ray::Ray(const Vec3& _origin, const Vec3& _direction)
	: m_origin(_origin)
	, m_direction(_direction)
{
}
LineSegment::LineSegment(const Vec3& _start, const Vec3& _end)
	: m_start(_start)
	, m_end(_end)
{
}
Sphere::Sphere(const Vec3& _origin, float _radius)
	: m_origin(_origin)
	, m_radius(_radius)
{
}
Plane::Plane(const Vec3& _normal, float _offset)
	: m_normal(_normal)
	, m_offset(_offset)
{
}
Plane::Plane(const Vec3& _normal, const Vec3& _origin)
	: m_normal(_normal)
	, m_offset(Dot(_normal, _origin))
{
}
Capsule::Capsule(const Vec3& _start, const Vec3& _end, float _radius)
	: m_start(_start)
	, m_end(_end)
	, m_radius(_radius)
{
}

bool Im3d::Intersects(const Ray& _ray, const Plane& _plane)
{
	float x = Dot(_plane.m_normal, _ray.m_direction);
	return x <= 0.0f;
}
bool Im3d::Intersect(const Ray& _ray, const Plane& _plane, float& t0_)
{
	t0_ = Dot(_plane.m_normal, (_plane.m_normal * _plane.m_offset) - _ray.m_origin) / Dot(_plane.m_normal, _ray.m_direction);
	return t0_ >= 0.0f;
}
bool Im3d::Intersects(const Ray& _r, const Sphere& _s)
{
	Vec3 p = _s.m_origin - _r.m_origin;
	float p2 = Length2(p);
	float q = Dot(p, _r.m_direction);
	float r2 = _s.m_radius * _s.m_radius;
	if (q < 0.0f && p2 > r2) {
		return false;
	}
	return p2 - (q * q) <= r2;
}
bool Im3d::Intersect(const Ray& _r, const Sphere& _s, float& t0_, float& t1_)
{
	Vec3 p = _s.m_origin - _r.m_origin;
	float q = Dot(p, _r.m_direction);
	if (q < 0.0f) {
		return false;
	}
	float p2 = Length2(p) - q * q;
	float r2 = _s.m_radius * _s.m_radius;
	if (p2 > r2) {
		return false;
	}
	float s = sqrtf(r2 - p2);
	t0_ = Max(q - s, 0.0f);
	t1_ = q + s;

	return true;
}
bool Im3d::Intersects(const Ray& _ray, const Capsule& _capsule)
{
	return Distance2(_ray, LineSegment(_capsule.m_start, _capsule.m_end)) < _capsule.m_radius * _capsule.m_radius;
}
bool Im3d::Intersect(const Ray& _ray, const Capsule& _capsule, float& t0_, float& t1_)
{
	//IM3D_ASSERT(false); // \todo implement
	t0_ = t1_ = 0.0f;
	return Intersects(_ray, _capsule);
}

void Im3d::Nearest(const Line& _line0, const Line& _line1, float& t0_, float& t1_)
{
	Vec3 p = _line0.m_origin - _line1.m_origin;
	float q = Dot(_line0.m_direction, _line1.m_direction);
	float s = Dot(_line1.m_direction, p);

	float d = 1.0f - q * q;
	if (d < FLT_EPSILON) { // lines are parallel
		t0_ = 0.0f;
		t1_ = s;
	} else {
		float r = Dot(_line0.m_direction, p);
		t0_ = (q * s - r) / d;
		t1_ = (s - q * r) / d;
	}
}
void Im3d::Nearest(const Ray& _ray, const Line& _line, float& tr_, float& tl_)
{
	Nearest(Line(_ray.m_origin, _ray.m_direction), _line, tr_, tl_);
	tr_ = Max(tr_, 0.0f);
}
Vec3 Im3d::Nearest(const Ray& _ray, const LineSegment& _segment, float& tr_)
{
	Vec3 ldir = _segment.m_end - _segment.m_start;
	Vec3 p = _segment.m_start - _ray.m_origin;
	float q = Length2(ldir);
	float r = Dot(ldir, _ray.m_direction);
	float s = Dot(ldir, p);
	float t = Dot(_ray.m_direction, p);

	float sn, sd, tn, td;
	float denom = q - r * r;
	if (denom < FLT_EPSILON) {
		sd = td = 1.0f;
		sn = 0.0f;
		tn = t;
	} else {
		sd = td = denom;
		sn = r * t - s;
		tn = q * t - r * s;
		if (sn < 0.0f) {
		    sn = 0.0f;
		    tn = t;
		    td = 1.0f;
		} else if (sn > sd) {
			sn = sd;
			tn = t + r;
			td = 1.0f;
		}
	}

	float ts;
	if (tn < 0.0f) {
		tr_ = 0.0f;
		if (r >= 0.0f) {
		    ts = 0.0f;
		} else if (s <= q) {
		    ts = 1.0f;
		} else {
		    ts = -s / q;
		}
	} else {
		tr_ = tn / td;
		ts = sn / sd;
	}
	return _segment.m_start + ldir * ts;
}
float Im3d::Distance2(const Ray& _ray, const LineSegment& _segment)
{
	float tr;
	Vec3 p = Nearest(_ray, _segment, tr);
	return Length2(_ray.m_origin + _ray.m_direction * tr - p);
}

#define IM3D_STATIC_ASSERT(e) { (void)sizeof(char[(e) ? 1 : -1]); }
static void StaticAsserts()
{
	IM3D_STATIC_ASSERT(sizeof (Vec2) == sizeof (float[2]));
	IM3D_STATIC_ASSERT(alignof(Vec2) == alignof(float[2]));
	IM3D_STATIC_ASSERT(sizeof (Vec3) == sizeof (float[3]));
	IM3D_STATIC_ASSERT(alignof(Vec3) == alignof(float[3]));
	IM3D_STATIC_ASSERT(sizeof (Vec4) == sizeof (float[4]));
	IM3D_STATIC_ASSERT(alignof(Vec4) == alignof(float[4]));
	IM3D_STATIC_ASSERT(sizeof (Mat3) == sizeof (float[9]));
	IM3D_STATIC_ASSERT(alignof(Mat3) == alignof(float[9]));
	IM3D_STATIC_ASSERT(sizeof (Mat4) == sizeof (float[16]));
	IM3D_STATIC_ASSERT(alignof(Mat4) == alignof(float[16]));
}
