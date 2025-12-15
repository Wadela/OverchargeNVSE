#pragma once

#include "NiPoint3.hpp"
#include "NiPoint4.hpp"

class hkTransform;
class hkQsTransform;
class hkQuaternion;
class NiPoint4;

ALIGN16 class hkVector4 {
public:
	hkVector4() : m_quad(_mm_set1_ps(0.0f)) {}

	hkVector4(const float& a_x) : m_quad(_mm_set1_ps(a_x)) {}

	hkVector4(const float& a_x, const float& a_y, const float& a_z, const float& a_w) :
		m_quad(_mm_setr_ps(a_x, a_y, a_z, a_w))
	{}

	hkVector4(const hkVector4& a_rhs) : m_quad(a_rhs.m_quad) {}

	hkVector4(const NiPoint3& a_point) :
		m_quad(_mm_setr_ps(a_point.x, a_point.y, a_point.z, 0.0f))
	{}

	inline void operator= (const hkVector4& arOther) {
		m_quad = arOther.m_quad;
	}

	operator __m128() const {
		return m_quad;
	}

	operator __m128& () {
		return m_quad;
	}

	inline hkVector4 operator-(const hkVector4& rhs) const {
		return hkVector4(x() - rhs.x(), y() - rhs.y(), z() - rhs.z(), w() - rhs.w());
	}

	inline hkVector4& operator-=(const hkVector4& rhs) {
		m_quad = _mm_setr_ps(x() - rhs.x(), y() - rhs.y(), z() - rhs.z(), w() - rhs.w());
		return *this;
	}

	inline hkVector4 operator*(float s) const {
		return hkVector4(x() * s, y() * s, z() * s, w() * s);
	}

	inline hkVector4& operator*=(float s) {
		m_quad = _mm_setr_ps(x() * s, y() * s, z() * s, w() * s);
		return *this;
	}

	inline void Set(float x, float y, float z, float w) {
		m_quad = _mm_setr_ps(x, y, z, w);
	}

	inline void Set(float v) {
		m_quad = _mm_set1_ps(v);
	}

	inline void Set(const NiPoint4& p) {
		m_quad = _mm_setr_ps(p.x, p.y, p.z, p.w);
	}

	inline NiPoint4 GetNiPoint4() const {
		alignas(16) float vals[4];
		_mm_store_ps(vals, m_quad);
		return NiPoint4{ vals[0], vals[1], vals[2], vals[3] };
	}

	inline void Negate() {
		m_quad = _mm_xor_ps(m_quad, _mm_set1_ps(-0.0f));
	}

	inline float x() const { return _mm_cvtss_f32(m_quad); }
	inline float y() const { return _mm_cvtss_f32(_mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(1, 1, 1, 1))); }
	inline float z() const { return _mm_cvtss_f32(_mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(2, 2, 2, 2))); }
	inline float w() const { return _mm_cvtss_f32(_mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(3, 3, 3, 3))); }

	inline void setX(float v) { m_quad = _mm_move_ss(m_quad, _mm_set_ss(v)); }

	inline void setY(float v) {
		alignas(16) float f[4];
		_mm_store_ps(f, m_quad);
		f[1] = v;
		m_quad = _mm_load_ps(f);
	}

	inline void Reflect(const hkVector4& normal)
	{
		float dot = x() * normal.x() + y() * normal.y() + z() * normal.z();
		float nx = normal.x() * 2.0f * dot;
		float ny = normal.y() * 2.0f * dot;
		float nz = normal.z() * 2.0f * dot;

		m_quad = _mm_setr_ps(x() - nx, y() - ny, z() - nz, w());
	}

	__m128 m_quad;
};
ASSERT_SIZE(hkVector4, 0x10);