#include <curses.h>
#undef getch
#undef border
#undef hline
#undef addch

namespace nc {
    /* Exceptions and Error Checking */

    struct nc_exception : public std::exception {
        const char *what() const throw() { return "nc::nc_exception"; }
    };

    struct failed_assert : public std::exception {
        const char *what() const throw() { return "failed assertion"; }
    };

    inline void check(int ret) {
        if (ERR == ret) {
            throw nc_exception();
        }
    }

    inline void ncassert(bool cond) {
        if (!cond) throw failed_assert();
    }
}

namespace nc {

    namespace geom {
        /* Useful Geometric Functionality */
        struct coord {
            size_t row;
            size_t col;
            coord(size_t r, size_t c) : row(r), col(c) {}
        };
        bool operator<(const coord& left, const coord& right) {
            return (left.row < right.row) ||
                (left.row == right.row && left.col < right.col);
        }
        bool operator==(const coord& left, const coord& right) {
            return left.row == right.row && left.col == right.col;
        }

        struct rect {
            coord tl;
            coord br;
            rect(coord ul, coord lr) : tl(ul), br(lr) { ncassert(ul < br); }

            size_t width() const { return br.col - tl.col; }
            size_t height() const { return br.row - tl.row; }
        };
        bool operator==(const rect& left, const rect& right) {
            return left.tl == right.tl && left.br == right.br;
        }
    }
    using namespace geom;


    /* Create & store a border specification */
    struct border_t {
        typedef int char_type;

        char_type l;
        char_type r;
        char_type t;
        char_type b;
        char_type ul;
        char_type ur;
        char_type ll;
        char_type lr;

        border_t(char_type c)
            : l(c), r(c), t(c), b(c), ul(c), ur(c), ll(c), lr(c) {}

        border_t(char_type v, char_type h, char_type c)
            : l(v), r(v), t(h), b(h), ul(c), ur(c), ll(c), lr(c) {}

        border_t(char_type v, char_type h,
                 char_type c1, char_type c2, char_type c3, char_type c4)
            : l(v), r(v), t(h), b(h), ul(c1), ur(c2), ll(c3), lr(c4) {}

        border_t(char_type vl, char_type vr, char_type ht, char_type hb,
                 char_type c1, char_type c2, char_type c3, char_type c4)
            : l(vl), r(vr), t(ht), b(hb), ul(c1), ur(c2), ll(c3), lr(c4) {}

        static border_t blank() {
            return border_t(' ');
        }

        static border_t line() {
            return border_t(
                ACS_VLINE,
                ACS_VLINE,
                ACS_HLINE,
                ACS_HLINE,
                ACS_ULCORNER,
                ACS_URCORNER,
                ACS_LLCORNER,
                ACS_LRCORNER
            );
        }
    };

    struct attr {

        attr() : _att(A_NORMAL) {}

        attr& color(short idx) {
            _att = (_att & ~A_COLOR) | COLOR_PAIR(idx);
            return *this;
        }

        attr& reverse(bool set) {
            if (set) { _att |= A_REVERSE; }
            else { _att &= ~A_REVERSE; }
            return *this;
        }
        attr& reverse() { return reverse(!(_att & A_REVERSE)); }

        attr_t _att;
    };

    struct window {
        /* Types for storing a pointer to a WINDOW and releasing it */
    private:
        struct deleter {
            bool owns;
            deleter() : owns(true) {}
            deleter(bool o) : owns(o) {}
            void operator()(WINDOW *p) {
                if (owns && (p)) {
                    delwin(p);
                }
            }
        };
        typedef std::unique_ptr<WINDOW, deleter> window_ptr;

        /* Creation */
    public:
        window(WINDOW *p, bool take_ownership=true)
            : _p(p, deleter(take_ownership)) {
                get_attr();
            }

        static window create(size_t height, size_t width, size_t row, size_t col) {
            return window(newwin(height, width, row, col), true);
        }

        static window create(rect bounds) {
            return create(bounds.height(), bounds.width(), bounds.tl.row, bounds.tl.col);
        }

        /* Accessors */

        size_t width() const { return getmaxx(ptr()); }
        size_t height() const { return getmaxy(ptr()); }

        coord beg() const { return coord(getbegy(ptr()), getbegx(ptr())); }
        coord cur() const { return coord(getcury(ptr()), getcurx(ptr())); }

