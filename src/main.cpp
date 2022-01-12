#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <assert.h>

#include "sh_tools.cpp"
#include "gl_load_funcs.h"
#include "sh_simple_vec_math.cpp"
#include "opengl_window.cpp"

#define SH_DO_FACE_AND_DIREDGE

#include "common.cpp"

typedef struct Token {
	char *start;
	int size;
} Token;

Token get_next_token(ReaderContext *ctx) {

	while(isspace(ctx->file[0])) {
		ctx->col++;
		if(ctx->file[0] == '\n') {
			ctx->row++;
			ctx->col = 0;
		}
		ctx->file++;
	};

	char *file = ctx->file;

	Token new_tok = {ctx->file, 0};
	switch(ctx->file[0]) {
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z': case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v':
		case 'w': case 'x': case 'y': case 'z': {

			while(isalpha(file[0]) || file[0] == '_') file++;
			size_t str_size = file - new_tok.start;
			ctx->file = file;
			new_tok.size = (int)str_size;

		} break;

		case '-': case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7': case '8': case '9': {

			if(file[0] == '-' && !isdigit(file[1])) {
				new_tok.size = 1;
				ctx->file++;
				return new_tok;
			}

			if(isdigit(file[0]) || file[0] == '.' || file[0] == '-') {

				if(file[0] == '-') {
					file++;
					ctx->col++;
				}

				while(isdigit(file[0])) {
					file++;
					ctx->col++;
				}

				if(file[0] == '.') {

					file++;
					ctx->col++;

					while(isdigit(file[0])) {
						file++;
						ctx->col++;
					}
				}
			}

			size_t str_size = file - new_tok.start;
			ctx->file = file;
			new_tok.size = (int)str_size;

		} break;


		case '_':
		case '.':
		case '\\':
		case '/':
		case ':':
		case '=':
		case '#': {
			new_tok.size = 1;
			ctx->file++;
		} break;

		case '\0':
		case EOF: {
			new_tok.start = NULL;
			new_tok.size = -1;
		} break;

		default: {
			printf("Unrecognized token while parsing %c, this is actually bad.\n", file[0]);
			exit(1);
		} break;

	}

	return new_tok;
}

void print_token(Token *tok) {
	printf("%.*s\n", tok->size, tok->start);
}

int is_token(Token *t, char *str) {

	for(int i = 0; i < t->size; i++) {
		t->start[i] = tolower(t->start[i]);
	}

	if(strncmp(t->start, str, t->size) == 0) return 1;

	return 0;
}

// go through the different tokens of the file until you find the "Vertices" token
// if its validly formated file, i.e: it has a "Vertices = #of vertices"
// then we move forward
int get_vertex_count(ReaderContext *ctx) {

	Token tok = get_next_token(ctx);
	while(tok.size != -1 && tok.start != NULL) {
		if(is_token(&tok, (char*)"vertices")) {
			tok = get_next_token(ctx);
			int count = (int)read_number(ctx);
			return count;
		}


		tok = get_next_token(ctx);
	}

	return -1;
}

// we look for the word/token "Faces" and read the number
// after the '=' token
int get_face_count(ReaderContext *ctx) {

	Token tok = get_next_token(ctx);
	while(tok.size != -1 && tok.start != NULL) {
		if(is_token(&tok, (char*)"faces")) {
			get_next_token(ctx);
			int count = (int)read_number(ctx);
			return count;
		}
		tok = get_next_token(ctx);
	}

	return -1;
}

