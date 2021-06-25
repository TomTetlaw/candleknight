#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_PI_F
#define M_PI_F ((float)(M_PI))
#endif

inline float deg2rad(float x) {
	return x * (M_PI_F / 180.0f);
}

inline float rad2deg(float x) {
	return x * (180.0f / M_PI_F);
}

inline float approach(float current, float goal, float dt) {
	float dist = goal - current;
	if (dist > dt) {
		return current + dt;
	}
	else if (dist < -dt) {
		return current - dt;
	}
	else {
		return goal;
	}
}

inline bool within_tolerance(float x, float y, float tolerance) {
	if (x > y + tolerance) {
		return false;
	}
	else if (x < y - tolerance) {
		return false;
	}
	else {
		return true;
	}
}

template<typename T>
inline T clamp(T x, T min_value, T max_value) {
	if (x < min_value) {
		return min_value;
	}
	else if (x > max_value) {
		return max_value;
	}
	else {
		return x;
	}
}

inline float map_range(float x, float min1, float max1, float min2, float max2) {
	return (max2 - min2) * (x / (max1 - min1));
}

inline float random_float(float min_value, float max_value) {
	return min_value + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max_value - min_value)));
}

#define V4PARMS(v) v.x,v.y,v.z,v.w
struct Vec4 {
	float x = 0, y = 0, z = 0, w = 0;
    
	Vec4() {
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}
    
	Vec4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
    
	float distance_to(Vec4 &other) {
		Vec4 delta = *this - other;
		return delta.length();
	}
    
	float length() {
		return sqrtf(x*x + y*y + z*z + w*w);
	}
    
	Vec4 operator +(Vec4 &other) {
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}
	Vec4 operator -(Vec4 &other) {
		return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
	}
	Vec4 operator *(Vec4 &other) {
		return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
	}
	Vec4 operator +(float other) {
		return Vec4(x + other, y + other, z + other, w + other);
	}
	Vec4 operator -(float other) {
		return Vec4(x - other, y - other, z - other, w - other);
	}
	Vec4 operator *(float other) {
		return Vec4(x * other, y * other, z * other, w * other);
	}
};

#define V3PARMS(v) v.x,v.y,v.z
struct Vec3 {
	float x = 0, y = 0, z = 0;
    
	Vec3() {
		x = 0;
		y = 0;
		z = 0;
	}
    
	Vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
    
	float distance_to(Vec3 &other) {
		Vec3 delta = *this - other;
		return delta.length();
	}
    
	float length() {
		return sqrtf(x*x + y*y + z*z);
	}
    
	Vec3 normalize() {
		float len = length();
		return Vec3(x / len, y / len, z / len);
	}
    
	float dot(Vec3 &other) {
		return x*other.x + y*other.y + z*other.z;
	}
    
	Vec3 cross(Vec3 &other) {
		Vec3 out;
		out.x = y * other.z - z * other.y;
        out.y = z * other.x - x * other.z;
    	out.z = x * other.y - y * other.x;
		return out;
	}
    
	Vec3 negate() {
		return Vec3(-x, -y, -z);
	}
    
	Vec3 operator +(Vec3 &other) {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}
	Vec3 operator -(Vec3 &other) {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}
	Vec3 operator *(Vec3 &other) {
		return Vec3(x * other.x, y * other.y, z * other.z);
	}
	Vec3 operator +(float other) {
		return Vec3(x + other, y + other, z + other);
	}
	Vec3 operator -(float other) {
		return Vec3(x - other, y - other, z - other);
	}
	Vec3 operator *(float other) {
		return Vec3(x * other, y * other, z * other);
	}
}; 

#define V2PARMS(v) v.x, v.y
struct Vec2 {
    union {
        struct {
            float x, y;
        };
        float data[2];
    };
    
	Vec2() {
		x = 0;
		y = 0;
	}
    
	Vec2(float x, float y) {
		this->x = x;
		this->y = y;
	}
    
	float distance_to(Vec2 &other) {
		Vec2 delta = *this - other;
		return delta.length();
	}
    
	float length() {
		return sqrtf(x*x + y*y);
	}
    
    float length_squared() {
        return x*x + y*y;
    }
    
	float dot(Vec2 &other) {
		return x*other.x + y*other.y;
	}
    
	float determinant(Vec2 &other) {
		return x*other.y - y*other.x;
	}
    
	Vec2 normalize() {
		float len = length();
		return Vec2(x / len, y / len);
	}
    
	float angle_to(Vec2 &other) {
		Vec2 up, right, dir;
		float dot_product, side, angle;
		
		up = normalize();
		right = Vec2(-up.y, up.x);
		dir = other.normalize();
		dot_product = up.dot(dir);
		side = right.dot(dir);
		angle = acosf(dot_product);
		
		if (side < 0.0f) {
			angle *= -1.0f;
		}
		
		return angle;
	}
    
