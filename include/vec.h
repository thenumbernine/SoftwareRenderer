#pragma once

#include <math.h>
#include <ostream>

#include <Common/Meta.h>
#include <Common/crtp_cast.h>

//vector math

//// constants

#if !defined(M_PI)
#define M_PI            3.14159265358979323846
#endif

#if !defined(M_SQRT_2)
#define M_SQRT_2		1.4142135623730951454746218587388284504410
#endif

#if !defined(M_SQRT_3)
#define M_SQRT_3		1.732050807568877193176604123436845839024
#endif

#if !defined(DEGREE_TO_RADIAN)
#define DEGREE_TO_RADIAN	0.017453292519943295474371680597869271878
#endif

#if !defined(RADIAN_TO_DEGREE)
#define RADIAN_TO_DEGREE	(1.0/DEGREE_TO_RADIAN)
#endif

#if !defined(NULL)
#define NULL 0
#endif


template<typename T>
struct assign {
	template<int i>
	struct vec_assign {
		static bool exec(T& v, const T& o) {
			v.template get<i>() = o.template get<i>();
			return false;
		}
	};
	
	inline T& operator=(const T& o) {
		Common::ForLoop<0, T::dim, vec_assign>::exec(crtp_cast<T>(*this), o);
	}
};

//addition

template<typename T>
struct add_op {
	template<int i>
	struct vec_add {
		static bool exec(T& c, const T& a, const T& b) {
			c.template get<i>() = a.template get<i>() + b.template get<i>();
			return false;
		}
	};
	
	inline T operator+(const T& b) const {
		T t;
		Common::ForLoop<0, T::dim, vec_add>::exec(t, crtp_cast<T>(*this), b);
		return t;
	}

	inline T& operator+=(const T& b) {
		return (crtp_cast<T>(*this) = crtp_cast<T>(*this) + b);
	}
};

//subtraction

template<typename T>
struct sub_op {
	template<int i>
	struct vec_sub {
		static bool exec(T& c, const T& a, const T& b) {
			c.template get<i>() = a.template get<i>() - b.template get<i>();
			return false;
		}
	};
	
	inline T operator-(const T& b) const {
		T t;
		Common::ForLoop<0, T::dim, vec_sub>::exec(t, crtp_cast<T>(*this), b);
		return t;
	}

	inline T& operator-=(const T& b) {
		return (crtp_cast<T>(*this) = crtp_cast<T>(*this) - b);
	}
};

//multiplication

template<typename T, typename S>
struct mul_op {
	template<int i>
	struct vec_mul {
		static bool exec(T& c, const T& a, const S& b) {
			c.template get<i>() = a.template get<i>() * b;
			return false;
		}
	};
	
	inline T operator*(const S& b) const {
		T t;
		Common::ForLoop<0, T::dim, vec_mul>::exec(t, crtp_cast<T>(*this), b);
		return t;
	}

	inline T& operator*=(const S& b) {
		return (crtp_cast<T>(*this) = crtp_cast<T>(*this) * b);
	}
};

//division

template<typename T, typename S>
struct div_op {
	template<int i>
	struct vec_div {
		static bool exec(T& c, const T& a, const S& b) {
			c.template get<i>() = a.template get<i>() / b;
			return false;
		}
	};
	
	inline T operator/(const S& b) const {
		T t;
		Common::ForLoop<0, T::dim, vec_div>::exec(t, crtp_cast<T>(*this), b);
		return t;
	}

	inline T& operator/=(const S& b) {
		return (crtp_cast<T>(*this) = crtp_cast<T>(*this) / b);
	}
};

// ==

template<typename T>
struct eq_op {
	template<int i>
	struct vec_eq {
		static bool exec(bool& result, const T& a, const T& b) {
			result = result && (a.template get<i>() == b.template get<i>());
			return !result;
		}
	};
	
	inline bool operator==(const T& v) const {
		bool result = true;
		Common::ForLoop<0, T::dim, vec_eq>::exec(result, crtp_cast<T>(*this), v);
		return result;
	}
	
	inline bool operator!=(const T& v) const {
		return !operator==(v);
	}
};

//ostream

template<typename T>
struct ostream_op {
	template<int i>
	struct vec_ostream {
		static bool exec(std::ostream& o, const T& v) {
			if (i != 0) o << ", ";
			o << v.template get<i>();
			return false;
		}
	};

