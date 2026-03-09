// Dimitrescu Robert 312CA
#include "header.h"
#include <math.h>

// reads data from the ppm file and allocs enough memory where needed
int ppm_setup(ppm_file *ppm_img, char *filename)
{
	FILE *ppm_data = fopen(filename, "rb");
	if (!ppm_data) {
		printf("Failed to load %s\n", filename);
		return 0;
	}

	// read header info from file
	fscanf(ppm_data, " %2s", ppm_img->ppm_type);
	fscanf(ppm_data, "%u %u", &ppm_img->width, &ppm_img->height);
	fscanf(ppm_data, "%d", &ppm_img->maxval);

	fgetc(ppm_data);

	// alloc memory for ppm image based on height and width
	// make sure the data cluster is alloced properly by checking every alloc
	ppm_img->data = (int ***)malloc(sizeof(int **) * ppm_img->height);
	if (!ppm_img->data) { // memory allocation fails
		return 0;
	}
	for (unsigned int i = 0; i < ppm_img->height; i++) {
		ppm_img->data[i] = (int **)malloc(sizeof(int *) * ppm_img->width);

		if (!ppm_img->data[i]) { // memory allocation fails
			for (unsigned int k = 0; k < i; k++) {
				free(ppm_img->data[k]);
			}

			free(ppm_img->data);
			return 0;
		}
	}
	for (unsigned int i = 0; i < ppm_img->height; i++) {
		for (unsigned int  j = 0; j < ppm_img->width; j++) {
			ppm_img->data[i][j] = (int *)malloc(sizeof(int) * 3);

			if (!ppm_img->data[i][j]) { // memory allocation fails

				// free current row of pixels where allocation failed
				for (unsigned int p = 0; p < j; p++) {
					free(ppm_img->data[i][p]);
				}
				free(ppm_img->data[i]);

				// free all the other pixels
				for (unsigned int k = 0; k < i; k++) {
					for (unsigned int p = 0; p < ppm_img->width; p++) {
						free(ppm_img->data[k][p]);
					}
					free(ppm_img->data[k]);
				}
				free(ppm_img->data);
				return 0;
			}
		}
	}

	// we know that we are reading exactly one byte so we use fgetc that
	// returns an integer, and to avoid having the most significant bit being
	// treated as the "sign" of our number, out of 8 bites we get exactly a
	// number from 0-255 (or 2^8 - 1)
	// https://www.tutorialspoint.com/c_standard_library/c_function_fgetc.htm
	for (unsigned int i = 0; i < ppm_img->height; i++) {
		for (unsigned int j = 0; j < ppm_img->width; j++) {
			for (int k = 0; k < 3; k++) {
				ppm_img->data[i][j][k] = (unsigned int)fgetc(ppm_data);
			}
		}
	}

	printf("Loaded %s (PPM image %dx%d)\n", filename,
		   ppm_img->width, ppm_img->height);
	fclose(ppm_data);

	return 1;
}

// deep copy from destination to source of ppm type structs
int deep_copy_ppm(ppm_file *ppm_dest, ppm_file ppm_source)
{
	free_ppm(ppm_dest);
	// deep copy basic info inside the destination
	strcpy(ppm_dest->ppm_type, ppm_source.ppm_type);
	ppm_dest->height = ppm_source.height;
	ppm_dest->width = ppm_source.width;
	ppm_dest->maxval = ppm_source.maxval;
	// alloc memory for ppm image based on height and width
	ppm_dest->data = (int ***)malloc(sizeof(int **) * ppm_dest->height);
	if (!ppm_dest->data) {
		return 0;
	}
	for (unsigned int i = 0; i < ppm_dest->height; i++) {
		ppm_dest->data[i] = (int **)malloc(sizeof(int *) * ppm_dest->width);

		if (!ppm_dest->data[i]) {
			for (unsigned int k = 0; k < i; k++) {
				free(ppm_dest->data[k]);
			}

			free(ppm_dest->data);
			return 0;
		}
	}
	for (unsigned int i = 0; i < ppm_dest->height; i++) {
		for (unsigned int  j = 0; j < ppm_dest->width; j++) {
			ppm_dest->data[i][j] = (int *)malloc(sizeof(int) * 3);

			if (!ppm_dest->data[i][j]) { // memory allocation fails

				// free current row of pixels where allocation failed
				for (unsigned int p = 0; p < j; p++) {
					free(ppm_dest->data[i][p]);
				}
				free(ppm_dest->data[i]);

				// free all the other pixels
				for (unsigned int k = 0; k < i; k++) {
					for (unsigned int p = 0; p < ppm_dest->width; p++) {
						free(ppm_dest->data[k][p]);
					}
					free(ppm_dest->data[k]);
				}
				free(ppm_dest->data);
				return 0;
			}
		}
	}

	// deep copy data inside the destination
	for (int i = 0; i < (int)ppm_source.height; i++) {
		for (int j = 0; j < (int)ppm_source.width; j++) {
			memcpy(ppm_dest->data[i][j], ppm_source.data[i][j],
				   3 * sizeof(int));
		}
	}

	return 1;
}

