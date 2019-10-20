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


template<typename T, int index>
struct vec_get {
	//static inline T& get(vec<dim,T>& v)
	//static inline const T& get(const vec<dim,T>& v)
};



template<typename T>
struct assign {
	template<int i>
	struct vec_assign {
		static bool exec(T& v, const T& o) {
			vec_get<T, i>::get(v) = vec_get<T, i>::get(o);
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
			vec_get<T, i>::get(c) = vec_get<T, i>::get(a) + vec_get<T, i>::get(b);
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
			vec_get<T, i>::get(c) = vec_get<T, i>::get(a) - vec_get<T, i>::get(b);
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
			vec_get<T, i>::get(c) = vec_get<T, i>::get(a) * b;
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
			vec_get<T, i>::get(c) = vec_get<T, i>::get(a) / b;
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
			result = result && (vec_get<T, i>::get(a) == vec_get<T, i>::get(b));
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
			o << vec_get<T, i>::get(v);
			return false;
		}
	};

	void output(std::ostream& o) const {
		o << "{";
		Common::ForLoop<0, T::dim, vec_ostream>::exec(o, crtp_cast<T>(*this));
		o << "}";
	}
};

//// n-D vectors

template<int dim_, typename T, typename Parent>
struct vec_base : public 
	assign<Parent>,
	add_op<Parent>,
	sub_op<Parent>,
	mul_op<Parent,T>,
	div_op<Parent,T>,
	eq_op<Parent>,
	ostream_op<Parent>
{
	using scalar = T;
	enum { dim = dim_ };
};

template<int dim, typename T>
struct vec : public vec_base<dim, T, vec<dim,T>> {
};

//ostream

template<int dim, typename T>
inline std::ostream& operator<<(std::ostream& o, const vec<dim,T>& v) {
	v.output(o);
	return o;
}

//unary

template<typename T>
struct un_op {
	template<int i>
	struct vec_un {
		static bool exec(T& c, const T& a) {
			vec_get<T, i>::get(c) = -vec_get<T, i>::get(a);
			return false;
		}
	};
};

template<int dim, typename T>
inline T operator-(const vec<dim,T>& v) {
	T t;
	Common::ForLoop<0, vec<dim,T>::dim, un_op<vec<dim,T>>::template vec_un>::exec(t, v);
	return t;
}

//linear interpolation

template<int dim, typename T>
inline vec<dim,T> vec3lerp(const vec<dim,T> &src, const vec<dim,T> &dst, typename vec<dim,T>::scalar coeff) {
	typename vec<dim,T>::scalar one_minus = 1 - coeff;
	return src * one_minus + dst * coeff;
}

//tensor operations

	//dot product

template<typename T>
struct vec_sum {
	using scalar = typename T::scalar;
	template<int i>
	struct op {
		static bool exec(scalar& sum, const T& a, const T& b) {
			sum += vec_get<T, i>::get(a) * vec_get<T, i>::get(b);
			return false;
		}
	};
};

template<int dim, typename T>
inline T dot(const vec<dim,T> &a, const vec<dim,T> &b) {
	T sum = T();
	Common::ForLoop<0, vec<dim,T>::dim, vec_sum<vec<dim,T>>::template op>::exec(sum, a, b);
	return sum;
}


//magnitude

	//L-2

template<int dim, typename T>
inline T vecLengthSq(const vec<dim,T> &v) {
	return dot(v,v);
}

template<int dim, typename T>
inline T vecLength(const vec<dim,T> &v) {
	return (T)sqrt(dot(v,v));
}

	//L-Infinite

template<typename T>
struct vec_linflen {
	using scalar = typename T::scalar;
	template<int i>
	struct op {
		static bool exec(scalar& result, const T& v) {
			result = std::max(result, std::abs(vec_get<T, i>::get(v)));
			return false;
		}
	};
};

template<int dim, typename T>
inline T vecLengthLInf(const vec<dim,T> &v) {
	T result = std::abs(vec_get<vec<dim, T>, 0>::get(v));
	Common::ForLoop<1, dim, vec_linflen<vec<dim,T>>::template op>::exec(result, v);
	return result;
}

//normalization

template<int dim, typename T>
inline vec<dim,T> vecUnit(const vec<dim,T> &v) {
	T len = vecLength(v);
	if (!len) {
		return v;
	} else {
		return v / len;
	}
}



//// 2D vectors

#define VEC2ELEM(i)	(i).x,(i).y

