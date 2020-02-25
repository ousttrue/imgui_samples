#pragma once
#ifndef im3d_h
#define im3d_h

#include "im3d_config.h"

#define IM3D_VERSION "1.14"

namespace Im3d {

struct Vec2;
struct Vec3;
struct Vec4;
struct Mat3;
struct Mat4;
struct Color;
typedef unsigned int U32;
typedef U32 Id;
// constexpr Id Id_Invalid = 0;
struct AppData;
struct DrawList;
class  Context;

// Get AppData struct from the current context, fill before calling NewFrame().
AppData& GetAppData();

// Call at the start of each frame, after filling the AppData struct.
void  NewFrame();
// Call after all Im3d calls have been made for the current frame, before accessing draw data.
void  EndFrame();

// Access draw data. Draw lists are valid after calling EndFrame() and before calling NewFrame().
const DrawList* GetDrawLists();
U32   GetDrawListCount();

// DEPRECATED (use EndFrame() + GetDrawLists()).
// Call after all Im3d calls have been made for the current frame.
void  Draw();

// Begin/end primitive. End() must be called before starting each new primitive type.
void  BeginPoints();
void  BeginLines();
void  BeginLineLoop();
void  BeginLineStrip();
void  BeginTriangles();
void  BeginTriangleStrip();
void  End();

// Add a vertex to the current primitive (call between Begin*() and End()).
void  Vertex(const Vec3& _position);
void  Vertex(const Vec3& _position, Color _color);
void  Vertex(const Vec3& _position, float _size);
void  Vertex(const Vec3& _position, float _size, Color _color);
void  Vertex(float _x, float _y, float _z);
void  Vertex(float _x, float _y, float _z, Color _color);
void  Vertex(float _x, float _y, float _z, float _size);
void  Vertex(float _x, float _y, float _z, float _size, Color _color);

// Color draw state (per vertex).
void  PushColor(); // push the stack top
void  PushColor(Color _color);
void  PopColor();
void  SetColor(Color _color);
void  SetColor(float _r, float _g, float _b, float _a = 1.0f);
Color GetColor();

// Alpha draw state, multiplies the alpha set by the color draw state (per vertex).
void  PushAlpha(); // push the stack top
void  PushAlpha(float _alpha);
void  PopAlpha();
void  SetAlpha(float _alpha);
float GetAlpha();

// Size draw state, for points/lines this is the radius/width in pixels (per vertex).
void  PushSize(); // push the stack top
void  PushSize(float _size);
void  PopSize();
void  SetSize(float _size);
float GetSize();

// Sorting draw state, enable depth sorting between primitives (per primitive).
void  PushEnableSorting(); // push the stack top
void  PushEnableSorting(bool _enable);
void  PopEnableSorting();
void  EnableSorting(bool _enable);

// Push/pop all draw states (color, alpha, size, sorting).
void  PushDrawState();
void  PopDrawState();

// Transform state (per vertex).
void  PushMatrix(); // push stack top
void  PushMatrix(const Mat4& _mat4);
void  PopMatrix();
void  SetMatrix(const Mat4& _mat4);
void  SetIdentity();
void  MulMatrix(const Mat4& _mat4);
void  Translate(float _x, float _y, float _z);
void  Translate(const Vec3& _vec3);
void  Rotate(const Vec3& _axis, float _angle);
void  Rotate(const Mat3& _rotation);
void  Scale(float _x, float _y, float _z);

// High order shapes. Where _detail = -1, an automatic level of detail is chosen based on the distance to the view origin (as specified via the AppData struct).
void  DrawXyzAxes();
void  DrawPoint(const Vec3& _position, float _size, Color _color);
void  DrawLine(const Vec3& _a, const Vec3& _b, float _size, Color _color);
void  DrawQuad(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d);
void  DrawQuad(const Vec3& _origin, const Vec3& _normal, const Vec2& _size);
void  DrawQuadFilled(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d);
void  DrawQuadFilled(const Vec3& _origin, const Vec3& _normal, const Vec2& _size);
void  DrawCircle(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail = -1);
void  DrawCircleFilled(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail = -1);
void  DrawSphere(const Vec3& _origin, float _radius, int _detail = -1);
void  DrawSphereFilled(const Vec3& _origin, float _radius, int _detail = -1);
void  DrawAlignedBox(const Vec3& _min, const Vec3& _max);
void  DrawAlignedBoxFilled(const Vec3& _min, const Vec3& _max);
void  DrawCylinder(const Vec3& _start, const Vec3& _end, float _radius, int _detail = -1);
void  DrawCapsule(const Vec3& _start, const Vec3& _end, float _radius, int _detail = -1);
void  DrawPrism(const Vec3& _start, const Vec3& _end, float _radius, int _sides);
void  DrawArrow(const Vec3& _start, const Vec3& _end, float _headLength = -1.0f, float _headThickness = -1.0f);

// Ids are used to uniquely identify gizmos and layers. Gizmo should have a unique id during a frame.
// Note that ids are a hash of the whole id stack, see PushId(), PopId().
Id    MakeId(const char* _str);
Id    MakeId(const void* _ptr);
Id    MakeId(int _i);

// PushId(), PopId() affect the result of subsequent calls to MakeId(), use when creating gizmos in a loop.
void  PushId(); // push stack top
void  PushId(Id _id);
void  PushId(const char* _str);
void  PushId(const void* _ptr);
void  PushId(int _i);
void  PopId();
Id    GetId();
Id    GetActiveId(); // GetActiveId() != Id_Invalid means that a gizmo is in use
Id    GetHotId();

// Layer id state, subsequent primitives are added to a separate draw list associated with the id (per primitive).
void  PushLayerId(Id _layer);
void  PushLayerId(const char* _str); // calls PushLayerId(MakeId(_str))
void  PopLayerId();
Id    GetLayerId();

// Manipulate translation/rotation/scale via a gizmo. Return true if the gizmo is 'active' (if it modified the output parameter).
// If _local is true, the Gizmo* functions expect that the local matrix is on the matrix stack; in general the application should
// push the local matrix before calling any of the following.
bool  GizmoTranslation(const char* _id, float _translation_[3], bool _local = false);
bool  GizmoRotation(const char* _id, float _rotation_[3*3], bool _local = false);
bool  GizmoScale(const char* _id, float _scale_[3]); // local scale only
// Unified gizmo, selects local/global, translation/rotation/scale based on the context-global gizmo modes. Return true if the gizmo is active.
bool  Gizmo(const char* _id, float _translation_[3], float _rotation_[3*3], float _scale_[3]); // any of _translation_/_rotation_/_scale_ may be null.
bool  Gizmo(const char* _id, float _transform_[4*4]);

// Gizmo* overloads which take an Id directly. In some cases the app may want to call MakeId() separately, usually to change the gizmo appearance if hot/active.
bool  GizmoTranslation(Id _id, float _translation_[3], bool _local = false);
bool  GizmoRotation(Id _id, float _rotation_[3*3], bool _local = false);
bool  GizmoScale(Id _id, float _scale_[3]);
bool  Gizmo(Id _id, float _transform_[4*4]);
bool  Gizmo(Id _id, float _translation_[3], float _rotation_[3*3], float _scale_[3]);

// Visibility tests. The application must set a culling frustum via AppData.
bool  IsVisible(const Vec3& _origin, float _radius); // sphere
bool  IsVisible(const Vec3& _min, const Vec3& _max); // axis-aligned bounding box

// Get/set the current context. All Im3d calls affect the currently bound context.
Context& GetContext();
void     SetContext(Context& _ctx);

// Merge vertex data from _src into _dst_. Layers are preserved. Call before EndFrame().
void     MergeContexts(Context& _dst_, const Context& _src);

} // namespac Im3d

#endif // im3d_h
