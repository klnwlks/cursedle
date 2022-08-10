#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "words.h"

char *WOTD = WORDS[0]; // placeholder
const int HEIGHT = 20, WIDTH = 45;

char *get_time(const char *fmt){
    time_t raw_time;
    static char day[10];
    struct tm* timeinfo;

    raw_time = time(NULL);
    timeinfo = localtime(&raw_time);

    strftime(day, sizeof(day), fmt, timeinfo);

    return day;
}

void finish_game(WINDOW *win, int status, int attempts){
    noecho();
    FILE *rstats = fopen("stats", "r");
    int wins = 0, streak = 0, games = 0;
    char *day = get_time("%a"), *past_day = get_time("%a");
    
    fscanf(rstats, "%s %d %d %d", past_day, &games, &streak, &wins); 
    FILE* stats = freopen(NULL, "w", rstats);

    switch (status){
	case 0: // already answered today
	    mvwprintw(win, 2, (WIDTH - 23) / 2, "Already answered today.");
	    fprintf(stats, "%s %d %d %d", past_day, games, streak, wins);
	    break;	
	case 1: // failed game
	    mvwprintw(win, 2, (WIDTH - 19) / 2, "Try again tomorrow.");
	    games++;
	    streak = 0;
	    fprintf(stats, "%s %d %d %d", day, games, streak, wins);
	    break;	
	case 2: // win game
	    mvwprintw(win, 2, (WIDTH - 5) / 2, "Nice.");
	    wins++;
	    streak++;
	    games++;
	    fprintf(stats, "%s %d %d %d", day, games, streak, wins);
	    break;	
    }
    fclose(stats);

    // display stats.
    mvwprintw(win, 4, (WIDTH - 9) / 2, "wins: %d", wins);
    mvwprintw(win, 5, (WIDTH - 9) / 2, "games: %d", games);
    mvwprintw(win, 6, (WIDTH - 12) / 2, "attempt %d/6", attempts);
    mvwprintw(win, 7, (WIDTH - 17) / 2, "current streak: %d", streak);
    if (status != 0) mvwprintw(win, 10, (WIDTH - 6) / 2, "%s", WOTD);
    
    wrefresh(win);
    wgetch(win);
    endwin();
    exit(0);
}

void format_ans(WINDOW *win, char *ans, int *y){
    noecho();
    if (strcmp(ans, "pquit") == 0) { endwin(); exit(0);};

    for (int i = 0; i < 5; i++){
	if ((int)ans[i] < 65 || (int)ans[i] > 122 || ((int)ans[i] > 90 && (int)ans[i] < 97)) {
	    mvwprintw(win, 3, (WIDTH - 14) / 2, "invalid input.");
	    --(*y);
	    wgetch(win);
	    mvwprintw(win, 3, (WIDTH - 14) / 2, "              ");
	    return;
	};

	if ((int)ans[i] > 96 && (int)ans[i] < 123) ans[i] -= 32;

	if (strchr(WOTD, ans[i]) != NULL) wattron(win, COLOR_PAIR(2));
	if (strchr(WOTD, ans[i]) - WOTD == i) {wattroff(win, COLOR_PAIR(2)); wattron(win, COLOR_PAIR(1));}
	wattron(win, A_UNDERLINE);

	wmove(win, (*y) + 4, (WIDTH - (6 - i * 2)) / 2); // i have no idea how i * 2 fixed this
	waddch(win, ans[i]);
	
	wattroff(win, A_UNDERLINE);
	wattroff(win, COLOR_PAIR(1));
	wattroff(win, COLOR_PAIR(2));
    }
}

void cursedle(WINDOW *win){
    char answers[6][6];

    for (int i = 0; i < 6; i++){
	echo();
	mvwgetnstr(win, i + 4, (WIDTH-6) / 2, answers[i], 5); 
	format_ans(win, *answers + (i * 6), &i);
	wrefresh(win);
	if (strcmp(*answers + (i * 6), WOTD) == 0) finish_game(win, 2, i + 1);
    }
    if (strcmp(*answers + 30, WOTD) != 0) finish_game(win, 1, 6);
}

void init_game(){
    WINDOW *game_win = newwin(HEIGHT, WIDTH, (LINES - HEIGHT)  / 2, (COLS - WIDTH) / 2);
    box(game_win, 0, 0);
    char *curr_day = get_time("%a"), past_day[10];

    int word_size = sizeof(WORDS)/sizeof(WORDS[0]);
    strcpy(WOTD, WORDS[rand() % word_size]);

    touchwin(game_win);
    mvwprintw(game_win, 1, (WIDTH - 8) / 2, "CURSEDLE");
    mvwprintw(game_win, HEIGHT - 2, (WIDTH - 20)/ 2, "type \"pquit\" to exit");
    wrefresh(game_win);

    FILE *stats = fopen("stats", "r");
    if (stats == NULL) finish_game(game_win, 0, 0);
    else fscanf(stats, "%s", past_day);
    fclose(stats);
    if (strcmp(curr_day, past_day) == 0) finish_game(game_win, 0, 0);
    cursedle(game_win);
}

int main(){
    initscr();
    start_color();
    srand(time(NULL));
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    if (has_colors() == FALSE) {
	printf("Your terminal does not support color.");
	return 1;
    }

    init_game();
    endwin();
    return 0;
}
