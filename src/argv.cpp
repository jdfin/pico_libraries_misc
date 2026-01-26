
#include <ctype.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
// pico
#include "pico/stdlib.h"
// misc
#include "argv.h"

Argv::Argv(int verbosity) :
    _verbosity(verbosity)
{
    reset();
}

void Argv::reset()
{
    memset(_line, 0xee, _line_max);
    _line_cnt = 0;
    //_line[0] = '\0';
    _complete = false;
}

void Argv::print() const
{
    if (_verbosity >= 2) {
        printf("input=\"");
        for (int ch = 0; ch < _line_cnt; ch++) {
            assert(ch < _line_max);
            char c = _line[ch];
            if (c <= 0x20 || c >= 0x7f)
                printf("'\\x%02X'", unsigned(c)); // not printable
            else
                printf("%c", c);
        }
        printf("\" ");
    }

    int cnt = argc();
    printf("argc=%d argv=[", cnt);

    for (int tok = 0; tok < cnt; tok++)
        printf(" %s", (*this)[tok]);

    printf(" ]\n");
}

int Argv::argc() const
{
    if (!_complete)
        return 0;

    // How many tokens before we get to _line_cnt?
    // Each token is terminated by '\0'.
    int tokens = 0;
    for (int i = 0; i < _line_cnt; i++) {
        assert(i < _line_max);
        if (_line[i] == '\0')
            tokens++;
    }
    return tokens;
}

// Add character to the line buffer.
// When a '\r' or '\n' is found, the command line is complete (return true).
bool Argv::add_char(char c)
{
    assert(_line_cnt < _line_max);
    // invariant();

    // If we get an escape, reset and start over.
    // If have a complete command yet still get more input, reset and start over.
    if (c == escape || _complete) {
        reset();
        if (_verbosity > 0)
            printf("input reset\n");
        return false;
    }

    if (_verbosity > 0) {
        printf("%c", c); // echo
        if (c == '\r')
            printf("\n");
    }

    if (isspace(c)) {
        // ignore all whitespace at the start
        if (_line_cnt == 0)
            return false;
        // _line_cnt > 0
        if (c == '\r' || c == '\n')
            _complete = true;
        // if the most recent character received was whitespace, don't add another
        assert((_line_cnt - 1) < _line_max);
        if (_line[_line_cnt - 1] == '\0')
            return _complete;
        c = '\0'; // all whitespace is replaced
    }

    // very last character can only be '\0'
    if (_line_cnt < (_line_max - 1) ||
        (_line_cnt == (_line_max - 1) && c == '\0')) {
        assert(_line_cnt < _line_max);
        _line[_line_cnt++] = c;
    } else {
        reset();
        if (_verbosity > 0)
            printf("input too long (reset)\n");
    }

    // invariant();

    return _complete;
}

bool Argv::add_str(const char *s)
{
    while (*s != '\0')
        if (add_char(*s++))
            return true;
    return false;
}

const char *Argv::operator[](int arg_num) const
{
    assert(arg_num >= 0);

    if (!_complete)
        return nullptr;

    int idx = 0;

    while (arg_num-- > 0) {
        // find next '\0' and move idx one past it
        // if we get to the end of the line, idx points at the final '\0'
        while (idx < (_line_cnt - 1) && _line[idx++] != '\0')
            ;
    }

    assert(idx < _line_max);
    return _line + idx;
}

void Argv::invariant() const
{
    assert(_line_cnt >= 0);
    // _line_cnt can be _line_max if the last character is \0
    assert((_line_cnt < _line_max) ||
           (_line_cnt == _line_max && _line[_line_max - 1] == '\0'));
    // first character not '\0' (if there is one)
    assert(_line_cnt == 0 || _line[0] != '\0');
    // no whitespace (whitespace runs all collapsed to one '\0')
    for (int i = 0; i < _line_cnt; i++)
        assert(!isspace(_line[i]));
    // no more than one '\0' in a row
    for (int i = 0; i < (_line_cnt - 1); i++)
        assert(_line[i] != '\0' || _line[i + 1] != '\0');
}

bool Argv::check(int line_cnt, const char *line, bool complete) const
{
    return (line_cnt == _line_cnt) && (memcmp(line, _line, _line_cnt) == 0) &&
           (complete == _complete);
}