	void output(std::ostream& o) {
		o << "{";
		Common::ForLoop<0, T::dim, vec_ostream>::exec(o, crtp_cast<T>(*this));
		o << "}";
	}
};

//// 2D vectors

#define VEC2ELEM(i)	(i).x,(i).y
	

template<typename T>
struct vec2 : public 
	assign<vec2<T>>,
	add_op<vec2<T>>,
	sub_op<vec2<T>>,
	mul_op<vec2<T>,T>,
	div_op<vec2<T>,T>,
	eq_op<vec2<T>>,
	ostream_op<vec2<T>>
{
	using type = T;
	
	enum { dim = 2 };
	template<int i> inline T& get();
	template<int i> inline const T& get() const;

	T x, y;

	vec2() : x(0), y(0) {}
	vec2(T x_, T y_) : x(x_), y(y_) {}

	T *fp() { return &x; }
	const T *fp() const { return &x; }
};

using vec2f = vec2<float>;

//hmm... how to make this flexible
template<> template<> inline float& vec2<float>::get<0>() { return x; };
template<> template<> inline float& vec2<float>::get<1>() { return y; };

template<> template<> inline const float& vec2<float>::get<0>() const { return x; };
template<> template<> inline const float& vec2<float>::get<1>() const { return y; };


//ostream

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const vec2<T>& v) {
	v.output(o);
	return o;
}

//// 3D vectors

#define VEC3ELEM(i)	(i).x,(i).y,(i).z

template<typename T>
struct vec3 : public
	assign<vec3<T>>,
	add_op<vec3<T>>,
	sub_op<vec3<T>>,
	mul_op<vec3<T>,T>,
	div_op<vec3<T>,T>,
	eq_op<vec3<T>>,
	ostream_op<vec3<T>>
{
	using type = T;

	enum { dim = 3 };
	template<int i> inline T& get();
	template<int i> inline const T& get() const;

	T x, y, z;

	vec3() : x(0), y(0), z(0) {}
	vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

	float *fp() { return &x; }
	const float *fp() const { return &x; }
};

using vec3f = vec3<float>;

//hmm... how to make this flexible
template<> template<> inline float& vec3<float>::get<0>() { return x; };
template<> template<> inline float& vec3<float>::get<1>() { return y; };
template<> template<> inline float& vec3<float>::get<2>() { return z; };

template<> template<> inline const float& vec3<float>::get<0>() const { return x; };
template<> template<> inline const float& vec3<float>::get<1>() const { return y; };
template<> template<> inline const float& vec3<float>::get<2>() const { return z; };

//unary

#if 1
template<typename T>
inline T operator-(const vec3<T>& v) {
	return vec3<T>(-v.x, -v.y, -v.z);
}
#else
template<typename T>
struct un_op {
	template<int i>
	struct vec_un {
		static bool exec(T& c, const T& a) {
			c.template get<i>() = -a.template get<i>();
			return false;
		}
	};
};

template<typename T>
inline T operator-(const vec3<T>& v) {
	T t;
	Common::ForLoop<0, T::dim, un_op<vec3<T>>::vec_un>::exec(t, v);
	return t;
}
#endif

//linear interpolation

inline vec3f vec3lerp(const vec3f &src, const vec3f &dst, float coeff) {
	float one_minus = 1.f - coeff;
	return vec3f(
		src.x * one_minus + dst.x * coeff,
		src.y * one_minus + dst.y * coeff,
		src.z * one_minus + dst.z * coeff);
}

//tensor operations

	//dot product