// updates our ppm_log by adding the latest image into our ppm_log
// important because by doing this we can trace back to past images
void update_ppm_log(ppm_file **ppm_log, ppm_file ppm_img, int *nr_ppm_logged)
{
	ppm_file *temp = realloc(*ppm_log, (*nr_ppm_logged + 1) * sizeof(ppm_file));
	if (!temp) {
		return;
	}
	*ppm_log = temp;

	// make sure to clean the memory before using it
	memset(&(*ppm_log)[*nr_ppm_logged], 0, sizeof(ppm_file));

	// copy current ppm_img information inside the ppm_log
	deep_copy_ppm(&(*ppm_log)[*nr_ppm_logged], ppm_img);

	(*nr_ppm_logged)++;
}

// free up allocated space for a ppm object
void free_ppm(ppm_file *ppm_img)
{
	for (unsigned int i = 0; i < ppm_img->height; i++) {
		for (unsigned int  j = 0; j < ppm_img->width; j++) {
			free(ppm_img->data[i][j]);
		}
		free(ppm_img->data[i]);
	}
	free(ppm_img->data);

	ppm_img->data = NULL;
	ppm_img->height = 0;
	ppm_img->width = 0;
	ppm_img->maxval = 0;
}

// free up our ppm_log of all the images it is currently storing
void free_ppm_logs(ppm_file **ppm_log, ppm_file *ppm_img, int nr_ppm_logged)
{
	if (nr_ppm_logged != 0) {
		free_ppm(ppm_img);
		for (int i = 0; i < nr_ppm_logged; i++) {
			free_ppm(&((*ppm_log)[i]));
		}
	}

	free(*ppm_log);
}

// save current ppm image into a specified file read at console
void save_img(ppm_file ppm_img, actions **action_timeline, int current_action)
{
	if (search_load(*action_timeline, current_action, 'P') == -1) {
		printf("No image loaded\n");
		return;
	}

	char filename[STR_LEN];
	scanf("%s", filename);
	FILE *file_out = fopen(filename, "wb");
	if (!file_out) {
		return;
	}

	// write normal human_readable text (the header)
	fprintf(file_out, "%s\n", ppm_img.ppm_type);
	fprintf(file_out, "%u %u\n", ppm_img.width, ppm_img.height);
	fprintf(file_out, "%d\n", ppm_img.maxval);

	// write binary image data
	for (unsigned int i = 0; i < ppm_img.height; i++) {
		for (unsigned int j = 0; j < ppm_img.width; j++) {
			for (unsigned int k = 0; k < 3; k++) {
				unsigned char value = (unsigned char)ppm_img.data[i][j][k];
				fwrite(&value, sizeof(unsigned char), 1, file_out);
			}
		}
	}

	fclose(file_out);

	printf("Saved %s\n", filename);
}

// check if we have an L-system and a PPM image loaded
int check_turtle_setup(actions *action_timeline, int current_action)
{
	int loaded_ok = 1; // presume it loads ok at first, change otherwise
	if (search_load(action_timeline, current_action, 'L') == -1) {
		printf("No L-system loaded\n");
		loaded_ok = 0;
	}
	if (search_load(action_timeline, current_action, 'P') == -1) {
		printf("No image loaded\n");
		loaded_ok = 0;
	}

	return loaded_ok;
}

