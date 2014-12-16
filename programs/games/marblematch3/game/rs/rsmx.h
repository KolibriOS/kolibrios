#ifndef rs_mx_t_H_INCLUDED
#define rs_mx_t_H_INCLUDED

// Some matrix and vector stuff
// by Roman Shuvalov

#define RS_SQR(x) ((x)*(x))


// actual period is 1024 ms (1.024 sec)
#define RS_TIME_KOEF_SEC (M_PI * 2.0 * 64.0 / 65536.0)



typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
        };
        float v[3];
    };

} rs_vec3_t;


typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        float v[4];
    };

} rs_vec4_t;






rs_vec3_t rs_vec3_sub(rs_vec3_t v1, rs_vec3_t v2);

rs_vec3_t rs_vec3_add(rs_vec3_t v1, rs_vec3_t v2);

rs_vec3_t rs_vec3_mult(rs_vec3_t v, float s);


rs_vec4_t rs_vec4_sub(rs_vec4_t v1, rs_vec4_t v2);

void rs_vec4_add(rs_vec4_t dest, rs_vec4_t v1, rs_vec4_t v2);

rs_vec4_t rs_vec4(float x, float y, float z, float w);

rs_vec3_t rs_vec3(float x, float y, float z);




float rs_vec4_length_sqr(rs_vec4_t src);
float rs_vec3_length_sqr(rs_vec3_t src);



float rs_vec4_length(rs_vec4_t v);
float rs_vec3_length(rs_vec3_t v);

float rs_vec4_dot(rs_vec4_t v1, rs_vec4_t v2);
float rs_vec3_dot(rs_vec3_t v1, rs_vec3_t v2);


rs_vec3_t rs_vec3_cross(rs_vec3_t u, rs_vec3_t v);

rs_vec3_t rs_vec3_normalize(rs_vec3_t v);

float rs_vec4_angle(rs_vec4_t u, rs_vec4_t v);

float rs_vec3_cos_angle(rs_vec3_t u, rs_vec3_t v);



float rs_vec3_distance_sqr(rs_vec3_t u, rs_vec3_t v);

float rs_clamp(float x, float min1, float max1);
int rs_clamp_i(int x, int min1, int max1);

float rs_max(float x, float y);
float rs_min(float x, float y);

float rs_sign(float f);
float rs_pow(float f, float p);
float rs_fract(float f);

float rs_exp_interpolate(float v_from, float v_to, float dt);
float rs_linear_interpolate(float v_from, float v_to, float t);


float rs_clamp_angle(float f);

#endif // rs_mx_t_H_INCLUDED
