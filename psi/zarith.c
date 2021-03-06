/* Copyright (C) 2001-2012 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  7 Mt. Lassen Drive - Suite A-134, San Rafael,
   CA  94903, U.S.A., +1(415)492-9861, for further information.
*/


/* Arithmetic operators */
#include "math_.h"
#include "ghost.h"
#include "oper.h"
#include "store.h"

/****** NOTE: none of the arithmetic operators  ******/
/****** currently check for floating exceptions ******/

/*
 * Many of the procedures in this file are public only so they can be
 * called from the FunctionType 4 interpreter (zfunc4.c).
 */

/* Define max and min values for what will fit in value.intval. */
#define MIN_INTVAL 0x80000000
#define MAX_INTVAL 0x7fffffff

/* <num1> <num2> add <sum> */
/* We make this into a separate procedure because */
/* the interpreter will almost always call it directly. */
int
zop_add(register os_ptr op)
{
    switch (r_type(op)) {
    default:
        return_op_typecheck(op);
    case t_real:
        switch (r_type(op - 1)) {
        default:
            return_op_typecheck(op - 1);
        case t_real:
            op[-1].value.realval += op->value.realval;
            break;
        case t_integer:
            make_real(op - 1, (double)op[-1].value.intval + op->value.realval);
        }
        break;
    case t_integer:
        switch (r_type(op - 1)) {
        default:
            return_op_typecheck(op - 1);
        case t_real:
            op[-1].value.realval += (double)op->value.intval;
            break;
        case t_integer: {
            int int2 = op->value.intval;

            if (((op[-1].value.intval += int2) ^ int2) < 0 &&
                ((op[-1].value.intval - int2) ^ int2) >= 0
                ) {			/* Overflow, convert to real */
                make_real(op - 1, (double)(op[-1].value.intval - int2) + int2);
            }
        }
        }
    }
    return 0;
}
int
zadd(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    int code = zop_add(op);

    if (code == 0) {
        pop(1);
    }
    return code;
}

/* <num1> <num2> div <real_quotient> */
int
zdiv(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    os_ptr op1 = op - 1;

    /* We can't use the non_int_cases macro, */
    /* because we have to check explicitly for op == 0. */
    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
        case t_real:
            if (op->value.realval == 0)
                return_error(e_undefinedresult);
            switch (r_type(op1)) {
                default:
                    return_op_typecheck(op1);
                case t_real:
                    op1->value.realval /= op->value.realval;
                    break;
                case t_integer:
                    make_real(op1, (double)op1->value.intval / op->value.realval);
            }
            break;
        case t_integer:
            if (op->value.intval == 0)
                return_error(e_undefinedresult);
            switch (r_type(op1)) {
                default:
                    return_op_typecheck(op1);
                case t_real:
                    op1->value.realval /= (double)op->value.intval;
                    break;
                case t_integer:
                    make_real(op1, (double)op1->value.intval / (double)op->value.intval);
            }
    }
    pop(1);
    return 0;
}

/* <num1> <num2> mul <product> */
int
zmul(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    switch (r_type(op)) {
    default:
        return_op_typecheck(op);
    case t_real:
        switch (r_type(op - 1)) {
        default:
            return_op_typecheck(op - 1);
        case t_real:
            op[-1].value.realval *= op->value.realval;
            break;
        case t_integer:
            make_real(op - 1, (double)op[-1].value.intval * op->value.realval);
        }
        break;
    case t_integer:
        switch (r_type(op - 1)) {
        default:
            return_op_typecheck(op - 1);
        case t_real:
            op[-1].value.realval *= (double)op->value.intval;
            break;
        case t_integer: {
            double ab = (double)op[-1].value.intval * op->value.intval;
            if (ab > 2147483647.)       /* (double)0x7fffffff */
                make_real(op - 1, ab);
            else if (ab < -2147483648.) /* (double)(int)0x80000000 */
                make_real(op - 1, ab);
            else
                op[-1].value.intval = (int)ab;
        }
        }
    }
    pop(1);
    return 0;
}

