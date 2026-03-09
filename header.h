//Dimitrescu Robert 312CA
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#define STR_LEN 100
#define PI 3.14159265358979323846

// structure with elements same as in a PPM file
typedef struct {
	char *filename;
	char ppm_type[2];
	unsigned int width;
	unsigned int height;
	int maxval;
	int ***data; // width * height * 3 (RGB VALUES)
} ppm_file;

// for each past action we care about what type of action it was
// and where we can find it's contents and relevant information
// the type of action is represented by a char with certain values:
// L - Loaded L-system
// P - Loaded PPM_IMG
// D - Turtle drawing
typedef struct {
	char type;
	int log_position; // for it's specified type where exactly we can find it
} actions;

// simple key -> value hashmap (for us simbol -> succesor)
// used for derving rules for "Grafeme fractale"
// used https://benhoyt.com/writings/hash-table-in-c/ for C hashmap design
typedef struct{
	char simbol;
	char *succesor;
} hashmap;

// structure with elements that can be associated to lsys file content
// structure will be used to parse lsys files and derive them properly
typedef struct {
	char *lsys_filename; // current lsys_filename
	char *axiom;
	int nr_rules;
	hashmap *rules; // deriving rules will be stored here
} lsys_file;

// a point is defined by its coordonates on the X, Y axis
typedef struct {
	double x;
	double y;
} point;

// a RGB pixel is defined by three 0-255 values
typedef struct {
	unsigned int R;
	unsigned int G;
	unsigned int B;
} RGB_color;

// current turtle position defined by its location inside cartisian system and
// the angle at which it is oriented at
typedef struct {
	point p;
	unsigned int orientation;
} turtle_state;

char *strdup(const char *string);

// lsys functions
int lsys_setup(lsys_file *l_system, char filename[STR_LEN], int prints);
void free_lsys(lsys_file *l_system);
void free_lsys_logs(lsys_file *l_system, lsys_file **past_lsys,
					int nr_lsys_logged);
char *lsys_derivation(lsys_file l_system, unsigned long long derv_order);
void update_lsys_history(lsys_file l_system, lsys_file **past_lsys,
						 int *nr_lsys_logged);

// undoable functions
void update_timeline(actions **action_timeline, int *last_action,
					 int *current_action, int action_index, char action_type);
void free_timeline(actions **action_timeline);
void apply_action(actions *action_timeline, int *current_action, char mode,
				  lsys_file *l_system, lsys_file *past_lsys, ppm_file *ppm_img,
				  ppm_file *ppm_log);
int search_load(actions *action_timeline, int current_action, char type);

// ppm functions
int ppm_setup(ppm_file *ppm_img, char *filename);
void free_ppm(ppm_file *ppm_img);
void update_ppm_log(ppm_file **ppm_log, ppm_file ppm_img, int *nr_ppm_logged);
void free_ppm_logs(ppm_file **ppm_log, ppm_file *ppm_img, int nr_ppm_logged);
void save_img(ppm_file ppm_img, actions **action_timeline, int current_action);
int check_turtle_setup(actions *action_timeline, int current_action);
void turtle_draw(lsys_file l_system, ppm_file *ppm_img, ppm_file **ppm_log,
				 int *nr_ppm_logged, int *current_action, int *last_action,
				 actions **action_timeline);
int deep_copy_ppm(ppm_file *ppm_dest, ppm_file ppm_source);

// bitcheck functions
void binary_bitcheck(ppm_file ppm_img, actions *action_timeline,
					 int current_action);