template<typename T>
struct vec<2,T> : public vec_base<2,T,vec<2,T>> {
	T x, y;

	vec() : x(0), y(0) {}
	vec(T x_, T y_) : x(x_), y(y_) {}

	T *fp() { return &x; }
	const T *fp() const { return &x; }
};

template<typename T>
struct vec_get<vec<2,T>,0> {
	static inline T& get(vec<2,T>& v) { return v.x; }
	static inline const T& get(const vec<2,T>& v) { return v.x; }
};

template<typename T>
struct vec_get<vec<2,T>,1> {
	static inline T& get(vec<2,T>& v) { return v.y; }
	static inline const T& get(const vec<2,T>& v) { return v.y; }
};

//// 3D vectors

#define VEC3ELEM(i)	(i).x,(i).y,(i).z

template<typename T>
struct vec<3,T> : public vec_base<3,T,vec<3,T>> {
	T x, y, z;

	vec() : x(0), y(0), z(0) {}
	vec(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

	T *fp() { return &x; }
	const T *fp() const { return &x; }
};

template<typename T>
struct vec_get<vec<3,T>,0> {
	static inline T& get(vec<3,T>& v) { return v.x; }
	static inline const T& get(const vec<3,T>& v) { return v.x; }
};

template<typename T>
struct vec_get<vec<3,T>,1> {
	static inline T& get(vec<3,T>& v) { return v.y; }
	static inline const T& get(const vec<3,T>& v) { return v.y; }
};

template<typename T>
struct vec_get<vec<3,T>,2> {
	static inline T& get(vec<3,T>& v) { return v.z; }
	static inline const T& get(const vec<3,T>& v) { return v.z; }
};

//tensor operations
	
