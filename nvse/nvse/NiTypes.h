#pragma once
#include "Utilities.h"

#if RUNTIME

const UInt32 _NiTMap_Lookup = 0x00853130;

#endif

// 8
struct NiRTTI
{
	const char	* name;
	NiRTTI		* parent;
};

//Copied from JIP LN NVSE - Needed for Overcharge NVSE
// 08
struct NiPoint2
{
	float	x, y;

	NiPoint2() {}
	__forceinline NiPoint2(float _x, float _y) : x(_x), y(_y) {}
	__forceinline NiPoint2(const NiPoint2& rhs) { *this = rhs; }
	__forceinline explicit NiPoint2(const __m128 rhs) { SetPS(rhs); }

	__forceinline void operator=(NiPoint2&& rhs)
	{
		x = rhs.x;
		y = rhs.y;
	}
	__forceinline void operator=(const NiPoint2& rhs) { _mm_storeu_si64(this, _mm_loadu_si64(&rhs)); }

	__forceinline NiPoint2& SetPS(const __m128 rhs)
	{
		_mm_storeu_si64(this, _mm_castps_si128(rhs));
		return *this;
	}

	inline operator float* () { return &x; }

	__forceinline __m128 PS() const { return _mm_castsi128_ps(_mm_loadu_si64(this)); }
};

// C
struct NiVector3
{
	float	x, y, z;
};

// 10 - always aligned?
struct NiVector4
{
	float	x, y, z, w;
};

// 10 - always aligned?
struct NiQuaternion
{
	float	x, y, z, w;

	NiQuaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f)
		: w(w), x(x), y(y), z(z) {}

	void setFromEuler(float eulerX, float eulerY, float eulerZ);

	static NiQuaternion createFromEuler(float eulerX, float eulerY, float eulerZ);

};

// 24
struct NiMatrix33
{
	float	cr[3][3];

	float __vectorcall ExtractPitch() const;
	float __vectorcall ExtractRoll() const;
	float __vectorcall ExtractYaw() const;
	void ExtractAngles(double& rotX, double& rotY, double& rotZ);
	void ExtractAnglesDegrees(double& rotX, double& rotY, double& rotZ);
	void RotationMatrix(float rotX, float rotY, float rotZ);
	void Rotate(float rotX, float rotY, float rotZ);
	void MultiplyMatrices(NiMatrix33& matA, NiMatrix33& matB);
	void Dump(const char* title = NULL);
	void Copy(NiMatrix33* toCopy);
	float Determinant();
	void Inverse(NiMatrix33& in);
	void FromAxisAngle(float theta, float x, float y, float z);
	void RotateDegrees(float rotX, float rotY, float rotZ);
	void SetRotationFromEulerDegrees(float rotX, float rotY, float rotZ);
	void RotationMatrixDegrees(float rotX, float rotY, float rotZ);
};

// 34
struct NiTransform
{
	NiMatrix33	rotate;		// 00
	NiVector3	translate;	// 24
	float		scale;		// 30
};

// 10
struct NiSphere
{
	float	x, y, z, radius;
};

// 10
struct NiBound
{
	NiVector3	center;
	float		radius;

	NiBound() {}
	__forceinline NiBound(float cX, float cY, float cZ, float rad) : center(cX, cY, cZ), radius(rad) {}
	__forceinline NiBound(const NiBound& rhs) { *this = rhs; }
	__forceinline explicit NiBound(const __m128 rhs) { SetPS(rhs); }

	__forceinline void operator=(NiBound&& rhs)
	{
		center.x = rhs.center.x;
		center.y = rhs.center.y;
		center.z = rhs.center.z;
		radius = rhs.radius;
	}
	__forceinline void operator=(const NiBound& rhs) { SetPS(rhs.PS()); }

	__forceinline NiBound& SetPS(const __m128 rhs)
	{
		_mm_storeu_ps(&center.x, rhs);
		return *this;
	}

	inline operator float* () { return &center.x; }
	__forceinline __m128 PS() const { return _mm_loadu_ps(&center.x); }

	void Merge(const NiBound* other) { ThisCall(0xA7F3F0, this, other); }
};

// 1C
struct NiFrustum
{
	float	l;			// 00
	float	r;			// 04
	float	t;			// 08
	float	b;			// 0C
	float	n;			// 10
	float	f;			// 14
	UInt8	o;			// 18
	UInt8	pad19[3];	// 19
};

