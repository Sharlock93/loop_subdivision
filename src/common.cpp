//structs & types
typedef struct Point3f {
	float x, y, z;
} Point3f;

typedef struct ReaderContext {
	int row; 
	int col;
	char *file;
} ReaderContext;

typedef union TriangleRef {
	struct {
		int p1;
		int p2;
		int p3;
	};

	int vert_ids[3];
} TriangleRef;

typedef union Edge {
	struct {
		int p0;
		int p1;
	};
	int pts[2];
} Edge;


typedef struct Mesh {
	//faces and edges, assuming "edgeFrom", match up, no need to
	//allocate extra memory
	union {
		TriangleRef *faces;
		Edge *edges;
	};

	//other half will have the same size as edges, thus 3*face_size at most
	int *other_half;
	int face_size;

	// vertices and fde have the same size
	Point3f *vertices;
	Point3f *normals;
	int *fde;
	int vert_number;

	char *object_name;
} Mesh;




// some string manipulation to strip out the
// last part of a file and use it as object name
char* get_object_name(char *path) {
	size_t path_len = strlen(path);

	char *last_slash = path + path_len;
	while((*last_slash != '/' && *last_slash != '\\') && last_slash != path) last_slash--;
	if(last_slash[0] == '/' || last_slash[0] == '\\') last_slash++;

	char *last_dot = path + path_len;
	while(*last_dot != '.' && last_dot != last_slash) last_dot--;
	int object_name_size = (int)( last_dot - last_slash );

	char *object_name = (char*)calloc(object_name_size+1, sizeof(char));

	memcpy(object_name, last_slash, object_name_size);

	object_name[0] = toupper(object_name[0]);


	return object_name;
}


// simple function to read the entire file given
// will fail if file is too big and system doesn't have enough 
// memory
char* read_entire_file(char *file, int *size) {

	if(file) {
		FILE *f = fopen(file, "rb");
		if(f) {
			fseek(f, 0, SEEK_END);
			int file_size = ftell(f);
			fseek(f, 0, SEEK_SET);
			char *file_content = (char *)calloc(file_size + 1, sizeof(char));
			fread(file_content, file_size, 1, f);
			file_content[file_size] = 0;
			if(size) *size = file_size;
			return file_content;
		}

	} 

	return NULL;
}

void assert_exit_msg(bool valid, char *str, ...) {
	if(!valid) {
		va_list other_str;
		va_start(other_str, str);
		vprintf(str, other_str);
		va_end(other_str);
		exit(1);
	}
}

char *concat_str(char *str1, char *str2) {
	size_t str1_len = strlen(str1);
	size_t str2_len = strlen(str2);

	char *new_str = (char*) calloc(str1_len + str2_len, sizeof(char));

	memcpy(new_str, str1, str1_len);
	memcpy(new_str + str1_len, str2, str2_len);

	return new_str;
}

void print_header(FILE *f, Mesh *m) {
	fputs("# University of Leeds 2021 - 2021\n", f);
	fputs("# COMP 5812M Assignment 3\n", f);
	fputs("# Sharo Qadir Hama Karim\n", f);
	fputs("# 201565480\n", f);
	fputs("# University of Leeds 2021 - 2021\n", f);
	fputs("#\n", f);
	fprintf(f, "# Object Name: %s\n", m->object_name);
	fprintf(f, "# Vertices = %d Faces = %d\n", m->vert_number, m->face_size);
	fputs("#\n", f);
}

