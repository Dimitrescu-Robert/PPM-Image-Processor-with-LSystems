// Dimitrescu Robert 312CA
#include "header.h"

// take byte and translate it to a integer from 0-255 based on its values
// each 8 bites represent one of the triplet cromatic values
void binary_transform(char *byte, unsigned int *R_val, unsigned int *G_val,
					  unsigned int *B_val)
{
	int tw0_pow = 128, i = 0;
	*R_val = 0, *B_val = 0, *G_val = 0;

	// obtain R value
	for (; i < 8; i++) {
		if (byte[i] == '1') {
			*R_val += tw0_pow;
		}
		tw0_pow /= 2;
	}

	// obtain G value
	tw0_pow = 128;
	for (; i < 16; i++) {
		if (byte[i] == '1') {
			*G_val += tw0_pow;
		}
		tw0_pow /= 2;
	}

	// obtain B value
	tw0_pow = 128;
	for (; i < 24; i++) {
		if (byte[i] == '1') {
			*B_val += tw0_pow;
		}
		tw0_pow /= 2;
	}
}

void pixel_problem(char *string, ppm_file ppm_img, unsigned long long i)
{
	unsigned int R_val = 0, G_val = 0, B_val = 0;
	// get coordonates by selecting every pixel (3 * 8) characters
	// go back to matrix coordonates by sectioning off the array
	// check coordonates of the problem bit
	// for our Y-value we have to invert it based on height
	int row = ((i + 2) / 24) % ppm_img.width;
	int col = ppm_img.height - 1 - (((i + 2) / 24) / ppm_img.width);

	char *pixel_val = malloc(sizeof(char) * 24); // RGB triplet
	memcpy(pixel_val, string + ((i + 2) / 24) * 24, sizeof(char) * 24);
	binary_transform(pixel_val, &R_val, &G_val, &B_val);

	printf("Warning: pixel at (%d, %d) may be read as"
			" (%d, %d, %d)\n", row, col, R_val, G_val, B_val);
	free(pixel_val);
}

void binary_bitcheck(ppm_file ppm_img, actions *action_timeline,
					 int current_action)
{
	if (search_load(action_timeline, current_action, 'P') == -1) {
		printf("No image loaded\n");
		return;
	}

	char *string;
	// convert amount of pixels in a image to each bite they represent
	unsigned long long max_len = ppm_img.height * ppm_img.width * 3 * 8 + 1;
	string = malloc(sizeof(char) * max_len);

	// convert every single pixel RGB value to bites and store it as char
	unsigned long long str_index = 0;
	for (unsigned int i = 0; i < ppm_img.height; i++) {
		for (unsigned int j = 0; j < ppm_img.width; j++) {
			for (int k = 0; k < 3; k++) {
				int num = ppm_img.data[i][j][k];
				for (int p = 7; p >= 0; p--) {
					// extract integer bit by bit (pun intended)
					string[str_index + 7 - p] = ((num >> p) & 1) + '0';
				}
				str_index += 8;
			}
		}
	}
	string[str_index] = '\0';

	// parse the newly formed string and check for problems
	for (unsigned long long i = 0; i <= str_index - 4; i++) {
		if (string[i] == '0' && string[i + 1] == '0' && string[i + 2] == '1' &&
			string[i + 3] == '0') {
			string[i + 2] = '0'; // change bit to what it might be read as
			pixel_problem(string, ppm_img, i);
			string[i + 2] = '1';
		} else if (string[i] == '1' && string[i + 1] == '1' &&
				   string[i + 2] == '0' && string[i + 3] == '1') {
			string[i + 2] = '1'; // change bit to what it might be read as
			pixel_problem(string, ppm_img, i);
			string[i + 2] = '0';
		}
	}
	free(string);
}