	//cross product

template<typename T>
inline vec<3,T> cross(const vec<3,T> &a, const vec<3,T> &b) {
	return vec<3,T>(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

// useful functions

template<typename T>
inline vec<3,T> vecPlaneNormal(const vec<3,T> &a, const vec<3,T> &b, const vec<3,T> &c) {
	return cross(b - a, c - b);
}

template<typename T>
inline vec<3,T> vecUnitNormal(const vec<3,T> &a, const vec<3,T> &b, const vec<3,T> &c) {
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
	using scalar = fixed_t;

	enum { dim = 3 };

	scalar x, y, z;
	
	vec3fixed() : x(0), y(0), z(0) {}
	vec3fixed(scalar x_, scalar y_, scalar z_) : x(x_), y(y_), z(z_) {}

	scalar *ip() { return &x; }
	const scalar *ip() const { return &x; }
};

template<>
struct vec_get<vec3fixed,0> {
	static inline vec3fixed::scalar& get(vec3fixed& v) { return v.x; }
	static inline const vec3fixed::scalar& get(const vec3fixed& v) { return v.x; }
};

template<>
struct vec_get<vec3fixed,1> {
	static inline vec3fixed::scalar& get(vec3fixed& v) { return v.y; }
	static inline const vec3fixed::scalar& get(const vec3fixed& v) { return v.y; }
};

template<>
struct vec_get<vec3fixed,2> {
	static inline vec3fixed::scalar& get(vec3fixed& v) { return v.z; }
	static inline const vec3fixed::scalar& get(const vec3fixed& v) { return v.z; }
};

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
struct quat : public vec_base<4,T,quat<T>> {
	T x,y,z,w;

	quat() : x(0), y(0), z(0), w(1) {}
	quat(const vec<3,T> &v, T w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
	quat(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}

	T *fp() { return &x; }
	const T *fp() const { return &x; }

	vec<3,T> *vp() { return (vec<3,T> *)&x; }
	const vec<3,T> *vp() const { return (vec<3,T> *)&x; }
};

template<typename T>
struct vec_get<quat<T>,0> {
	static inline T& get(quat<T>& v) { return v.x; }
	static inline const T& get(const quat<T>& v) { return v.x; }
};

template<typename T>
struct vec_get<quat<T>,1> {
	static inline T& get(quat<T>& v) { return v.y; }
	static inline const T& get(const quat<T>& v) { return v.y; }
};

template<typename T>
struct vec_get<quat<T>,2> {
	static inline T& get(quat<T>& v) { return v.z; }
	static inline const T& get(const quat<T>& v) { return v.z; }
};

template<typename T>
struct vec_get<quat<T>,3> {
	static inline T& get(quat<T>& v) { return v.w; }
	static inline const T& get(const quat<T>& v) { return v.w; }
};

//multiplication

template<typename T>
inline quat<T> operator*(const quat<T> &q, const quat<T> &r) {
	T a,b,c,d,e,f,g,h;

	a = (q.w + q.x) * (r.w + r.x);
	b = (q.z - q.y) * (r.y - r.z);
	c = (q.x - q.w) * (r.y + r.z);
	d = (q.y + q.z) * (r.x - r.w);
	e = (q.x + q.z) * (r.x + r.y);
	f = (q.x - q.z) * (r.x - r.y);
	g = (q.w + q.y) * (r.w - r.z);
	h = (q.w - q.y) * (r.w + r.z);

	return quat<T>(	a - .5 * ( e + f + g + h),
					-c + .5 * ( e - f + g - h),
					-d + .5 * ( e - f - g + h),
					b + .5 * (-e - f + g + h));
}

//conjugation

template<typename T>
inline quat<T> quatConj(const quat<T> &q) {
	return quat<T>(q.x, q.y, q.z, -q.w);
}

//dot product

template<typename T>
inline T dot(const quat<T> &a, const quat<T> &b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

//magnitude

template<typename T>
inline T quatLength(const quat<T> &q) {
	return (T)sqrt(dot(q, q));
}

//normalization

template<typename T>
inline quat<T> quatUnit(const quat<T> &q) {
	return q / quatLength(q);
}

template<typename T>
inline quat<T> lerp(const quat<T> &src, const quat<T> &dst, T coeff) {
	T one_minus = 1 - coeff;
	return quat<T>(
		src.x * one_minus + dst.x * coeff,
		src.y * one_minus + dst.y * coeff,
		src.z * one_minus + dst.z * coeff,
		src.w * one_minus + dst.w * coeff);
}


/**
 * This method converts an object from a quaternion to angle-axis form.
 * The resultant quaternion 'w' component holds the quaternion angle, in degrees.
 */
template<typename T>
inline quat<T> quatToAngleAxis(const quat<T> &q) {
	T cosom = q.w;
	if (cosom < -1) cosom = -1;
	else if (cosom > 1) cosom = 1;

	T halfangle = (T)acos(cosom);
	T scale = (T)sin(halfangle);

	//TODO - check epsilon
	if (!scale) {
		return quat<T>(0,0,1,0);
	}

	scale = (T)1 / scale;

	return quat<T>(q.x * scale, q.y * scale, q.z * scale, halfangle * 360.f / (T)M_PI);
}

/**
 * This method converts an object from angle-axis form to a quaternion.
 * It assumes the 'w' component of the incoming quaternion is the angle, measured in degrees.
 * It also assumes the <x,y,z> components of the incoming quaternion form a unit vector.
 */
template<typename T>
inline quat<T> angleAxisToQuat(const quat<T> &q) {
	T halfangle = q.w * (T)M_PI / 360.f;
	T scale = (T)sin(halfangle);

	return quat<T>(
		q.x * scale,
		q.y * scale,
		q.z * scale,
		(T)cos(halfangle));
}

/**
 * Returns the value of exp(v) = exp(ix + jy + kz) using quaternion mathematics.
 * Equivalent to 'angleAxisToQuat(quatf(v.x, v.y, v.z, vecLength(v) / M_PI * 360.f))'
 * for some vec3f 'v'.
 */
template<typename T>
inline quat<T> quatExp(const vec<3,T> &a) {
	T length = vecLength(a);
	if (!length) {
		return quat<T>(0,0,0,1);
	}

	//compensate for numerical inaccuracies at sin(x)/x for x->0
	T sinth = 1.f;
	if (length > 0.00001f || length < -0.00001f) sinth = (T)sin(length) / length;
	
	return quat<T>(a * sinth, (T)cos(length));
}

/**
 * Rotates vector 'v' by quaternion 'q' and returns the rotated vector.
 * This is equivalent to the operation 'q * quatf(v.x,v.y,v.z,0) * quatConj(q)'
 * This is mathematically correct only when 'q' is a unit quaternion.
 */
template<typename T>
inline vec<3,T> quatRotate(const quat<T> &q, const vec<3,T> &v) {
	T w2x2 = (q.w + q.x) * (q.w - q.x);
	T w2y2 = (q.w + q.y) * (q.w - q.y);
	T x2z2 = (q.x + q.z) * (q.x - q.z);
	T y2z2 = (q.y + q.z) * (q.y - q.z);

	return vec<3,T>(
		v.x * (w2y2 + x2z2) + 2.f*(v.y * (q.x * q.y - q.w * q.z) + v.z * (q.x * q.z + q.w * q.y)),
		v.y * (w2x2 + y2z2) + 2.f*(v.z * (q.y * q.z - q.w * q.x) + v.x * (q.x * q.y + q.w * q.z)),
		v.z * (w2y2 - x2z2) + 2.f*(v.x * (q.x * q.z - q.w * q.y) + v.y * (q.y * q.z + q.w * q.x)));
}

/**
 * Returns the X-axis of the basis formed by converting 'q' from a quaternion to a matrix
 * and returning the first column vector components.
 */
template<typename T>
inline vec<3,T> quatXAxis(const quat<T> &q) {
	return vec<3,T>(
		1 - 2 * (q.y * q.y + q.z * q.z),
		2 * (q.x * q.y + q.z * q.w),
		2 * (q.x * q.z - q.w * q.y));
}

/**
 * Returns the Y-axis of the basis formed by converting 'q' from a quaternion to a matrix
 * and returning the first column vector components.
 */
template<typename T>
inline vec<3,T> quatYAxis(const quat<T> &q) {
	return vec<3,T>(
		2 * (q.x * q.y - q.w * q.z),
		1 - 2 * (q.x * q.x + q.z * q.z),
		2 * (q.y * q.z + q.w * q.x));
}

/**
 * Returns the Z-axis of the basis formed by converting 'q' from a quaternion to a matrix
 * and returning the first column vector components.
 */
template<typename T>
inline vec<3,T> quatZAxis(const quat<T> &q) {
	return vec<3,T>(
		2 * (q.x * q.z + q.w * q.y),
		2 * (q.y * q.z - q.w * q.x),
		1 - 2 * (q.x * q.x + q.y * q.y));
}

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const quat<T>& v) {
	v.output(o);
	return o;
}

//// 4x4 matrices

/**
 * This class is used for our 4x4 matrices.
 * Maybe I'll make a 3x3 class later...
 * Maybe even a 3x4 for humor's sake
 */
template<typename T>
class mat44 {
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
	quat<T> x,y,z,w;

	/**
	 * constructor: identity matrix
	 */
	mat44() 
		: x(1,0,0,0)
		, y(0,1,0,0)
		, z(0,0,1,0)
		, w(0,0,0,1)
	{}

	/**
	 * constructor: real analogy. scaled identity.
	 */
	mat44(T r) 
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
	mat44(	T xx, T xy, T xz, T xw,
			T yx, T yy, T yz, T yw,
			T zx, T zy, T zz, T zw,
			T wx, T wy, T wz, T ww)
		: x(xx, xy, xz, xw)
		, y(yx, yy, yz, yw)
		, z(zx, zy, zz, zw)
		, w(wx, wy, wz, ww)
	{}

	vec<3,T> transformVector(const vec<3,T> &a) const {		//with vectors, assume w component is 0
		return vec<3,T>(
			x.x * a.x + y.x * a.y + z.x * a.z,
			x.y * a.x + y.y * a.y + z.y * a.z,
			x.z * a.x + y.z * a.y + z.z * a.z);
	}

	T *fp() { return &x.x; }
	const T *fp() const { return &x.x; }

	mat44 transpose() {
		return mat44(
			x.x, y.x, z.x, w.x,
			x.y, y.y, z.y, w.y,
			x.z, y.z, z.z, w.z,
			x.w, y.w, z.w, w.w);
	}
};

template<typename T>
inline quat<T> operator*(const mat44<T> &a, const quat<T> &b) {
	return quat<T>(
		a.x.x * b.x + a.y.x * b.y + a.z.x * b.z + a.w.x * b.w,
		a.x.y * b.x + a.y.y * b.y + a.z.y * b.z + a.w.y * b.w,
		a.x.z * b.x + a.y.z * b.y + a.z.z * b.z + a.w.z * b.w,
		a.x.w * b.x + a.y.w * b.y + a.z.w * b.z + a.w.w * b.w);
}

template<typename T>
inline mat44<T> operator*(const mat44<T> &a, const mat44<T> &b) {
	mat44<T> ret;
	const T *fa = (const T *)(&a.x.x);
	const T *fb = (const T *)(&b.x.x);
	T *fr = (T *)(&ret.x.x);
	for (int i = 0; i < 4; i++) {			//dest column
		for (int j = 0; j < 4; j++) {		//dest row
			T s = 0;
			for (int k = 0; k < 4; k++) {
				T a0 = fa[ j + (k << 2) ];
				T a1 = fb[ k + (i << 2) ];
				s += a0 * a1;
			}
			fr[ j + (i << 2) ] = s;
		}
	}
	return ret;
}

template<typename T>
inline mat44<T> operator*=(mat44<T> &a, const mat44<T> &b) {
	return (a = a * b);
}

template<typename T>
inline bool operator==(const mat44<T> &a, const mat44<T> &b) {
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

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const mat44<T>& m) {
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
template<typename T>
class basis {
public:
	quat<T> r;
	vec<3,T> t;

	/**
	 * constructs a default identity quaternion and zero vector
	 */
	basis() {}

	/**
	 * constructs a basis with the specified orientation and translation
	 */
	basis(const quat<T> &r_, const vec<3,T> &t_) : r(r_), t(t_) {}

	/**
	* returns a * b
	*
	* [Ra Ta] [Rb Tb]   [Ra*Rb  Ra*Tb + Ta]
	* [0  1 ]*[0  1 ] = [  0         1    ]
	*/
	basis operator*(const basis &b) const {
		return basis(r * b.r, quatRotate(r, b.t) + t);
	}

	basis operator*=(const basis &b) {
		return *this = *this * b;
	}
};

//orthogonal basis, uniform scale, with transform components
template<typename T>
class basisu {
public:
	quat<T> r;
	T s;
	vec<3,T> t;

	/**
	 * constructs a default identity quaternion, zero vector, and unit uniform scale
	 */
	basisu() : s(1) {}

	/**
	 * constructs a basis with the specified orientation scale and translation
	 */
	basisu(const quat<T> &r_, T s_, const vec<3,T> &t_) : r(r_), s(s_), t(t_) {}

	/**
	* returns a * b
	*
	* [Ra*Sa Ta] [Rb*Sb Tb]   [Ra*Sa*Rb*Sb	Ra*Sa*Tb+Ta ]	[(Ra*Rb)*(Sa*Sb) Ra*Sa*Tb+Ta]
	* [0     1 ]*[0     1 ] = [0			1			] = [0				 1			]
	*/
	basisu operator*(const basisu &b) const {
		return basisu(r * b.r, s * b.s, quatRotate(r, b.t) * s + t);
	}

	basisu operator*=(const basisu &b) {
		return *this = *this * b;
	}
};

//orthogonal basis, non-uniform scale, with transform components
template<typename T>
class basisn {
public:
	quat<T> r;
	vec<3,T> s;
	vec<3,T> t;

	/**
	 * constructs a default identity quaternion, zero vector, and unit scale vector
	 */
	basisn() : s(1,1,1) {}

	/**
	 * constructs a basis with the specified orientation scale and translation
	 */
	basisn(const quat<T>& r_, const vec<3,T>& s_, const vec<3,T>& t_) : r(r_), s(s_), t(t_) {}

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
template<typename T>
struct plane : public quat<T> {
	using super = quat<T>;
	using scalar = typename super::scalar;

	/**
	 * constructs an empty plane with normal <0,0,0> and distance from origin 0
	 */
	plane() : super(0,0,0,0) {}

	/**
	 * constructs a plane from the specified normal at zero distance
	 */
	plane(const vec<3,T> &normal) : super(normal, 0) {}

	/**
	 * constructs a plane from the specified normal at the specified distance
	 */
	plane(const vec<3,T> &normal, T dist) {
		*super::vp() = normal;
		super::w = -dist;
	}

	vec<3,T> *normal() { return super::vp(); }
	const vec<3,T> *normal() const { return super::vp(); }
	T *negDist() { return &this->super::w; }
	const T *negDist() const { return &this->super::w; }

	/**
	 * calc the 'dist' variable based upon the current 'normal' and the point provided
	 */
	void calcDist(const vec<3,T> &v) {
		*negDist() = -dot(*super::vp(), v);
	}
};

/**
 * builds a plane from the three points
 * creates the normal assuming a right-handed system from points a->b->c
 */
template<typename T>
inline plane<T> planeBuildUnit(const vec<3,T> &a, const vec<3,T> &b, const vec<3,T> &c) {
	plane<T> plane(vecUnitNormal(a,b,c));
	plane.calcDist(a);
	return plane;
}

/**
 * same as above but with non-normalized normal
 */
template<typename T>
inline plane<T> planeBuild(const vec<3,T> &a, const vec<3,T> &b, const vec<3,T> &c) {
	plane<T> plane(vecPlaneNormal(a,b,c));
	plane.calcDist(a);
	return plane;
}

/**
 * returns the distance the vector 'v' is from the plane 'plane'
 * distance is scaled by the magnitude of the plane's normal
 * a negative distance implies the point is on the back of the plane
 */
template<typename T>
inline T planePointDist(const plane<T> &plane, const vec<3,T> &v) {
	return dot(*plane.normal(), v) + *plane.negDist();
}

/**
 * returns the R3 L2 distance the vector 'v' is from the plane 'plane'
 * the distance is scaled by the inverse of the plane normal L2 length
 * use this with planes containing non-unit normal vectors
 */
template<typename T>
inline T planePointDistNormalized(const plane<T> &plane, const vec<3,T> &v) {
	return planePointDist(plane, v) / vecLength(*plane.normal());
}

/**
 * builds a plane with the specified normal placed on the specified point
 */
template<typename T>
inline plane<T> planeBuildNormalPoint(const vec<3,T> &n, const vec<3,T> &p) {
	plane<T> plane(n);
	plane.calcDist(p);
	return plane;
}

/**
 * returns the point 'v' projected to the plane 'plane'
 * assumes plane.normal is a unit vector
 */
template<typename T>
inline vec<3,T> planeProjectPoint(const plane<T> &plane, const vec<3,T> &v) {
	return v - (*plane.normal()) * planePointDist(plane, v);
}

/**
 * returns the point 'v' projected to the plane 'plane'
 * compensates for plane.normal's magnitude
 */
template<typename T>
inline vec<3,T> planeProjectPointNormalized(const plane<T> &plane, const vec<3,T> &v) {
	return v + (*plane.normal()) * (-((*plane.negDist()) + dot(*plane.normal(), v) / dot(*plane.normal(), *plane.normal())));
}

//// lines

template<typename T>
struct line {
	T pos, dir;

	/**
	 * line starting at the origin in the dir of the x axis
	 */
	line() : dir(T(1,0,0)) {}

	line(const T &pos_, const T &dir_) : pos(pos_), dir(dir_) {}
};

/**
 * returns the coefficient 'c' for which
 * line.pos + line.dir * c lies on the specified plane
 */
template<typename T>
inline T linePlaneIntersectFraction(const line<vec<3,T>> &line, const plane<T> &plane) {
	T src_dist = planePointDist(plane, line.pos);
	T dest_dist = planePointDist(plane, line.pos + line.dir);
	return src_dist / (src_dist - dest_dist);
}

/**
 * returns the point at which the line and plane intersect
 */
template<typename T>
inline vec<3,T> linePlaneIntersect(const line<vec<3,T>> &line, const plane<T> &plane) {
	return line.pos + line.dir * linePlaneIntersectFraction(line, plane);
}

//// bounding boxes

template<typename T>
class box {
public:
	T min, max;

	/**
	 * default create a box around <0,0,0>
	 */
	box() {}

	box(const T &min_, const T &max_) : min(min_), max(max_) {}

	template<int i>
	struct stretch_for {
		static bool exec(box<T>& b, const T& min, const T& max) {
			if (vec_get<T,i>::get(min) < vec_get<T,i>::get(b.min)) vec_get<T,i>::get(b.min) = vec_get<T,i>::get(min);
			if (vec_get<T,i>::get(max) > vec_get<T,i>::get(b.max)) vec_get<T,i>::get(b.max) = vec_get<T,i>::get(max);
			return false;
		}
	};

	/**
	 * stretches the volume of 'this' to include the two specified points
	 */
	void stretch(const T &min, const T &max) {
		Common::ForLoop<0, T::dim, stretch_for>::exec(*this, min, max);
	}

	/**
	 * stretches the volume of 'this' to include the specified point
	 */
	void stretch(const T &v) {
		stretch(v, v);
	}

	/**
	 * vec3f pointer to the box structure
	 * THIS IS DEPENDANT UPON CLASS VARIABLE ORDER
	 */
	const T *vp() const { return &min; }
};


// specific types:

using vec2f = vec<2,float>;
using vec3f = vec<3,float>;
using quatf = quat<float>;
using mat44f = mat44<float>;
using basis_t = basis<float>;
using basisu_t = basisu<float>;
using basisn_t = basisn<float>;
using plane_t = plane<float>;
using line3f = line<vec3f>;
using box3f = box<vec3f>;
