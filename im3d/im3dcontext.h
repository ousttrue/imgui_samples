#pragma once

namespace Im3d
{

enum PrimitiveMode
{
    PrimitiveMode_None,
    PrimitiveMode_Points,
    PrimitiveMode_Lines,
    PrimitiveMode_LineStrip,
    PrimitiveMode_LineLoop,
    PrimitiveMode_Triangles,
    PrimitiveMode_TriangleStrip
};
enum GizmoMode
{
    GizmoMode_Translation,
    GizmoMode_Rotation,
    GizmoMode_Scale
};

// Minimal vector.
template <typename T>
class Vector
{
    T *m_data = nullptr;
    U32 m_size = 0;
    U32 m_capacity = 0;

public:
    Vector() {}
    ~Vector();

    T &operator[](U32 _i)
    {
        IM3D_ASSERT(_i < m_size);
        return m_data[_i];
    }
    const T &operator[](U32 _i) const
    {
        IM3D_ASSERT(_i < m_size);
        return m_data[_i];
    }
    T *data() { return m_data; }
    const T *data() const { return m_data; }

    void push_back(const T &_v)
    {
        T tmp = _v;
        if (m_size == m_capacity)
        {
            reserve(m_capacity + m_capacity / 2);
        }
        m_data[m_size++] = tmp;
    }
    void pop_back()
    {
        IM3D_ASSERT(m_size > 0);
        --m_size;
    }
    void append(const T *_v, U32 _count);
    void append(const Vector<T> &_other) { append(_other.data(), _other.size()); }

    T *begin() { return m_data; }
    const T *begin() const { return m_data; }
    T *end() { return m_data + m_size; }
    const T *end() const { return m_data + m_size; }
    T &front()
    {
        IM3D_ASSERT(m_size > 0);
        return m_data[0];
    }
    const T &front() const
    {
        IM3D_ASSERT(m_size > 0);
        return m_data[0];
    }
    T &back()
    {
        IM3D_ASSERT(m_size > 0);
        return m_data[m_size - 1];
    }
    const T &back() const
    {
        IM3D_ASSERT(m_size > 0);
        return m_data[m_size - 1];
    }

    U32 size() const { return m_size; }
    U32 capacity() const { return m_capacity; }
    bool empty() const { return m_size == 0; }

    void clear() { m_size = 0; }
    void reserve(U32 _capacity);
    void resize(U32 _size, const T &_val);

    static void swap(Vector<T> &_a_, Vector<T> &_b_);
};

// Context stores all relevant state - main interface affects the context currently bound via SetCurrentContext().
class Context
{
public:
    void begin(PrimitiveMode _mode);
    void end();
    void vertex(const Vec3 &_position, float _size, Color _color);
    void vertex(const Vec3 &_position) { vertex(_position, getSize(), getColor()); }

    void reset();
    void merge(const Context &_src);
    void endFrame();
    void draw(); // DEPRECATED (see Im3d::Draw)

    const DrawList *getDrawLists() const { return m_drawLists.data(); }
    U32 getDrawListCount() const { return m_drawLists.size(); }

    void setColor(Color _color) { m_colorStack.back() = _color; }
    Color getColor() const { return m_colorStack.back(); }
    void pushColor(Color _color) { m_colorStack.push_back(_color); }
    void popColor()
    {
        IM3D_ASSERT(m_colorStack.size() > 1);
        m_colorStack.pop_back();
    }

    void setAlpha(float _alpha) { m_alphaStack.back() = _alpha; }
    float getAlpha() const { return m_alphaStack.back(); }
    void pushAlpha(float _alpha) { m_alphaStack.push_back(_alpha); }
    void popAlpha()
    {
        IM3D_ASSERT(m_alphaStack.size() > 1);
        m_alphaStack.pop_back();
    }

    void setSize(float _size) { m_sizeStack.back() = _size; }
    float getSize() const { return m_sizeStack.back(); }
    void pushSize(float _size) { m_sizeStack.push_back(_size); }
    void popSize()
    {
        IM3D_ASSERT(m_sizeStack.size() > 1);
        m_sizeStack.pop_back();
    }

    void setEnableSorting(bool _enable);
    bool getEnableSorting() const { return m_enableSortingStack.back(); }
    void pushEnableSorting(bool _enable);
    void popEnableSorting();

    Id getLayerId() const { return m_layerIdStack.back(); }
    void pushLayerId(Id _layer);
    void popLayerId();

    void setMatrix(const Mat4 &_mat4) { m_matrixStack.back() = _mat4; }
    const Mat4 &getMatrix() const { return m_matrixStack.back(); }
    void pushMatrix(const Mat4 &_mat4) { m_matrixStack.push_back(_mat4); }
    void popMatrix()
    {
        IM3D_ASSERT(m_matrixStack.size() > 1);
        m_matrixStack.pop_back();
    }

    void setId(Id _id) { m_idStack.back() = _id; }
    Id getId() const { return m_idStack.back(); }
    void pushId(Id _id) { m_idStack.push_back(_id); }
    void popId()
    {
        IM3D_ASSERT(m_idStack.size() > 1);
        m_idStack.pop_back();
    }

    AppData &getAppData() { return m_appData; }

    Context();
    ~Context();

    // low-level interface for internal and app-defined gizmos, may be subject to breaking changes

