typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

#define VP_LOC 0
#define COLOR_LOC 1
#define UV_LOC 2
#define NORMAL_LOC 9
#define MODEL_UNIFORM_LOC 3
#define CAMERA_UNIFORM_LOC 4
#define PROJECTION_UNIFORM_LOC 5
#define USE_UNIFORM_COLOR_LOC 6
#define UNIFORM_COLOR_LOC 7
#define HAS_TEXTURE_LOC 8
#define USE_LIGHT 10



float yaw = 0;
float pitch = 0;

typedef struct sh_reloadable_shader {
	FILETIME last_write;
	i32 type;
	i32 handle;
	i8 needs_reload;
	char *file_name;

} sh_reloadable_shader;

typedef struct key_button {
	u8 pressed;
	u8 pressed_once;
} key_button;


typedef struct mouse_info {
	i32 cur_x;
	i32 cur_y;
	i32 prev_x;
	i32 prev_y;

	i32 delta_x;
	i32 delta_y;

	key_button left;
	key_button right;


} mouse_info;

typedef struct time_info {
	uint64 tick;
	uint64 delta_tick;
	
	uint64 time_nano;
	uint64 time_micro;
	uint64 time_milli;
	float  time_sec;
	
	uint64 delta_time_nano;
	uint64 delta_time_micro;
	uint64 delta_time_milli;
	float delta_time_sec;

	uint64 tick_per_sec;
	uint64 start_tick;
} time_info;


time_info gl_time;
mouse_info mouse;

typedef struct sh_window_context {
	i16 height;
	i16 width;
	i16 x;
	i16 y;
	i8 should_close;
	u8 move_camera;
	char *window_name;
	HDC hdc;
	mat4 screen_scale;
	mat4 inv_screen_scale;
	mat4 text_ortho;
	mat4 camera;
	mat4 inv_camera;

	u32 vbo;
	u32 vao;
	i32 font_size;
	f32 quad_height;
	f32 quad_width;
	char input[256];
	i32 input_size;
	u8 *font_atlas;
	u8 *ft_atlas;
	u32 font_atlas_texture;
	u32 ft_atlas_texture;
	i32 atlas_height;
	i32 atlas_width;
	i32 ft_atlas_height;
	i32 ft_atlas_width;


	char *text;
	i32 line_numbers;
	float left_add;
	float top_add;


	sh_vec3 eye_pos;
	sh_vec3 look_at;
	sh_vec3 up;
	key_button keyboard[256];
	i32 frame_number;

	i32 program;

} sh_window_context;


sh_window_context gl_ctx;

i32 sh_create_shader(char *file_name, i32 type) {
	char *shader_source = read_file(file_name, NULL);
	i32 shader = glCreateShader(type);
	glShaderSource(shader, 1, &shader_source, NULL);
	glCompileShader(shader);

	GLint shader_compile_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compile_status);

	free(shader_source);
	if(shader_compile_status != GL_TRUE) {
		int log_size;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		char *log = (char*)malloc(log_size);
		glGetShaderInfoLog(shader, log_size, &log_size, log);
		printf("shader (%s) has errors: %s\n", file_name, log);
		free(log);
		return 0;
	}

	return shader;
}

sh_reloadable_shader sh_create_reloadable_shader(char *file_name, i32 type) {
	sh_reloadable_shader shader = {0};

	shader.handle = sh_create_shader(file_name, type);

	if(shader.handle == 0) {
		printf("error creating reloadable shader.\n");
	} else {
		i32 file_name_size = (i32)strlen(file_name)+1;
		shader.file_name = (char*)malloc(file_name_size);
		CopyMemory(shader.file_name, file_name, file_name_size);
		shader.type = type;
		shader.last_write = sh_get_file_last_write(shader.file_name);
	}


	return shader;
}

i32 sh_create_program(i32 *shaders, i32 shader_count) {
	i32 program = glCreateProgram();
	for(int i = 0; i < shader_count; ++i) {
		glAttachShader(program, shaders[i]);
		// printf("shader %d\n", shaders[i]);
	}
	glLinkProgram(program);

	i32 program_link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &program_link_status);

	if(program_link_status != GL_TRUE) {
		i32 log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

		char *log = (char*)malloc(log_length);
		glGetProgramInfoLog(program, log_length, &log_length, log);
		printf("program linking error(%d): %s\n", __LINE__, log);
		free(log);
		return 0;
	}

	return program;
}