// reading a .diredgenormal file means we already have
// all the needed faces, vertices, and normals without duplicates
// we assume the file has data for vertex, normal, firstdirectedge, face, otherhalf and in that specific order
// if this isn't followed our program will crash
Mesh build_mesh(ReaderContext *ctx) {
	Mesh m = {0};

	int *faces;

	// we find the number of vertices and faces
	m.vert_number = (int)get_vertex_count(ctx);
	m.face_size = (int)get_face_count(ctx);

	if(m.vert_number == -1 || m.vert_number == -1 ) {
		printf("Couldn't find vertex or face count, exiting...");
		exit(1);
	}

	m.vertices = (Point3f*)calloc(m.vert_number, sizeof(Point3f));
	m.normals = (Point3f*)calloc(m.vert_number, sizeof(Point3f));
	m.faces = (TriangleRef*)calloc(m.face_size*3, sizeof(int));
	m.fde = (int*)calloc(m.vert_number, sizeof(int));
	m.other_half = (int*)calloc(m.face_size*3, sizeof(int));


	// for a valid .direedgenormal file, the vertices come first, 
	// a line of vertex info is as follows
	// Vertex <vert_id> <float_x> <float_y> <float_z>
	Token tok = {0};
	for(int i = 0; i < m.vert_number; i++) {
		tok = get_next_token(ctx);

		while(!is_token(&tok, (char*)"vertex")) {
			tok = get_next_token(ctx);
		}

		int vert_index = (int)read_number(ctx);

		m.vertices[vert_index].x = read_number(ctx);
		m.vertices[vert_index].y = read_number(ctx);
		m.vertices[vert_index].z = read_number(ctx);
	}

	// parse the normals
	tok = {0};
	for(int i = 0; i < m.vert_number; i++) {
		tok = get_next_token(ctx);

		while(!is_token(&tok, (char*)"normal")) {
			tok = get_next_token(ctx);
		}

		int index = (int)read_number(ctx);

		m.normals[index].x = read_number(ctx);
		m.normals[index].y = read_number(ctx);
		m.normals[index].z = read_number(ctx);
	}

	tok = {0};
	for(int i = 0; i < m.vert_number; i++) {
		tok = get_next_token(ctx);

		while(!is_token(&tok, (char*)"firstdirectededge")) {
			tok = get_next_token(ctx);
		}

		int index = (int)read_number(ctx);

		m.fde[index] = (int)read_number(ctx);
	}


	// Face values follow and have the following format
	// Face <face_id> <vert_id p0> <vert_id p1> <vert_id p2>
	faces = (int*)m.faces;
	for(int i = 0; i < m.face_size; i++) {
		tok = get_next_token(ctx);

		while(!is_token(&tok, (char*)"face")) {
			tok = get_next_token(ctx);
		}

		int index = (int)read_number(ctx);

		faces[index*3 + 0] = (int)read_number(ctx);
		faces[index*3 + 1] = (int)read_number(ctx);
		faces[index*3 + 2] = (int)read_number(ctx);
	}


	tok = {0};
	for(int i = 0; i < m.face_size*3; i++) {
		tok = get_next_token(ctx);

		while(!is_token(&tok, (char*)"otherhalf")) {
			tok = get_next_token(ctx);
		}

		int index = (int)read_number(ctx);

		m.other_half[index] = (int)read_number(ctx);
	}


	return m;
}


void setup(void) {

	gl_ctx = { 720, 1366, 0, 0, 0};
	gl_ctx.window_name = (char*)"editor" ;

	HWND wnd = sh_window_setup();

	glClearColor(61.0f/255.0f, 41.0f/255.0f, 94.0f/255.0f, 1);
}

u32 index_vbo = 0;

void render_mesh(Mesh *m) {
	glBindVertexArray(gl_ctx.vao);
	if(gl_ctx.vbo == 0) {
		glGenBuffers(1, &gl_ctx.vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_ctx.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f)*m->vert_number*2, NULL, GL_STATIC_DRAW);

	if(index_vbo == 0) {
		glGenBuffers(1, &index_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_ctx.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Point3f)*m->vert_number, m->vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Point3f)*m->vert_number, sizeof(Point3f)*m->vert_number, m->normals);

	glEnableVertexAttribArray(VP_LOC);
	glEnableVertexAttribArray(NORMAL_LOC);

	glVertexAttribPointer(VP_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(Point3f)*m->vert_number));

	glUniform1f(USE_LIGHT, 1);

	glDisableVertexAttribArray(COLOR_LOC);
	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	float c[4] = {0, 1, 0, 1};
	glUniform4fv(UNIFORM_COLOR_LOC, 1, c);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*m->face_size*3, m->faces, GL_STATIC_DRAW);


	glDrawElements(GL_TRIANGLES, m->face_size*3, GL_UNSIGNED_INT, 0);


}