    bool gizmoAxisTranslation_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _snap, float _worldHeight, float _worldSize, Vec3 *_out_);
    void gizmoAxisTranslation_Draw(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _worldHeight, float _worldSize, Color _color);

    bool gizmoPlaneTranslation_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_normal, float _snap, float _worldSize, Vec3 *_out_);
    void gizmoPlaneTranslation_Draw(Id _id, const Vec3 &_origin, const Vec3 &_normal, float _worldSize, Color _color);

    bool gizmoAxislAngle_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _snap, float _worldRadius, float _worldSize, float *_out_);
    void gizmoAxislAngle_Draw(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _worldRadius, float _angle, Color _color);

    bool gizmoAxisScale_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _snap, float _worldHeight, float _worldSize, float *_out_);
    void gizmoAxisScale_Draw(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _worldHeight, float _worldSize, Color _color);

    // Convert pixels -> world space size based on distance between _position and view origin.
    float pixelsToWorldSize(const Vec3 &_position, float _pixels);
    // Convert world space size -> pixels based on distance between _position and view origin.
    float worldSizeToPixels(const Vec3 &_position, float _pixels);
    // Blend between _min and _max based on distance betwen _position and view origin.
    int estimateLevelOfDetail(const Vec3 &_position, float _worldSize, int _min = 4, int _max = 256);

    // Make _id hot if _depth < m_hotDepth && _intersects.
    bool makeHot(Id _id, float _depth, bool _intersects);
    // Make _id active.
    void makeActive(Id _id);
    // Reset the acive/hot ids and the hot depth.
    void resetId();

    // Interpret key state.
    bool isKeyDown(Key _key) const { return m_keyDownCurr[_key]; }
    bool wasKeyPressed(Key _key) const { return m_keyDownCurr[_key] && !m_keyDownPrev[_key]; }

    // Visibiity tests for culling.
    bool isVisible(const VertexData *_vdata, DrawPrimitiveType _prim); // per-vertex
    bool isVisible(const Vec3 &_origin, float _radius);                // sphere
    bool isVisible(const Vec3 &_min, const Vec3 &_max);                // axis-aligned box

    // gizmo state
    bool m_gizmoLocal;     // Global mode selection for gizmos.
    GizmoMode m_gizmoMode; //               "
    Id m_activeId;         // Currently active gizmo. If set, this is the same as m_hotId.
    Id m_hotId;
    Id m_appId;
    Id m_appActiveId;
    Id m_appHotId;
    float m_hotDepth;          // Depth of the current hot gizmo along the cursor ray, for handling occlusion.
    Vec3 m_gizmoStateVec3;     // Stored state for the active gizmo.
    Mat3 m_gizmoStateMat3;     //               "
    float m_gizmoStateFloat;   //               "
    float m_gizmoHeightPixels; // Height/radius of gizmos.
    float m_gizmoSizePixels;   // Thickness of gizmo lines.

    // stats/debugging

    // Return the total number of primitives (sorted + unsorted) of the given _type in all layers.
    U32 getPrimitiveCount(DrawPrimitiveType _type) const;

    // Return the number of layers.
    U32 getLayerCount() const { return m_layerIdMap.size(); }

private:
    // state stacks
    Vector<Color> m_colorStack;
    Vector<float> m_alphaStack;
    Vector<float> m_sizeStack;
    Vector<bool> m_enableSortingStack;
    Vector<Mat4> m_matrixStack;
    Vector<Id> m_idStack;
    Vector<Id> m_layerIdStack;

    // vertex data: one list per layer, per primitive type, *2 for sorted/unsorted
    typedef Vector<VertexData> VertexList;
    Vector<VertexList *> m_vertexData[2]; // Each layer is DrawPrimitive_Count consecutive lists.
    int m_vertexDataIndex;                // 0, or 1 if sorting enabled.
    Vector<Id> m_layerIdMap;              // Map Id -> vertex data index.
    int m_layerIndex;                     // Index of the currently active layer in m_layerIdMap.
    Vector<DrawList> m_drawLists;         // All draw lists for the current frame, available after calling endFrame() before calling reset().
    bool m_sortCalled;                    // Avoid calling sort() during every call to draw().
    bool m_endFrameCalled;                // For assert, if vertices are pushed after endFrame() was called.

    // primitive state
    PrimitiveMode m_primMode;
    DrawPrimitiveType m_primType;
    U32 m_firstVertThisPrim; // Index of the first vertex pushed during this primitive.
    U32 m_vertCountThisPrim; // # calls to vertex() since the last call to begin().
    Vec3 m_minVertThisPrim;
    Vec3 m_maxVertThisPrim;

    // app data
    AppData m_appData;
    bool m_keyDownCurr[Key_Count];          // Key state captured during reset().
    bool m_keyDownPrev[Key_Count];          // Key state from previous frame.
    Vec4 m_cullFrustum[FrustumPlane_Count]; // Optimized frustum planes from m_appData.m_cullFrustum.
    int m_cullFrustumCount;                 // # valid frustum planes in m_cullFrustum.

    // Sort primitive data.
    void sort();

    // Return -1 if _id not found.
    int findLayerIndex(Id _id) const;

    VertexList *getCurrentVertexList();
};
} // namespace Im3d