void init_opengl() {

	glGenVertexArrays(1, &gl_ctx.vao);
	glBindVertexArray(gl_ctx.vao);

	sh_reloadable_shader vs = sh_create_reloadable_shader((char*)"shader/vertex_shader.glsl", GL_VERTEX_SHADER);
	sh_reloadable_shader fg = sh_create_reloadable_shader((char*)"shader/fragment_shader.glsl", GL_FRAGMENT_SHADER);

	i32 shaders[2] = {vs.handle, fg.handle};
	i32 texture_prog = sh_create_program(shaders, 2);

	gl_ctx.program = texture_prog;

	glUseProgram(texture_prog);

	float width_half = gl_ctx.width/2.0f;
	float height_half = gl_ctx.height/2.0f;

	gl_ctx.screen_scale = perspective(60.0f, (float)gl_ctx.width/gl_ctx.height, 0.1f, 1000.0f);
	gl_ctx.inv_screen_scale = inverse(&gl_ctx.screen_scale);
	gl_ctx.text_ortho = ortho(-width_half, width_half, -height_half, height_half, -1, 1);

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.screen_scale.m);

	gl_ctx.eye_pos = {2, 2, 2};
	// gl_ctx.eye_pos = (sh_vec3){2, 2, 2};
	gl_ctx.look_at = {-1, -1, -1};
	gl_ctx.up = {0, 1, 0};

	gl_ctx.camera = lookat(gl_ctx.eye_pos, gl_ctx.look_at, gl_ctx.up);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	glViewport(0, 0, gl_ctx.width, gl_ctx.height);

	// glGenBuffers(1, &gl_ctx.vbo);

	glEnableVertexAttribArray(VP_LOC);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	fflush(stdout);
}

LRESULT sh_window_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param) {

	LRESULT result = 0; 

	switch(msg) {
		case WM_DESTROY: {
			PostQuitMessage(0);
			gl_ctx.should_close = 1;
		} break;

		case WM_CHAR: {

			gl_ctx.input[gl_ctx.input_size] = (char)w_param;
			gl_ctx.input_size++;

		} break;

		case WM_LBUTTONDOWN: {
			mouse.cur_y = (i32)(l_param >> 16 ) ;
			mouse.cur_x = (i32)( l_param & 0xFFFF );


			mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
			mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

			mouse.left.pressed = 1;
		} break;

		case WM_LBUTTONUP: {
			mouse.cur_y = (i32)(l_param >> 16 ) ;
			mouse.cur_x = (i32)( l_param & 0xFFFF );

			mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
			mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

			mouse.left.pressed_once = 1;
			mouse.left.pressed = 0;
		} break;

		case WM_MOUSEMOVE: {
			mouse.cur_y = (i32)(l_param >> 16 ) ;
			mouse.cur_x = (i32)( l_param & 0xFFFF );

			mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
			mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

		} break;

		default: result =  DefWindowProc(h_wnd, msg, w_param, l_param);
	}

	return result;
}

void load_modern_wgl() {

	HWND fake_window = CreateWindowEx(
			0, "STATIC", "TEMP",
			WS_OVERLAPPED,
			CW_USEDEFAULT,
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			CW_USEDEFAULT,
			NULL,
			NULL,
			NULL,
			NULL
			);


	HDC fake_dc = GetDC(fake_window);

	PIXELFORMATDESCRIPTOR sh_px = {0};
	sh_px.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	sh_px.nVersion = 1;
	sh_px.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	sh_px.cColorBits = 24;

	int index = ChoosePixelFormat(fake_dc, &sh_px);


	DescribePixelFormat(fake_dc, index, sizeof(sh_px), &sh_px);
	SetPixelFormat(fake_dc, index, &sh_px);

	HGLRC gl_temp = wglCreateContext(fake_dc);
	wglMakeCurrent(fake_dc, gl_temp);

	load_modern_context_creation();

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(gl_temp);
	ReleaseDC(fake_window, fake_dc);
	DestroyWindow(fake_window);


}


void setup_opengl(HWND h_wnd) {

	GLint pixel_format_attr[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 24,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_SAMPLES_ARB, 4,
		0
	};


	HDC dc = GetDC(h_wnd);

	i32 pixel_format = 0;
	u32 num_format = 0;

	wglChoosePixelFormatARB(dc, pixel_format_attr, NULL, 1, &pixel_format, &num_format);

	PIXELFORMATDESCRIPTOR m = {0};
	m.nSize = sizeof(PIXELFORMATDESCRIPTOR);

	DescribePixelFormat(dc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &m);

	SetPixelFormat(dc, pixel_format, &m);

	GLint context_attr[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 5,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	HGLRC gl_c = wglCreateContextAttribsARB(dc, NULL, context_attr);

	wglMakeCurrent(dc, gl_c);

	load_ext();
	gl_ctx.hdc = dc;


	init_opengl();
}

void init_time() {
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	gl_time.tick_per_sec = li.QuadPart;

	QueryPerformanceCounter(&li);

	gl_time.start_tick = li.QuadPart;
	gl_time.tick = li.QuadPart;
}




HWND sh_window_setup(void) {

	load_modern_wgl();

	WNDCLASS wndclass = {0};
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)sh_window_proc;
	wndclass.hInstance = NULL;
	wndclass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND);
	wndclass.lpszClassName = "sh_gui";
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wndclass);

	i32 style = WS_OVERLAPPEDWINDOW;
	RECT win_size = {
		.left = 20,
		.top = 20,
		.right = gl_ctx.width,
		.bottom = gl_ctx.height };

	AdjustWindowRect(&win_size, style, FALSE);

	HWND wn = CreateWindow(
			"sh_gui",
			gl_ctx.window_name,
			WS_VISIBLE | style,
			win_size.left,
			win_size.top,
			win_size.right - win_size.left,
			win_size.bottom - win_size.top,
			NULL,
			NULL,
			NULL,
			NULL
			);


	setup_opengl(wn);


	init_time();
	// init_raw_input();
	return wn;

}



