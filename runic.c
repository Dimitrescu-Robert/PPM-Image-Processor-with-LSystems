// Dimitrescu Robert 312CA
#include "header.h"

int main(void)
{
	char message[STR_LEN] = "";
	lsys_file l_system, *past_lsys = NULL;
	actions *action_timeline = NULL;
	ppm_file ppm_img, *ppm_log = NULL;
	int nr_lsys_logged = 0, last_action = -1, current_action = -1;
	int nr_ppm_logged = 0;

	while (strcmp(message, "EXIT") != 0) {
		scanf("%s", message);

		if (strcmp(message, "LSYSTEM") == 0) {
			if (nr_lsys_logged != 0) {
				free_lsys(&l_system);
			}
			char filename[STR_LEN];
			scanf("%s", filename);
			if (!lsys_setup(&l_system, filename, 1)) {
				continue; // we skip log saving when action fails
			}

			update_lsys_history(l_system, &past_lsys, &nr_lsys_logged);
			update_timeline(&action_timeline, &last_action, &current_action,
							nr_lsys_logged - 1, 'L');
		} else if (strcmp(message, "DERIVE") == 0) {
			unsigned long long derv_order; // how many times we have to derive
			scanf("%llu", &derv_order);
			if (search_load(action_timeline, current_action, 'L') == -1) {
				printf("No L-system loaded\n");
			} else {
				char *derv_result = lsys_derivation(l_system, derv_order);
				printf("%s\n", derv_result);
				free(derv_result);
			}
		} else if (strcmp(message, "UNDO") == 0) {
			if (current_action == -1) {
				printf("Nothing to undo\n");
			} else {
				apply_action(action_timeline, &current_action, 'U', &l_system,
							 past_lsys, &ppm_img, ppm_log);
			}
		} else if (strcmp(message, "REDO") == 0) {
			if (current_action == last_action) {
				printf("Nothing to redo\n");
			} else {
				apply_action(action_timeline, &current_action, 'R', &l_system,
							 past_lsys, &ppm_img, ppm_log);
			}
		} else if (strcmp(message, "LOAD") == 0) {
			char filename[STR_LEN];
			scanf("%s", filename);
			if (nr_ppm_logged != 0) {
				free_ppm(&ppm_img);
			}
			if (!ppm_setup(&ppm_img, filename)) {
				continue; // we skip log saving when action fails
			}

			update_ppm_log(&ppm_log, ppm_img, &nr_ppm_logged);
			update_timeline(&action_timeline, &last_action, &current_action,
							nr_ppm_logged - 1, 'P');
		} else if (strcmp(message, "SAVE") == 0) {
			save_img(ppm_img, &action_timeline, current_action);
		} else if (strcmp(message, "TURTLE") == 0) {
			turtle_draw(l_system, &ppm_img, &ppm_log, &nr_ppm_logged,
						&current_action, &last_action, &action_timeline);
		} else if (strcmp(message, "BITCHECK") == 0) {
			binary_bitcheck(ppm_img, action_timeline, current_action);
		} else {
			break;
		}
	}

	// make sure we dont leave memory hanging and free everything alloced
	free_lsys_logs(&l_system, &past_lsys, nr_lsys_logged);
	free_ppm_logs(&ppm_log, &ppm_img, nr_ppm_logged);
	free_timeline(&action_timeline);
	return 0;
}