//outputs the mesh data into the requested file format, 
//if the SH_DO_FACE_AND_DIREDGE macro is defined
//then the .face file will also contain the half edge
//data struct + the FDE structure too
void print_mesh_assignment_order(Mesh *m, char *ext) {

	char *object_name = m->object_name;
	if(object_name == NULL) {
		object_name = (char*)"output";
	}

	char *face_file_name = concat_str(object_name, ext);
	printf("%s\n", face_file_name);

	FILE *face_file = fopen(face_file_name, "w");
	
	if(!face_file) {
		printf("Failed to open the file %s\n", face_file_name);
	}

	print_header(face_file, m);

	for(int i = 0; i < m->vert_number; i++) {
		fprintf(face_file, "Vertex %d %8.3f %8.3f %8.3f\n", i, m->vertices[i].x, m->vertices[i].y, m->vertices[i].z);
	}

#ifdef SH_DO_FACE_AND_DIREDGE
	for(int i = 0; i < m->vert_number; i++) {
		fprintf(face_file, "FirstDirectEdge %3d %3d \n", i, m->fde[i]);
	}
#endif

	for(int i = 0; i < m->face_size; i++) {
		TriangleRef *t = m->faces + i;
		fprintf(face_file, "Face %3d %5d %5d %5d\n", i, t->p1, t->p2, t->p3);
	}

#ifdef SH_DO_FACE_AND_DIREDGE
	for(int i = 0; i < m->face_size*3; i++) {
		fprintf(face_file, "OtherHalf %3d %3d\n", i, m->other_half[i]);
	}
#endif

}

void print_edges(Mesh *m) {

	int face_start = 0;
	int other_face_start = 0;
	for(int i = 0; i < (m->face_size*3)/2; i++) {
		printf("Edge %d %d %d\n", i, m->edges[i].p0, m->edges[i].p1);

	}
}

void print_edges_direct(Mesh *m) {
	int *edges = (int*)m->edges;
	for(int i = 0; i < m->face_size*3; i++) {
		printf("%d %d\n", i, edges[i]);
	}

}


// function to read a number from the read context file stream
// floats have the following syntax digit* ['.' digit+] anything else is invalid.
// assume we will only use floats and the decimal points are not too small needing doubles
float read_number(ReaderContext *read_ctx) {

	assert_exit_msg(read_ctx->file[0] != EOF, (char *)"Trying to read past file end.");

	while(isspace(read_ctx->file[0])) {
		read_ctx->col++;
		if(read_ctx->file[0] == '\n') {
			read_ctx->row++;
			read_ctx->col = 0;
		}
		read_ctx->file++;
	};

	char *start = read_ctx->file;

	if(isdigit(read_ctx->file[0]) || read_ctx->file[0] == '.' || read_ctx->file[0] == '-') {

		if(read_ctx->file[0] == '-') {
			read_ctx->file++;
			read_ctx->col++;
		}

		while(isdigit(read_ctx->file[0])) {
			read_ctx->file++;
			read_ctx->col++;
		}

		if(read_ctx->file[0] == '.') {

			read_ctx->file++;
			read_ctx->col++;

			assert_exit_msg(
					isdigit(read_ctx->file[0]),
					(char*)"Malformed float, no digit after floating point %.*s (%d %d)",
					(size_t)( (char*)read_ctx->file - (char*)start ), start, read_ctx->row, read_ctx->col);

			while(isdigit(read_ctx->file[0])) {
				read_ctx->file++;
				read_ctx->col++;
			}
		}

		char last_char = read_ctx->file[0];
		assert_exit_msg(
				isspace(last_char) || last_char == '\0' || last_char == EOF,
				(char*)"Malformed number %.*s", (char*)read_ctx->file - (char*)start + 1, start, read_ctx->row, read_ctx->col);


		return strtof(start, NULL);
	} else {
		assert_exit_msg(true,
				(char*)"Malformed number %.*s (%d %d)",
				(size_t)( (char*)read_ctx->file - (char*)start ), start, read_ctx->row, read_ctx->col);
	}

	return 0;
}