// add a new vertex based on the mid point of the vert_index_1 and vert_index_2
// this assumes the mesh vertices array has enough space
int make_new_vert(Mesh *m, int vert_1, int vert_2, int vert_3, int vert_4) {

	Point3f *v1 = m->vertices + vert_1;
	Point3f *v2 = m->vertices + vert_2;
	Point3f *v3 = m->vertices + vert_3;
	Point3f *v4 = m->vertices + vert_4;

	int new_vert_index = m->vert_number;

	m->vertices[new_vert_index].x = v1->x*3.0f/8.0f + v2->x*3.0f/8.0f + v3->x/8.0f + v4->x/8.0f;
	m->vertices[new_vert_index].y = v1->y*3.0f/8.0f + v2->y*3.0f/8.0f + v3->y/8.0f + v4->y/8.0f;
	m->vertices[new_vert_index].z = v1->z*3.0f/8.0f + v2->z*3.0f/8.0f + v3->z/8.0f + v4->z/8.0f;

	// m->vertices[new_vert_index].y = v1->y;
	// m->vertices[new_vert_index].z = v1->z;

	m->vert_number++;

	return new_vert_index;
}


int find_edge_index(Edge *edges,  int edge_count, int edge_start, int edge_end) {

	for(int i = 0; i < edge_count; i++) {
		int es = edges[i].p0;
		int ee = edges[i].p1;

		if((es == edge_start || ee == edge_start) && (es == edge_end || ee == edge_end)) {
			return i;
		}
	}

	return -1;
}

void build_unique_edges(Edge *e, Mesh *m, int unique_edge_count) {
	int edge_counter = 0;
	for(int i = 0; i < m->face_size; i++) {
		TriangleRef *f = m->faces + i;

		for(int j = 0; j < 3; j++) {
			int es = f->vert_ids[j];
			int ed = f->vert_ids[(j+1)%3];

			int found = 0;
			for(int k = 0; k < edge_counter; k++) {
				Edge *ee = e + k;
				if((ee->p0 == es || ee->p1 == es ) && (ee->p0 == ed || ee->p1 == ed)) {
					found = 1;
					break;
				}
			}

			if(found == 0) {
				e[edge_counter].p0 = es;
				e[edge_counter].p1 = ed;
				edge_counter++;
			}

			if(unique_edge_count == edge_counter) return;

		}

	}
}

void add_new_face(Mesh *m, int v1, int v2, int v3) {
	int face_index = m->face_size;
	m->faces[face_index].p1 = v1;
	m->faces[face_index].p2 = v2;
	m->faces[face_index].p3 = v3;
	m->face_size++;
}

// void rebuild_half_edges(Mesh *m) {

// 	for(int i = 0; i < 0; i++) {

// 	}


// }
//
//

void build_fde(Mesh *m) {
	m->fde = (int *)calloc(m->vert_number, sizeof(int));

	memset(m->fde, -1, m->vert_number*sizeof(int));

	int counter = 0; 

	int *e = (int*)m->edges;

	// we know how many vertices we have so we stop once counter becomes that number
	// given that its "edgeFrom" the first time we see an edge it means it will be unique
	// and means its the First Directed Edge too
	for(int i = 0; i < m->face_size*3 && counter < m->vert_number; i++) {
		if(e[i] == counter) {
			m->fde[counter] = i;
			i = counter;
			counter++;
		}
	}

}

