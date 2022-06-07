#include <stdlib.h>
#include <filesystem>
#include <string>
#include <vector>
#include <curses.h>
#include <menu.h>
using namespace std;

int main() {
	// Initialize curses
	initscr();

	// Enable colors
	start_color();

	// Don't wait for enter key
	cbreak();

	// Turn off echoing of keys
	noecho();

	// Enable keypad mode
	keypad(stdscr, TRUE);

	// Initialize color pairs
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);

	// Initialize items
	WINDOW *my_menu_win;
	MENU *my_menu;
	int n_choices;
	ITEM **my_items;

	// Create vector of pointers to strings to hold directory entries
	vector<string*>* entry_names = new vector<string*>();
	
	// Get current directory
	filesystem::path dir = filesystem::current_path();

    if (std::filesystem::exists(dir)) {
        if (std::filesystem::is_directory(dir)) {

			// Count number of entries
			n_choices = 0;
			for (auto it = filesystem::begin(filesystem::directory_iterator(dir)); it != filesystem::end(filesystem::directory_iterator(dir)); ++it) {
				n_choices++;
			}

			// Allocate space for items
			my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

			// Create items
			int i = 0;
			for (auto it = filesystem::begin(filesystem::directory_iterator(dir)); it != filesystem::end(filesystem::directory_iterator(dir)); ++it) {
				string name = it->path().filename().string();
				entry_names->push_back(new string(name));
				my_items[i] = new_item((*entry_names)[i]->c_str(), "");
				i++;
			}
        } else {
			mvprintw(LINES - 2, 0, "Path %s is not a directory", dir.c_str());
			getch();
			endwin();
			return 1;
		}
    } else {
		mvprintw(LINES - 2, 0, "Path %s does not exist", dir.c_str());
        getch();
        endwin();
        return 1;
    }

	// Create menu
	my_menu = new_menu((ITEM **)my_items);

	// Create the window to be associated with the menu
	my_menu_win = newwin((LINES - 2) / 2, (COLS - 2) / 2, 1, 1);
	keypad(my_menu_win, TRUE);

	// Set main window and sub window
	set_menu_win(my_menu, my_menu_win);
	set_menu_sub(my_menu, derwin(my_menu_win, ((LINES - 2) / 2) - 4, ((COLS - 2) / 2) - 2, 3, 1));
	set_menu_format(my_menu, ((LINES - 2) / 2) - 4, 1);

	// Set menu mark to the string " * "
	set_menu_mark(my_menu, " * ");

	// Print a border around the main window
	box(my_menu_win, 0, 0);

	// Print the directory path at the top
	mvwprintw(my_menu_win, 1, 4, "%s", dir.c_str());
	refresh();

	// Draw line under the directory path
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, ((COLS - 2) / 2) - 2);
	mvwaddch(my_menu_win, 2, ((COLS - 2) / 2)-1, ACS_RTEE);

	// Post the menu
	post_menu(my_menu);
	wrefresh(my_menu_win);

	// Help message
	attron(COLOR_PAIR(3));
	mvprintw(LINES - 2, 0, "Use PageUp and PageDown to scoll down or up a page of items");
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (q or Q to Exit)");
	attroff(COLOR_PAIR(3));
	refresh();

	// Event loop
	int c;
	while ((c = wgetch(my_menu_win)) != 'q' && c != 'Q') {
		switch (c) {
		case KEY_DOWN:
			menu_driver(my_menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			menu_driver(my_menu, REQ_UP_ITEM);
			break;
		case KEY_NPAGE:
			menu_driver(my_menu, REQ_SCR_DPAGE);
			break;
		case KEY_PPAGE:
			menu_driver(my_menu, REQ_SCR_UPAGE);
			break;
		}
		wrefresh(my_menu_win);
	}

	// Unpost and free memory taken up by the menu
	unpost_menu(my_menu);
	free_menu(my_menu);

	// Free memory taken up by items
	for (int i = 0; i < n_choices; ++i)
		free_item(my_items[i]);

	// Free memory taken up by item names
	for (int i = 0; i < n_choices; ++i)
		delete entry_names->at(i);
	delete entry_names;

	// End curses mode
	endwin();
	return 0;
}