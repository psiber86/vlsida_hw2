#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <map>
#include <utility>
#include <math.h>
#include <algorithm>

#define CELL        0x01
#define TERM1       0x02
#define TERM2       0x03
#define TERM3       0x04
#define TERM4       0x05
#define WIRE1       0x10
#define WIRE2       0x20

enum cellTypes {
    STDCELL = 1,
    FEEDTHRU
};