int find_half_edge_for_end(Mesh *m, int v_s, int v_e) {

	int e_fde = m->fde[v_s];
	int e_other_half_edge = m->other_half[e_fde];


	int f_other_edge = e_other_half_edge/3;
	int vf_other_edge_index = e_other_half_edge%3;

	int v_other_end_vertex = m->faces[f_other_edge].vert_ids[vf_other_edge_index];

	int e_edge_index = -1;

	if(v_e == v_other_end_vertex) {
		e_edge_index = e_fde;
	}

	e_other_half_edge = e_fde;
	while(v_e != v_other_end_vertex) {
		e_other_half_edge = m->other_half[e_other_half_edge];

		f_other_edge = e_other_half_edge/3;
		e_other_half_edge = f_other_edge*3 + (e_other_half_edge + 1)%3;

		vf_other_edge_index = (e_other_half_edge+1)%3;
		v_other_end_vertex = m->faces[f_other_edge].vert_ids[vf_other_edge_index];
		e_edge_index = e_other_half_edge;
	}

	// int half_edge_index = other_end_index;

	// int *edges = (int*)m->edges;

	// while(edges[half_edge_index] != v_e) {

	// }

	return e_edge_index;
}

void get_opposite_vert_ids(Mesh *m, int edge, int *o_v1, int *o_v2) {

	int next = (edge/3)*3 + (edge+1)%3;
	int other_half_next  = m->other_half[edge];
	int n = other_half_next;
	other_half_next = (other_half_next/3)*3;
	other_half_next = other_half_next + (n+1)%3;

	int next_end_vertex = m->faces[next/3].vert_ids[((next%3) + 1)%3];
	int other_half_next_end_vertex = m->faces[other_half_next/3].vert_ids[((other_half_next%3) + 1)%3];

	*o_v1 = next_end_vertex;
	*o_v2 = other_half_next_end_vertex;
}

void build_one_ring(Mesh *m, int vert_index, int *one_ring, int one_ring_size) {

	int fde = m->fde[vert_index];

	int counter = 0;
	while(counter != one_ring_size) {
		int face_id = fde/3;
		int next_vert = ((fde%3) + 1)%3;
		one_ring[counter] = m->faces[face_id].vert_ids[next_vert];
		counter++;

		fde = m->other_half[fde];
		fde = (fde/3)*3 + (fde + 1)%3;
	}
	
}