// 10
struct NiViewport
{
	float	l;
	float	r;
	float	t;
	float	b;
};

// C
struct NiColor
{
	float	r;
	float	g;
	float	b;

	//Copied from JIP LN NVSE - Needed for OverchargeNVSE
	NiColor() {}
	__forceinline NiColor(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
	__forceinline NiColor(const NiColor& rhs) { *this = rhs;}
	__forceinline explicit NiColor(const __m128 rhs) { SetPS(rhs); }

	__forceinline void operator=(NiColor&& rhs)
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
	}
	__forceinline void operator=(const NiColor& rhs)
	{
		_mm_storeu_si64(this, _mm_loadu_si64(&rhs));
		b = rhs.b;
	}

	__forceinline NiColor& SetPS(const __m128 rhs)
	{
		_mm_storeu_si64(this, _mm_castps_si128(rhs));
		_mm_store_ss(&r, _mm_unpackhi_ps(rhs, rhs));
		return *this;
	}

	inline operator float* () { return &r; }
	__forceinline __m128 PS() const { return _mm_loadu_ps(&r); }
};

// 10
struct NiColorAlpha
{
	float	r, g, b, a;

	NiColorAlpha() {}
	__forceinline NiColorAlpha(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	__forceinline NiColorAlpha(const NiColorAlpha& rhs) { *this = rhs; }
	__forceinline explicit NiColorAlpha(const __m128 rhs) { SetPS(rhs); }

	__forceinline void operator=(NiColorAlpha&& rgba)
	{
		r = rgba.r;
		g = rgba.g;
		b = rgba.b;
		a = rgba.a;
	}
	__forceinline void operator=(const NiColorAlpha& rhs) { SetPS(rhs.PS()); }

	__forceinline NiColorAlpha& SetPS(const __m128 rhs)
	{
		_mm_storeu_ps(&r, rhs);
		return *this;
	}

	__forceinline NiColorAlpha& operator*=(float value) { return SetPS(_mm_mul_ps(PS(), _mm_set_ps1(value))); }

	inline operator float* () { return &r; }
	__forceinline __m128 PS() const { return _mm_loadu_ps(&r); }
};

// 10
struct NiPlane
{
	NiVector3	nrm;
	float		offset;
};

// 10
// NiTArrays are slightly weird: they can be sparse
// this implies that they can only be used with types that can be NULL?
// not sure on the above, but some code only works if this is true
// this can obviously lead to fragmentation, but the accessors don't seem to care
// weird stuff
template <typename T>
struct NiTArray
{
	void	** _vtbl;		// 00
	T		* data;			// 04
	UInt16	capacity;		// 08 - init'd to size of preallocation
	UInt16	firstFreeEntry;	// 0A - index of the first free entry in the block of free entries at the end of the array (or numObjs if full)
	UInt16	numObjs;		// 0C - init'd to 0
	UInt16	growSize;		// 0E - init'd to size of preallocation

	T operator[](UInt32 idx) {
		if (idx < firstFreeEntry)
			return data[idx];
		return NULL;
	}

	T Get(UInt32 idx) { return (*this)[idx]; }

	UInt16 Length(void) { return firstFreeEntry; }
	void AddAtIndex(UInt32 index, T* item);	// no bounds checking
	void SetCapacity(UInt16 newCapacity);	// grow and copy data if needed
};

#if RUNTIME

template <typename T> void NiTArray<T>::AddAtIndex(UInt32 index, T* item)
{
	ThisStdCall(0x00869640, this, index, item);
}

template <typename T> void NiTArray<T>::SetCapacity(UInt16 newCapacity)
{
	ThisStdCall(0x008696E0, this, newCapacity);
}

#endif

// 18
// an NiTArray that can go above 0xFFFF, probably with all the same weirdness
// this implies that they make fragmentable arrays with 0x10000 elements, wtf
template <typename T>
class NiTLargeArray
{
public:
	NiTLargeArray();
	~NiTLargeArray();

	void	** _vtbl;		// 00
	T		* data;			// 04
	UInt32	capacity;		// 08 - init'd to size of preallocation
	UInt32	firstFreeEntry;	// 0C - index of the first free entry in the block of free entries at the end of the array (or numObjs if full)
	UInt32	numObjs;		// 10 - init'd to 0
	UInt32	growSize;		// 14 - init'd to size of preallocation

