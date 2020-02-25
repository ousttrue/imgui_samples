#include "im3d_context.h"
#include "im3d.h"
#include "im3d_math.h"
#include <cstdlib>
#include <cstring>
#include <cfloat>

namespace Im3d
{
#if defined(IM3D_MALLOC) && !defined(IM3D_FREE)
#error im3d: IM3D_MALLOC defined without IM3D_FREE; define both or neither
#endif
#if defined(IM3D_FREE) && !defined(IM3D_MALLOC)
#error im3d: IM3D_FREE defined without IM3D_MALLOC; define both or neither
#endif
#ifndef IM3D_MALLOC
#define IM3D_MALLOC(size) malloc(size)
#endif
#ifndef IM3D_FREE
#define IM3D_FREE(ptr) free(ptr)
#endif

static const int VertsPerDrawPrimitive[DrawPrimitive_Count] =
    {
        3, //DrawPrimitive_Triangles,
        2, //DrawPrimitive_Lines,
        1  //DrawPrimitive_Points,
};

/*******************************************************************************

                                  Vector

*******************************************************************************/

static void *AlignedMalloc(size_t _size, size_t _align)
{
    IM3D_ASSERT(_size > 0);
    IM3D_ASSERT(_align > 0);
    size_t grow = (_align - 1) + sizeof(void *);
    size_t mem = (size_t)IM3D_MALLOC(_size + grow);
    if (mem)
    {
        size_t ret = (mem + grow) & (~(_align - 1));
        IM3D_ASSERT(ret % _align == 0);           // aligned correctly
        IM3D_ASSERT(ret >= mem + sizeof(void *)); // header large enough to store a ptr
        *((void **)(ret - sizeof(void *))) = (void *)mem;
        return (void *)ret;
    }
    else
    {
        return nullptr;
    }
}

static void AlignedFree(void *_ptr_)
{
    void *mem = *((void **)((size_t)_ptr_ - sizeof(void *)));
    IM3D_FREE(mem);
}

template <typename T>
Vector<T>::~Vector()
{
    if (m_data)
    {
        AlignedFree(m_data);
        m_data = 0;
    }
}

template <typename T>
void Vector<T>::append(const T *_v, U32 _count)
{
    if (_count == 0)
    {
        return;
    }
    U32 sz = m_size + _count;
    reserve(sz);
    memcpy(end(), _v, sizeof(T) * _count);
    m_size = sz;
}

template <typename T>
void Vector<T>::reserve(U32 _capacity)
{
    _capacity = _capacity < 8 ? 8 : _capacity;
    if (_capacity <= m_capacity)
    {
        return;
    }
    T *data = (T *)AlignedMalloc(sizeof(T) * _capacity, alignof(T));
    if (m_data)
    {
        memcpy(data, m_data, sizeof(T) * m_size);
        AlignedFree(m_data);
        ;
    }
    m_data = data;
    m_capacity = _capacity;
}

template <typename T>
void Vector<T>::resize(U32 _size, const T &_val)
{
    reserve(_size);
    while (m_size < _size)
    {
        push_back(_val);
    }
    m_size = _size;
}

template <typename T>
void Vector<T>::swap(Vector<T> &_a_, Vector<T> &_b_)
{
    T *data = _a_.m_data;
    U32 capacity = _a_.m_capacity;
    U32 size = _a_.m_size;
    _a_.m_data = _b_.m_data;
    _a_.m_capacity = _b_.m_capacity;
    _a_.m_size = _b_.m_size;
    _b_.m_data = data;
    _b_.m_capacity = capacity;
    _b_.m_size = size;
}

template class Vector<bool>;
template class Vector<char>;
template class Vector<float>;
template class Vector<Id>;
template class Vector<Mat4>;
template class Vector<Color>;
template class Vector<DrawList>;

/*******************************************************************************

                                 Context

*******************************************************************************/

static Context g_DefaultContext;
IM3D_THREAD_LOCAL Context *Im3d::internal::g_CurrentContext = &g_DefaultContext;

void Context::begin(PrimitiveMode _mode)
{
    IM3D_ASSERT(!m_endFrameCalled);                // Begin*() called after EndFrame() but before NewFrame(), or forgot to call NewFrame()
    IM3D_ASSERT(m_primMode == PrimitiveMode_None); // forgot to call End()
    m_primMode = _mode;
    m_vertCountThisPrim = 0;
    switch (m_primMode)
    {
    case PrimitiveMode_Points:
        m_primType = DrawPrimitive_Points;
        break;
    case PrimitiveMode_Lines:
    case PrimitiveMode_LineStrip:
    case PrimitiveMode_LineLoop:
        m_primType = DrawPrimitive_Lines;
        break;
    case PrimitiveMode_Triangles:
    case PrimitiveMode_TriangleStrip:
        m_primType = DrawPrimitive_Triangles;
        break;
    default:
        break;
    };
    m_firstVertThisPrim = getCurrentVertexList()->size();
}

void Context::end()
{
    IM3D_ASSERT(m_primMode != PrimitiveMode_None); // End() called without Begin*()
    if (m_vertCountThisPrim > 0)
    {
        VertexList *vertexList = getCurrentVertexList();
        switch (m_primMode)
        {
        case PrimitiveMode_Points:
            break;
        case PrimitiveMode_Lines:
            IM3D_ASSERT(m_vertCountThisPrim % 2 == 0);
            break;
        case PrimitiveMode_LineStrip:
            IM3D_ASSERT(m_vertCountThisPrim > 1);
            break;
        case PrimitiveMode_LineLoop:
            IM3D_ASSERT(m_vertCountThisPrim > 1);
            vertexList->push_back(vertexList->back());
            vertexList->push_back((*vertexList)[m_firstVertThisPrim]);
            break;
        case PrimitiveMode_Triangles:
            IM3D_ASSERT(m_vertCountThisPrim % 3 == 0);
            break;
        case PrimitiveMode_TriangleStrip:
            IM3D_ASSERT(m_vertCountThisPrim >= 3);
            break;
        default:
            break;
        };
#if IM3D_CULL_PRIMITIVES
        // \hack force the bounds to be slightly conservative to account for point/line size
        m_minVertThisPrim = m_minVertThisPrim - Vec3(1.0f);
        m_maxVertThisPrim = m_maxVertThisPrim + Vec3(1.0f);
        if (!isVisible(m_minVertThisPrim, m_maxVertThisPrim))
        {
            vertexList->resize(m_firstVertThisPrim, VertexData());
        }
#endif
    }
    m_primMode = PrimitiveMode_None;
    m_primType = DrawPrimitive_Count;
#if IM3D_CULL_PRIMITIVES
    // \debug draw primitive BBs
    //if (m_enableCulling) {
    //	m_enableCulling = false;
    //	pushColor(Im3d::Color_Magenta);
    //	pushSize(1.0f);
    //	pushMatrix(Mat4(1.0f));
    //	DrawAlignedBox(m_minVertThisPrim, m_maxVertThisPrim);
    //	popMatrix();
    //	popColor();
    //	popSize();
    //	m_enableCulling = true;
    //}
#endif
}

void Context::vertex(const Vec3 &_position, float _size, Color _color)
{
    IM3D_ASSERT(m_primMode != PrimitiveMode_None); // Vertex() called without Begin*()

    VertexData vd(_position, _size, _color);
    if (m_matrixStack.size() > 1)
    { // optim, skip the matrix multiplication when the stack size is 1
        vd.m_positionSize = Vec4(m_matrixStack.back() * _position, _size);
    }
    vd.m_color.setA(vd.m_color.getA() * m_alphaStack.back());

#if IM3D_CULL_PRIMITIVES
    Vec3 p = Vec3(vd.m_positionSize);
    if (m_vertCountThisPrim == 0)
    { // p is the first vertex
        m_minVertThisPrim = m_maxVertThisPrim = p;
    }
    else
    {
        m_minVertThisPrim = Min(m_minVertThisPrim, p);
        m_maxVertThisPrim = Max(m_maxVertThisPrim, p);
    }
#endif

    VertexList *vertexList = getCurrentVertexList();
    switch (m_primMode)
    {
    case PrimitiveMode_Points:
    case PrimitiveMode_Lines:
    case PrimitiveMode_Triangles:
        vertexList->push_back(vd);
        break;
    case PrimitiveMode_LineStrip:
    case PrimitiveMode_LineLoop:
        if (m_vertCountThisPrim >= 2)
        {
            vertexList->push_back(vertexList->back());
            ++m_vertCountThisPrim;
        }
        vertexList->push_back(vd);
        break;
    case PrimitiveMode_TriangleStrip:
        if (m_vertCountThisPrim >= 3)
        {
            vertexList->push_back(*(vertexList->end() - 2));
            vertexList->push_back(*(vertexList->end() - 2));
            m_vertCountThisPrim += 2;
        }
        vertexList->push_back(vd);
        break;
    default:
        break;
    };
    ++m_vertCountThisPrim;

#if 0
	 // per-vertex primitive culling; this method is generally too expensive to be practical (and can't cull line loops).

	 // check if the primitive was visible and rewind vertex data if not
		switch (m_primMode) {
			case PrimitiveMode_Points:
				if (!isVisible(&vertexList->back(), DrawPrimitive_Points)) {
					vertexList->pop_back();
					--m_vertCountThisPrim;
				}
				break;
			case PrimitiveMode_LineLoop:
				break; // can't cull line loops; end() may add an invalid line if any vertices are culled
			case PrimitiveMode_Lines:
			case PrimitiveMode_LineStrip:
				if (m_vertCountThisPrim % 2 == 0) {
					if (!isVisible(&vertexList->back() - 1, DrawPrimitive_Lines)) {
						for (int i = 0; i < 2; ++i) {
							vertexList->pop_back();
							--m_vertCountThisPrim;
						}
					}
				}
				break;
			case PrimitiveMode_Triangles:
			case PrimitiveMode_TriangleStrip:
				if (m_vertCountThisPrim % 3 == 0) {
					if (!isVisible(&vertexList->back() - 2, DrawPrimitive_Triangles)) {
						for (int i = 0; i < 3; ++i) {
							vertexList->pop_back();
							--m_vertCountThisPrim;
						}
					}
				}
				break;
			default:
				break;
		};
#endif
}

void Context::reset()
{
    // all state stacks should be default here, else there was a mismatched Push*()/Pop*()
    IM3D_ASSERT(m_colorStack.size() == 1);
    IM3D_ASSERT(m_alphaStack.size() == 1);
    IM3D_ASSERT(m_sizeStack.size() == 1);
    IM3D_ASSERT(m_enableSortingStack.size() == 1);
    IM3D_ASSERT(m_layerIdStack.size() == 1);
    IM3D_ASSERT(m_matrixStack.size() == 1);
    IM3D_ASSERT(m_idStack.size() == 1);

    IM3D_ASSERT(m_primMode == PrimitiveMode_None);
    m_primMode = PrimitiveMode_None;
    m_primType = DrawPrimitive_Count;

    for (U32 i = 0; i < m_vertexData[0].size(); ++i)
    {
        m_vertexData[0][i]->clear();
        m_vertexData[1][i]->clear();
    }
    m_drawLists.clear();
    m_sortCalled = false;
    m_endFrameCalled = false;

    m_appData.m_viewDirection = Normalize(m_appData.m_viewDirection);

    // copy keydown array internally so that we can make a delta to detect key presses
    memcpy(m_keyDownPrev, m_keyDownCurr, Key_Count);       // \todo avoid this copy, use an index
    memcpy(m_keyDownCurr, m_appData.m_keyDown, Key_Count); // must copy in case m_keyDown is updated after reset (e.g. by an app callback)

    // process cull frustum
    m_cullFrustumCount = 0;
    for (int i = 0; i < FrustumPlane_Count; ++i)
    {
        const Vec4 &plane = m_appData.m_cullFrustum[i];
        if (m_appData.m_projOrtho && i == FrustumPlane_Near)
        { // skip near plane if perspective
            continue;
        }
        if (std::isinf(plane.w))
        { // may be the case e.g. for the far plane if projection is infinite
            continue;
        }
        m_cullFrustum[m_cullFrustumCount++] = plane;
    }

    // update gizmo modes
    if (wasKeyPressed(Action_GizmoTranslation))
    {
        m_gizmoMode = GizmoMode_Translation;
        resetId();
    }
    else if (wasKeyPressed(Action_GizmoRotation))
    {
        m_gizmoMode = GizmoMode_Rotation;
        resetId();
    }
    else if (wasKeyPressed(Action_GizmoScale))
    {
        m_gizmoMode = GizmoMode_Scale;
        resetId();
    }
    if (wasKeyPressed(Action_GizmoLocal))
    {
        m_gizmoLocal = !m_gizmoLocal;
        resetId();
    }
}

void Context::merge(const Context &_src)
{
    IM3D_ASSERT(!m_endFrameCalled && !_src.m_endFrameCalled); // call MergeContexts() before calling EndFrame()

    // layer IDs
    for (auto &id : _src.m_layerIdMap)
    {
        pushLayerId(id); // add a new layer if id doesn't alrady exist
        popLayerId();
    }

    // vertex data
    for (U32 i = 0; i < 2; ++i)
    {
        auto &vertexData = _src.m_vertexData[i];
        for (U32 j = 0; j < vertexData.size(); ++j)
        {
            // for each layer in _src, find the matching layer in this
            Id layerId = _src.m_layerIdMap[j / DrawPrimitive_Count];
            int layerIndex = findLayerIndex(layerId);
            IM3D_ASSERT(layerIndex >= 0);
            U32 k = j % DrawPrimitive_Count;
            m_vertexData[i][layerIndex * DrawPrimitive_Count + k]->append(*vertexData[j]);
        }
    }
}

void Context::endFrame()
{
    IM3D_ASSERT(!m_endFrameCalled); // EndFrame() was called multiple times for this frame
    m_endFrameCalled = true;

    // draw unsorted primitives first
    for (U32 i = 0; i < m_vertexData[0].size(); ++i)
    {
        if (m_vertexData[0][i]->size() > 0)
        {
            DrawList dl;
            dl.m_layerId = m_layerIdMap[i / DrawPrimitive_Count];
            dl.m_primType = (DrawPrimitiveType)(i % DrawPrimitive_Count);
            dl.m_vertexData = m_vertexData[0][i]->data();
            dl.m_vertexCount = m_vertexData[0][i]->size();
            m_drawLists.push_back(dl);
        }
    }

    // draw sorted primitives second
    if (!m_sortCalled)
    {
        sort();
    }
}

void Context::draw()
{
    if (m_drawLists.empty())
    {
        endFrame();
    }

    IM3D_ASSERT(m_appData.drawCallback);
    for (auto &drawList : m_drawLists)
    {
        m_appData.drawCallback(drawList);
    }
}

void Context::pushEnableSorting(bool _enable)
{
    IM3D_ASSERT(m_primMode == PrimitiveMode_None); // can't change sort mode mid-primitive
    m_vertexDataIndex = _enable ? 1 : 0;
    m_enableSortingStack.push_back(_enable);
}
void Context::popEnableSorting()
{
    IM3D_ASSERT(m_primMode == PrimitiveMode_None); // can't change sort mode mid-primitive
    m_enableSortingStack.pop_back();
    m_vertexDataIndex = m_enableSortingStack.back() ? 1 : 0;
}
void Context::setEnableSorting(bool _enable)
{
    IM3D_ASSERT(m_primMode == PrimitiveMode_None); // can't change sort mode mid-primitive
    m_vertexDataIndex = _enable ? 1 : 0;
    m_enableSortingStack.back() = _enable;
}

void Context::pushLayerId(Id _layer)
{
    IM3D_ASSERT(m_primMode == PrimitiveMode_None); // can't change layer mid-primitive
    int idx = findLayerIndex(_layer);
    if (idx == -1)
    { // not found, push new layer
        idx = m_layerIdMap.size();
        m_layerIdMap.push_back(_layer);
        for (int i = 0; i < DrawPrimitive_Count; ++i)
        {
            m_vertexData[0].push_back((VertexList *)IM3D_MALLOC(sizeof(VertexList)));
            *m_vertexData[0].back() = VertexList();
            m_vertexData[1].push_back((VertexList *)IM3D_MALLOC(sizeof(VertexList)));
            *m_vertexData[1].back() = VertexList();
        }
    }
    m_layerIdStack.push_back(_layer);
    m_layerIndex = idx;
}
void Context::popLayerId()
{
    IM3D_ASSERT(m_layerIdStack.size() > 1);
    m_layerIdStack.pop_back();
    m_layerIndex = findLayerIndex(m_layerIdStack.back());
}

Context::Context()
{
    m_sortCalled = false;
    m_endFrameCalled = false;
    m_primMode = PrimitiveMode_None;
    m_vertexDataIndex = 0; // = sorting disabled
    m_layerIndex = 0;
    m_firstVertThisPrim = 0;
    m_vertCountThisPrim = 0;

    m_gizmoLocal = false;
    m_gizmoMode = GizmoMode_Translation;
    m_hotId = Id_Invalid;
    m_activeId = Id_Invalid;
    m_appId = Id_Invalid;
    m_appActiveId = Id_Invalid;
    m_appHotId = Id_Invalid;
    m_hotDepth = FLT_MAX;
    m_gizmoHeightPixels = 64.0f;
    m_gizmoSizePixels = 5.0f;

    memset(&m_appData, 0, sizeof(m_appData));
    memset(&m_keyDownCurr, 0, sizeof(m_keyDownCurr));
    memset(&m_keyDownPrev, 0, sizeof(m_keyDownPrev));

    // init cull frustum to INF effectively disables culling
    for (int i = 0; i < FrustumPlane_Count; ++i)
    {
        m_appData.m_cullFrustum[i] = Vec4(INFINITY);
    }

    pushMatrix(Mat4(1.0f));
    pushColor(Color_White);
    pushAlpha(1.0f);
    pushSize(1.0f);
    pushEnableSorting(false);
    pushLayerId(0);
    pushId(0x811C9DC5u); // fnv1 hash base
}

Context::~Context()
{
    for (int i = 0; i < 2; ++i)
    {
        while (!m_vertexData[i].empty())
        {
            m_vertexData[i].back()->~Vector(); // manually call dtor (vector is allocated via IM3D_MALLOC during pushLayerId)
            IM3D_FREE(m_vertexData[i].back());
            m_vertexData[i].pop_back();
        }
    }
}

namespace
{
struct SortData
{
    float m_key;
    VertexData *m_start;
    SortData() {}
    SortData(float _key, VertexData *_start) : m_key(_key), m_start(_start) {}
};

int SortCmp(const void *_a, const void *_b)
{
    float ka = ((SortData *)_a)->m_key;
    float kb = ((SortData *)_b)->m_key;
    if (ka < kb)
    {
        return 1;
    }
    else if (ka > kb)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void Reorder(Vector<VertexData> &_data_, const SortData *_sort, U32 _sortCount, U32 _primSize)
{
    Vector<VertexData> ret;
    ret.reserve(_data_.size());
    for (U32 i = 0; i < _sortCount; ++i)
    {
        for (U32 j = 0; j < _primSize; ++j)
        {
            ret.push_back(*(_sort[i].m_start + j));
        }
    }
    Vector<VertexData>::swap(_data_, ret);
}
} // namespace

void Context::sort()
{
    static IM3D_THREAD_LOCAL Vector<SortData> sortData[DrawPrimitive_Count]; // reduces # allocs

    for (U32 layer = 0; layer < m_layerIdMap.size(); ++layer)
    {
        Vec3 viewOrigin = m_appData.m_viewOrigin;

        // sort each primitive list internally
        for (int i = 0; i < DrawPrimitive_Count; ++i)
        {
            Vector<VertexData> &vertexData = *(m_vertexData[1][layer * DrawPrimitive_Count + i]);
            sortData[i].clear();
            if (!vertexData.empty())
            {
                sortData[i].reserve(vertexData.size() / VertsPerDrawPrimitive[i]);
                for (VertexData *v = vertexData.begin(); v != vertexData.end();)
                {
                    sortData[i].push_back(SortData(0.0f, v));
                    IM3D_ASSERT(v < vertexData.end());
                    for (int j = 0; j < VertsPerDrawPrimitive[i]; ++j, ++v)
                    {
                        // sort key is the primitive midpoint distance to view origin
                        sortData[i].back().m_key += Length2(Vec3(v->m_positionSize) - viewOrigin);
                    }
                    sortData[i].back().m_key /= (float)VertsPerDrawPrimitive[i];
                }
                // qsort is not necessarily stable but it doesn't matter assuming the prims are pushed in roughly the same order each frame
                qsort(sortData[i].data(), sortData[i].size(), sizeof(SortData), SortCmp);
                Reorder(vertexData, sortData[i].data(), sortData[i].size(), VertsPerDrawPrimitive[i]);
            }
        }

        // construct draw lists - partition sort data into non-overlapping lists
        int cprim = 0;
        SortData *search[DrawPrimitive_Count];
        int emptyCount = 0;
        for (int i = 0; i < DrawPrimitive_Count; ++i)
        {
            if (sortData[i].empty())
            {
                search[i] = 0;
                ++emptyCount;
            }
            else
            {
                search[i] = sortData[i].begin();
            }
        }
        bool first = true;
#define modinc(v) ((v + 1) % DrawPrimitive_Count)
        while (emptyCount != DrawPrimitive_Count)
        {
            while (search[cprim] == 0)
            {
                cprim = modinc(cprim);
            }
            // find the max key at the current position across all sort data
            float mxkey = search[cprim]->m_key;
            int mxprim = cprim;
            for (int p = modinc(cprim); p != cprim; p = modinc(p))
            {
                if (search[p] != 0 && search[p]->m_key > mxkey)
                {
                    mxkey = search[p]->m_key;
                    mxprim = p;
                }
            }

            // if draw list is empty or the layer or primitive changed, start a new draw list
            if (
                first ||
                m_drawLists.back().m_layerId != layer ||
                m_drawLists.back().m_primType != mxprim)
            {
                cprim = mxprim;
                DrawList dl;
                dl.m_layerId = layer;
                dl.m_primType = (DrawPrimitiveType)cprim;
                dl.m_vertexData = m_vertexData[1][layer * DrawPrimitive_Count + cprim]->data() + (search[cprim] - sortData[cprim].data()) * VertsPerDrawPrimitive[cprim];
                dl.m_vertexCount = 0;
                m_drawLists.push_back(dl);
                first = false;
            }

            // increment the vertex count for the current draw list
            m_drawLists.back().m_vertexCount += VertsPerDrawPrimitive[cprim];
            ++search[cprim];
            if (search[cprim] == sortData[cprim].end())
            {
                search[cprim] = 0;
                ++emptyCount;
            }
        }
#undef modinc
    }

    m_sortCalled = true;
}

int Context::findLayerIndex(Id _id) const
{
    for (int i = 0; i < (int)m_layerIdMap.size(); ++i)
    {
        if (m_layerIdMap[i] == _id)
        {
            return i;
        }
    }
    return -1;
}

bool Context::isVisible(const VertexData *_vdata, DrawPrimitiveType _prim)
{
    Vec3 pos[3];
    float size[3];
    for (int i = 0; i < VertsPerDrawPrimitive[_prim]; ++i)
    {
        pos[i] = Vec3(_vdata[i].m_positionSize);
        size[i] = _prim == DrawPrimitive_Triangles ? 0.0f : pixelsToWorldSize(pos[i], _vdata[i].m_positionSize.w);
    }
    for (int i = 0; i < m_cullFrustumCount; ++i)
    {
        const Vec4 &plane = m_cullFrustum[i];
        bool isVisible = false;
        for (int j = 0; j < VertsPerDrawPrimitive[_prim]; ++j)
        {
            isVisible |= Distance(plane, pos[j]) > -size[j];
        }
        if (!isVisible)
        {
            return false;
        }
    }
    return true;
}

bool Context::isVisible(const Vec3 &_origin, float _radius)
{
    for (int i = 0; i < m_cullFrustumCount; ++i)
    {
        const Vec4 &plane = m_cullFrustum[i];
        if (Distance(plane, _origin) < -_radius)
        {
            return false;
        }
    }
    return true;
}

bool Context::isVisible(const Vec3 &_min, const Vec3 &_max)
{
#if 0
 	const Vec3 points[] = {
		Vec3(_min.x, _min.y, _min.z),
		Vec3(_max.x, _min.y, _min.z),
		Vec3(_max.x, _max.y, _min.z),
		Vec3(_min.x, _max.y, _min.z),

		Vec3(_min.x, _min.y, _max.z),
		Vec3(_max.x, _min.y, _max.z),
		Vec3(_max.x, _max.y, _max.z),
		Vec3(_min.x, _max.y, _max.z)
	};

 	for (int i = 0; i < m_cullFrustumCount; ++i) {
		const Vec4& plane = m_cullFrustum[i];
		bool inside = false;
		for (int j = 0; j < 8; ++j) {
			if (Distance(plane, points[j]) > 0.0f) {
				inside = true;
				break;
			}
		}
		if (!inside) {
			return false;
		}
	}
	return true;
#else
    for (int i = 0; i < m_cullFrustumCount; ++i)
    {
        const Vec4 &plane = m_cullFrustum[i];
        float d =
            Max(_min.x * plane.x, _max.x * plane.x) +
            Max(_min.y * plane.y, _max.y * plane.y) +
            Max(_min.z * plane.z, _max.z * plane.z) -
            plane.w;
        if (d < 0.0f)
        {
            return false;
        }
    }
    return true;
#endif
}

Context::VertexList *Context::getCurrentVertexList()
{
    return m_vertexData[m_vertexDataIndex][m_layerIndex * DrawPrimitive_Count + m_primType];
}

float Context::pixelsToWorldSize(const Vec3 &_position, float _pixels)
{
    float d = m_appData.m_projOrtho ? 1.0f : Length(_position - m_appData.m_viewOrigin);
    return m_appData.m_projScaleY * d * (_pixels / m_appData.m_viewportSize.y);
}

float Context::worldSizeToPixels(const Vec3 &_position, float _size)
{
    float d = m_appData.m_projOrtho ? 1.0f : Length(_position - m_appData.m_viewOrigin);
    return (_size * m_appData.m_viewportSize.y) / d / m_appData.m_projScaleY;
}

int Context::estimateLevelOfDetail(const Vec3 &_position, float _worldSize, int _min, int _max)
{
    if (m_appData.m_projOrtho)
    {
        return _max;
    }
    float d = Length(_position - m_appData.m_viewOrigin);
    float x = Clamp(2.0f * atanf(_worldSize / (2.0f * d)), 0.0f, 1.0f);
    float fmin = (float)_min;
    float fmax = (float)_max;
    return (int)(fmin + (fmax - fmin) * x);
}

bool Context::gizmoAxisTranslation_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _snap, float _worldHeight, float _worldSize, Vec3 *_out_)
{
    if (_id != m_hotId)
    {
        // disable behavior when aligned
        Vec3 viewDir = m_appData.m_projOrtho
                           ? m_appData.m_viewDirection
                           : Normalize(m_appData.m_viewOrigin - _origin);
        float aligned = 1.0f - fabs(Dot(_axis, viewDir));
        if (aligned < 0.01f)
        {
            return false;
        }
    }

    Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
    Line axisLine(_origin, _axis);
    Capsule axisCapsule(_origin + _axis * (0.2f * _worldHeight), _origin + _axis * _worldHeight, _worldSize);

#if IM3D_GIZMO_DEBUG
    if (_id == m_hotId)
    {
        PushDrawState();
        EnableSorting(false);
        SetColor(Color_Magenta);
        SetAlpha(1.0f);
        DrawCapsule(axisCapsule.m_start, axisCapsule.m_end, axisCapsule.m_radius);
        PopDrawState();
    }
#endif

    Vec3 &storedPosition = m_gizmoStateVec3;

    if (_id == m_activeId)
    {
        if (isKeyDown(Action_Select))
        {
            float tr, tl;
            Nearest(ray, axisLine, tr, tl);
#if IM3D_RELATIVE_SNAP
            *_out_ = *_out_ + Snap(_axis * tl - storedPosition, _snap);
#else
            *_out_ = Snap(*_out_ + _axis * tl - storedPosition, _snap);
#endif

            return true;
        }
        else
        {
            makeActive(Id_Invalid);
        }
    }
    else if (_id == m_hotId)
    {
        if (Intersects(ray, axisCapsule))
        {
            if (isKeyDown(Action_Select))
            {
                makeActive(_id);
                float tr, tl;
                Nearest(ray, axisLine, tr, tl);
                storedPosition = _axis * tl;
            }
        }
        else
        {
            resetId();
        }
    }
    else
    {
        float t0, t1;
        bool intersects = Intersect(ray, axisCapsule, t0, t1);
        makeHot(_id, t0, intersects);
    }

    return false;
}

void Context::gizmoAxisTranslation_Draw(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _worldHeight, float _worldSize, Color _color)
{
    Vec3 viewDir = m_appData.m_projOrtho
                       ? m_appData.m_viewDirection
                       : Normalize(m_appData.m_viewOrigin - _origin);
    float aligned = 1.0f - fabs(Dot(_axis, viewDir));
    aligned = Remap(aligned, 0.05f, 0.1f);
    Color color = _color;
    if (_id == m_activeId)
    {
        color = Color_GizmoHighlight;
        pushEnableSorting(false);
        begin(PrimitiveMode_Lines);
        vertex(_origin - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
        vertex(_origin + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
        end();
        popEnableSorting();
    }
    else if (_id == m_hotId)
    {
        color = Color_GizmoHighlight;
        aligned = 1.0f;
    }
    color.setA(color.getA() * aligned);
    pushColor(color);
    pushSize(m_gizmoSizePixels);
    DrawArrow(
        _origin + _axis * (0.2f * _worldHeight),
        _origin + _axis * _worldHeight);
    popSize();
    popColor();
}

bool Context::gizmoPlaneTranslation_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_normal, float _snap, float _worldSize, Vec3 *_out_)
{
    Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
    Plane plane(_normal, _origin);

#if IM3D_GIZMO_DEBUG
    if (_id == m_hotId)
    {
        PushDrawState();
        EnableSorting(false);
        SetColor(Color_Magenta);
        SetAlpha(0.1f);
        DrawQuadFilled(_origin, _normal, Vec2(2.0f));
        SetAlpha(0.75f);
        SetSize(1.0f);
        DrawQuad(_origin, _normal, Vec2(2.0f));
        SetSize(2.0f);
        DrawCircle(_origin, _normal, 2.0f);
        PopDrawState();
    }
#endif

    float tr;
    bool intersects = Intersect(ray, plane, tr);
    if (!intersects)
    {
        return false;
    }
    Vec3 intersection = ray.m_origin + ray.m_direction * tr;
    intersects &= AllLess(Abs(intersection - _origin), Vec3(_worldSize));

    Vec3 &storedPosition = m_gizmoStateVec3;

    if (_id == m_activeId)
    {
        if (isKeyDown(Action_Select))
        {
#if IM3D_RELATIVE_SNAP
            intersection = Snap(intersection, plane, _snap);
            *_out_ = intersection + storedPosition;
#else
            *_out_ = Snap(intersection + storedPosition, plane, _snap);
#endif
            return true;
        }
        else
        {
            makeActive(Id_Invalid);
        }
    }
    else if (_id == m_hotId)
    {
        if (intersects)
        {
            if (isKeyDown(Action_Select))
            {
                makeActive(_id);
                storedPosition = *_out_ - intersection;
            }
        }
        else
        {
            resetId();
        }
    }
    else
    {
        makeHot(_id, tr, intersects);
    }

    return false;
}
void Context::gizmoPlaneTranslation_Draw(Id _id, const Vec3 &_origin, const Vec3 &_normal, float _worldSize, Color _color)
{
    Vec3 viewDir = m_appData.m_projOrtho
                       ? m_appData.m_viewDirection
                       : Normalize(m_appData.m_viewOrigin - _origin);
    Vec3 n = Mat3(m_matrixStack.back()) * _normal; // _normal may be in local space, need to transform to world space for the dot with viewDir to make sense
    float aligned = fabs(Dot(n, viewDir));
    aligned = Remap(aligned, 0.1f, 0.2f);
    Color color = _color;
    color.setA(color.getA() * aligned);
    pushColor(color);
    pushAlpha(_id == m_hotId ? 0.7f : 0.1f * getAlpha());
    DrawQuadFilled(_origin, _normal, Vec2(_worldSize));
    popAlpha();
    DrawQuad(_origin, _normal, Vec2(_worldSize));
    popColor();
}

bool Context::gizmoAxislAngle_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _snap, float _worldRadius, float _worldSize, float *_out_)
{
    Vec3 viewDir = m_appData.m_projOrtho
                       ? m_appData.m_viewDirection
                       : Normalize(m_appData.m_viewOrigin - _origin);
    float aligned = fabs(Dot(_axis, viewDir));
    float tr = 0.0f;
    Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
    bool intersects = false;
    Vec3 intersection;
    if (aligned < 0.05f)
    {
        // ray-plane intersection fails at grazing angles, use capsule interesection
        float t1;
        Vec3 capsuleAxis = Cross(viewDir, _axis);
        Capsule capsule(_origin + capsuleAxis * _worldRadius, _origin - capsuleAxis * _worldRadius, _worldSize * 0.5f);
        intersects = Intersect(ray, capsule, tr, t1);
        intersection = ray.m_origin + ray.m_direction * tr;
#if IM3D_GIZMO_DEBUG
        if (_id == m_hotId)
        {
            PushDrawState();
            SetColor(Im3d::Color_Magenta);
            SetSize(3.0f);
            DrawCapsule(capsule.m_start, capsule.m_end, capsule.m_radius);
            PopDrawState();
        }
#endif
    }
    else
    {
        Plane plane(_axis, _origin);
        intersects = Intersect(ray, plane, tr);
        intersection = ray.m_origin + ray.m_direction * tr;
        float dist = Length(intersection - _origin);
        intersects &= fabs(dist - _worldRadius) < (_worldSize + _worldSize * (1.0f - aligned) * 2.0f);
    }

    Vec3 &storedVec = m_gizmoStateVec3;
    float &storedAngle = m_gizmoStateFloat;
    bool ret = false;

    // use a view-aligned plane intersection to generate the rotation delta
    Plane viewPlane(viewDir, _origin);
    Intersect(ray, viewPlane, tr);
    intersection = ray.m_origin + ray.m_direction * tr;

    if (_id == m_activeId)
    {
        if (isKeyDown(Action_Select))
        {
            Vec3 delta = Normalize(intersection - _origin);
            float sign = Dot(Cross(storedVec, delta), _axis);
            float angle = acosf(Clamp(Dot(delta, storedVec), -1.0f, 1.0f));
#if IM3D_RELATIVE_SNAP
            *_out_ = storedAngle + copysignf(Snap(angle, _snap), sign);
#else
            *_out_ = Snap(storedAngle + copysignf(angle, sign), _snap);
#endif
            return true;
        }
        else
        {
            makeActive(Id_Invalid);
        }
    }
    else if (_id == m_hotId)
    {
        if (intersects)
        {
            if (isKeyDown(Action_Select))
            {
                makeActive(_id);
                storedVec = Normalize(intersection - _origin);
                storedAngle = Snap(*_out_, m_appData.m_snapRotation);
            }
        }
        else
        {
            resetId();
        }
    }
    else
    {
        makeHot(_id, tr, intersects);
    }
    return false;
}
void Context::gizmoAxislAngle_Draw(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _worldRadius, float _angle, Color _color)
{
    Vec3 viewDir = m_appData.m_projOrtho
                       ? m_appData.m_viewDirection
                       : Normalize(m_appData.m_viewOrigin - _origin);
    float aligned = fabs(Dot(_axis, viewDir));

    Vec3 &storedVec = m_gizmoStateVec3;
    Color color = _color;

    if (_id == m_activeId)
    {
        color = Color_GizmoHighlight;
        Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
        Plane plane(_axis, _origin);
        float tr;
        if (Intersect(ray, plane, tr))
        {
            Vec3 intersection = ray.m_origin + ray.m_direction * tr;
            Vec3 delta = Normalize(intersection - _origin);

            pushAlpha(Remap(aligned, 1.0f, 0.99f));
            pushEnableSorting(false);
            begin(PrimitiveMode_Lines);
            vertex(_origin - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
            vertex(_origin + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
            //vertex(_origin, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
            //vertex(_origin + storedVec * _worldRadius, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
            end();
            popEnableSorting();
            popAlpha();

            pushColor(Color_GizmoHighlight);
            pushSize(m_gizmoSizePixels);
            DrawArrow(_origin, _origin + delta * _worldRadius);
            popSize();
            popColor();
            begin(PrimitiveMode_Points);
            vertex(_origin, m_gizmoSizePixels * 2.0f, Color_GizmoHighlight);
            end();
        }
    }
    else if (_id == m_hotId)
    {
        color = Color_GizmoHighlight;
    }
    aligned = Max(Remap(aligned, 0.9f, 1.0f), 0.1f);
    if (m_activeId == _id)
    {
        aligned = 1.0f;
    }
    pushColor(color);
    pushSize(m_gizmoSizePixels);
    pushMatrix(getMatrix() * LookAt(_origin, _origin + _axis, m_appData.m_worldUp));
    begin(PrimitiveMode_LineLoop);
    const int detail = estimateLevelOfDetail(_origin, _worldRadius, 16, 128);
    for (int i = 0; i < detail; ++i)
    {
        float rad = TwoPi * ((float)i / (float)detail);
        vertex(Vec3(cosf(rad) * _worldRadius, sinf(rad) * _worldRadius, 0.0f));

        // post-modify the alpha for parts of the ring occluded by the sphere
        VertexData &vd = getCurrentVertexList()->back();
        Vec3 v = vd.m_positionSize;
        float d = Dot(Normalize(_origin - v), m_appData.m_viewDirection);
        d = Max(Remap(d, 0.1f, 0.2f), aligned);
        vd.m_color.setA(vd.m_color.getA() * d);
    }
    end();
    popMatrix();
    popSize();
    popColor();
}

bool Context::gizmoAxisScale_Behavior(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _snap, float _worldHeight, float _worldSize, float *_out_)
{
    Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
    Line axisLine(_origin, _axis);
    Capsule axisCapsule(_origin + _axis * (0.2f * _worldHeight), _origin + _axis * _worldHeight, _worldSize);

#if IM3D_GIZMO_DEBUG
    if (_id == m_hotId)
    {
        PushDrawState();
        EnableSorting(false);
        SetColor(Color_Magenta);
        SetAlpha(1.0f);
        DrawCapsule(axisCapsule.m_start, axisCapsule.m_end, axisCapsule.m_radius);
        PopDrawState();
    }
#endif

    Vec3 &storedPosition = m_gizmoStateVec3;
    float &storedScale = m_gizmoStateFloat;

    if (_id == m_activeId)
    {
        if (isKeyDown(Action_Select))
        {
            float tr, tl;
            Nearest(ray, axisLine, tr, tl);
            Vec3 intersection = _axis * tl;
            Vec3 delta = intersection - storedPosition;
            float sign = Dot(delta, _axis);
#if 1
            // relative snap
            float scale = Snap(Length(delta) / _worldHeight, _snap);
            *_out_ = storedScale * Max(1.0f + copysignf(scale, sign), 1e-3f);
#else
            // absolute snap
            float scale = Length(delta) / _worldHeight;
            *_out_ = Max(Snap(storedScale * (1.0f + copysignf(scale, sign)), _snap), 1e-3f);
#endif
            return true;
        }
        else
        {
            makeActive(Id_Invalid);
        }
    }
    else if (_id == m_hotId)
    {
        if (Intersects(ray, axisCapsule))
        {
            if (isKeyDown(Action_Select))
            {
                makeActive(_id);
                float tr, tl;
                Nearest(ray, axisLine, tr, tl);
                storedPosition = _axis * tl;
                storedScale = *_out_;
            }
        }
        else
        {
            resetId();
        }
    }
    else
    {
        float t0, t1;
        bool intersects = Intersect(ray, axisCapsule, t0, t1);
        makeHot(_id, t0, intersects);
    }

    return false;
}
void Context::gizmoAxisScale_Draw(Id _id, const Vec3 &_origin, const Vec3 &_axis, float _worldHeight, float _worldSize, Color _color)
{
    Vec3 viewDir = m_appData.m_projOrtho
                       ? m_appData.m_viewDirection
                       : Normalize(m_appData.m_viewOrigin - _origin);
    float aligned = 1.0f - fabs(Dot(_axis, viewDir));
    aligned = Remap(aligned, 0.05f, 0.1f);
    Color color = _color;
    if (_id == m_activeId)
    {
        color = Color_GizmoHighlight;
        pushEnableSorting(false);
        begin(PrimitiveMode_Lines);
        vertex(_origin - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
        vertex(_origin + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
        end();
        popEnableSorting();
    }
    else if (_id == m_hotId)
    {
        color = Color_GizmoHighlight;
        aligned = 1.0f;
    }
    color.setA(color.getA() * aligned);
    begin(PrimitiveMode_LineLoop);
    vertex(_origin + _axis * (0.2f * _worldHeight), m_gizmoSizePixels, color);
    vertex(_origin + _axis * _worldHeight, m_gizmoSizePixels, color);
    end();
    begin(PrimitiveMode_Points);
    vertex(_origin + _axis * _worldHeight, m_gizmoSizePixels * 2.0f, color);
    end();
}

bool Context::makeHot(Id _id, float _depth, bool _intersects)
{
    if (m_activeId == Id_Invalid && _depth < m_hotDepth && _intersects && !isKeyDown(Action_Select))
    {
        m_hotId = _id;
        m_appHotId = m_appId;
        m_hotDepth = _depth;
        return true;
    }
    return false;
}

void Context::makeActive(Id _id)
{
    m_activeId = _id;
    m_appActiveId = _id == Id_Invalid ? Id_Invalid : m_appId;
}

void Context::resetId()
{
    m_activeId = m_hotId = m_appActiveId = m_appHotId = Id_Invalid;
    m_hotDepth = FLT_MAX;
}

U32 Context::getPrimitiveCount(DrawPrimitiveType _type) const
{
    U32 ret = 0;
    for (U32 i = 0; i < m_layerIdMap.size(); ++i)
    {
        U32 j = i * DrawPrimitive_Count + _type;
        ret += m_vertexData[0][j]->size() + m_vertexData[1][j]->size();
    }
    ret /= VertsPerDrawPrimitive[_type];
    return ret;
}
} // namespace Im3d