void sub_divide(Mesh *m) {

	int new_faces = m->face_size*4 - m->face_size;
	int new_edges = (new_faces*3)/2 - (m->face_size*3)/2;
	int new_verts = (m->face_size*3)/2;

	m->faces = (TriangleRef*)realloc(m->faces, (new_faces + m->face_size)*sizeof(TriangleRef));
	m->vertices = (Point3f*)realloc(m->vertices, (m->vert_number + new_verts)*sizeof(Point3f));
	m->normals = (Point3f*)realloc(m->normals, (m->vert_number + new_verts)*sizeof(Point3f) );
	// m->fde = (int*)realloc(m->fde, sizeof(int)*(m->vert_number + new_verts));
	// m->other_half = (int*)realloc(m->other_half, sizeof(int)*(m->face_size*3 + new_edges));

	int *edge_has_been_dived = (int*)calloc(sizeof(int), (m->face_size*3)/2); //
	int edge_count = (m->face_size*3)/2;
	int current_face_size = m->face_size;

	Edge *unique_edges = (Edge*)calloc(sizeof(Edge), edge_count);
	build_unique_edges(unique_edges, m, edge_count);

	// unfortunately we have to back up all the vertices because we will change them as we go
	Point3f *temp_verts = (Point3f *)calloc(m->vert_number, sizeof(Point3f));
	memcpy(temp_verts, m->vertices, sizeof(Point3f)*m->vert_number);
	int old_vert_count = m->vert_number;

	for(int i = 0; i < old_vert_count; i++ ) {

		int one_ring_count = count_one_ring_vertex(m, i);
		int *one_ring_vert_index = (int*)calloc(one_ring_count, sizeof(int));
		build_one_ring(m, i, one_ring_vert_index, one_ring_count);

		float beta = 0;
		if(one_ring_count == 3) {
			beta = 3.0f/16.f;
		} else {
			beta = 3.0f/(8.0f*one_ring_count);
		}

		float center_scale = 1.0f - one_ring_count*beta;

		temp_verts[i].x *= center_scale;
		temp_verts[i].y *= center_scale;
		temp_verts[i].z *= center_scale;

		for(int j = 0; j < one_ring_count; j++) {
			temp_verts[i].x += m->vertices[one_ring_vert_index[j]].x*beta;
			temp_verts[i].y += m->vertices[one_ring_vert_index[j]].y*beta;
			temp_verts[i].z += m->vertices[one_ring_vert_index[j]].z*beta;
		}

		free(one_ring_vert_index);
	}



	for(int i = 0; i < current_face_size; i++) {
		TriangleRef *face = m->faces + i;

		int edge1_index = find_edge_index(unique_edges, edge_count,  face->p1, face->p2);
		int edge2_index = find_edge_index(unique_edges, edge_count, face->p2, face->p3) ;
		int edge3_index = find_edge_index(unique_edges, edge_count, face->p3, face->p1) ;

		int new_v1 = edge_has_been_dived[edge1_index];
		int new_v2 = edge_has_been_dived[edge2_index];
		int new_v3 = edge_has_been_dived[edge3_index];

		if(edge1_index == -1 || edge2_index == -1 || edge3_index == -1) {
			printf("This should not happen\n");
			exit(1);
		}
		
		if(new_v1 == 0) {
			int o1 = 0;
			int o2 = 0;
			int half_edge_index = find_half_edge_for_end(m, face->p1, face->p2);
			get_opposite_vert_ids(m, half_edge_index, &o1, &o2);

			new_v1 = make_new_vert(m, face->p1, face->p2, o1, o2);
			edge_has_been_dived[edge1_index] = new_v1;
		}

		if(new_v2 == 0) {

			int o1 = 0;
			int o2 = 0;
			int half_edge_index = find_half_edge_for_end(m, face->p2, face->p3);
			get_opposite_vert_ids(m, half_edge_index, &o1, &o2);

			new_v2 = make_new_vert(m, face->p2, face->p3, o1, o2);
			edge_has_been_dived[edge2_index] = new_v2;
		}

		if(new_v3 == 0) {

			int o1 = 0;
			int o2 = 0;
			int half_edge_index = find_half_edge_for_end(m, face->p3, face->p1);
			get_opposite_vert_ids(m, half_edge_index, &o1, &o2);

			new_v3 = make_new_vert(m, face->p3, face->p1, o1, o2);

			edge_has_been_dived[edge3_index] = new_v3;
		}

		add_new_face(m, new_v1, face->p2, new_v2);
		add_new_face(m, new_v2, face->p3, new_v3);
		add_new_face(m, new_v1, new_v2, new_v3);


		face->p2 = new_v1;
		face->p3 = new_v3;
	}

	for(int i = 0; i < old_vert_count; i++) {
		m->vertices[i] = temp_verts[i];
	}

	free(m->fde);
	free(m->other_half);

	build_half_edges(m);
	build_fde(m);

	free(edge_has_been_dived);
	free(unique_edges);
	free(temp_verts);
	
}



int main(int argc, char ** argv) {

	if(argc < 2) {
		printf("usage: %s <path_to_tri_object>\n", argv[0]);
		exit(1);
	}

	char *file_name = argv[1];


	ReaderContext read_context = {0};
	read_context.file = read_entire_file(file_name, NULL);

	if(read_context.file == NULL) {
		printf("Failed to open object file  %s\n", argv[1]);
	}

	Point3f *verts = NULL;
	Point3f *normals = NULL;
	int     *faces = NULL;

	int vert_size = 0;
	int face_size = 0;


	Mesh m = build_mesh(&read_context);


	m.object_name = get_object_name(file_name);


	build_half_edges(&m);
	build_fde(&m);

	sub_divide(&m);

	print_mesh_assignment_order(&m, (char*)".diredgednormal");

	setup();

	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while(!gl_ctx.should_close) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		handle_events();
		if(gl_ctx.keyboard[VK_UP].pressed_once) {
			sub_divide(&m);
		}
		render_mesh(&m);
		update_time();
		SwapBuffers(gl_ctx.hdc);
	}


	
	// sub_divide_face(&m, 0);
	// test_manifold(&m);
	// print_genus(&m);

	return 0;
}