	T operator[](UInt32 idx) {
		if (idx < firstFreeEntry)
			return data[idx];
		return NULL;
	}

	T Get(UInt32 idx) { return (*this)[idx]; }

	UInt32 Length(void) { return firstFreeEntry; }
};

// 8
template <typename T>
struct NiTSet
{
	T		* data;		// 00
	UInt16	capacity;	// 04
	UInt16	length;		// 06
};

// 10
// this is a NiTPointerMap <UInt32, T_Data>
// todo: generalize key
template <typename T_Data>
class NiTPointerMap
{
public:
	NiTPointerMap();
	virtual ~NiTPointerMap();

	struct Entry
	{
		Entry	* next;
		UInt32	key;
		T_Data	* data;
	};

	// note: traverses in non-numerical order
	class Iterator
	{
		friend NiTPointerMap;

	public:
		Iterator(NiTPointerMap * table, Entry * entry = NULL, UInt32 bucket = 0)
			:m_table(table), m_entry(entry), m_bucket(bucket) { FindValid(); }
		~Iterator() { }

		T_Data *	Get(void);
		UInt32		GetKey(void);
		bool		Next(void);
		bool		Done(void);

	private:
		void		FindValid(void);

		NiTPointerMap	* m_table;
		Entry		* m_entry;
		UInt32		m_bucket;
	};

	virtual UInt32	CalculateBucket(UInt32 key);
	virtual bool	CompareKey(UInt32 lhs, UInt32 rhs);
	virtual void	Fn_03(UInt32 arg0, UInt32 arg1, UInt32 arg2);	// assign to entry
	virtual void	Fn_04(UInt32 arg);
	virtual Entry*	AllocNewEntry(void);	// locked operations
	virtual void	FreeEntry(Entry*);	// locked operations

	T_Data *	Lookup(UInt32 key);
	bool		Insert(Entry* nuEntry);
	const Iterator Begin() const { return Iterator((NiTPointerMap<T_Data>*)this); }

	// use the vtable method for getting next bucket
	Entry* GetNextFreeEntryAlt(Entry* entry)
	{
		if (entry->next) return entry->next;
		for (UInt32 i = this->CalculateBucket(entry->key) + 1; i < m_numBuckets; ++i)
		{
			if (m_buckets[i])
			{
				return m_buckets[i];
			}
		}
		return nullptr;
	}

//	void	** _vtbl;		// 0
	UInt32	m_numBuckets;	// 4
	Entry	** m_buckets;	// 8
	UInt32	m_numItems;		// C
};

template <typename T_Data>
T_Data * NiTPointerMap <T_Data>::Lookup(UInt32 key)
{
	for(Entry * traverse = m_buckets[key % m_numBuckets]; traverse; traverse = traverse->next)
		if(traverse->key == key)
			return traverse->data;
	
	return NULL;
}

template <typename T_Data>
bool NiTPointerMap<T_Data>::Insert(Entry* nuEntry)
{
	// game code does not appear to care about ordering of entries in buckets
	UInt32 bucket = nuEntry->key % m_numBuckets;
	Entry* prev = NULL;
	for (Entry* cur = m_buckets[bucket]; cur; cur = cur->next) {
		if (cur->key == nuEntry->key) {
			return false;
		}
		else if (!cur->next) {
			prev = cur;
			break;
		}
	}

	if (prev) {
		prev->next = nuEntry;
	}
	else {
		m_buckets[bucket] = nuEntry;
	}

	m_numBuckets++;
	return true;
}

template <typename T_Data>
T_Data * NiTPointerMap <T_Data>::Iterator::Get(void)
{
	if(m_entry)
		return m_entry->data;

	return NULL;
}

template <typename T_Data>
UInt32 NiTPointerMap <T_Data>::Iterator::GetKey(void)
{
	if(m_entry)
		return m_entry->key;

	return 0;
}

template <typename T_Data>
bool NiTPointerMap <T_Data>::Iterator::Next(void)
{
	if(m_entry)
		m_entry = m_entry->next;

	while(!m_entry && (m_bucket < (m_table->m_numBuckets - 1)))
	{
		m_bucket++;

		m_entry = m_table->m_buckets[m_bucket];
	}

	return m_entry != NULL;
}

template <typename T_Data>
bool NiTPointerMap <T_Data>::Iterator::Done(void)
{
	return m_entry == NULL;
}

template <typename T_Data>
void NiTPointerMap <T_Data>::Iterator::FindValid(void)
{
	// validate bucket
	if(m_bucket >= m_table->m_numBuckets) return;

	// get bucket
	m_entry = m_table->m_buckets[m_bucket];

	// find non-empty bucket
	while(!m_entry && (m_bucket < (m_table->m_numBuckets - 1)))
	{
		m_bucket++;

		m_entry = m_table->m_buckets[m_bucket];
	}
}

// 10
// todo: NiTPointerMap should derive from this
// cleaning that up now could cause problems, so it will wait
template <typename T_Key, typename T_Data>
class NiTMapBase
{
public:
	NiTMapBase();
	~NiTMapBase();

