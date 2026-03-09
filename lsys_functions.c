// Dimitrescu Robert 312CA
#include "header.h"

// created own strdup function as it does not work
// used this for help https://c-for-dummies.com/blog/?p=1699
char *strdup(const char *string)
{
	unsigned long long lenght = strlen(string) + 1;
	char *result = (char *)malloc(lenght * sizeof(char));
	if (result) {
		memcpy(result, string, lenght);
	}

	return result;
}

int lsys_setup(lsys_file *l_system, char filename[STR_LEN], int prints)
{
	// check if required file exists, if yes then save it as the current
	// l_system file that is being used, if not return error message without
	// changing the last well configured l_system
	FILE *lsys_data = fopen(filename, "rt");
	if (!lsys_data) {
		printf("Failed to load %s\n", filename);
		return 0;
	}

	// copy data inside l_system now that we are sure the file exists
	l_system->lsys_filename = strdup(filename);

	char temp[STR_LEN];
	fscanf(lsys_data, " %s", temp);
	l_system->axiom = strdup(temp);

	fscanf(lsys_data, "%d", &l_system->nr_rules);

	// alloc memory and copy rules data inside our hashmap
	l_system->rules = (hashmap *)malloc(sizeof(hashmap) * l_system->nr_rules);
	if (!l_system->rules) {
		return 0;
	}

	for (int i = 0; i < l_system->nr_rules; i++) {
		fscanf(lsys_data, " %c", &l_system->rules[i].simbol);

		char temp[STR_LEN];
		fscanf(lsys_data, "%s", temp);
		l_system->rules[i].succesor = strdup(temp);
	}

	fclose(lsys_data);

	if (prints == 1) {
		printf("Loaded %s (L-system with %d rules)\n",
			   l_system->lsys_filename, l_system->nr_rules);
	}

	return 1;
}

// handles freeing memory for the current L-system that it's going to
// be replaced by a new one so that no old data is kept
void free_lsys(lsys_file *l_system)
{
	free(l_system->lsys_filename);
	free(l_system->axiom);

	for (int i = 0; i < l_system->nr_rules; i++) {
		free(l_system->rules[i].succesor);
	}
	free(l_system->rules);

	// reset the pointers to NULL so that the next L-system can be setup
	l_system->lsys_filename = NULL;
	l_system->axiom = NULL;
	l_system->nr_rules = 0;
	l_system->rules = NULL;
}

void free_lsys_logs(lsys_file *l_system, lsys_file **past_lsys,
					int nr_lsys_logged)
{
	if (nr_lsys_logged != 0) {
		free_lsys(l_system);
		for (int i = 0; i < nr_lsys_logged; i++) {
			free_lsys(&((*past_lsys)[i]));
		}
	}

	free(*past_lsys);
}

// add memory and store in the log a newly succesfully loaded lsys system
void update_lsys_history(lsys_file l_system, lsys_file **past_lsys,
						 int *nr_lsys_logged)
{
	// make space for our new entry inside our log of past L-systems
	lsys_file *temp = realloc(*past_lsys, (*nr_lsys_logged + 1) *
								sizeof(lsys_file));
	if (!temp) {
		return;
	}
	*past_lsys = temp;

	// copy the newest L-system into our log
	(*past_lsys)[*nr_lsys_logged].lsys_filename =
												 strdup(l_system.lsys_filename);
	(*past_lsys)[*nr_lsys_logged].axiom = strdup(l_system.axiom);
	(*past_lsys)[*nr_lsys_logged].nr_rules = l_system.nr_rules;

	(*past_lsys)[*nr_lsys_logged].rules = (hashmap *)malloc(sizeof(hashmap) *
									   l_system.nr_rules);

	for (int i = 0; i < l_system.nr_rules; i++) {
		(*past_lsys)[*nr_lsys_logged].rules[i].simbol =
												l_system.rules[i].simbol;
		(*past_lsys)[*nr_lsys_logged].rules[i].succesor =
											strdup(l_system.rules[i].succesor);
	}

	(*nr_lsys_logged)++;
}

// return the result from derivinga l-system in string format
char *lsys_derivation(lsys_file l_system, unsigned long long derv_order)
{
	char *last_derv = strdup(l_system.axiom);
	unsigned long long last_derv_len = strlen(l_system.axiom);

	// find the derviation that has the largest result
	// will use this later to safely alloc enough memory for each derivation
	unsigned long long max_derv_size = 0;
	for (int i = 0; i < l_system.nr_rules; i++) {
		unsigned long long succesor_size = strlen(l_system.rules[i].succesor);
		if (succesor_size > max_derv_size) {
			max_derv_size = succesor_size;
		}
	}

	for (unsigned long long i = 0; i < derv_order; i++) {
		char *next_derv = (char *)malloc(last_derv_len * max_derv_size
						* sizeof(char) + 1);
		next_derv[0] = '\0';
		unsigned long long next_derv_len = 0;

		for (unsigned long long j = 0; j < last_derv_len; j++) {
			int found_rule = 0, succesor_idx;
			for (int k = 0; k < l_system.nr_rules; k++) {
				if (last_derv[j] == l_system.rules[k].simbol) {
					found_rule = 1;
					succesor_idx = k;
					break;
				}
			}

			if (found_rule == 0) { // add the same character to next derivative
				next_derv[next_derv_len] = last_derv[j];
				next_derv_len++;
			} else { // if a succesor/rule is found replace current character
				unsigned long long succesor_len;
				succesor_len = strlen(l_system.rules[succesor_idx].succesor);

				memcpy(next_derv + next_derv_len,
					   l_system.rules[succesor_idx].succesor, succesor_len);

				next_derv_len += succesor_len;
			}
			next_derv[next_derv_len] = '\0';
		}

		// make sure we adjust the memory alloced earlier to be minimum
		char *optimized_derv = realloc(next_derv, next_derv_len + 1);
		if (!optimized_derv) {
			free(next_derv);
			return NULL;
		}

		free(last_derv);
		last_derv = optimized_derv;
		last_derv_len = next_derv_len;
	}

	return last_derv;
}
