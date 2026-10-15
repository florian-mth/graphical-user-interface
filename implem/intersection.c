//
// Created by ringuede on 05/05/2026.
//

#include "ei_types.h"
ei_rect_t ei_rect_intersection(const ei_rect_t* a, const ei_rect_t* b) {
    ei_rect_t rect1= *a;
    ei_rect_t rect2= *b;

    int ax0= rect1.top_left.x;
    int ay0= rect1.top_left.y;
    int bx0= rect2.top_left.x;
    int by0= rect2.top_left.y;

    int ix0 = (ax0>bx0) ? ax0 : bx0;
    int iy0 = (ay0>by0) ? ay0 : by0;
    int ix1=( (ax0+rect1.size.width)<(bx0+rect2.size.width) ) ? (ax0+rect1.size.width) : (bx0+rect2.size.width);
    int iy1=( (ay0+rect1.size.height)<(by0+rect2.size.height) ) ? (ay0+rect1.size.height) : (by0+rect2.size.height);

    if (ix1<ix0 || iy1<iy0) return (ei_rect_t) {0,0,0,0};

    ei_rect_t rect = { {ix0, iy0}, {ix1-ix0, iy1-iy0} };

    return rect;
}