	struct Entry
	{
		Entry	* next;	// 000
		T_Key	key;	// 004
		T_Data	data;	// 008
	};

	virtual NiTMapBase<T_Key, T_Data>*	Destructor(bool doFree);						// 000
	virtual UInt32						Hash(T_Key key);								// 001
	virtual void						Equal(T_Key key1, T_Key key2);					// 002
	virtual void						FillEntry(Entry entry, T_Key key, T_Data data);	// 003
	//virtual	void						Unk_004(void * arg0);							// 004
	virtual void						FreeEntry(Entry*);	// locked operations
	virtual	void						Unk_005(void);									// 005
	virtual	void						Unk_006();										// 006

	//void	** _vtbl;	// 0
	UInt32	numBuckets;	// 4
	Entry	** buckets;	// 8
	UInt32	numItems;	// C

	T_Data Lookup(UInt32 key) const
	{
		for (Entry* traverse = buckets[key % numBuckets]; traverse; traverse = traverse->next)
			if (traverse->key == key) return traverse->data;
		return NULL;
	}

	class Iterator
	{
		NiTMapBase* table;
		Entry** bucket;
		Entry* entry;

		void FindNonEmpty()
		{
			for (Entry** end = &table->buckets[table->numBuckets]; bucket != end; bucket++)
				if (entry = *bucket) break;
		}

	public:
		Iterator(NiTMapBase& _table) : table(&_table), bucket(table->buckets), entry(nullptr) { FindNonEmpty(); }

		explicit operator bool() const { return entry != nullptr; }
		void operator++()
		{
			entry = entry->next;
			if (!entry)
			{
				bucket++;
				FindNonEmpty();
			}
		}
		T_Data Get() const { return entry->data; }
		T_Key Key() const { return entry->key; }
	};

	Iterator Begin() { return Iterator(*this); }

	Entry* Get(T_Key key);

#if RUNTIME
	DEFINE_MEMBER_FN_LONG(NiTMapBase, Lookup, bool, _NiTMap_Lookup, T_Key key, T_Data * dataOut);
#endif
};

template <typename T_Key, typename T_Data>
struct MapNode
{
	MapNode* next;
	T_Key	key;
	T_Data* data;
};

template <typename T_Key, typename T_Data>
class NiTPointerMap_t
{
public:

	virtual ~NiTPointerMap_t();

	typedef MapNode<T_Key, T_Data> Entry;

	// note: traverses in non-numerical order
	class Iterator
	{
		friend NiTPointerMap_t;

	public:
		Iterator(NiTPointerMap_t* table, Entry* entry = nullptr, UInt32 bucket = 0)
			:m_table(table), m_entry(entry), m_bucket(bucket) {
			FindValid();
		}
		~Iterator() { }

		T_Data* Get(void);
		UInt32		GetKey(void);
		bool		Next(void);
		bool		Done(void);

		Iterator operator++()
		{
			Next();
			return *this;
		}

		Entry* operator*() const
		{
			return m_entry;
		}

		bool operator!=(const Iterator& other)
		{
			return m_entry != other.m_entry;
		}

	private:
		void		FindValid(void);

		NiTPointerMap_t* m_table;
		Entry* m_entry;
		UInt32		m_bucket;
	};

