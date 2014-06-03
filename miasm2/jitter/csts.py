#!/usr/bin/env python
#-*- coding:utf-8 -*-


# VM Mngr Exceptions
EXCEPT_DO_NOT_UPDATE_PC = 1 << 25

EXCEPT_CODE_AUTOMOD = (1 << 0)
EXCEPT_SOFT_BP = (1 << 1)
EXCEPT_INT_XX = (1 << 2)
EXCEPT_BREAKPOINT_INTERN = (1 << 10)

EXCEPT_ACCESS_VIOL = ((1 << 14) | EXCEPT_DO_NOT_UPDATE_PC)
# VM Mngr constants

PAGE_READ = 1
PAGE_WRITE = 2
PAGE_EXEC = 4

BREAKPOINT_READ = 1
BREAKPOINT_WRITE = 2

