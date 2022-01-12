#ifndef SH_SIMPLE_MATH
#define SH_SIMPLE_MATH
#define TO_RADIANS(angle) (angle)*3.14159f/180.0f

typedef union {
	struct {
		f32 x;
		f32 y;
		f32 z;
	};

	f32 _d[3];
} sh_vec3;


typedef union {
	struct {
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	f32 _d[4];
} pos4;



typedef union {
	struct {
		pos4 a;
		pos4 b;
		pos4 c;
		pos4 d;
	};

	f32 m[16];
}  mat4;



typedef struct sh_vec2 {
	f32 x;
	f32 y;
} sh_vec2, pos2;


void sh_vec2_add(sh_vec2 *r, sh_vec2 *a, sh_vec2 *b);
void sh_vec2_sub(sh_vec2 *r, sh_vec2 *a, sh_vec2 *b);
void sh_vec2_mul(sh_vec2 *r, sh_vec2 *a, float val);
float sh_vec2_dot(sh_vec2 *a, sh_vec2 *b);
void sh_vec2_normal(sh_vec2 *r, sh_vec2 *a);
float sh_vec2_lensq(sh_vec2 *a);
float sh_vec2_len(sh_vec2 *a);
void sh_vec2_normalize(sh_vec2 *r, sh_vec2 *a);

// void sh_vec2_add(sh_vec2 *r, sh_vec2 *a, sh_vec2 *b) {
// 	r->x = a->x + b->x;
// 	r->y = a->y + b->y;
// }

// void sh_vec2_sub(sh_vec2 *r, sh_vec2 *a, sh_vec2 *b) {
// 	r->x = a->x - b->x;
// 	r->y = a->y - b->y;
// }

// void sh_vec2_mul(sh_vec2 *r, sh_vec2 *a, float val) {
// 	r->x = a->x*val;
// 	r->y = a->y*val;
// }


// void sh_vec2_normal(sh_vec2 *r, sh_vec2 *a) {
// 	r->x = -a->y;
// 	r->y =  a->x;
// }

// void sh_vec2_normalize(sh_vec2 *r, sh_vec2 *a) {
// 	float length = sh_vec2_len(a);
// 	r->x = a->x/length;
// 	r->y = a->y/length;
// }

// float sh_vec2_lensq(sh_vec2 *a) {
// 	return a->x*a->x + a->y*a->y;
// }

// float sh_vec2_len(sh_vec2 *a) {
// 	return sqrt(sh_vec2_lensq(a));
// }

// float sh_vec2_dot(sh_vec2 *a, sh_vec2 *b) {
// 	return a->x*b->x + a->y*b->y;
// }


//debug funcs

void print_sh_vec2(sh_vec2 *p) {
	printf("(x: %f, y: %f)", p->x, p->y);
}


void print_mat4(mat4 *m) {
	printf(
			"%.4f %.4f %.4f %.4f\n"\
			"%.4f %.4f %.4f %.4f\n"\
			"%.4f %.4f %.4f %.4f\n"\
			"%.4f %.4f %.4f %.4f\n",
			m->m[0], m->m[1],  m->m[2],  m->m[3],
			m->m[4], m->m[5],  m->m[6],  m->m[7],
			m->m[8], m->m[9],  m->m[10], m->m[11],
			m->m[12],m->m[13], m->m[14], m->m[15]);

}

mat4 ortho(float left, float right, float bottom, float top, float pnear, float pfar) {

	float width = (right-left);
	float height = (top - bottom);
	mat4 m = {0};


	m.a.x =  2.0f/(width);
	m.b.y =  2.0f/(height);
	m.c.z =  -2.0f/(pfar - pnear);

	m.a.w = - (right + left) / (right - left);
	m.b.w = - (top + bottom) / (top - bottom);
	m.c.w = - (pfar + pnear) / (pfar - pnear);
	m.d.w = 1;
	return m;
}

mat4 fr_test(float left,    float right,
                  float bottom,  float top,
                  float nearZ, float farZ) {
  float rl, tb, fn, nv;

  mat4 m = {0};
  
  

  rl = 1.0f / (right  - left);
  tb = 1.0f / (top    - bottom);
  fn =-1.0f / (farZ - nearZ);
  nv = 2.0f * nearZ;

  // m.m[0 + 4*0] = nv * rl;
  // m.m[1 + 4*1] = nv * tb;
  // m.m[2 + 4*0] = (right  + left)    * rl;
  // m.m[2 + 4*1] = (top    + bottom)  * tb;
  // m.m[2 + 4*2] = (farZ + nearZ) * fn;
  // m.m[2 + 4*3] =-1.0f;
  // m.m[3 + 4*2] = farZ * nv * fn;

  m.m[0 + 4*0] = nv * rl;
  m.m[1 + 4*1] = nv * tb;
  m.m[2 + 4*0] = (right  + left)    * rl;
  m.m[2 + 4*1] = (top    + bottom)  * tb;
  m.m[2 + 4*2] = farZ * fn;
  m.m[2 + 4*3] =-1.0f;
  m.m[3 + 4*2] = farZ * nearZ * fn;


  return m;
}

mat4 frustum(float left, float right, float bottom, float top, float pnear, float pfar) {

	float width = (right-left);
	float height = (top - bottom);

	mat4 m = {0};

	m.a.x = 2.0f*pnear/(width);
	m.b.y = 2.0f*pnear/(height);
	m.c.z = -(pfar + pnear) / (pfar - pnear);

	m.a.z = (right + left) / (width);
	m.b.z = (top + bottom) / (height);

	m.c.w = - 2*pfar*pnear / (pfar - pnear);
	m.d.z = -1;

	return m;
}



sh_vec3 sh_vec3_sub(sh_vec3 a, sh_vec3 b) {

	sh_vec3 sub = {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
	};

	return sub;
}

float sh_vec3_length(sh_vec3 *p) {
	return (float)sqrt( p->x*p->x + p->y*p->y + p->z*p->z );
}

void sh_vec3_normalize_ref(sh_vec3 *p) {
	float len = sh_vec3_length(p);

	p->x /= len;
	p->y /= len;
	p->z /= len;
}

sh_vec3 sh_vec3_cross(sh_vec3 *p1, sh_vec3 *p2) {

	return {
		p1->y * p2->z - p1->z * p2->y, //x
	    p1->z * p2->x - p1->x * p2->z, //y
		p1->x * p2->y - p1->y * p2->x  //z
	};



}

f32 sh_vec3_dot(sh_vec3 *p1, sh_vec3 *p2) {
	return p1->x*p2->x + p1->y*p2->y + p1->z*p2->z;
}



void sh_vec3_div_scaler(sh_vec3 *v, float scaler) {

	v->x /= scaler;
	v->y /= scaler;
	v->z /= scaler;
}

void sh_vec3_mul_scaler(sh_vec3 *v, float scaler) {
	v->x *= scaler;
	v->y *= scaler;
	v->z *= scaler;
}

void sh_vec3_mul_vec3(sh_vec3 *v, sh_vec3 *v2) {
	v->x *= v2->x;
	v->y *= v2->y;
	v->z *= v2->z;
}

void sh_vec3_add_vec3(sh_vec3 *v, sh_vec3 *v2) {
	v->x += v2->x;
	v->y += v2->y;
	v->z += v2->z;
}

void sh_vec3_sub_vec3(sh_vec3 *v, sh_vec3 *v2) {
	v->x -= v2->x;
	v->y -= v2->y;
	v->z -= v2->z;
}


sh_vec3 sh_vec3_new_add_vec3(sh_vec3 *v, sh_vec3 *v2) {
	return {
		v->x + v2->x,
		v->y + v2->y,
		v->z + v2->z
	};
}


sh_vec3 sh_vec3_new_mul_scaler(sh_vec3 *v, float scaler) {
	return {
		v->x * scaler,
		v->y * scaler,
		v->z * scaler
	};
}




sh_vec3 sh_vec3_new_sub_vec3(sh_vec3 *v, sh_vec3 *v2) {
	return {
		v->x - v2->x,
		v->y - v2->y,
		v->z - v2->z
	};
}

sh_vec3 sh_pos4_as_vec3_new_mul_scaler(pos4 *v, float scaler) {
	return {
		v->x * scaler,
		v->y * scaler,
		v->z * scaler
	};
}

void sh_pos4_as_vec3_add_vec3(pos4 *v, sh_vec3 *v2) {
	v->x += v2->x;
	v->y += v2->y;
	v->z += v2->z;
}



void sh_vec3_vec3_length_and_direction(sh_vec3 *v, sh_vec3 *v2, float *len, sh_vec3 *direction) {

	sh_vec3 vec = sh_vec3_new_sub_vec3(v, v2);

	*len = sh_vec3_length(&vec);

	direction->x = vec.x/(*len);
	direction->y = vec.y/(*len);
	direction->z = vec.z/(*len);

}


void sh_vec3_length_and_direction(sh_vec3 *v, float *len, sh_vec3 *direction) {

	*len = sh_vec3_length(v);

	direction->x = v->x/(*len);
	direction->y = v->y/(*len);
	direction->z = v->z/(*len);

}


void sh_pos4_as_vec3_pos4_as_vec3_length_and_direction(pos4 *v, pos4 *v2, float *len, sh_vec3 *direction) {

	sh_vec3 vec = {v2->x - v->x, v2->y - v->y, v2->z - v->z};

	*len = sh_vec3_length(&vec);

	direction->x = vec.x/(*len);
	direction->y = vec.y/(*len);
	direction->z = vec.z/(*len);

}





mat4 lookat(sh_vec3 eye_pos, sh_vec3 look_point, sh_vec3 up_direction) {
	
	sh_vec3 forward = sh_vec3_sub(eye_pos, look_point);
	sh_vec3_normalize_ref(&forward);

	sh_vec3 left = sh_vec3_cross(&up_direction, &forward);
	sh_vec3_normalize_ref(&left);

	sh_vec3 up = sh_vec3_cross(&forward, &left);

	mat4 cam = {
		left.x, left.y, left.z, -left.x*eye_pos.x - left.y*eye_pos.y - left.z*eye_pos.z,
		up.x, up.y, up.z,         -up.x*eye_pos.x -   up.y*eye_pos.y -   up.z*eye_pos.z,
		forward.x, forward.y, forward.z, -forward.x*eye_pos.x - forward.y*eye_pos.y - forward.z*eye_pos.z,
		0, 0, 0, 1,
	};


	return cam;
}


mat4 perspective(float fov, float aspect, float pnear, float pfar) {

    float top = (float)tan((fov / 2.0f) * 3.1459f/180.0f) * pnear;
    float right = top * aspect;

	mat4 m = {
		pnear / right, 0, 0, 0,
        0, pnear / top, 0, 0,
        0, 0, -(pfar + pnear) / (pfar - pnear), -2 * pfar * pnear / (pfar - pnear),
        0, 0, -1,0 };

    return m;
}

mat4 identity_matrix() {
	return {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
}

void mul_mat4_x_rot(mat4 *m, float x_angle) {
	float ang_rad = x_angle * 3.14159f/180.0f;
	float c = cosf(ang_rad);
	float s = sinf(ang_rad);
	float row_two[4] = {0};
	float row_three[4] = {0};

	for(int i = 0; i < 4; i++) {
		row_two[i] = c*m->m[1*4 + i] - s*m->m[2*4 + i];
		row_three[i] = s*m->m[1*4 + i] + c*m->m[2*4 + i];
	}

	for(int i = 0; i < 4; i++) {
		m->m[1*4 + i] = row_two[i];
		m->m[2*4 + i] = row_three[i];
	}

}


void mul_mat4_y_rot(mat4 *m, float x_angle) {
	float ang_rad = x_angle * 3.14159f/180.0f;
	float c = cosf(ang_rad);
	float s = sinf(ang_rad);
	float row_two[4] = {0};
	float row_three[4] = {0};

	for(int i = 0; i < 4; i++) {
		row_two[i] =    c*m->m[0*4 + i] + s*m->m[2*4 + i];
		row_three[i] = -s*m->m[0*4 + i] + c*m->m[2*4 + i];
	}

	for(int i = 0; i < 4; i++) {
		m->m[0*4 + i] = row_two[i];
		m->m[2*4 + i] = row_three[i];
	}

}

void mul_mat4_z_rot(mat4 *m, float x_angle) {
	float ang_rad = x_angle * 3.14159f/180.0f;
	float c = cosf(ang_rad);
	float s = sinf(ang_rad);
	float row_two[4] = {0};
	float row_three[4] = {0};

	for(int i = 0; i < 4; i++) {
		row_two[i] =    c*m->m[0*4 + i] - s*m->m[1*4 + i];
		row_three[i] =  s*m->m[0*4 + i] + c*m->m[1*4 + i];
	}

	for(int i = 0; i < 4; i++) {
		m->m[0*4 + i] = row_two[i];
		m->m[1*4 + i] = row_three[i];
	}

}

mat4 make_mat4_translate(float x, float y, float z) {
	mat4 m = identity_matrix();

	m.a.w = x;
	m.b.w = y;
	m.c.w = z;

	return m;
}


mat4 make_mat4_y_rot(float x_angle) {
	float ang_rad = x_angle * 3.14159f/180.0f;
	float c = cosf(ang_rad);
	float s = sinf(ang_rad);

	return {
		c, 0, s, 0,
		0, 1, 0, 0,
		-s, 0, c, 0,
		0, 0, 0, 1
	};
}

mat4 make_mat4_x_rot(float x_angle) {
	float ang_rad = x_angle * 3.14159f/180.0f;
	float c = cosf(ang_rad);
	float s = sinf(ang_rad);

	return {
		1, 0,  0, 0,
		0, c, -s, 0,
		0, s,  c, 0,
		0, 0,  0, 1
	};
}

mat4 make_mat4_z_rot(float x_angle) {
	float ang_rad = x_angle * 3.14159f/180.0f;
	float c = cosf(ang_rad);
	float s = sinf(ang_rad);

	return {
		c, -s,  0, 0,
		s,  c, 0, 0,
		0, 0,  1, 0,
		0, 0,  0, 1
	};
}




void mul_mat4_mat4(mat4 *m, mat4 *m2) {

	mat4 mm = {0};
	
	mm.a.x = m->a.x * m2->a.x + m->a.y * m2->b.x + m->a.z * m2->c.x + m->a.w * m2->d.x;
	mm.a.y = m->a.x * m2->a.y + m->a.y * m2->b.y + m->a.z * m2->c.y + m->a.w * m2->d.y;
	mm.a.z = m->a.x * m2->a.z + m->a.y * m2->b.z + m->a.z * m2->c.z + m->a.w * m2->d.z;
	mm.a.w = m->a.x * m2->a.w + m->a.y * m2->b.w + m->a.z * m2->c.w + m->a.w * m2->d.w;

	mm.b.x = m->b.x * m2->a.x + m->b.y * m2->b.x + m->b.z * m2->c.x + m->b.w * m2->d.x;
	mm.b.y = m->b.x * m2->a.y + m->b.y * m2->b.y + m->b.z * m2->c.y + m->b.w * m2->d.y;
	mm.b.z = m->b.x * m2->a.z + m->b.y * m2->b.z + m->b.z * m2->c.z + m->b.w * m2->d.z;
	mm.b.w = m->b.x * m2->a.w + m->b.y * m2->b.w + m->b.z * m2->c.w + m->b.w * m2->d.w;

	mm.c.x = m->c.x * m2->a.x + m->c.y * m2->b.x + m->c.z * m2->c.x + m->c.w * m2->d.x;
	mm.c.y = m->c.x * m2->a.y + m->c.y * m2->b.y + m->c.z * m2->c.y + m->c.w * m2->d.y;
	mm.c.z = m->c.x * m2->a.z + m->c.y * m2->b.z + m->c.z * m2->c.z + m->c.w * m2->d.z;
	mm.c.w = m->c.x * m2->a.w + m->c.y * m2->b.w + m->c.z * m2->c.w + m->c.w * m2->d.w;

	mm.d.x = m->d.x * m2->a.x + m->d.y * m2->b.x + m->d.z * m2->c.x + m->d.w * m2->d.x;
	mm.d.y = m->d.x * m2->a.y + m->d.y * m2->b.y + m->d.z * m2->c.y + m->d.w * m2->d.y;
	mm.d.z = m->d.x * m2->a.z + m->d.y * m2->b.z + m->d.z * m2->c.z + m->d.w * m2->d.z;
	mm.d.w = m->d.x * m2->a.w + m->d.y * m2->b.w + m->d.z * m2->c.w + m->d.w * m2->d.w;

	memcpy(m->m, mm.m, sizeof(float)*16);

}


pos4 mul_mat4_pos4(pos4 *p, mat4 *current_matrix) {

	pos4 p_update = {0, 0, 0, 1};

	p_update._d[0] = current_matrix->a.x*p->x + current_matrix->a.y*p->y + current_matrix->a.z*p->z + current_matrix->a.w*p->w;
	p_update._d[1] = current_matrix->b.x*p->x + current_matrix->b.y*p->y + current_matrix->b.z*p->z + current_matrix->b.w*p->w;
	p_update._d[2] = current_matrix->c.x*p->x + current_matrix->c.y*p->y + current_matrix->c.z*p->z + current_matrix->c.w*p->w;
	p_update._d[3] = current_matrix->d.x*p->x + current_matrix->d.y*p->y + current_matrix->d.z*p->z + current_matrix->d.w*p->w;

	return p_update;
}


float determinate(mat4 *mat) {
    return (+mat->a.x * mat->b.y  * mat->c.z * mat->d.w + mat->a.x * mat->b.z * mat->c.w * mat->d.y + mat->a.x * mat->b.w * mat->c.y * mat->d.z
            + mat->a.y * mat->b.x * mat->c.w * mat->d.z + mat->a.y * mat->b.z * mat->c.x * mat->d.w + mat->a.y * mat->b.w * mat->c.z * mat->d.x
            + mat->a.z * mat->b.x * mat->c.y * mat->d.w + mat->a.z * mat->b.y * mat->c.w * mat->d.x + mat->a.z * mat->b.w * mat->c.x * mat->d.y
            + mat->a.w * mat->b.x * mat->c.z * mat->d.y + mat->a.w * mat->b.y * mat->c.x * mat->d.z + mat->a.w * mat->b.z * mat->c.y * mat->d.x
            - mat->a.x * mat->b.y * mat->c.w * mat->d.z - mat->a.x * mat->b.z * mat->c.y * mat->d.w - mat->a.x * mat->b.w * mat->c.z * mat->d.y
            - mat->a.y * mat->b.x * mat->c.z * mat->d.w - mat->a.y * mat->b.z * mat->c.w * mat->d.x - mat->a.y * mat->b.w * mat->c.x * mat->d.z
            - mat->a.z * mat->b.x * mat->c.w * mat->d.y - mat->a.z * mat->b.y * mat->c.x * mat->d.w - mat->a.z * mat->b.w * mat->c.y * mat->d.x
            - mat->a.w * mat->b.x * mat->c.y * mat->d.z - mat->a.w * mat->b.y * mat->c.z * mat->d.x - mat->a.w * mat->b.z * mat->c.x * mat->d.y);
}


//Im sorry, this was the only way,
mat4 inverse(mat4 *mat) {
    float det = determinate(mat);
    return {
               (mat->b.y * mat->c.z * mat->d.w + mat->b.z * mat->c.w * mat->d.y + mat->b.w * mat->c.y * mat->d.z - mat->b.y * mat->c.w * mat->d.z - mat->b.z * mat->c.y * mat->d.w - mat->b.w * mat->c.z * mat->d.y) / det,
               (mat->a.y * mat->c.w * mat->d.z + mat->a.z * mat->c.y * mat->d.w + mat->a.w * mat->c.z * mat->d.y - mat->a.y * mat->c.z * mat->d.w - mat->a.z * mat->c.w * mat->d.y - mat->a.w * mat->c.y * mat->d.z) / det,
               (mat->a.y * mat->b.z * mat->d.w + mat->a.z * mat->b.w * mat->d.y + mat->a.w * mat->b.y * mat->d.z - mat->a.y * mat->b.w * mat->d.z - mat->a.z * mat->b.y * mat->d.w - mat->a.w * mat->b.z * mat->d.y) / det,
               (mat->a.y * mat->b.w * mat->c.z + mat->a.z * mat->b.y * mat->c.w + mat->a.w * mat->b.z * mat->c.y - mat->a.y * mat->b.z * mat->c.w - mat->a.z * mat->b.w * mat->c.y - mat->a.w * mat->b.y * mat->c.z) / det,
               (mat->b.x * mat->c.w * mat->d.z + mat->b.z * mat->c.x * mat->d.w + mat->b.w * mat->c.z * mat->d.x - mat->b.x * mat->c.z * mat->d.w - mat->b.z * mat->c.w * mat->d.x - mat->b.w * mat->c.x * mat->d.z) / det,
               (mat->a.x * mat->c.z * mat->d.w + mat->a.z * mat->c.w * mat->d.x + mat->a.w * mat->c.x * mat->d.z - mat->a.x * mat->c.w * mat->d.z - mat->a.z * mat->c.x * mat->d.w - mat->a.w * mat->c.z * mat->d.x) / det,
               (mat->a.x * mat->b.w * mat->d.z + mat->a.z * mat->b.x * mat->d.w + mat->a.w * mat->b.z * mat->d.x - mat->a.x * mat->b.z * mat->d.w - mat->a.z * mat->b.w * mat->d.x - mat->a.w * mat->b.x * mat->d.z) / det,
               (mat->a.x * mat->b.z * mat->c.w + mat->a.z * mat->b.w * mat->c.x + mat->a.w * mat->b.x * mat->c.z - mat->a.x * mat->b.w * mat->c.z - mat->a.z * mat->b.x * mat->c.w - mat->a.w * mat->b.z * mat->c.x) / det,
               (mat->b.x * mat->c.y * mat->d.w + mat->b.y * mat->c.w * mat->d.x + mat->b.w * mat->c.x * mat->d.y - mat->b.x * mat->c.w * mat->d.y - mat->b.y * mat->c.x * mat->d.w - mat->b.w * mat->c.y * mat->d.x) / det,
               (mat->a.x * mat->c.w * mat->d.y + mat->a.y * mat->c.x * mat->d.w + mat->a.w * mat->c.y * mat->d.x - mat->a.x * mat->c.y * mat->d.w - mat->a.y * mat->c.w * mat->d.x - mat->a.w * mat->c.x * mat->d.y) / det,
               (mat->a.x * mat->b.y * mat->d.w + mat->a.y * mat->b.w * mat->d.x + mat->a.w * mat->b.x * mat->d.y - mat->a.x * mat->b.w * mat->d.y - mat->a.y * mat->b.x * mat->d.w - mat->a.w * mat->b.y * mat->d.x) / det,
               (mat->a.x * mat->b.w * mat->c.y + mat->a.y * mat->b.x * mat->c.w + mat->a.w * mat->b.y * mat->c.x - mat->a.x * mat->b.y * mat->c.w - mat->a.y * mat->b.w * mat->c.x - mat->a.w * mat->b.x * mat->c.y) / det,
               (mat->b.x * mat->c.z * mat->d.y + mat->b.y * mat->c.x * mat->d.z + mat->b.z * mat->c.y * mat->d.x - mat->b.x * mat->c.y * mat->d.z - mat->b.y * mat->c.z * mat->d.x - mat->b.z * mat->c.x * mat->d.y) / det,
               (mat->a.x * mat->c.y * mat->d.z + mat->a.y * mat->c.z * mat->d.x + mat->a.z * mat->c.x * mat->d.y - mat->a.x * mat->c.z * mat->d.y - mat->a.y * mat->c.x * mat->d.z - mat->a.z * mat->c.y * mat->d.x) / det,
               (mat->a.x * mat->b.z * mat->d.y + mat->a.y * mat->b.x * mat->d.z + mat->a.z * mat->b.y * mat->d.x - mat->a.x * mat->b.y * mat->d.z - mat->a.y * mat->b.z * mat->d.x - mat->a.z * mat->b.x * mat->d.y) / det,
			   (mat->a.x * mat->b.y * mat->c.z + mat->a.y * mat->b.z * mat->c.x + mat->a.z * mat->b.x * mat->c.y - mat->a.x * mat->b.z * mat->c.y - mat->a.y * mat->b.x * mat->c.z - mat->a.z * mat->b.y * mat->c.x) / det};
}


float sh_pos4_pos4_length(pos4 *p, pos4 *p2) {

	f32 x = p2->x - p->x;
	f32 y = p2->y - p->y;
	f32 z = p2->z - p->z;

	return (float)sqrt( x*x + y*y + z*z );
}


float sh_pos4_length(pos4 *p) {
	return (float)sqrt( p->x*p->x + p->y*p->y + p->z*p->z );
}

void sh_pos4_normalize_ref(pos4 *p) {
	float len = sh_pos4_length(p);

	p->x /= len;
	p->y /= len;
	p->z /= len;
}



#endif
