
extern u64_table * control_tree;
bool control_has_child(u64 control, u64 child);
void control_add_child(u64 control, u64 child);

void window_request_size(u64 window, int width, int height);

extern alignments * alignment_table;
extern margin_table * margins;
extern margin_table * padding;
extern margin_table * color;
extern margin_table * layout;
extern u64_to_vec2_table * desired_size;
extern u64_to_f32_table * min_width;
extern u64_to_f32_table * min_height;
extern u64_to_f32_table * max_width;
extern u64_to_f32_table * max_height;


extern u64 arrange_method;
extern u64 measure_method;
extern u64 render_method;
extern u64 mouse_down_method;
extern u64 class_window, class_uielement, class_button, class_green_button;
//void render_control(u64 control);
//void green_button_render(u64 control);
//void control_arrange(u64 control);
//void control_measure(u64 control);
void gui_init();

// experiment
void render_octree_control(u64 control);
  
