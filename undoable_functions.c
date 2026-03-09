// Dimitrescu Robert 312CA
#include "header.h"

// update the current timeline of undoable actions we can do
void update_timeline(actions **action_timeline, int *last_action,
					 int *current_action, int action_index, char action_type)
{
	(*current_action)++; // we go forward to a new action in our timeline

	// store only the points in our timeline that we can go back/forward to
	actions *temp = realloc(*action_timeline, (*current_action + 1) *
							sizeof(actions));
	if (!temp) {
		return;
	}
	*action_timeline = temp;

	*last_action = *current_action;

	// store the type of action and it's location inside corresponing log
	(*action_timeline)[*last_action].type = action_type;
	(*action_timeline)[*last_action].log_position = action_index;
}

// free action logged if it contains anything
void free_timeline(actions **action_timeline)
{
	if (!(*action_timeline)) {
		return;
	}
	free(*action_timeline);
	*action_timeline = NULL;
}

// apply either the last action taken (for undo) or the one after it (for redo)
// mode signifies which one of the two we want to apply
void apply_action(actions *action_timeline, int *current_action, char mode,
				  lsys_file *l_system, lsys_file *past_lsys, ppm_file *ppm_img,
				  ppm_file *ppm_log)
{
	if (mode == 'R') {
		(*current_action)++;
	} else {
		(*current_action)--;
	}

	// in case of UNDO where we trace back to
	// the origin (before there was no action)
	if (*current_action == -1) {
		if (action_timeline[(*current_action) + 1].type == 'L') {
			free_lsys(l_system);
		} else if (action_timeline[(*current_action) + 1].type == 'P' ||
				   action_timeline[(*current_action) + 1].type == 'D') {
			free_ppm(ppm_img);
		}

		return;
	}

	int index = action_timeline[*current_action].log_position;
	int prints;
	if (mode == 'R') {
		prints = 1;
	} else {
		prints = 0;
	}

	// find in our timeline relevant information about the action
	// we want to execute
	if (action_timeline[*current_action].type == 'L') {
		free_lsys(l_system);

		int ran_ok = lsys_setup(l_system, past_lsys[index].lsys_filename,
								prints);
		if (!ran_ok) {
			return;
		}
	} else if (action_timeline[*current_action].type == 'P') {
		int ran_ok = deep_copy_ppm(ppm_img, ppm_log[index]);
		if (!ran_ok) {
			return;
		}
	} else if (action_timeline[*current_action].type == 'D') {
		int ran_ok = deep_copy_ppm(ppm_img, ppm_log[index]);
		if (!ran_ok) {
			return;
		}

		// we have the last drawn image saved so we can just save it back
		// we dont need to draw it again as it is slower
		if (prints == 1) {
			printf("Drawing done\n");
		}
	}
}

// for the type we want to search for we keep the logic of how types
// are stored inside the "actions" struct
int search_load(actions *action_timeline, int current_action, char type)
{
	for (int i = current_action ; i >= 0; i--) {
		if (action_timeline[i].type == type) {
			return i; // we have neccesary type loaded, return index
		}
	}

	return -1; // an index wasnt found, no neccesary type was loaded
}
