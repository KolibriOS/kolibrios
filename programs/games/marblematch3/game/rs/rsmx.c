#include "rsmx.h"

#ifdef RS_USE_C_LIBS
    #include <string.h>
    #include <math.h>
#else
    #include "rsplatform.h" 
#endif

#include "rsdebug.h"

// Some matrix and vector stuff
// by Roman Shuvalov


rs_vec3_t rs_vec3_sub(rs_vec3_t v1, rs_vec3_t v2) {
    return rs_vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
};

rs_vec3_t rs_vec3_add(rs_vec3_t v1, rs_vec3_t v2) {
    return rs_vec3( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
};

rs_vec3_t rs_vec3_mult(rs_vec3_t v, float s) {
    return rs_vec3( v.x * s, v.y * s, v.z * s );
};


rs_vec4_t rs_vec4_sub(rs_vec4_t v1, rs_vec4_t v2) {
    rs_vec4_t dest;
    dest.x = v1.x - v2.x;
    dest.y = v1.y - v2.y;
    dest.z = v1.z - v2.z;
    dest.w = v1.z - v2.w;
    return dest;
};


rs_vec4_t rs_vec4(float x, float y, float z, float w) {
    rs_vec4_t r;
    r.x = x;
    r.y = y;
    r.z = z;
    r.w = w;
    return r;
};

rs_vec3_t rs_vec3(float x, float y, float z) {
    rs_vec3_t r;
    r.x = x;
    r.y = y;
    r.z = z;
    return r;
};






float rs_vec4_length_sqr(rs_vec4_t src) {
    return src.x*src.x + src.y*src.y + src.z*src.z + src.w*src.w;
};

float rs_vec3_length_sqr(rs_vec3_t src) {
    return src.x*src.x + src.y*src.y + src.z*src.z;
};




float rs_vec4_length(rs_vec4_t v) {
    return sqrtf( rs_vec4_length_sqr(v) );
};

float rs_vec3_length(rs_vec3_t v) {
    return sqrtf( rs_vec3_length_sqr(v) );
};




rs_vec3_t rs_vec3_normalize(rs_vec3_t v) {
    float s = rs_vec3_length(v);
    if (s > 0.00001) {
        return rs_vec3( v.x / s, v.y / s, v.z / s );
    }
    return rs_vec3(0.0, 0.0, 0.0);
};



float rs_vec4_dot(rs_vec4_t v1, rs_vec4_t v2) {
    return ( (v1.x) * (v2.x) ) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
};

float rs_vec3_dot(rs_vec3_t v1, rs_vec3_t v2) {
    return (v1.x) * (v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
};




rs_vec3_t rs_vec3_cross(rs_vec3_t u, rs_vec3_t v) {
    rs_vec3_t d;

    d.x = u.y * v.z - u.z * v.y;
    d.y = u.z * v.x - u.x * v.z;
    d.z = u.x * v.y - u.y * v.x;

    return d;

};



float rs_vec4_angle(rs_vec4_t u, rs_vec4_t v) {
    return rs_vec4_dot(u, v) / (rs_vec4_length(u) * rs_vec4_length(v) );
};

float rs_vec3_cos_angle(rs_vec3_t u, rs_vec3_t v) {
    float ret = rs_vec3_dot(u, v) / (rs_vec3_length(u) * rs_vec3_length(v) );
    return ret;
};

float rs_vec3_distance_sqr(rs_vec3_t u, rs_vec3_t v) {
    return rs_vec3_length_sqr( rs_vec3(u.x - v.x, u.y - v.y, u.z - v.z) );
};


float rs_clamp(float x, float min1, float max1) {
    if (x < min1) {
        return min1;
    };
    if (x > max1) {
        return max1;
    };
    return x;
};

int rs_clamp_i(int x, int min1, int max1) {
    if (x < min1) {
        return min1;
    };
    if (x > max1) {
        return max1;
    };
    return x;
};

float rs_exp_interpolate(float v_from, float v_to, float dt) {
    return v_from + ( v_to - v_from ) * ( 1 - exp(-dt/1.0) );
};

float rs_max(float x, float y) {
    return x > y ? x : y;
};

float rs_min(float x, float y) {
    return x < y ? x : y;
};

float rs_sign(float f) {
    return (f >= 0.0) ? 1.0 : -1.0;
};

float rs_pow(float f, float p) {
    return rs_sign(f) * pow( fabs(f), p );
};

float rs_clamp_angle(float f) { // !!!! only one iteration
    if (f > 2.0*M_PI) {
        return f - 2.0*M_PI;
    };
    
    if (f < -2.0*M_PI) {
        return f + 2.0*M_PI;
    };
    
    return f;
};

float rs_linear_interpolate(float v_from, float v_to, float t) {
    return v_from*(1.0-t) + v_to*t;
};

float rs_fract(float f) {
    float r = floor(f);
    return f - r;
};


