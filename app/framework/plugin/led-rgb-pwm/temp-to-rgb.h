// *******************************************************************
// * temp-to-rgb.h
// *
// *
// * Copyright 2015 Silicon Laboratories, Inc.                              *80*
// *******************************************************************
#ifndef __TEMP_TO_RGB_H__
#define __TEMP_TO_RGB_H__

#define TEMP_TABLE_LENGTH 100

// simple color to RGB transform tables based on algorithm from Robertson

#define RED_VALUES \
  109, \
  132, \
  147, \
  156, \
  164, \
  171, \
  177, \
  183, \
  189, \
  195, \
  202, \
  209, \
  217, \
  226, \
  238, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255 

#define GREEN_VALUES \
  154, \
  172, \
  182, \
  189, \
  194, \
  198, \
  203, \
  206, \
  210, \
  214, \
  218, \
  222, \
  227, \
  233, \
  240, \
  250, \
  250, \
  244, \
  238, \
  233, \
  228, \
  223, \
  219, \
  214, \
  210, \
  206, \
  202, \
  198, \
  195, \
  191, \
  188, \
  184, \
  181, \
  178, \
  175, \
  172, \
  170, \
  167, \
  164, \
  162, \
  159, \
  157, \
  154, \
  152, \
  150, \
  147, \
  145, \
  143, \
  141, \
  139, \
  137, \
  135, \
  133, \
  131, \
  129, \
  127, \
  126, \
  124, \
  122, \
  120, \
  119, \
  117, \
  115, \
  114, \
  112, \
  111, \
  109, \
  108, \
  106, \
  105, \
  103, \
  102, \
  101, \
  99, \
  98, \
  97, \
  95, \
  94, \
  93, \
  91, \
  90, \
  89, \
  88, \
  86, \
  85, \
  84, \
  83, \
  82, \
  81, \
  80, \
  78, \
  77, \
  76, \
  75, \
  74, \
  73, \
  72, \
  71, \
  70, \
  69

#define BLUE_VALUES \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  255, \
  244, \
  234, \
  224, \
  215, \
  206, \
  197, \
  189, \
  181, \
  174, \
  166, \
  159, \
  152, \
  145, \
  138, \
  131, \
  125, \
  118, \
  112, \
  106, \
  100, \
  94, \
  88, \
  82, \
  76, \
  70, \
  64, \
  59, \
  53, \
  47, \
  42, \
  36, \
  31, \
  25, \
  19, \
  14, \
  8, \
  3, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0, \
  0

#endif