/* <num1> <num2> sub <difference> */
/* We make this into a separate procedure because */
/* the interpreter will almost always call it directly. */
int
zop_sub(register os_ptr op)
{
    switch (r_type(op)) {
    default:
        return_op_typecheck(op);
    case t_real:
        switch (r_type(op - 1)) {
        default:
            return_op_typecheck(op - 1);
        case t_real:
            op[-1].value.realval -= op->value.realval;
            break;
        case t_integer:
            make_real(op - 1, (double)op[-1].value.intval - op->value.realval);
        }
        break;
    case t_integer:
        switch (r_type(op - 1)) {
        default:
            return_op_typecheck(op - 1);
        case t_real:
            op[-1].value.realval -= (double)op->value.intval;
            break;
        case t_integer: {
            int int1 = op[-1].value.intval;

            if ((int1 ^ (op[-1].value.intval = int1 - op->value.intval)) < 0 &&
                (int1 ^ op->value.intval) < 0
                ) {			/* Overflow, convert to real */
                make_real(op - 1, (float)int1 - op->value.intval);
            }
        }
        }
    }
    return 0;
}
int
zsub(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    int code = zop_sub(op);

    if (code == 0) {
        pop(1);
    }
    return code;
}

/* <num1> <num2> idiv <int_quotient> */
int
zidiv(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    check_type(*op, t_integer);
    check_type(op[-1], t_integer);
    if ((op->value.intval == 0) || (op[-1].value.intval == MIN_INTVAL && op->value.intval == -1)) {
        /* Anomalous boundary case: -MININT / -1, fail. */
        return_error(e_undefinedresult);
    }
    op[-1].value.intval /= op->value.intval;
    pop(1);
    return 0;
}

/* <int1> <int2> mod <remainder> */
int
zmod(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    check_type(*op, t_integer);
    check_type(op[-1], t_integer);
    if (op->value.intval == 0)
        return_error(e_undefinedresult);
    op[-1].value.intval %= op->value.intval;
    pop(1);
    return 0;
}

/* <num1> neg <num2> */
int
zneg(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
        case t_real:
            op->value.realval = -op->value.realval;
            break;
        case t_integer:
            if (op->value.intval == MIN_INTVAL)
                make_real(op, -(float)MIN_INTVAL);
            else
                op->value.intval = -op->value.intval;
    }
    return 0;
}

/* <num1> abs <num2> */
int
zabs(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
        case t_real:
            if (op->value.realval >= 0)
                return 0;
            break;
        case t_integer:
            if (op->value.intval >= 0)
                return 0;
            break;
    }
    return zneg(i_ctx_p);
}

/* <num1> ceiling <num2> */
int
zceiling(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
        case t_real:
            op->value.realval = ceil(op->value.realval);
        case t_integer:;
    }
    return 0;
}

/* <num1> floor <num2> */
int
zfloor(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
        case t_real:
            op->value.realval = floor(op->value.realval);
        case t_integer:;
    }
    return 0;
}

/* <num1> round <num2> */
int
zround(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
        case t_real:
            op->value.realval = floor(op->value.realval + 0.5);
        case t_integer:;
    }
    return 0;
}

/* <num1> truncate <num2> */
int
ztruncate(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    switch (r_type(op)) {
        default:
            return_op_typecheck(op);
        case t_real:
            op->value.realval =
                (op->value.realval < 0.0 ?
                 ceil(op->value.realval) :
                 floor(op->value.realval));
        case t_integer:;
    }
    return 0;
}

/* Non-standard operators */

/* <int1> <int2> .bitadd <sum> */
static int
zbitadd(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    check_type(*op, t_integer);
    check_type(op[-1], t_integer);
    op[-1].value.intval += op->value.intval;
    pop(1);
    return 0;
}

/* ------ Initialization table ------ */

const op_def zarith_op_defs[] =
{
    {"1abs", zabs},
    {"2add", zadd},
    {"2.bitadd", zbitadd},
    {"1ceiling", zceiling},
    {"2div", zdiv},
    {"2idiv", zidiv},
    {"1floor", zfloor},
    {"2mod", zmod},
    {"2mul", zmul},
    {"1neg", zneg},
    {"1round", zround},
    {"2sub", zsub},
    {"1truncate", ztruncate},
    op_def_end(0)
};
