#include <exception>
#include <memory>
#include <iostream>
#include <sstream>

#include "nc.h"


int main(int argc, char **argv)
{
    try {
        nc::standard_screen scr(nc::input_mode::raw);

        if (!has_key(0403)) {
            scr.print("No UP key :(");
            scr.getch();
            return 0;
        }

        nc::colors cs;
        cs.add_pair(nc::color_t::red, nc::color_t::black);
        cs.add_pair(nc::color_t::blue, nc::color_t::black);
        cs.add_pair(nc::color_t::green, nc::color_t::black);
        cs.add_pair(nc::color_t::yellow, nc::color_t::black);
        scr.colors(cs);

        size_t screen_width = scr.width();
        size_t screen_height = scr.height();

        //Create 3 windows:
        //  - At top, ~10 rows tall, shows player statistics
        //  - Main window, shows world (HEIGHT - 10 - 3)
        //  - Bottom: status bar-kind of thing

        nc::window stats(nc::window::create(10, screen_width, 0, 0));
        stats.attrs().color(1);
        stats.border(nc::border_t::line());

        // Show some dummy statistics
        stats.print("Str: 1", 1, 5);
        stats.print("Dex: 1", 1, 16);
        stats.print("Spd: 1", 2, 5);
        stats.print("Mag: 1", 2, 16);

        nc::window main(nc::window::create(screen_height-13, screen_width, 10, 0));
        main.keypad(true);
        main.attrs().color(2);
        main.border(nc::border_t::line());

        main.fill('#');

        size_t midx = screen_width / 2;
        size_t midy = screen_height / 2;

        for (size_t y = midy - 15; y < midy + 15; ++y)
            main.hline('.', nc::coord(y, midx-30), 60);

        nc::coord player(midy, midx);
        main.attrs().color(4);
        main.addch('@', player);

        nc::window bar(nc::window::create(3, screen_width, screen_height-3, 0));
        bar.attrs().color(3);
        bar.border(nc::border_t::line());

        {
            std::stringstream ss;
            ss << "Screen is " << screen_height << "x" << screen_width
               << "     "
               << "Player at: (" << player.row << "," << player.col << ")";
            bar.print(ss.str(), 1, 4);
        }

        stats.refresh();
        main.refresh();
        bar.refresh();

        int ch = 0;
        while ((ch = main.getch()) != 'q') {
            nc::coord last = player;
            switch (ch) {
                case KEY_UP:
                    player = nc::coord(player.row - 1, player.col);
                    break;
                case KEY_DOWN:
                    player = nc::coord(player.row + 1, player.col);
                    break;
                case KEY_LEFT:
                    player = nc::coord(player.row, player.col - 1);
                    break;
                case KEY_RIGHT:
                    player = nc::coord(player.row, player.col + 1);
                    break;
                default:
                    bar.print("Pressed: ", 1, 50);
                    bar.addch(ch);
                    break;
            }
            main.attrs().color(2);
            main.addch('.', last);
            main.attrs().color(4);
            main.addch('@', player);
            main.refresh();

            {
                std::stringstream ss;
                ss << "Screen is " << screen_height << "x" << screen_width
                << "     "
                << "Player at: (" << player.row << "," << player.col << ")";
                bar.print(ss.str(), 1, 4);
                bar.refresh();
            }
        }

        return 0;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return -1;
    }
}