//function to build the "other half" data structure,
//an edge is defined by 2 vertex ids, first id is the start
//second id is the end, to find the other half
//we have to find two vertex ids corresponding to an edge in a face that have
//the opposit order to the current edge we are looking for
void build_half_edges(Mesh *m) {

	m->other_half = (int*)calloc(m->face_size*3, sizeof(int));
	
	// -1 has the same representation irregardless of bit size
	// i.e: all 1s in 2s compliment
	memset(m->other_half, -1, m->face_size*3*4);

	// track the start of a face
	int face_start = 0;
	int other_face_start = 0;
	int *edges = (int*)m->edges;
	for(int i = 0; i < m->face_size*3; i++) {

		// every 3 points we have another face
		if(i%3 == 0) {
			face_start = i;
		}

		if(m->other_half[i] != -1) continue;

		int start_index = i;
		int end_index = face_start + ((i+1)%3);

		int start = edges[start_index];
		int end = edges[end_index];

		// we only need to search forward starting on the next face
		for(int j = face_start + 3; j < m->face_size*3; j++) {

			// we also need to keep track the index of the faces we are checking against
			if(j % 3 == 0) { other_face_start = j; }

			int other_start_index = j;
			int other_end_index = (other_face_start) + (j+1)%3;

			int other_start = edges[other_start_index];
			int other_end = edges[other_end_index];

			//the start and end edges must be swapped for the half edge
			//we stop when we find the first half edge, this leaves
			//further half edges down the list as -1 thus helping us detect if 
			//an edge is sharing more than 2 faces
			if(other_end == start && other_start == end) {
				m->other_half[j] = i;
				m->other_half[i] = j;
				break;
			}
		}
	}

}

int test_edges(Mesh *m) {
	for(int i = 0; i < m->face_size*3; i++) {
		if(m->other_half[i] == -1) {
			printf("Edge id %d failed, no other half found.\n", i);
			return 0;
		}
	}
	return 1;
}


// given a vertex id, count its one ring cycle or return -1 if
// a one ring isn't found, i.e: the case for a vertex at the border
int count_one_ring_vertex(Mesh *m, int vertex_id) {
	int edge_out = m->fde[vertex_id];
	int other_half = 0; 
	int next = edge_out;
	int counter = 0;

	do {
		other_half = m->other_half[next];
		if(other_half == -1) { return -1; }

		// we find which face this edge belongs to
		// and we can also find which vertex on the face it is
		int face_id = other_half/3;
		int vertex_id = other_half % 3;
		next = face_id*3 + (vertex_id + 1)%3;
		counter++;
	} while(next != edge_out);

	return counter;
}

int count_vertex_edges(Mesh *m, int vertex_id) {
	int counter = 0;
	//here we are just checking to see if this vertex shows up on any face
	//if it does we know it will be part of one of the edges, we don't count all the half edges
	int *edges = (int*)m->edges;
	for(int i = 0; i < m->face_size*3; i++) {
		if(edges[i] == vertex_id) {
			counter++;
		}
	}
	return counter;
}

// we test each vertex by counting the 1-ring of the vertex
// and the number of edges attached to that vert
// if those two match up then there is no pinch point, if they don't
// then the vertex is a pinch point hence not manifold
int test_vertices(Mesh *m) {

	for(int i = 0; i < m->vert_number; i++) {
		int count = count_one_ring_vertex(m, i);
		int count_edge = count_vertex_edges(m, i);
		if(count != count_edge) {
			if(count == -1) {
				printf("Vertex %d has no 1-ring cycle.\n", i);
			} else {
				printf("Pinch Vertex found at ID %d, cycle count (%d) !=  edge count (%d)\n", i,  count, count_edge);
			}

			return 0;
		}
	}

	return 1;
}

void test_manifold(Mesh *m) {

	if(test_edges(m) == 0) {
		printf("Edge test failed\n");
		exit(1);
	}

	if(test_vertices(m) == 0) {
		printf("Vertex test failed\n");
		exit(1);
	}

	puts("Polygon passed manifold check.");
}

void print_genus(Mesh *m) {

	int edge_count = (m->face_size*3)/2 + (m->face_size*3)%2;
	int vert_count = m->vert_number;
	int face_count = m->face_size;
	int genus = ( (vert_count - edge_count + face_count) - 2 )/-2;
	printf("V(%d) -  E(%d) + F(%d) = (%d) - 2G(%d) \n", vert_count, edge_count, face_count, vert_count - edge_count + face_count, genus);

	printf("Genus of object is: %d\n", genus);

}

