#include <stdio.h>
#include "math.h"
#include "errno.h"
#include "assert.h"

/******************************************************************************/

#define SIMPLE_STRTOD_CONTINUE 0
#define SIMPLE_STRTOD_STOP 1
#define SIMPLE_STRTOD_DENOMINATOR_MUL 10.00
#define SIMPLE_STRTOD_MUL 10.00
#define ISZERO(x) ((x) == '0')

/******************************************************************************/

struct Strtod_data
{
    double result;
    double exponent;
    double denominator;             /* Делитель десятичной дроби */
    int has_exponent;
    int lead_zeros_of_exponent;
    int lead_zeros_of_significand;
    char result_sign;
    char exponent_sign;
};

/******************************************************************************/

static inline void init_strtod(struct Strtod_data* d)
{
    d->result = 0.00;
    d->exponent = 0.00;
    d->denominator = 0.00;
    d->has_exponent = 0;
    d->lead_zeros_of_exponent = 0;
    d->lead_zeros_of_significand = 0;
    d->result_sign = 0;
    d->exponent_sign = 0;
}

/******************************************************************************/

static inline double char_to_digit(char ch)
{
    return ch - '0';
}

/******************************************************************************/

static inline void process_integral(double* dst, char ch)
{
    static const double radix = 10.0;
    *dst = *dst * radix + char_to_digit(ch);
}

/******************************************************************************/

int process_exponent(struct Strtod_data* d, char ch)
{
    if (!d->exponent && ISZERO(ch)) {         // Пропускаем лидирующие 0
        if (!d->lead_zeros_of_exponent)
            ++d->lead_zeros_of_exponent;
        return SIMPLE_STRTOD_CONTINUE;
    }
    static const double max_exponent = 308.00; // Грубое максимальное
                                               // значение порядка
    process_integral(&(d->exponent), ch);
    if (max_exponent <= d->exponent) {
        errno = ERANGE;
        return SIMPLE_STRTOD_STOP;
    }
    return SIMPLE_STRTOD_CONTINUE;
}

/******************************************************************************/

int process_significand(struct Strtod_data* d, char ch)
{
    /* Пропускаем лидирующие нули у целой части */
    if (!d->result && !d->denominator && ISZERO(ch)) {
        if (!d->lead_zeros_of_significand)
            ++d->lead_zeros_of_significand;
        return SIMPLE_STRTOD_CONTINUE;
    }

    if (d->denominator) {         /* add frational part to result */
        if (!ISZERO(ch))
            d->result += char_to_digit(ch) / d->denominator;
        d->denominator *= SIMPLE_STRTOD_DENOMINATOR_MUL;

    }
    else
        process_integral(&(d->result), ch);    /* add integral part to result */
    return SIMPLE_STRTOD_CONTINUE;
}

/******************************************************************************/

static inline int process_digits(struct Strtod_data* d, char ch)
{
    return d->has_exponent ? process_exponent(d, ch)
                           : process_significand(d, ch);
}

/******************************************************************************/

static inline int process_e(struct Strtod_data* d)
{
    return d->has_exponent++ ? SIMPLE_STRTOD_STOP : SIMPLE_STRTOD_CONTINUE;
}

/******************************************************************************/

static inline int process_point(struct Strtod_data* d)
{
    if (d->denominator || d->has_exponent)
        return SIMPLE_STRTOD_STOP;
    d->denominator = SIMPLE_STRTOD_DENOMINATOR_MUL;
    return SIMPLE_STRTOD_CONTINUE;
}

/******************************************************************************/

int process_sign(struct Strtod_data* d, char ch)
{
    if (d->has_exponent) {
        if (d->exponent_sign || d->exponent || d->lead_zeros_of_exponent)
            return SIMPLE_STRTOD_STOP;
        d->exponent_sign = ch;
        return SIMPLE_STRTOD_CONTINUE;
    }
    if (d->result_sign || d->result || d->lead_zeros_of_significand)
        return SIMPLE_STRTOD_STOP;
    d->result_sign = ch;
    return SIMPLE_STRTOD_CONTINUE;
}

/******************************************************************************/

int check_and_calculate_result(struct Strtod_data* d)
{
    if (        d->has_exponent
            && !d->exponent
            && !d->lead_zeros_of_exponent)
        return SIMPLE_STRTOD_STOP;

    if (       !d->result
            && !d->lead_zeros_of_significand
            && (d->result_sign || d->denominator))
        return SIMPLE_STRTOD_STOP;

    for (; d->exponent; --d->exponent) {
        if (d->exponent_sign == '-')
            d->result /= SIMPLE_STRTOD_DENOMINATOR_MUL;
        else
            d->result *= SIMPLE_STRTOD_MUL;
    }
    d->result = d->result_sign == '-' ? -d->result : d->result;

    return SIMPLE_STRTOD_CONTINUE;
}

/******************************************************************************/

double simple_strtod(const char* str)
{
    assert(str && "simple_strtod(): invalid arg");
    int code = SIMPLE_STRTOD_CONTINUE;
    struct Strtod_data d;
    init_strtod(&d);
    for (; *str && code != SIMPLE_STRTOD_STOP; ++str) {
        switch (*str) {
        case '0' ... '9':
            code = process_digits(&d, *str);
            break;
        case 'E': case 'e':
            code = process_e(&d);
            break;
        case '.':
            code = process_point(&d);
            break;
        case '-': case '+':
            code = process_sign(&d, *str);
            break;
        default:
            code = SIMPLE_STRTOD_STOP;
            break;
        };
    }
    if (code == SIMPLE_STRTOD_STOP)
        return NAN;
    if (check_and_calculate_result(&d) == SIMPLE_STRTOD_STOP)
        return NAN;

    return d.result;
}

/******************************************************************************/