	virtual UInt32	CalculateBucket(UInt32 key);
	virtual bool	CompareKey(UInt32 lhs, UInt32 rhs);
	virtual void	Fn_03(UInt32 arg0, UInt32 arg1, UInt32 arg2);	// assign to entry
	virtual void	Fn_04(UInt32 arg);
	virtual void	Fn_05(void);	// locked operations
	virtual void	Fn_06(void);	// locked operations

	T_Data*	Lookup(T_Key key);
	bool		Insert(Entry* nuEntry);

	void Replace(T_Key key, T_Data* data)
	{
		for (Entry* traverse = m_buckets[key % m_numBuckets]; traverse; traverse = traverse->next)
			if (traverse->key == key)
			{
				traverse->data = data;
				break;
			}
	}

	[[nodiscard]] Iterator begin()
	{
		return Iterator(this);
	}

	[[nodiscard]] Iterator end()
	{
		return Iterator(this, static_cast<Entry*>(nullptr), m_numBuckets + 1);
	}

	//	void	** _vtbl;		// 0
	UInt32	m_numBuckets;	// 4
	Entry** m_buckets;	// 8
	UInt32	m_numItems;		// C
};

template <typename T_Key, typename T_Data>
T_Data* NiTPointerMap_t <T_Key, T_Data>::Lookup(T_Key key)
{
	const auto hashNiString = [](const char* str)
		{
			// 0x486DF0
			UInt32 hash = 0;
			while (*str)
			{
				hash = *str + 33 * hash;
				++str;
			}
			return hash;
		};
	UInt32 hashIndex;
	if constexpr (std::is_same_v<T_Key, const char*>)
	{
		hashIndex = hashNiString(key) % m_numBuckets;
	}
	else
	{
		hashIndex = key % m_numBuckets;
	}
	for (Entry* traverse = m_buckets[hashIndex]; traverse; traverse = traverse->next)
	{
		if constexpr (std::is_same_v<T_Key, const char*>)
		{
			if (!_stricmp(traverse->key, key))
			{
				return traverse->data;
			}
		}
		else if (traverse->key == key)
		{
			return traverse->data;
		}
	}
	return nullptr;
}

template <typename T_Key, typename T_Data>
bool NiTPointerMap_t <T_Key, T_Data>::Insert(Entry* nuEntry)
{
	// game code does not appear to care about ordering of entries in buckets
	UInt32 bucket = nuEntry->key % m_numBuckets;
	Entry* prev = NULL;
	for (Entry* cur = m_buckets[bucket]; cur; cur = cur->next) {
		if (cur->key == nuEntry->key) {
			return false;
		}
		else if (!cur->next) {
			prev = cur;
			break;
		}
	}

	if (prev) {
		prev->next = nuEntry;
	}
	else {
		m_buckets[bucket] = nuEntry;
	}

	m_numBuckets++;
	return true;
}

// 14
template <typename T_Data>
class NiTStringPointerMap : public NiTPointerMap_t <const char*, T_Data>
{
public:
	NiTStringPointerMap();
	~NiTStringPointerMap();

	UInt32	unk010;
};

// not sure how much of this is in NiTListBase and how much is in NiTPointerListBase
// 10
template <typename T>
class NiTListBase
{
public:
	NiTListBase();
	~NiTListBase();

	struct Node
	{
		Node	* next;
		Node	* prev;
		T		* data;
	};

	virtual void	Destructor(void);
	virtual Node *	AllocateNode(void);
	virtual void	FreeNode(Node * node);

//	void	** _vtbl;	// 000
	Node	* start;	// 004
	Node	* end;		// 008
	UInt32	numItems;	// 00C
};

// 10
template <typename T>
class NiTPointerListBase : public NiTListBase <T>
{
public:
	NiTPointerListBase();
	~NiTPointerListBase();
};

// 10
template <typename T>
class NiTPointerList : public NiTPointerListBase <T>
{
public:
	NiTPointerList();
	~NiTPointerList();
};

// 4
template <typename T>
class NiPointer
{
public:
	NiPointer(T *init) : data(init)		{	}

	T	* data;

	const T&	operator *() const { return *data; }
	T&			operator *() { return *data; }

	operator const T*() const { return data; }
	operator T*() { return data; }
};

namespace NiPointerNew
{
	//via CommonPrefix.hpp
	template <typename T_Ret = UInt32, typename ...Args>
	__forceinline T_Ret ThisCall(UInt32 _addr, void* _this, Args ...args)
	{
		class T {};
		union {
			UInt32  addr;
			T_Ret(T::* func)(Args...);
		} u = { _addr };
		return ((T*)_this->*u.func)(std::forward<Args>(args)...);
	}