// handles changing the color of a pixel a.k.a drawing a pixel
void draw_pixel(ppm_file *curr_img, int x0, int y0, RGB_color color)
{
	// adjust because we have to invert the height for the pixel to be accurate
	y0 = (int)curr_img->height - 1 - y0;

	// change the color of the pixel's RGB values to the new color
	int *pixel_location = (*curr_img).data[y0][x0];
	pixel_location[0] = color.R;
	pixel_location[1] = color.G;
	pixel_location[2] = color.B;
}

// handles the drawing of a line by using Bresenham algortithm logic
void draw_line(ppm_file *curr_img, int x0, int y0, int x1, int y1,
			   RGB_color color)
{

	int delta_x = abs((int)(x0 - x1));
	int delta_y = -abs((int)(y0 - y1));
	int error = delta_x + delta_y;

	int step_x;
	if (x0 < x1) {
		step_x = 1; // goal is to the right move to the right on X axis
	} else {
		step_x = -1; // goal is to the left move to the left on X axis
	}

	int step_y;
	if (y0 < y1) {
		step_y = 1; // goal is up move up on the Y axis
	} else {
		step_y = -1; // goal is down move down on Y axis
	}

	int ok = 1;
	while (ok) {
		draw_pixel(curr_img, x0, y0, color);
		if (x0 == x1 && y0 == y1) { // reached our goal
			break;
		}

		if (error * 2 >= delta_y) {
			error += delta_y;
			x0 += step_x;
		}
		if (error * 2 <= delta_x) {
			error += delta_x;
			y0 += step_y;
		}
	}
}

// handles the logic for drawing on a ppm image
// we save our changes directly into the current image variable, as we can
// always go back to it since it is stored in our log
void turtle_draw(lsys_file l_system, ppm_file *ppm_img, ppm_file **ppm_log,
				 int *nr_ppm_logged, int *current_action, int *last_action,
				 actions **action_timeline)
{
	point poz;
	unsigned int step, angle, angle_step;
	unsigned long long n;
	RGB_color color;
	scanf("%lf%lf%u%u%u%llu%u%u%u", &poz.x, &poz.y, &step, &angle, &angle_step,
		  &n, &color.R, &color.G, &color.B);

	if (check_turtle_setup(*action_timeline, *current_action) == 0) {
		return; // check if all resources needed are loaded
	}

	char *instruct = lsys_derivation(l_system, n);
	unsigned long long instruct_len = strlen(instruct);

	// use a buffer for out state history, alloc more when we pass the buffer
	// this way we dont alloc everytime, just when we pass a certain amount
	int buffer = 100, amnt_alloced = 0, state_index = 0;
	turtle_state *state = (turtle_state *)malloc(sizeof(turtle_state) * buffer);
	amnt_alloced += buffer;

	for (unsigned long long i = 0; i < instruct_len; i++) {
		if (amnt_alloced == state_index + 1) { // alloc more memory for our log
			amnt_alloced += buffer;
			turtle_state *temp = realloc(state,
										 sizeof(turtle_state) * amnt_alloced);
			if (!temp) {
				free(state);
				return;
			}
			state = temp;
		}

		if (instruct[i] == 'F') {
			point goal_point;
			double radians = angle  * (PI / 180.0);
			goal_point.x = poz.x + step * cos(radians);
			goal_point.y = poz.y + step * sin(radians);

			draw_line(ppm_img, (int)round(poz.x), (int)round(poz.y),
					  (int)round(goal_point.x),
					  (int)round(goal_point.y), color);

			// move current position to the new point
			poz.x = goal_point.x;
			poz.y = goal_point.y;
		} else if (instruct[i] == '+') {
			angle += angle_step;
		} else if (instruct[i] == '-') {
			angle -= angle_step;
		} else if (instruct[i] == '[') {
			// store our current turtle position and angle
			state[state_index].orientation = angle;
			state[state_index].p.x = poz.x;
			state[state_index].p.y = poz.y;

			state_index++;
		} else if (instruct[i] == ']') {
			// refer back to the last stored turtle position and angle
			state_index--;

			angle = state[state_index].orientation;
			poz.x = state[state_index].p.x;
			poz.y = state[state_index].p.y;
		} else { // different character than one expected so we skip
			continue;
		}
	}

	printf("Drawing done\n");
	free(state);
	free(instruct);

	update_ppm_log(ppm_log, *ppm_img, nr_ppm_logged);
	update_timeline(action_timeline, last_action, current_action,
					*nr_ppm_logged - 1, 'D');
}