        /* Drawing to the screen */

        void print(const std::string& s) {
            set_attr();
            check(::wprintw(ptr(), s.c_str()));
        }

        void print(const std::string& s, size_t row, size_t col) {
            set_attr();
            check(::mvwprintw(ptr(), row, col, s.c_str()));
        }

        void box(int v, int h) {
            set_attr();
            check(::box(ptr(), v, h));
        }

        void border(border_t b) {
            set_attr();
            check(::wborder(ptr(),
                            b.l, b.r, b.t, b.b, b.ul, b.ur, b.ll, b.lr));
        }

        void hline(int ch, coord start, size_t len) {
            set_attr();
            check(mvwhline(ptr(), start.row, start.col, ch, len));
        }

        void fill(int ch) {
            for (size_t y = 1; y < height() - 1; ++y)
                hline(ch, coord(y, 1), width() - 2);
        }

        void addch(int ch, coord pos) {
            set_attr();
            check(mvwaddch(ptr(), pos.row, pos.col, ch));
        }
        void addch(int ch) {
            set_attr();
            check(waddch(ptr(), ch));
        }

        void refresh() { check(::wrefresh(ptr())); }

        /* Manipulate window attributes to be applied on next draw command */
        attr &attrs() { return _att; }

        /* Input */

        int getch() { return ::wgetch(ptr()); }

        void keypad(bool enable_keypad) {
            check(::keypad(ptr(), enable_keypad));
        }

        /* Allow derived types access to pointer */
    protected:
        WINDOW *ptr() { return _p.get(); }
        const WINDOW *ptr() const { return _p.get(); }

    private:
        void set_attr() { wattrset(ptr(), _att._att); }
        void get_attr() { _att._att = ptr()->_attrs; }

    private:
        window_ptr _p;
        attr _att;
    };


    struct attr_restorer {
        window &ref;
        attr saved;

        attr_restorer(window &win) : ref(win), saved(win.attrs()) {}
        ~attr_restorer() { ref.attrs() = saved; }
    };



    enum class color_t { black, red, green, yellow, blue, magenta, cyan, white };
    struct color_pair {
        typedef short index_type;
        index_type idx;
        color_t fg;
        color_t bg;

        color_pair() : idx(0), fg(color_t::black), bg(color_t::black) {}
        color_pair(index_type i, color_t f, color_t b) : idx(i), fg(f), bg(b) {}
    };

    struct colors {
        enum { max_pairs = 8 };

        void add_pair(color_t fg, color_t bg) {
            ncassert(_count < max_pairs);
            size_t idx = _count++;
            _pairs[idx] = color_pair(static_cast<short>(idx+1), fg, bg);
        }

        size_t size() const { return _count; }

        color_pair& operator[](size_t idx) {
            ncassert( idx > 0 && idx <= _count);
            return _pairs[idx-1];
        }

        const color_pair& operator[](size_t idx) const {
            ncassert(idx > 0 && idx <= _count);
            return _pairs[idx-1];
        }

        colors() : _count(0) {}

    private:
        size_t _count;
        color_pair _pairs[max_pairs];
    };


    enum class input_mode { line, cbreak, raw };

    /* Initialize NCurses system, manage global term settings etc. */
    struct standard_screen : public window {

        standard_screen(
                nc::input_mode im = nc::input_mode::cbreak,
                bool kp = true,
                bool e = false) : window(::initscr(), false) {

            input_mode(im);
            keypad(kp);
            echo(e);

            if (::has_colors()) {
                ::start_color();
            }
        }

        void colors(nc::colors cs) {
            _colors = cs;
            for (size_t i = 1; i <= cs.size(); ++i) {
                ::init_pair(cs[i].idx, static_cast<short>(cs[i].fg), static_cast<short>(cs[i].bg));
            }
        }

        void input_mode(nc::input_mode im) {
            switch (im) {
                case nc::input_mode::line:
                    ::noraw();
                    ::nocbreak();
                    break;
                case nc::input_mode::cbreak:
                    ::cbreak();
                    break;
                case nc::input_mode::raw:
                    ::raw();
                    break;
            }
        }

        void echo(bool enable) {
            if (enable) {
                ::echo();
            } else {
                ::noecho();
            }
        }

        ~standard_screen() {
            check(::endwin());
        }

    private:
        nc::colors _colors;
    };

}
