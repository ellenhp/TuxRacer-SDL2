#include "scoreboard.h"
#include "gui_label.h"
#include <stdio.h>
#include <stdlib.h>

widget_t* rank_labels[SCOREBOARD_SIZE+1];
widget_t* name_labels[SCOREBOARD_SIZE+1];
widget_t* score_labels[SCOREBOARD_SIZE+1];

void refresh_scores()
{
}

void refresh_scores_for_course(char* course)
{
}

void init_scoreboard_labels()
{
	int i;
	char buf[5];
	widget_t* tmp;
	for (i=1; i<=SCOREBOARD_SIZE; i++)
	{
		sprintf(buf, "%d", i);
		rank_labels[i]=create_label(buf);

		name_labels[i]=create_label("[No Name]");

		score_labels[i]=create_label("0");
	}
}

void update_scoreboard_labels()
{

}

widget_t* get_name_label(int rank)
{
	return name_labels[rank];
}

widget_t* get_score_label(int rank)
{
	return score_labels[rank];
}

widget_t* get_rank_label(int rank)
{
	return rank_labels[rank];
}