	template <typename T_Ret = UInt32, typename ...Args>
	__forceinline T_Ret ThisStdCall(UInt32 _addr, const void* _this, Args ...args)
	{
		return ((T_Ret(__thiscall*)(const void*, Args...))_addr)(_this, std::forward<Args>(args)...);
	}

	template <typename T_Ret = void, typename ...Args>
	__forceinline T_Ret StdCall(UInt32 _addr, Args ...args)
	{
		return ((T_Ret(__stdcall*)(Args...))_addr)(std::forward<Args>(args)...);
	}

	template <typename T_Ret = void, typename ...Args>
	__forceinline T_Ret CdeclCall(UInt32 _addr, Args ...args)
	{
		return ((T_Ret(__cdecl*)(Args...))_addr)(std::forward<Args>(args)...);
	}
#define ASSERT_SIZE(a, b) static_assert(sizeof(a) == b, "Wrong structure size!");
#define ASSERT_OFFSET(a, b, c) static_assert(offsetof(a, b) == c, "Wrong member offset!");
#define CREATE_OBJECT(CLASS, ADDRESS) static CLASS* CreateObject() { return StdCall<CLASS*>(ADDRESS); };

	template <class T_Data>
	class NiPointer {
	public:
		NiPointer(T_Data* pObject = (T_Data*)0);
		NiPointer(const NiPointer& ptr);
		~NiPointer();

		T_Data* m_pObject;

		operator T_Data* () const;
		T_Data& operator*() const;
		T_Data* operator->() const;

		__forceinline NiPointer<T_Data>& operator =(const NiPointer& ptr) {
			if (m_pObject != ptr.m_pObject) {
				if (m_pObject)
					m_pObject->DecRefCount();
				m_pObject = ptr.m_pObject;
				if (m_pObject)
					m_pObject->IncRefCount();
			}
			return *this;
		}

		__forceinline NiPointer<T_Data>& operator =(T_Data* pObject) {
			if (m_pObject != pObject) {
				if (m_pObject)
					m_pObject->DecRefCount();
				m_pObject = pObject;
				if (m_pObject)
					m_pObject->IncRefCount();
			}
			return *this;
		}

		__forceinline bool operator==(T_Data* pObject) const { return (m_pObject == pObject); }

		//__forceinline bool operator!=(T_Data* pObject) const { return (m_pObject != pObject); }

		__forceinline bool operator==(const NiPointer& ptr) const { return (m_pObject == ptr.m_pObject); }

		//__forceinline bool operator!=(const NiPointer& ptr) const { return (m_pObject != ptr.m_pObject); }

		__forceinline operator bool() const { return m_pObject != nullptr; }
	};

#define NiSmartPointer(className) \
	class className; \
	typedef NiPointer<className> className##Ptr;


	template <class T_Data>
	inline NiPointer<T_Data>::NiPointer(T_Data* pObject) {
		m_pObject = pObject;
		if (m_pObject)
			m_pObject->IncRefCount();
	}

	template <class T_Data>
	inline NiPointer<T_Data>::NiPointer(const NiPointer& ptr) {
		m_pObject = ptr.m_pObject;
		if (m_pObject)
			m_pObject->IncRefCount();
	}

	template <class T_Data>
	inline NiPointer<T_Data>::~NiPointer() {
		if (m_pObject)
			m_pObject->DecRefCount();
	}

	template <class T_Data>
	inline NiPointer<T_Data>::operator T_Data* () const {
		return m_pObject;
	}

	template <class T_Data>
	inline T_Data& NiPointer<T_Data>::operator*() const {
		return *m_pObject;
	}

	template <class T_Data>
	inline T_Data* NiPointer<T_Data>::operator->() const {
		return m_pObject;
	}
}
// 14
template <typename T>
class BSTPersistentList
{
public:
	BSTPersistentList();
	~BSTPersistentList();

	virtual void	Destroy(bool destroy);

//	void	** _vtbl;	// 00
	UInt32	unk04;		// 04
	UInt32	unk08;		// 08
	UInt32	unk0C;		// 0C
	UInt32	unk10;		// 10
};