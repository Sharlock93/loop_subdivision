#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

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

	if(strncmp(t->start, str, t->size) == 0) return 1;
	return 0;
}

// go through the different tokens of the file until you find the "Vertices" token
// if its validly formated file, i.e: it has a "Vertices = #of vertices"
// then we move forward
int get_vertex_count(ReaderContext *ctx) {

	Token tok = get_next_token(ctx);
	while(tok.size != -1 && tok.start != NULL) {
		if(is_token(&tok, (char*)"Vertices")) {
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
		if(is_token(&tok, (char*)"Faces")) {
			get_next_token(ctx);
			int count = (int)read_number(ctx);
			return count;
		}
		tok = get_next_token(ctx);
	}

	return -1;
}

// reading a .face file means we already have
// all the needed faces and vertices without duplicates
void read_vert_face(ReaderContext *ctx, Point3f **verts, int **faces,  int *vert_size, int *face_size) {

	// we find the number of vertices and faces
	int vert_number = (int)get_vertex_count(ctx);
	int face_number = (int)get_face_count(ctx);

	if(vert_number == -1 || vert_number == -1 ) {
		printf("Couldn't find vertex or face count, exiting...");
		exit(1);
	}

	*verts = (Point3f*)calloc(vert_number, sizeof(Point3f));
	*faces = (int*)calloc(face_number*3, sizeof(int));

	// for a valid .face file, the vertices come first, 
	// a line of vertex info is as follows
	// Vertex <vert_id> <float_x> <float_y> <float_z>
	Token tok = {0};
	for(int i = 0; i < vert_number; i++) {
		tok = get_next_token(ctx);

		while(!is_token(&tok, (char*)"Vertex")) {
			tok = get_next_token(ctx);
		}

		int vert_index = (int)read_number(ctx);

		(*verts)[vert_index].x = read_number(ctx);
		(*verts)[vert_index].y = read_number(ctx);
		(*verts)[vert_index].z = read_number(ctx);
	}

	// Face values follow and have the following format
	// Face <face_id> <vert_id p0> <vert_id p1> <vert_id p2>
	for(int i = 0; i < face_number; i++) {
		tok = get_next_token(ctx);

		while(!is_token(&tok, (char*)"Face")) {
			tok = get_next_token(ctx);
		}


		int index = (int)read_number(ctx);

		(*faces)[index*3 + 0] = (int)read_number(ctx);
		(*faces)[index*3 + 1] = (int)read_number(ctx);
		(*faces)[index*3 + 2] = (int)read_number(ctx);
	}

	*vert_size = vert_number;
	*face_size = face_number;

}

Mesh build_mesh(Point3f *verts, int vert_size, int *faces, int face_size) {
	
	Mesh m = {0};

	m.faces = (TriangleRef*)faces;
	m.vertices = verts;

	m.fde = (int *)calloc(vert_size, sizeof(int));
	memset(m.fde, -1, vert_size*sizeof(int));
	int counter = 0; 

	// we know how many vertices we have so we stop once counter becomes that number
	// given that its "edgeFrom" the first time we see an edge it means it will be unique
	// and means its the First Directed Edge too
	for(int i = 0; i < face_size*3 && counter < vert_size; i++) {
		if(m.edges[i] == counter) {
			m.fde[counter] = i;
			counter++;
		}
	}

	m.vert_number = vert_size;
	m.face_size = face_size;

	return m;
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
	int     *faces = NULL;

	int vert_size = 0;
	int face_size = 0;


	read_vert_face(&read_context, &verts, &faces, &vert_size, &face_size);

	Mesh m = build_mesh(verts, vert_size, faces, face_size);

	build_half_edges(&m);

	m.object_name = get_object_name(file_name);

	print_mesh_assignment_order(&m, (char*)".diredge");

	test_manifold(&m);
	print_genus(&m);

	return 0;
}