    Vec2 operator-() {
        return Vec2(-x, -y);
    }
    
	Vec2 operator +(Vec2 &other) {
		return Vec2(x + other.x, y + other.y);
	}
	Vec2 operator -(Vec2 &other) {
		return Vec2(x - other.x, y - other.y);
	}
	Vec2 operator *(Vec2 &other) {
		return Vec2(x * other.x, y * other.y);
	}
	Vec2 operator +(float other) {
		return Vec2(x + other, y + other);
	}
	Vec2 operator -(float other) {
		return Vec2(x - other, y - other);
	}
	Vec2 operator *(float other) {
		return Vec2(x * other, y * other);
	}
};

inline Vec2 random_vector(Vec2 min_value, Vec2 max_value) {
    return Vec2(random_float(min_value.x, max_value.x), random_float(min_value.y, max_value.y));
}

inline Vec2 approach(Vec2 current, Vec2 goal, float dt) {
	return Vec2(
                approach(current.x, goal.x, dt),
                approach(current.y, goal.y, dt)
                );
}

struct Mat4 {
	union {
		float e[16];
		float m[4][4];
		Vec4 v[4];
		struct {
			float _00, _01, _02, _03, 
            _10, _11, _12, _13,
            _20, _21, _22, _23,
            _30, _31, _32, _33;
		};
	};
    
	Mat4() {
		for(int i = 0; i < 16; i++) {
			e[i] = 0;
		}
	}
};

inline Mat4 create_identity_matrix() {
    Mat4 out;
    out._00 = 1; out._01 = 0; out._02 = 0; out._03 = 0;
    out._10 = 0; out._11 = 1; out._12 = 0; out._13 = 0;
    out._20 = 0; out._21 = 0; out._22 = 1; out._23 = 0;
    out._30 = 0; out._31 = 0; out._32 = 0; out._33 = 1;
    return out;
}

inline Mat4 create_ortho_matrix(float left, float right, float bottom, float top) {
    float znear = -1.0f;
    float zfar = 1.0f;
    Mat4 out;
    
    out.v[0].x = 2.0 / (right - left);
    out.v[0].y = 0;
    out.v[0].z = 0;
    out.v[0].w = 0;
    out.v[1].x = 0;
    out.v[1].y = 2.0 / (top - bottom);
    out.v[1].z = 0;
    out.v[1].w = 0;
    out.v[2].x = 0;
    out.v[2].y = 0;
    out.v[2].z = 2.0 / (zfar - znear);
    out.v[2].w = 0;
    out.v[3].x = -(right + left) / (right - left);
    out.v[3].y = -(top + bottom) / (top - bottom);
    out.v[3].z = -(zfar + znear) / (zfar - znear);
    out.v[3].w = 1;
    
    return out;
}

inline Mat4 create_perspective_matrix(float fov, float width, float height, float znear, float zfar, float aspect = 1) {
    float yscale = 1.0 / tan(deg2rad(fov / 2));
    float xscale = yscale / aspect;
    float diff = znear - zfar;
    Mat4 out;
    
    out.m[0][0] = xscale; 
    out.m[0][1] = 0;
    out.m[0][2] = 0;
    out.m[0][3] = 0;
    
    out.m[1][0] = 0;
    out.m[1][1] = yscale;
    out.m[1][2] = 0;
    out.m[1][2] = 0;
    
    out.m[2][0] = 0;
    out.m[2][1] = 0;
    out.m[2][2] = (zfar + znear) / diff;
    out.m[2][2] = -1;
    
    out.m[3][0] = 0;
    out.m[3][1] = 0;
    out.m[3][2] = 2 * zfar * znear / diff;
    out.m[3][2] = 0;
    
    return out;
}

inline Mat4 create_translation_matrix(Vec3 t) {
    Mat4 out;
    float v[] = {1, 0, 0, t.x, 0, 1, 0, t.y, 0, 0, 1, t.z, 0, 0, 0, 1};
    memcpy(out.e, v, sizeof(float) * 16);
    return out;
}

inline Mat4 create_x_rotation_matrix(Vec3 axis, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 out;
    float v[] = { 1, 0, 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1};
    memcpy(out.e, v, sizeof(float) * 16);
    return out;
}

inline Mat4 create_y_rotation_matrix(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 out;
    float v[] = { c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1};
    memcpy(out.e, v, sizeof(float) * 16);
    return out;
}

inline Mat4 create_z_rotation_matrix(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 out;
    float v[] = { c, -s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
    memcpy(out.e, v, sizeof(float) * 16);
    return out;
}

inline Mat4 multiply_matrix(Mat4 a, Mat4 b) {
    Mat4 out;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            for(int k = 0; k < 4; ++k) {
                out.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return out;
}

inline Mat4 transpose_matrix(Mat4 m) {
    Mat4 out;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            out.m[j][i] = m.m[i][j];
        }
    }
    return out;
}

#endif