void update_time() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);

	uint64 cur_tick = li.QuadPart - gl_time.start_tick;

	gl_time.delta_tick = cur_tick - gl_time.tick;
	gl_time.tick = cur_tick;


	gl_time.time_nano = (1000*1000*1000*gl_time.tick)/(gl_time.tick_per_sec);
	gl_time.time_micro = (1000*1000*gl_time.tick)/(gl_time.tick_per_sec);
	gl_time.time_milli = (1000*gl_time.tick)/(gl_time.tick_per_sec);
	gl_time.time_sec = (float)gl_time.tick/(float)gl_time.tick_per_sec;
	
	gl_time.delta_time_nano = (1000*1000*1000*gl_time.delta_tick)/(gl_time.tick_per_sec);
	gl_time.delta_time_micro = (1000*1000*gl_time.delta_tick)/(gl_time.tick_per_sec);
	gl_time.delta_time_milli = (1000*gl_time.delta_tick)/(gl_time.tick_per_sec);
	gl_time.delta_time_sec = (float)gl_time.delta_tick/(float)gl_time.tick_per_sec;
}





void update_keys() {
	static u8 keys[256]; 
	GetKeyboardState(keys);

	for(i32 i = 0; i < 256; i++) {
		u8 new_state = keys[i] >> 7; 
		gl_ctx.keyboard[i].pressed_once = !gl_ctx.keyboard[i].pressed && new_state;
		gl_ctx.keyboard[i].pressed = new_state;
	}
}

void update_mouse() {

	mouse.delta_x = mouse.cur_x - mouse.prev_x;
	mouse.delta_y = mouse.cur_y - mouse.prev_y;

	mouse.prev_x = mouse.cur_x;
	mouse.prev_y = mouse.cur_y;
}

void handle_events() {

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	update_mouse();
	update_keys();

	if(gl_ctx.keyboard[VK_ESCAPE].pressed) {
		gl_ctx.should_close = 1;
	}

	float speed = 2;

	if(gl_ctx.keyboard['W'].pressed) {
		sh_vec3 f = gl_ctx.look_at;
		sh_vec3_mul_scaler(&f, speed*gl_time.delta_time_sec);
		sh_vec3_add_vec3(&gl_ctx.eye_pos, &f);
	}

	if(gl_ctx.keyboard['S'].pressed) {
		sh_vec3 f = gl_ctx.look_at;
		sh_vec3_mul_scaler(&f, speed*gl_time.delta_time_sec);
		sh_vec3_sub_vec3(&gl_ctx.eye_pos, &f);
	}

	if(gl_ctx.keyboard['A'].pressed) {
		sh_vec3 f = sh_vec3_cross(&gl_ctx.look_at, &gl_ctx.up );
		sh_vec3_mul_scaler(&f, speed*gl_time.delta_time_sec);
		sh_vec3_sub_vec3(&gl_ctx.eye_pos, &f);
	}

	if(gl_ctx.keyboard['D'].pressed) {

		sh_vec3 f = sh_vec3_cross(&gl_ctx.up, &gl_ctx.look_at);
		sh_vec3_mul_scaler(&f, speed*gl_time.delta_time_sec);
		sh_vec3_sub_vec3(&gl_ctx.eye_pos, &f);
	}


	if(mouse.left.pressed && (mouse.delta_x != 0 || mouse.delta_y != 0)) {
		yaw +=   0.1f*mouse.delta_x;
		pitch += 0.1f*mouse.delta_y;

		float sin_pitch = sinf(TO_RADIANS(pitch));
		float cos_pitch = cosf(TO_RADIANS(pitch));
		float cos_yaw = cosf(TO_RADIANS(yaw));
		float sin_yaw = sinf(TO_RADIANS(yaw));

		gl_ctx.look_at.x = cos_yaw*cos_pitch;
		gl_ctx.look_at.y = sin_pitch;
		gl_ctx.look_at.z = sin_yaw*cos_pitch;

		sh_vec3_normalize_ref(&gl_ctx.look_at);

	}

	gl_ctx.camera = lookat(gl_ctx.eye_pos, sh_vec3_new_add_vec3(&gl_ctx.eye_pos, &gl_ctx.look_at), gl_ctx.up);
	gl_ctx.inv_camera = inverse(&gl_ctx.camera);
	// gl_ctx.camera = lookat(gl_ctx.eye_pos, gl_ctx.look_at, gl_ctx.up);

	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);


	if(gl_ctx.keyboard['P'].pressed_once) {
		printf("%f %f - %f %f %f\n", pitch, yaw, gl_ctx.look_at.x, gl_ctx.look_at.y, gl_ctx.look_at.z);
		fflush(stdout);
	}
	

}
