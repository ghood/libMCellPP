/* File : libMCell.i */
%module libMCell

/* Without this being early, strings would crash in Python */
%include <std_string.i>

%{
#include "rng.h"
#include "libMCell.h"
%}

/* Use the original header file here */
%include "rng.h"
%include "libMCell.h"