template<typename T>
inline T dot(const vec3<T> &a, const vec3<T> &b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

	//cross product
inline vec3f cross(const vec3f &a, const vec3f &b) {
	return vec3f(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const vec3<T>& v) {
	v.output(o);
	return o;
}

//magnitude

	//L-2

inline float vecLengthSq(const vec3f &v) {
	return dot(v,v);
}

inline float vecLength(const vec3f &v) {
	return (float)sqrt(dot(v,v));
}

	//L-Infinite

inline float vecLengthLInf(const vec3f &v) {
	//start with |x|
	float a = v.x < 0 ? -v.x : v.x;
	//compare to |y|
	float b = v.y < 0 ? -v.y : v.y;
	if (a < b) a = b;
	//compare to |z|
	b = v.z < 0 ? -v.z : v.z;
	if (a < b) a = b;
	//return the greatest of all three
	return a;
}

//normalization

inline vec3f vecUnit(const vec3f &v) {
	float len = vecLength(v);
	if (!len) {
		return v;
	} else {
		return v / len;
	}
}

// useful functions

inline vec3f vecPlaneNormal(const vec3f &a, const vec3f &b, const vec3f &c) {
	return cross(b - a, c - b);
}

inline vec3f vecUnitNormal(const vec3f &a, const vec3f &b, const vec3f &c) {
	return vecUnit(cross(b - a, c - b));
}

typedef int32_t fixed_t;

#define FIXED_FRACTION_BITS		16
#define FIXED_FRACTION_MASK		((1 << FIXED_FRACTION_BITS) - 1)
#define FLOAT_TO_FIXED(x)		((int32_t)( (x) * (1 << FIXED_FRACTION_BITS) ))
#define FIXED_TO_INT(x)			((int32_t)(x) >> FIXED_FRACTION_BITS)
#define FIXED_TO_FLOAT(x)		((float)(x) / (float)(1 << FIXED_FRACTION_BITS))

/** 
 * This is the class used to represent 3D vectors in fixed-point-precision
 * 32 bits, 16 whole number and 16 fractional
 */
struct vec3fixed : public
	assign<vec3fixed>,
	add_op<vec3fixed>,
	sub_op<vec3fixed>
{
	using type = fixed_t;

	enum { dim = 3 };
	template<int i> inline type& get();
	template<int i> inline const type& get() const;

	type x, y, z;
	
	vec3fixed() : x(0), y(0), z(0) {}
	vec3fixed(type x_, type y_, type z_) : x(x_), y(y_), z(z_) {}

	type *ip() { return &x; }
	const type *ip() const { return &x; }
};

//hmm... how to make this flexible
template<> inline fixed_t& vec3fixed::get<0>() { return x; };
template<> inline fixed_t& vec3fixed::get<1>() { return y; };
template<> inline fixed_t& vec3fixed::get<2>() { return z; };

template<> inline const fixed_t& vec3fixed::get<0>() const { return x; };
template<> inline const fixed_t& vec3fixed::get<1>() const { return y; };
template<> inline const fixed_t& vec3fixed::get<2>() const { return z; };


//scaling

//64-bit true multiplication (no bits left behind!)
inline fixed_t fixedMul(fixed_t x, fixed_t y) {
//	return (x >> (FIXED_FRACTION_BITS >> 1)) * (y >> (FIXED_FRACTION_BITS >> 1));
	return (fixed_t)(((int64_t)x * (int64_t)y) >> FIXED_FRACTION_BITS);
}

//32-bit mul.  proly the same effect come to think of it.
inline fixed_t fixedMulLo(fixed_t x, fixed_t y) {
	return (x >> (FIXED_FRACTION_BITS >> 1)) * (y >> (FIXED_FRACTION_BITS >> 1));
//	return (fixed_t)(((int64_t)x * (int64_t)y) >> FIXED_FRACTION_BITS);
}

inline fixed_t fixedDiv(fixed_t x, fixed_t y) {
	//return (x / y) << FIXED_FRACTION_BITS;
	//todo - for less lost bits:
//	return (x << (FIXED_FRACTION_BITS >> 1)) / (y >> (FIXED_FRACTION_BITS >> 1));
	return (fixed_t)(
		(
		((int64_t)x << 32)
		/
		((int64_t)y)
		) >> (32 - FIXED_FRACTION_BITS)
		);
}

inline vec3fixed fixedMul(vec3fixed a, fixed_t b) {
	return vec3fixed(
		fixedMul(a.x, b),
		fixedMul(a.y, b),
		fixedMul(a.z, b));
}

inline vec3fixed operator*(const vec3fixed &a, float b) {
	return fixedMul(a, (fixed_t)(b * FIXED_FRACTION_BITS));
}


inline std::ostream& operator<<(std::ostream& o, const vec3fixed& v) {
	return o << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}


//// quaternions

#define VEC4ELEM(i)	(i).x,(i).y,(i).z,(i).w

/**
 * This class holds a quaternion, in <x,y,z,w> format.
 */
template<typename T>
struct vec4 : public 
	assign<vec4<T>>,
	add_op<vec4<T>>,
	sub_op<vec4<T>>,
	mul_op<vec4<T>,T>,
	div_op<vec4<T>,T>,
	eq_op<vec4<T>>,
	ostream_op<vec4<T>>
{
	using type = T;

	enum { dim = 4 };
	template<int i> inline T& get();
	template<int i> inline const T& get() const;
	
	T x,y,z,w;

	vec4() : x(0), y(0), z(0), w(1) {}
	vec4(const vec3<T> &v, T w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
	vec4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}

	T *fp() { return &x; }
	const T *fp() const { return &x; }

	vec3<T> *vp() { return (vec3<T> *)&x; }
	const vec3<T> *vp() const { return (vec3<T> *)&x; }
};

using vec4f = vec4<float>;

//hmm... how to make this flexible
template<> template<> inline float& vec4<float>::get<0>() { return x; };
template<> template<> inline float& vec4<float>::get<1>() { return y; };
template<> template<> inline float& vec4<float>::get<2>() { return z; };
template<> template<> inline float& vec4<float>::get<3>() { return w; };

template<> template<> inline const float& vec4<float>::get<0>() const { return x; };
template<> template<> inline const float& vec4<float>::get<1>() const { return y; };
template<> template<> inline const float& vec4<float>::get<2>() const { return z; };
template<> template<> inline const float& vec4<float>::get<3>() const { return w; };


/** 
 * This quaternion represents the identity quaternion.
 * TODO - put this in one spot and 'extern' it - to save that tiny bit of memory
 */
const static vec4f quat4fIdentity(0,0,0,1);

//multiplication

template<typename T>
inline vec4<T> operator*(const vec4<T> &q, const vec4<T> &r) {
	T a,b,c,d,e,f,g,h;

	a = (q.w + q.x) * (r.w + r.x);
	b = (q.z - q.y) * (r.y - r.z);
	c = (q.x - q.w) * (r.y + r.z);
	d = (q.y + q.z) * (r.x - r.w);
	e = (q.x + q.z) * (r.x + r.y);
	f = (q.x - q.z) * (r.x - r.y);
	g = (q.w + q.y) * (r.w - r.z);
	h = (q.w - q.y) * (r.w + r.z);

	return vec4<T>(	a - .5 * ( e + f + g + h),
					-c + .5 * ( e - f + g - h),
					-d + .5 * ( e - f - g + h),
					b + .5 * (-e - f + g + h));
}

//conjugation

inline vec4f quatConj(const vec4f &q) {
	return vec4f(q.x, q.y, q.z, -q.w);
}

//dot product

inline float dot(const vec4f &a, const vec4f &b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

//magnitude

inline float quatLength(const vec4f &q) {
	return (float)sqrt(dot(q, q));
}

//normalization

inline vec4f quatUnit(const vec4f &q) {
	return q / quatLength(q);
}

inline vec4f vec4lerp(const vec4f &src, const vec4f &dst, float coeff) {
	float one_minus = 1.f - coeff;
	return vec4f(
		src.x * one_minus + dst.x * coeff,
		src.y * one_minus + dst.y * coeff,
		src.z * one_minus + dst.z * coeff,
		src.w * one_minus + dst.w * coeff);
}


/**
 * This method converts an object from a quaternion to angle-axis form.
 * The resultant quaternion 'w' component holds the quaternion angle, in degrees.
 */
inline vec4f quatToAngleAxis(const vec4f &q) {
	float cosom = q.w;
	if (cosom < -1.f) cosom = -1.f;
	else if (cosom > 1.f) cosom = 1.f;

	float halfangle = (float)acos(cosom);
	float scale = (float)sin(halfangle);

	//TODO - check epsilon
	if (!scale) {
		return vec4f(0,0,1,0);
	}

	scale = (float)1.f / scale;

	return vec4f(q.x * scale, q.y * scale, q.z * scale, halfangle * 360.f / (float)M_PI);
}

/**
 * This method converts an object from angle-axis form to a quaternion.
 * It assumes the 'w' component of the incoming quaternion is the angle, measured in degrees.
 * It also assumes the <x,y,z> components of the incoming quaternion form a unit vector.
 */
inline vec4f angleAxisToQuat(const vec4f &q) {
	float halfangle = q.w * (float)M_PI / 360.f;
	float scale = (float)sin(halfangle);

	return vec4f(
		q.x * scale,
		q.y * scale,
		q.z * scale,
		(float)cos(halfangle));
}

/**
 * Returns the value of exp(v) = exp(ix + jy + kz) using quaternion mathematics.
 * Equivalent to 'angleAxisToQuat(quat4fNew(v.x, v.y, v.z, vecLength(v) / M_PI * 360.f))'
 * for some vec3f 'v'.
 */
inline vec4f quatExp(const vec3f &a) {
	float length = vecLength(a);
	if (!length) {
		return quat4fIdentity;
	}

	//compensate for numerical inaccuracies at sin(x)/x for x->0
	float sinth = 1.f;
	if (length > 0.00001f || length < -0.00001f) sinth = (float)sin(length) / length;
	
	return vec4f(a * sinth, (float)cos(length));
}

/**
 * Rotates vector 'v' by quaternion 'q' and returns the rotated vector.
 * This is equivalent to the operation 'q * quat4fNew(v.x,v.y,v.z,0) * quatConj(q)'
 * This is mathematically correct only when 'q' is a unit quaternion.
 */
inline vec3f quatRotate(const vec4f &q, const vec3f &v) {
	float w2x2 = (q.w + q.x) * (q.w - q.x);
	float w2y2 = (q.w + q.y) * (q.w - q.y);
	float x2z2 = (q.x + q.z) * (q.x - q.z);
	float y2z2 = (q.y + q.z) * (q.y - q.z);

	return vec3f(
		v.x * (w2y2 + x2z2) + 2.f*(v.y * (q.x * q.y - q.w * q.z) + v.z * (q.x * q.z + q.w * q.y)),
		v.y * (w2x2 + y2z2) + 2.f*(v.z * (q.y * q.z - q.w * q.x) + v.x * (q.x * q.y + q.w * q.z)),
		v.z * (w2y2 - x2z2) + 2.f*(v.x * (q.x * q.z - q.w * q.y) + v.y * (q.y * q.z + q.w * q.x)));
}

/**
 * Returns the X-axis of the basis formed by converting 'q' from a quaternion to a matrix
 * and returning the first column vector components.
 */
inline vec3f quatXAxis(const vec4f &q) {
	return vec3f(
		1.f - 2.f * (q.y * q.y + q.z * q.z),
		2.f * (q.x * q.y + q.z * q.w),
		2.f * (q.x * q.z - q.w * q.y));
}

/**
 * Returns the Y-axis of the basis formed by converting 'q' from a quaternion to a matrix
 * and returning the first column vector components.
 */
inline vec3f quatYAxis(const vec4f &q) {
	return vec3f(
		2.f * (q.x * q.y - q.w * q.z),
		1.f - 2.f * (q.x * q.x + q.z * q.z),
		2.f * (q.y * q.z + q.w * q.x));
}

/**
 * Returns the Y-axis of the basis formed by converting 'q' from a quaternion to a matrix
 * and returning the first column vector components.
 */
inline vec3f quatZAxis(const vec4f &q) {
	return vec3f(
		2.f * (q.x * q.z + q.w * q.y),
		2.f * (q.y * q.z - q.w * q.x),
		1.f - 2.f * (q.x * q.x + q.y * q.y));
}

inline std::ostream& operator<<(std::ostream& o, const vec4f& v) {
	return o << "{" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "}";
}

//// 4x4 matrices

/**
 * This class is used for our 4x4 matrices.
 * Maybe I'll make a 3x3 class later...
 * Maybe even a 3x4 for humor's sake
 */
class mat44f {
public:

	/*
	 * matrix components
	 * stored in column-major format (OpenGL-style)
	 * which means the order goes:
	 *   m[0][0] m[1][0] ...
	 *   m[0][1] m[1][1] ...
	 *    ...      ...   ...
	 * therefore we have fast access to the axis vectors
	 */
//	float m[4][4];
	vec4f x,y,z,w;

	/**
	 * constructor: identity matrix
	 */
	mat44f() 
		: x(1,0,0,0)
		, y(0,1,0,0)
		, z(0,0,1,0)
		, w(0,0,0,1)
	{}

	/**
	 * constructor: real analogy. scaled identity.
	 */
	mat44f(float r) 
		: x(r,0,0,0)
		, y(0,r,0,0)
		, z(0,0,r,0)
		, w(0,0,0,r)
	{}

	/**
	 * parameters are all in column-major order,
	 * so they'll look transposed when you write them out
	 * but you'll be able to expand axis vectors into the parameters more quickly
	 */
	mat44f(	float xx, float xy, float xz, float xw,
			float yx, float yy, float yz, float yw,
			float zx, float zy, float zz, float zw,
			float wx, float wy, float wz, float ww)
		: x(xx, xy, xz, xw)
		, y(yx, yy, yz, yw)
		, z(zx, zy, zz, zw)
		, w(wx, wy, wz, ww)
	{}

	vec3f transformVector(const vec3f &a) const {		//with vectors, assume w component is 0
		return vec3f(
			x.x * a.x + y.x * a.y + z.x * a.z,
			x.y * a.x + y.y * a.y + z.y * a.z,
			x.z * a.x + y.z * a.y + z.z * a.z);
	}

	float *fp() {
		return &x.x;
	}

	const float *fp() const {
		return &x.x;
	}

	mat44f transpose() {
		return mat44f(
			x.x, y.x, z.x, w.x,
			x.y, y.y, z.y, w.y,
			x.z, y.z, z.z, w.z,
			x.w, y.w, z.w, w.w);
	}

};

inline vec4f operator*(const mat44f &a, const vec4f &b) {
	return vec4f(
		a.x.x * b.x + a.y.x * b.y + a.z.x * b.z + a.w.x * b.w,
		a.x.y * b.x + a.y.y * b.y + a.z.y * b.z + a.w.y * b.w,
		a.x.z * b.x + a.y.z * b.y + a.z.z * b.z + a.w.z * b.w,
		a.x.w * b.x + a.y.w * b.y + a.z.w * b.z + a.w.w * b.w);
}

inline mat44f operator*(const mat44f &a, const mat44f &b) {
	mat44f ret;
	const float *fa = (const float *)(&a.x.x);
	const float *fb = (const float *)(&b.x.x);
	float *fr = (float *)(&ret.x.x);
	for (int i = 0; i < 4; i++) {			//dest column
		for (int j = 0; j < 4; j++) {		//dest row
			float s = 0;
			for (int k = 0; k < 4; k++) {
				float a0 = fa[ j + (k << 2) ];
				float a1 = fb[ k + (i << 2) ];
				s += a0 * a1;
			}
			fr[ j + (i << 2) ] = s;
		}
	}
	return ret;
}

inline mat44f operator*=(mat44f &a, const mat44f &b) {
	return (a = a * b);
}

inline bool operator==(const mat44f &a, const mat44f &b) {
	return a.x.x == b.x.x
		&& a.x.y == b.x.y
		&& a.x.z == b.x.z
		&& a.x.w == b.x.w

		&& a.y.x == b.y.x
		&& a.y.y == b.y.y
		&& a.y.z == b.y.z
		&& a.y.w == b.y.w

		&& a.z.x == b.z.x
		&& a.z.y == b.z.y
		&& a.z.z == b.z.z
		&& a.z.w == b.z.w

		&& a.w.x == b.w.x
		&& a.w.y == b.w.y
		&& a.w.z == b.w.z
		&& a.w.w == b.w.w;
}

//making MSVC5 able to use this with mat44f
inline bool operator<(const mat44f &a, const mat44f &b) {
	return a.x.x < b.x.x;
}

inline std::ostream& operator<<(std::ostream& o, const mat44f& m) {
	return o
		<< "{" << m.x
		<< ", " << m.y
		<< ", " << m.z
		<< ", " << m.w
		<< "}";
}

//// orthonormal basis w/translation

/**
 * this simulates a 4x4 matrix with the following layout:
 *
 * [. . .|.]
 * [. R .|T]
 * [. . .|.]
 * ------+--
 * [0 0 0|1]
 *
 * for R a R^3x3 orthonormal rotation matrix represented as a quaternion
 * and T a R^3 vector
 */
class basis_t {
public:
	vec4f r;
	vec3f t;

	/**
	 * constructs a default identity quaternion and zero vector
	 */
	basis_t() {}

	/**
	 * constructs a basis with the specified orientation and translation
	 */
	basis_t(const vec4f &r_, const vec3f &t_) : r(r_), t(t_) {}

	/**
	* returns a * b
	*
	* [Ra Ta] [Rb Tb]   [Ra*Rb  Ra*Tb + Ta]
	* [0  1 ]*[0  1 ] = [  0         1    ]
	*/
	basis_t operator*(const basis_t &b) const {
		return basis_t(r * b.r, quatRotate(r, b.t) + t);
	}

	basis_t operator*=(const basis_t &b) {
		return *this = *this * b;
	}
};

//orthogonal basis, uniform scale, with transform components
class basisu_t {
public:
	vec4f r;
	float s;
	vec3f t;

	/**
	 * constructs a default identity quaternion, zero vector, and unit uniform scale
	 */
	basisu_t() : s(1) {}

	/**
	 * constructs a basis with the specified orientation scale and translation
	 */
	basisu_t(const vec4f &r_, float s_, const vec3f &t_) : r(r_), s(s_), t(t_) {}

	/**
	* returns a * b
	*
	* [Ra*Sa Ta] [Rb*Sb Tb]   [Ra*Sa*Rb*Sb	Ra*Sa*Tb+Ta ]	[(Ra*Rb)*(Sa*Sb) Ra*Sa*Tb+Ta]
	* [0     1 ]*[0     1 ] = [0			1			] = [0				 1			]
	*/
	basisu_t operator*(const basisu_t &b) const {
		return basisu_t(r * b.r, s * b.s, quatRotate(r, b.t) * s + t);
	}

	basisu_t operator*=(const basisu_t &b) {
		return *this = *this * b;
	}
};

//orthogonal basis, non-uniform scale, with transform components
class basisn_t {
public:
	vec4f r;
	vec3f s;
	vec3f t;

	/**
	 * constructs a default identity quaternion, zero vector, and unit scale vector
	 */
	basisn_t() : s(1,1,1) {}

	/**
	 * constructs a basis with the specified orientation scale and translation
	 */
	basisn_t(const vec4f &r_, const vec3f &s_, const vec3f &t_) : r(r_), s(s_), t(t_) {}

	/**
	* returns a * b
	*
	* [Ra*Sa Ta] [Rb*Sb Tb]   [Ra*Sa*Rb*Sb	Ra*Sa*Tb+Ta ]
	* [0     1 ]*[0     1 ] = [0			1			] =
	*
	*				[sx 0  0 ][tx]		[sx*tx]
	* Ra*Sa*Ta = Ra*[0  sy 0 ][ty] = Ra*[sy*ty]
	*				[0  0  sz][tz]		[sz*tz]
	*/

	//conceptually..
	//if you scale a space on its world axis
	//and then rotate it
	//is that the same as rotating it
	//then scaling it on the rotated axis?

	//Ra*Sa = (Ra*Sa*Ra')*Ra
	//Sa*Ra = Ra*(Ra'*Sa*Ra)

	//Ra*(Sa*Rb)*Sb = Ra*(Rb*(Rb'*Sa*Rb))*Sb
	//= (Ra*Rb) * (Rb'*Sa*Rb)*Sb
	//but Sa is no longer a scale matrix... hmm... how does it perturb the resultant rotation matrix?
	//now i know why SVD turns stuff into U*Sigma*V'
};


//// planes

/**
 * typical plane equations are of the form Ax+By+Cz+D = 0
 * where D represents the negative distance from the origin divided by |A,B,C|
 * but - to shortcut calculations, we will store -D as 'dist'
 */
class plane_t : public vec4f {
public:

	/**
	 * constructs an empty plane with normal <0,0,0> and distance from origin 0
	 */
	plane_t() : vec4f(0,0,0,0) {}

	/**
	 * constructs a plane from the specified normal at zero distance
	 */
	plane_t(const vec3f &normal) : vec4f(normal, 0) {}

	/**
	 * constructs a plane from the specified normal at the specified distance
	 */
	plane_t(const vec3f &normal, float dist) {
		*vp() = normal;
		w = -dist;
	}

	vec3f *normal() { return vp(); }
	const vec3f *normal() const { return vp(); }
	float *negDist() { return &w; }
	const float *negDist() const { return &w; }

	/**
	 * calc the 'dist' variable based upon the current 'normal' and the point provided
	 */
	void calcDist(const vec3f &v) {
		*negDist() = -dot(*vp(), v);
	}

};

/**
 * builds a plane from the three points
 * creates the normal assuming a right-handed system from points a->b->c
 */
inline plane_t planeBuildUnit(const vec3f &a, const vec3f &b, const vec3f &c) {
	plane_t plane(vecUnitNormal(a,b,c));
	plane.calcDist(a);
	return plane;
}

/**
 * same as above but with non-normalized normal
 */
inline plane_t planeBuild(const vec3f &a, const vec3f &b, const vec3f &c) {
	plane_t plane(vecPlaneNormal(a,b,c));
	plane.calcDist(a);
	return plane;
}

/**
 * returns the distance the vector 'v' is from the plane 'plane'
 * distance is scaled by the magnitude of the plane's normal
 * a negative distance implies the point is on the back of the plane
 */
inline float planePointDist(const plane_t &plane, const vec3f &v) {
	return dot(*plane.normal(), v) + *plane.negDist();
}

/**
 * returns the R3 L2 distance the vector 'v' is from the plane 'plane'
 * the distance is scaled by the inverse of the plane normal L2 length
 * use this with planes containing non-unit normal vectors
 */
inline float planePointDistNormalized(const plane_t &plane, const vec3f &v) {
	return planePointDist(plane, v) / vecLength(*plane.normal());
}

/**
 * builds a plane with the specified normal placed on the specified point
 */
inline plane_t planeBuildNormalPoint(const vec3f &n, const vec3f &p) {
	plane_t plane(n);
	plane.calcDist(p);
	return plane;
}

/**
 * returns the point 'v' projected to the plane 'plane'
 * assumes plane.normal is a unit vector
 */
inline vec3f planeProjectPoint(const plane_t &plane, const vec3f &v) {
	return v - (*plane.normal()) * planePointDist(plane, v);
}

/**
 * returns the point 'v' projected to the plane 'plane'
 * compensates for plane.normal's magnitude
 */
inline vec3f planeProjectPointNormalized(const plane_t &plane, const vec3f &v) {
	return v + (*plane.normal()) * (-((*plane.negDist()) + dot(*plane.normal(), v) / dot(*plane.normal(), *plane.normal())));
}

//// lines

class line_t {
public:
	vec3f pos;
	vec3f dir;

	/**
	 * line starting at the origin in the dir of the x axis
	 */
	line_t() : dir(vec3f(1,0,0)) {}

	line_t(const vec3f &pos_, const vec3f &dir_) : pos(pos_), dir(dir_) {}
};

/**
 * returns the coefficient 'c' for which
 * line.pos + line.dir * c lies on the specified plane
 */
inline float linePlaneIntersectFraction(const line_t &line, const plane_t &plane) {
	float src_dist = planePointDist(plane, line.pos);
	float dest_dist = planePointDist(plane, line.pos + line.dir);
	return src_dist / (src_dist - dest_dist);
}

/**
 * returns the point at which the line and plane intersect
 */
inline vec3f linePlaneIntersect(const line_t &line, const plane_t &plane) {
	return line.pos + line.dir * linePlaneIntersectFraction(line, plane);
}

//// bounding boxes

class box_t {
public:
	vec3f min, max;

	/**
	 * default create a box around <0,0,0>
	 */
	box_t() {}

	box_t(const vec3f &min_, const vec3f &max_) : min(min_), max(max_) {}

	/**
	 * stretches the volume of 'this' to include the two specified points
	 */
	void stretch(const vec3f &min, const vec3f &max) {
		for (int i = 0; i < 3; i++) {
			if (min.fp()[i] < this->min.fp()[i]) this->min.fp()[i] = min.fp()[i];
			if (max.fp()[i] > this->max.fp()[i]) this->max.fp()[i] = max.fp()[i];
		}
	}

	/**
	 * stretches the volume of 'this' to include the specified point
	 */
	void stretch(const vec3f &v) {
		stretch(v,v);
	}

	/**
	 * vec3f pointer to the box structure
	 * THIS IS DEPENDANT UPON CLASS VARIABLE ORDER
	 */
	const vec3f *vp() const { return &min; }
};
