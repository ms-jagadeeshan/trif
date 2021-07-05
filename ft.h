#ifndef FT_H
#define FT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>

#ifndef NULL
#define NULL '\0'
#endif

#define FT_RED "\x1B[0;41m"
#define FT_GRN "\x1B[0;32m"
#define FT_YEL "\x1B[0;33m"
#define FT_BLU "\x1B[0;34m"
#define FT_MAG "\x1B[0;35m"
#define FT_CYN "\x1B[0;36m"
#define FT_WHT "\x1B[0;37m"
#define FT_NRM "\x1B[0m"

#define FT_B_RED "\x1B[1;31m"
#define FT_B_GRN "\x1B[1;32m"
#define FT_B_YEL "\x1B[1;33m"
#define FT_B_BLU "\x1B[1;34m"
#define FT_B_MAG "\x1B[1;35m"
#define FT_B_CYN "\x1B[1;36m"
#define FT_B_WHT "\x1B[1;37m"

#define FT_BG_NRM "\x1B[40m"
#define FT_BG_RED "\x1B[41m"
#define FT_BG_GRN "\x1B[42m"
#define FT_BG_YEL "\x1B[43m"
#define FT_BG_BLU "\x1B[44m"
#define FT_BG_MAG "\x1B[45m"
#define FT_BG_CYN "\x1B[46m"
#define FT_BG_WHT "\x1B[47m"

#define ft_clear() puts("\x1B[2J")

#define ft_move_cursor(X, Y) printf("\033[%d;%dH", X, Y)

void ft_get_rows_cols(int *rows, int *cols);

#define ft_enter_alt_screen() puts("\033[?1049h\033[H")
#define ft_exit_alt_screen() puts("\033[?1049l")

void tc_echo_off();

void ft_get_rows_cols(int *rows, int *cols)
{
    struct winsize size;
    ioctl(1, TIOCGWINSZ, &size);
    *cols = size.ws_col;
    *rows = size.ws_row;
}
void ft_echo_off()
{

    struct termios term;
    tcgetattr(1, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(1, TCSANOW, &term);
}

void tc_echo_on()
{

    struct termios term;
    tcgetattr(1, &term);
    term.c_lflag |= ECHO;
    tcsetattr(1, TCSANOW, &term);
}

#endif