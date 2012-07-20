/*
 * fast_corner_detect_9.c
 *
 * Code generation for function 'fast_corner_detect_9'
 *
 * C source code generated on: Wed May  2 02:55:06 2012
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "fast_corner_detect_9.h"

/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */

/* Function Definitions */

/*
 * function coords = fast_corner_detect_9(im, threshold)
 */
void fast_corner_detect_9(const real_T im_data[24048], const int32_T im_sizes[2], real_T threshold, real_T coords_data[10000], int32_T coords_sizes[2])
{
    int32_T loop_ub;
    uint8_T sz[2];
    uint8_T cs[10000];
    int32_T nc;
    int32_T x;
    int32_T b_loop_ub;
    int32_T y;
    real_T cb;
    real_T c_b;
    boolean_T guard1 = FALSE;
    /* FAST_CORNER_DETECT_9 perform an 9 point FAST corner detection. */
    /*     corners = FAST_CORNER_DETECT_9(image, threshold) performs the detection on the image */
    /*     and returns the X coordinates in corners(:,1) and the Y coordinares in corners(:,2). */
    /*  */
    /*      If you use this in published work, please cite: */
    /*        Fusing Points and Lines for High Performance Tracking, E. Rosten and T. Drummond, ICCV 2005 */
    /*        Machine learning for high-speed corner detection, E. Rosten and T. Drummond, ECCV 2006 */
    /*      The Bibtex entries are: */
    /*       */
    /*      @inproceedings{rosten_2005_tracking, */
    /*          title       =    "Fusing points and lines for high performance tracking.", */
    /*          author      =    "Edward Rosten and Tom Drummond", */
    /*          year        =    "2005", */
    /*          month       =    "October", */
    /*          pages       =    "1508--1511", */
    /*          volume      =    "2", */
    /*          booktitle   =    "IEEE International Conference on Computer Vision", */
    /*          notes       =    "Oral presentation", */
    /*          url         =    "http://mi.eng.cam.ac.uk/~er258/work/rosten_2005_tracking.pdf" */
    /*      } */
    /*       */
    /*      @inproceedings{rosten_2006_machine, */
    /*          title       =    "Machine learning for high-speed corner detection", */
    /*          author      =    "Edward Rosten and Tom Drummond", */
    /*          year        =    "2006", */
    /*          month       =    "May", */
    /*          booktitle   =    "European Conference on Computer Vision (to appear)", */
    /*          notes       =    "Poster presentation", */
    /*          url         =    "http://mi.eng.cam.ac.uk/~er258/work/rosten_2006_machine.pdf" */
    /*      } */
    /*  */
    /*  */
    /*      Additional information from the generating program: */
    /*  */
    /*      Automatically generated code */
    /*      Parameters: */
    /*      splat_subtree = 1 */
    /*      corner_pointers = 2 */
    /*      force_first_question = 0 */
    /*      corner_type = 9 */
    /*      barrier = 25 */
    /*       */
    /*      Data: */
    /*      Number of frames:    120 */
    /*      Potential features:  25786080 */
    /*      Real features:       219300 */
    /*      Questions per pixel: 2.27059 */
    /*  */
    /*  */
    /*     See also FAST_NONMAX FAST_CORNER_DETECT_9 FAST_CORNER_DETECT_10 FAST_CORNER_DETECT_11 FAST_CORNER_DETECT_12 */
    /*  */
    /* 'fast_corner_detect_9:54' sz = size(im); */
    for (loop_ub = 0; loop_ub < 2; loop_ub++) {
        sz[loop_ub] = (uint8_T)im_sizes[loop_ub];
    }
    /* 'fast_corner_detect_9:55' xsize=sz(2); */
    /* 'fast_corner_detect_9:56' ysize=sz(1); */
    /* 'fast_corner_detect_9:57' cs = zeros(5000, 2); */
    memset((void *)&cs[0], 0, 10000U * sizeof(uint8_T));
    /* 'fast_corner_detect_9:58' nc = 0; */
    nc = -1;
    /* 'fast_corner_detect_9:59' for x = 4 : xsize - 3 */
    loop_ub = sz[1] - 3;
    for (x = 0; x + 4 <= loop_ub; x++) {
        /* 'fast_corner_detect_9:60' for y = 4 : ysize -3 */
        b_loop_ub = sz[0] - 3;
        for (y = 6; y - 2 <= b_loop_ub; y++) {
            /* 'fast_corner_detect_9:61' cb = im(y,x) + threshold; */
            cb = im_data[(y + im_sizes[0] * (x + 3)) - 3] + threshold;
            /* 'fast_corner_detect_9:62' c_b= im(y,x) - threshold; */
            c_b = im_data[(y + im_sizes[0] * (x + 3)) - 3] - threshold;
            /* 'fast_corner_detect_9:63' if im(y+0,x+3) > cb */
            guard1 = FALSE;
            if (im_data[(y + im_sizes[0] * (x + 6)) - 3] > cb) {
                /* 'fast_corner_detect_9:64' if im(y+-3,x+1) > cb */
                if (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) {
                    /* 'fast_corner_detect_9:65' if im(y+2,x+2) > cb */
                    if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                        /* 'fast_corner_detect_9:66' if im(y+-2,x+-2) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                            /* 'fast_corner_detect_9:67' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                /* 'fast_corner_detect_9:68' if im(y+1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                    /* 'fast_corner_detect_9:69' if im(y+-2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                        /* 'fast_corner_detect_9:70' if im(y+-3,x+0) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) {
                                            /* 'fast_corner_detect_9:71' if im(y+-3,x+-1) > cb */
                                            if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb))))) {
                                                /* 'fast_corner_detect_9:74' else */
                                                /* 'fast_corner_detect_9:75' if im(y+3,x+0) > cb */
                                                /* 'fast_corner_detect_9:76' if im(y+3,x+1) > cb */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:72' elseif im(y+-3,x+-1) < c_b */
                                                /* 'fast_corner_detect_9:80' else */
                                                /* 'fast_corner_detect_9:77' else */
                                            }
                                        } else if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb))) {
                                            /* 'fast_corner_detect_9:84' elseif im(y+-3,x+0) < c_b */
                                            /* 'fast_corner_detect_9:96' else */
                                            /* 'fast_corner_detect_9:93' else */
                                            /* 'fast_corner_detect_9:90' else */
                                        } else {
                                            /* 'fast_corner_detect_9:86' else */
                                            /* 'fast_corner_detect_9:87' if im(y+3,x+-1) > cb */
                                            /* 'fast_corner_detect_9:88' if im(y+3,x+0) > cb */
                                            /* 'fast_corner_detect_9:89' if im(y+3,x+1) > cb */
                                            guard1 = TRUE;
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb))) {
                                        /* 'fast_corner_detect_9:100' elseif im(y+-2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:142' else */
                                        /* 'fast_corner_detect_9:139' else */
                                        /* 'fast_corner_detect_9:136' else */
                                    } else {
                                        /* 'fast_corner_detect_9:102' else */
                                        /* 'fast_corner_detect_9:103' if im(y+3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:104' if im(y+1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:105' if im(y+2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:106' if im(y+3,x+0) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 3)] > cb) {
                                            /* 'fast_corner_detect_9:107' if im(y+3,x+1) > cb */
                                            if ((im_data[y + im_sizes[0] * (x + 4)] > cb) || (!((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb))))) {
                                                /* 'fast_corner_detect_9:110' else */
                                                /* 'fast_corner_detect_9:111' if im(y+-1,x+-3) > cb */
                                                /* 'fast_corner_detect_9:112' if im(y+-3,x+-1) > cb */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:108' elseif im(y+3,x+1) < c_b */
                                                /* 'fast_corner_detect_9:116' else */
                                                /* 'fast_corner_detect_9:113' else */
                                            }
                                        } else if ((im_data[y + im_sizes[0] * (x + 3)] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb))) {
                                            /* 'fast_corner_detect_9:120' elseif im(y+3,x+0) < c_b */
                                            /* 'fast_corner_detect_9:132' else */
                                            /* 'fast_corner_detect_9:129' else */
                                            /* 'fast_corner_detect_9:126' else */
                                        } else {
                                            /* 'fast_corner_detect_9:122' else */
                                            /* 'fast_corner_detect_9:123' if im(y+-1,x+-3) > cb */
                                            /* 'fast_corner_detect_9:124' if im(y+-3,x+-1) > cb */
                                            /* 'fast_corner_detect_9:125' if im(y+0,x+-3) > cb */
                                            guard1 = TRUE;
                                        }
                                    }
                                } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb))) {
                                    /* 'fast_corner_detect_9:146' elseif im(y+1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:184' else */
                                    /* 'fast_corner_detect_9:181' else */
                                } else {
                                    /* 'fast_corner_detect_9:148' else */
                                    /* 'fast_corner_detect_9:149' if im(y+0,x+-3) > cb */
                                    /* 'fast_corner_detect_9:150' if im(y+-1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:151' if im(y+-2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                        /* 'fast_corner_detect_9:152' if im(y+-3,x+0) > cb */
                                        if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb))))) {
                                            /* 'fast_corner_detect_9:155' else */
                                            /* 'fast_corner_detect_9:156' if im(y+3,x+-1) > cb */
                                            /* 'fast_corner_detect_9:157' if im(y+1,x+-3) > cb */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:153' elseif im(y+-3,x+0) < c_b */
                                            /* 'fast_corner_detect_9:161' else */
                                            /* 'fast_corner_detect_9:158' else */
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb))) {
                                        /* 'fast_corner_detect_9:165' elseif im(y+-2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:177' else */
                                        /* 'fast_corner_detect_9:174' else */
                                        /* 'fast_corner_detect_9:171' else */
                                    } else {
                                        /* 'fast_corner_detect_9:167' else */
                                        /* 'fast_corner_detect_9:168' if im(y+3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:169' if im(y+1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:170' if im(y+2,x+-2) > cb */
                                        guard1 = TRUE;
                                    }
                                }
                            } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb))) {
                                /* 'fast_corner_detect_9:188' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:235' else */
                                /* 'fast_corner_detect_9:232' else */
                                /* 'fast_corner_detect_9:229' else */
                            } else {
                                /* 'fast_corner_detect_9:190' else */
                                /* 'fast_corner_detect_9:191' if im(y+0,x+-3) > cb */
                                /* 'fast_corner_detect_9:192' if im(y+2,x+-2) > cb */
                                /* 'fast_corner_detect_9:193' if im(y+1,x+-3) > cb */
                                /* 'fast_corner_detect_9:194' if im(y+3,x+-1) > cb */
                                if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                    /* 'fast_corner_detect_9:195' if im(y+3,x+1) > cb */
                                    if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                        /* 'fast_corner_detect_9:196' if im(y+1,x+3) > cb */
                                        if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) || (!((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb))))) {
                                            /* 'fast_corner_detect_9:199' else */
                                            /* 'fast_corner_detect_9:200' if im(y+-1,x+-3) > cb */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:197' elseif im(y+1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:201' else */
                                        }
                                    } else if ((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb))) {
                                        /* 'fast_corner_detect_9:205' elseif im(y+3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:209' else */
                                    } else {
                                        /* 'fast_corner_detect_9:207' else */
                                        /* 'fast_corner_detect_9:208' if im(y+-1,x+-3) > cb */
                                        guard1 = TRUE;
                                    }
                                } else if ((im_data[y + im_sizes[0] * (x + 2)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb))) {
                                    /* 'fast_corner_detect_9:213' elseif im(y+3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:225' else */
                                    /* 'fast_corner_detect_9:222' else */
                                    /* 'fast_corner_detect_9:219' else */
                                } else {
                                    /* 'fast_corner_detect_9:215' else */
                                    /* 'fast_corner_detect_9:216' if im(y+-2,x+2) > cb */
                                    /* 'fast_corner_detect_9:217' if im(y+-1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:218' if im(y+-3,x+-1) > cb */
                                    guard1 = TRUE;
                                }
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                            /* 'fast_corner_detect_9:239' elseif im(y+-2,x+-2) < c_b */
                            /* 'fast_corner_detect_9:240' if im(y+3,x+-1) > cb */
                            if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                /* 'fast_corner_detect_9:241' if im(y+-1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                    /* 'fast_corner_detect_9:242' if im(y+1,x+3) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                        /* 'fast_corner_detect_9:243' if im(y+-2,x+2) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                            /* 'fast_corner_detect_9:244' if im(y+3,x+1) > cb */
                                            if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:245' else */
                                            }
                                        } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb))) {
                                            /* 'fast_corner_detect_9:248' elseif im(y+-2,x+2) < c_b */
                                            /* 'fast_corner_detect_9:252' else */
                                        } else {
                                            /* 'fast_corner_detect_9:250' else */
                                            /* 'fast_corner_detect_9:251' if im(y+1,x+-3) > cb */
                                            guard1 = TRUE;
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:256' else */
                                    }
                                } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb))) {
                                    /* 'fast_corner_detect_9:259' elseif im(y+-1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:267' else */
                                    /* 'fast_corner_detect_9:264' else */
                                } else {
                                    /* 'fast_corner_detect_9:261' else */
                                    /* 'fast_corner_detect_9:262' if im(y+0,x+-3) > cb */
                                    /* 'fast_corner_detect_9:263' if im(y+3,x+0) > cb */
                                    guard1 = TRUE;
                                }
                            } else if (im_data[y + im_sizes[0] * (x + 2)] < c_b) {
                                /* 'fast_corner_detect_9:271' elseif im(y+3,x+-1) < c_b */
                                /* 'fast_corner_detect_9:272' if im(y+-3,x+-1) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                    /* 'fast_corner_detect_9:273' if im(y+3,x+1) > cb */
                                    if ((im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb)) {
                                        /* 'fast_corner_detect_9:274' if im(y+-2,x+2) > cb */
                                        /* 'fast_corner_detect_9:275' if im(y+-3,x+0) > cb */
                                        /* 'fast_corner_detect_9:276' if im(y+1,x+3) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:286' else */
                                        /* 'fast_corner_detect_9:283' else */
                                        /* 'fast_corner_detect_9:280' else */
                                        /* 'fast_corner_detect_9:277' else */
                                    }
                                } else if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b)) {
                                    /* 'fast_corner_detect_9:289' elseif im(y+-3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:290' if im(y+3,x+1) < c_b */
                                    /* 'fast_corner_detect_9:291' if im(y+-1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:292' if im(y+0,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:293' if im(y+1,x+-3) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:306' else */
                                    /* 'fast_corner_detect_9:303' else */
                                    /* 'fast_corner_detect_9:300' else */
                                    /* 'fast_corner_detect_9:297' else */
                                    /* 'fast_corner_detect_9:294' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:309' else */
                                /* 'fast_corner_detect_9:310' if im(y+-3,x+0) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) {
                                    /* 'fast_corner_detect_9:311' if im(y+3,x+0) > cb */
                                    if (im_data[y + im_sizes[0] * (x + 3)] > cb) {
                                        /* 'fast_corner_detect_9:312' if im(y+-2,x+2) > cb */
                                        if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb)) {
                                            /* 'fast_corner_detect_9:313' if im(y+1,x+3) > cb */
                                            /* 'fast_corner_detect_9:314' if im(y+-1,x+3) > cb */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:321' else */
                                            /* 'fast_corner_detect_9:318' else */
                                            /* 'fast_corner_detect_9:315' else */
                                        }
                                    } else if ((im_data[y + im_sizes[0] * (x + 3)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb))) {
                                        /* 'fast_corner_detect_9:324' elseif im(y+3,x+0) < c_b */
                                        /* 'fast_corner_detect_9:340' else */
                                        /* 'fast_corner_detect_9:337' else */
                                        /* 'fast_corner_detect_9:334' else */
                                        /* 'fast_corner_detect_9:331' else */
                                    } else {
                                        /* 'fast_corner_detect_9:326' else */
                                        /* 'fast_corner_detect_9:327' if im(y+-3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:328' if im(y+-2,x+2) > cb */
                                        /* 'fast_corner_detect_9:329' if im(y+3,x+1) > cb */
                                        /* 'fast_corner_detect_9:330' if im(y+1,x+3) > cb */
                                        guard1 = TRUE;
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:344' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:348' else */
                            /* 'fast_corner_detect_9:349' if im(y+3,x+1) > cb */
                            if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                /* 'fast_corner_detect_9:350' if im(y+-1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                    /* 'fast_corner_detect_9:351' if im(y+-3,x+-1) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                        /* 'fast_corner_detect_9:352' if im(y+1,x+3) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                            /* 'fast_corner_detect_9:353' if im(y+-2,x+2) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                                /* 'fast_corner_detect_9:354' if im(y+-3,x+0) > cb */
                                                if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb))))) {
                                                    /* 'fast_corner_detect_9:357' else */
                                                    /* 'fast_corner_detect_9:358' if im(y+3,x+-1) > cb */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:355' elseif im(y+-3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:359' else */
                                                }
                                            } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb))) {
                                                /* 'fast_corner_detect_9:363' elseif im(y+-2,x+2) < c_b */
                                                /* 'fast_corner_detect_9:379' else */
                                                /* 'fast_corner_detect_9:376' else */
                                                /* 'fast_corner_detect_9:373' else */
                                                /* 'fast_corner_detect_9:370' else */
                                            } else {
                                                /* 'fast_corner_detect_9:365' else */
                                                /* 'fast_corner_detect_9:366' if im(y+1,x+-3) > cb */
                                                /* 'fast_corner_detect_9:367' if im(y+3,x+-1) > cb */
                                                /* 'fast_corner_detect_9:368' if im(y+3,x+0) > cb */
                                                /* 'fast_corner_detect_9:369' if im(y+2,x+-2) > cb */
                                                guard1 = TRUE;
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:383' else */
                                        }
                                    } else if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                        /* 'fast_corner_detect_9:386' elseif im(y+-3,x+-1) < c_b */
                                        /* 'fast_corner_detect_9:387' if im(y+3,x+-1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:388' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:391' else */
                                        /* 'fast_corner_detect_9:392' if im(y+3,x+-1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                            /* 'fast_corner_detect_9:393' if im(y+1,x+3) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                                /* 'fast_corner_detect_9:394' if im(y+-2,x+2) > cb */
                                                if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                                    /* 'fast_corner_detect_9:395' if im(y+3,x+0) > cb */
                                                    if (im_data[y + im_sizes[0] * (x + 3)] > cb) {
                                                        guard1 = TRUE;
                                                    } else {
                                                        /* 'fast_corner_detect_9:396' else */
                                                    }
                                                } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb))) {
                                                    /* 'fast_corner_detect_9:399' elseif im(y+-2,x+2) < c_b */
                                                    /* 'fast_corner_detect_9:407' else */
                                                    /* 'fast_corner_detect_9:404' else */
                                                } else {
                                                    /* 'fast_corner_detect_9:401' else */
                                                    /* 'fast_corner_detect_9:402' if im(y+1,x+-3) > cb */
                                                    /* 'fast_corner_detect_9:403' if im(y+2,x+-2) > cb */
                                                    guard1 = TRUE;
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:411' else */
                                            }
                                        } else if ((im_data[y + im_sizes[0] * (x + 2)] < c_b) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb))) {
                                            /* 'fast_corner_detect_9:414' elseif im(y+3,x+-1) < c_b */
                                            /* 'fast_corner_detect_9:430' else */
                                            /* 'fast_corner_detect_9:427' else */
                                            /* 'fast_corner_detect_9:424' else */
                                            /* 'fast_corner_detect_9:421' else */
                                        } else {
                                            /* 'fast_corner_detect_9:416' else */
                                            /* 'fast_corner_detect_9:417' if im(y+3,x+0) > cb */
                                            /* 'fast_corner_detect_9:418' if im(y+-3,x+0) > cb */
                                            /* 'fast_corner_detect_9:419' if im(y+-2,x+2) > cb */
                                            /* 'fast_corner_detect_9:420' if im(y+1,x+3) > cb */
                                            guard1 = TRUE;
                                        }
                                    }
                                } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb))) {
                                    /* 'fast_corner_detect_9:435' elseif im(y+-1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:455' else */
                                    /* 'fast_corner_detect_9:452' else */
                                    /* 'fast_corner_detect_9:449' else */
                                    /* 'fast_corner_detect_9:446' else */
                                    /* 'fast_corner_detect_9:443' else */
                                } else {
                                    /* 'fast_corner_detect_9:437' else */
                                    /* 'fast_corner_detect_9:438' if im(y+0,x+-3) > cb */
                                    /* 'fast_corner_detect_9:439' if im(y+2,x+-2) > cb */
                                    /* 'fast_corner_detect_9:440' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:441' if im(y+3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:442' if im(y+1,x+3) > cb */
                                    guard1 = TRUE;
                                }
                            } else {
                                /* 'fast_corner_detect_9:459' else */
                            }
                        }
                    } else if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                        /* 'fast_corner_detect_9:463' elseif im(y+2,x+2) < c_b */
                        /* 'fast_corner_detect_9:464' if im(y+-2,x+-2) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                            /* 'fast_corner_detect_9:465' if im(y+0,x+-3) > cb */
                            if (im_data[(y + im_sizes[0] * x) - 3] > cb) {
                                /* 'fast_corner_detect_9:466' if im(y+-1,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) {
                                    /* 'fast_corner_detect_9:467' if im(y+-3,x+0) > cb */
                                    /* 'fast_corner_detect_9:468' if im(y+-2,x+2) > cb */
                                    /* 'fast_corner_detect_9:469' if im(y+-1,x+3) > cb */
                                    /* 'fast_corner_detect_9:470' if im(y+-3,x+-1) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:483' else */
                                    /* 'fast_corner_detect_9:480' else */
                                    /* 'fast_corner_detect_9:477' else */
                                    /* 'fast_corner_detect_9:474' else */
                                    /* 'fast_corner_detect_9:471' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * x) - 3] < c_b) {
                                /* 'fast_corner_detect_9:486' elseif im(y+0,x+-3) < c_b */
                                /* 'fast_corner_detect_9:487' if im(y+-1,x+-3) > cb */
                                if (im_data[(y + im_sizes[0] * x) - 4] > cb) {
                                    /* 'fast_corner_detect_9:488' if im(y+1,x+3) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:489' else */
                                    }
                                } else if ((im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b)) {
                                    /* 'fast_corner_detect_9:492' elseif im(y+-1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:493' if im(y+1,x+3) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:497' else */
                                    /* 'fast_corner_detect_9:494' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:500' else */
                                /* 'fast_corner_detect_9:501' if im(y+1,x+3) > cb */
                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) && (im_data[(y + im_sizes[0] * x) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb)) {
                                    /* 'fast_corner_detect_9:502' if im(y+-1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:503' if im(y+-1,x+3) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:510' else */
                                    /* 'fast_corner_detect_9:507' else */
                                    /* 'fast_corner_detect_9:504' else */
                                }
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                            /* 'fast_corner_detect_9:514' elseif im(y+-2,x+-2) < c_b */
                            /* 'fast_corner_detect_9:515' if im(y+3,x+0) < c_b */
                            if ((im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[y + im_sizes[0] * (x + 2)] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b)) {
                                /* 'fast_corner_detect_9:516' if im(y+-1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:517' if im(y+0,x+-3) < c_b */
                                /* 'fast_corner_detect_9:518' if im(y+3,x+-1) < c_b */
                                /* 'fast_corner_detect_9:519' if im(y+3,x+1) < c_b */
                                /* 'fast_corner_detect_9:520' if im(y+1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:521' if im(y+2,x+-2) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:540' else */
                                /* 'fast_corner_detect_9:537' else */
                                /* 'fast_corner_detect_9:534' else */
                                /* 'fast_corner_detect_9:531' else */
                                /* 'fast_corner_detect_9:528' else */
                                /* 'fast_corner_detect_9:525' else */
                                /* 'fast_corner_detect_9:522' else */
                            }
                        } else {
                            /* 'fast_corner_detect_9:543' else */
                            /* 'fast_corner_detect_9:544' if im(y+1,x+3) < c_b */
                            if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[y + im_sizes[0] * (x + 2)] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b)) {
                                /* 'fast_corner_detect_9:545' if im(y+-1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:546' if im(y+3,x+-1) < c_b */
                                /* 'fast_corner_detect_9:547' if im(y+3,x+1) < c_b */
                                /* 'fast_corner_detect_9:548' if im(y+3,x+0) < c_b */
                                /* 'fast_corner_detect_9:549' if im(y+-2,x+2) > cb */
                                /* 'fast_corner_detect_9:550' if im(y+0,x+-3) < c_b */
                                /* 'fast_corner_detect_9:551' if im(y+2,x+-2) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:573' else */
                                /* 'fast_corner_detect_9:570' else */
                                /* 'fast_corner_detect_9:567' else */
                                /* 'fast_corner_detect_9:564' else */
                                /* 'fast_corner_detect_9:561' else */
                                /* 'fast_corner_detect_9:558' else */
                                /* 'fast_corner_detect_9:555' else */
                                /* 'fast_corner_detect_9:552' else */
                            }
                        }
                    } else {
                        /* 'fast_corner_detect_9:577' else */
                        /* 'fast_corner_detect_9:578' if im(y+-1,x+-3) > cb */
                        if (im_data[(y + im_sizes[0] * x) - 4] > cb) {
                            /* 'fast_corner_detect_9:579' if im(y+1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                /* 'fast_corner_detect_9:580' if im(y+-2,x+-2) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                    /* 'fast_corner_detect_9:581' if im(y+-2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                        /* 'fast_corner_detect_9:582' if im(y+-3,x+0) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) {
                                            /* 'fast_corner_detect_9:583' if im(y+-1,x+3) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                                /* 'fast_corner_detect_9:584' if im(y+-3,x+-1) > cb */
                                                if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:585' else */
                                                }
                                            } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb))) {
                                                /* 'fast_corner_detect_9:588' elseif im(y+-1,x+3) < c_b */
                                                /* 'fast_corner_detect_9:600' else */
                                                /* 'fast_corner_detect_9:597' else */
                                                /* 'fast_corner_detect_9:594' else */
                                            } else {
                                                /* 'fast_corner_detect_9:590' else */
                                                /* 'fast_corner_detect_9:591' if im(y+2,x+-2) > cb */
                                                /* 'fast_corner_detect_9:592' if im(y+0,x+-3) > cb */
                                                /* 'fast_corner_detect_9:593' if im(y+1,x+-3) > cb */
                                                guard1 = TRUE;
                                            }
                                        } else if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb))) {
                                            /* 'fast_corner_detect_9:604' elseif im(y+-3,x+0) < c_b */
                                            /* 'fast_corner_detect_9:616' else */
                                            /* 'fast_corner_detect_9:613' else */
                                            /* 'fast_corner_detect_9:610' else */
                                        } else {
                                            /* 'fast_corner_detect_9:606' else */
                                            /* 'fast_corner_detect_9:607' if im(y+3,x+1) > cb */
                                            /* 'fast_corner_detect_9:608' if im(y+-3,x+-1) > cb */
                                            /* 'fast_corner_detect_9:609' if im(y+0,x+-3) > cb */
                                            guard1 = TRUE;
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb))) {
                                        /* 'fast_corner_detect_9:620' elseif im(y+-2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:640' else */
                                        /* 'fast_corner_detect_9:637' else */
                                        /* 'fast_corner_detect_9:634' else */
                                        /* 'fast_corner_detect_9:631' else */
                                        /* 'fast_corner_detect_9:628' else */
                                    } else {
                                        /* 'fast_corner_detect_9:622' else */
                                        /* 'fast_corner_detect_9:623' if im(y+3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:624' if im(y+2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:625' if im(y+0,x+-3) > cb */
                                        /* 'fast_corner_detect_9:626' if im(y+1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:627' if im(y+-3,x+0) > cb */
                                        guard1 = TRUE;
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:644' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                /* 'fast_corner_detect_9:647' elseif im(y+1,x+3) < c_b */
                                /* 'fast_corner_detect_9:648' if im(y+0,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) {
                                    /* 'fast_corner_detect_9:649' if im(y+-3,x+-1) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:653' else */
                                    /* 'fast_corner_detect_9:650' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:656' else */
                                /* 'fast_corner_detect_9:657' if im(y+0,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) {
                                    /* 'fast_corner_detect_9:658' if im(y+-3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:659' if im(y+-2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                        /* 'fast_corner_detect_9:660' if im(y+-2,x+-2) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                            /* 'fast_corner_detect_9:661' if im(y+-3,x+0) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) {
                                                /* 'fast_corner_detect_9:662' if im(y+-1,x+3) > cb */
                                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) || (!((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb))))) {
                                                    /* 'fast_corner_detect_9:665' else */
                                                    /* 'fast_corner_detect_9:666' if im(y+2,x+-2) > cb */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:663' elseif im(y+-1,x+3) < c_b */
                                                    /* 'fast_corner_detect_9:667' else */
                                                }
                                            } else if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb))) {
                                                /* 'fast_corner_detect_9:671' elseif im(y+-3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:683' else */
                                                /* 'fast_corner_detect_9:680' else */
                                                /* 'fast_corner_detect_9:677' else */
                                            } else {
                                                /* 'fast_corner_detect_9:673' else */
                                                /* 'fast_corner_detect_9:674' if im(y+3,x+1) > cb */
                                                /* 'fast_corner_detect_9:675' if im(y+2,x+-2) > cb */
                                                /* 'fast_corner_detect_9:676' if im(y+1,x+-3) > cb */
                                                guard1 = TRUE;
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:687' else */
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!((im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb))))))) {
                                        /* 'fast_corner_detect_9:690' elseif im(y+-2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:715' else */
                                        /* 'fast_corner_detect_9:712' else */
                                        /* 'fast_corner_detect_9:709' else */
                                        /* 'fast_corner_detect_9:706' else */
                                        /* 'fast_corner_detect_9:698' elseif im(y+-3,x+0) < c_b */
                                        /* 'fast_corner_detect_9:702' else */
                                    } else {
                                        /* 'fast_corner_detect_9:692' else */
                                        /* 'fast_corner_detect_9:693' if im(y+3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:694' if im(y+2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:695' if im(y+-2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:696' if im(y+1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:697' if im(y+-3,x+0) > cb */
                                        /* 'fast_corner_detect_9:700' else */
                                        /* 'fast_corner_detect_9:701' if im(y+3,x+1) > cb */
                                        guard1 = TRUE;
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:722' else */
                                    /* 'fast_corner_detect_9:719' else */
                                }
                            }
                        } else if ((im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b)) {
                            /* 'fast_corner_detect_9:726' elseif im(y+-1,x+-3) < c_b */
                            /* 'fast_corner_detect_9:727' if im(y+-3,x+-1) < c_b */
                            /* 'fast_corner_detect_9:728' if im(y+3,x+1) < c_b */
                            /* 'fast_corner_detect_9:729' if im(y+0,x+-3) < c_b */
                            /* 'fast_corner_detect_9:730' if im(y+3,x+0) < c_b */
                            /* 'fast_corner_detect_9:731' if im(y+2,x+-2) < c_b */
                            /* 'fast_corner_detect_9:732' if im(y+-2,x+-2) < c_b */
                            /* 'fast_corner_detect_9:733' if im(y+1,x+-3) < c_b */
                            guard1 = TRUE;
                        } else {
                            /* 'fast_corner_detect_9:755' else */
                            /* 'fast_corner_detect_9:752' else */
                            /* 'fast_corner_detect_9:749' else */
                            /* 'fast_corner_detect_9:746' else */
                            /* 'fast_corner_detect_9:743' else */
                            /* 'fast_corner_detect_9:740' else */
                            /* 'fast_corner_detect_9:737' else */
                            /* 'fast_corner_detect_9:734' else */
                        }
                    }
                } else if (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) {
                    /* 'fast_corner_detect_9:759' elseif im(y+-3,x+1) < c_b */
                    /* 'fast_corner_detect_9:760' if im(y+2,x+-2) > cb */
                    if (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) {
                        /* 'fast_corner_detect_9:761' if im(y+1,x+-3) > cb */
                        if (im_data[(y + im_sizes[0] * x) - 2] > cb) {
                            /* 'fast_corner_detect_9:762' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                /* 'fast_corner_detect_9:763' if im(y+3,x+0) > cb */
                                if (im_data[y + im_sizes[0] * (x + 3)] > cb) {
                                    /* 'fast_corner_detect_9:764' if im(y+1,x+3) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                        /* 'fast_corner_detect_9:765' if im(y+3,x+1) > cb */
                                        if ((im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb)) {
                                            /* 'fast_corner_detect_9:766' if im(y+2,x+2) > cb */
                                            /* 'fast_corner_detect_9:767' if im(y+3,x+-1) > cb */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:774' else */
                                            /* 'fast_corner_detect_9:771' else */
                                            /* 'fast_corner_detect_9:768' else */
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb))) {
                                        /* 'fast_corner_detect_9:777' elseif im(y+1,x+3) < c_b */
                                        /* 'fast_corner_detect_9:789' else */
                                        /* 'fast_corner_detect_9:786' else */
                                        /* 'fast_corner_detect_9:783' else */
                                    } else {
                                        /* 'fast_corner_detect_9:779' else */
                                        /* 'fast_corner_detect_9:780' if im(y+-2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:781' if im(y+0,x+-3) > cb */
                                        /* 'fast_corner_detect_9:782' if im(y+2,x+2) > cb */
                                        guard1 = TRUE;
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:793' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                /* 'fast_corner_detect_9:796' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:797' if im(y+0,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[y + im_sizes[0] * (x + 3)] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb)) {
                                    /* 'fast_corner_detect_9:798' if im(y+3,x+1) > cb */
                                    /* 'fast_corner_detect_9:799' if im(y+3,x+0) > cb */
                                    /* 'fast_corner_detect_9:800' if im(y+1,x+3) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:810' else */
                                    /* 'fast_corner_detect_9:807' else */
                                    /* 'fast_corner_detect_9:804' else */
                                    /* 'fast_corner_detect_9:801' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:813' else */
                                /* 'fast_corner_detect_9:814' if im(y+0,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[y + im_sizes[0] * (x + 3)] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb)) {
                                    /* 'fast_corner_detect_9:815' if im(y+3,x+0) > cb */
                                    /* 'fast_corner_detect_9:816' if im(y+3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:817' if im(y+2,x+2) > cb */
                                    /* 'fast_corner_detect_9:818' if im(y+1,x+3) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                        /* 'fast_corner_detect_9:819' if im(y+3,x+1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:820' else */
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb))) {
                                        /* 'fast_corner_detect_9:823' elseif im(y+1,x+3) < c_b */
                                        /* 'fast_corner_detect_9:827' else */
                                    } else {
                                        /* 'fast_corner_detect_9:825' else */
                                        /* 'fast_corner_detect_9:826' if im(y+-2,x+-2) > cb */
                                        guard1 = TRUE;
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:840' else */
                                    /* 'fast_corner_detect_9:837' else */
                                    /* 'fast_corner_detect_9:834' else */
                                    /* 'fast_corner_detect_9:831' else */
                                }
                            }
                        } else if (im_data[(y + im_sizes[0] * x) - 2] < c_b) {
                            /* 'fast_corner_detect_9:844' elseif im(y+1,x+-3) < c_b */
                            /* 'fast_corner_detect_9:845' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                /* 'fast_corner_detect_9:846' if im(y+-2,x+2) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:847' else */
                                }
                            } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b)) {
                                /* 'fast_corner_detect_9:850' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:851' if im(y+-2,x+-2) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:855' else */
                                /* 'fast_corner_detect_9:852' else */
                            }
                        } else {
                            /* 'fast_corner_detect_9:858' else */
                            /* 'fast_corner_detect_9:859' if im(y+-2,x+2) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                /* 'fast_corner_detect_9:860' if im(y+-1,x+-3) > cb */
                                if (im_data[(y + im_sizes[0] * x) - 4] > cb) {
                                    /* 'fast_corner_detect_9:861' if im(y+-3,x+0) > cb||im(y+-3,x+0) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) || (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb)) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb))) {
                                        /* 'fast_corner_detect_9:873' else */
                                        /* 'fast_corner_detect_9:870' else */
                                        /* 'fast_corner_detect_9:867' else */
                                    } else {
                                        /* 'fast_corner_detect_9:863' else */
                                        /* 'fast_corner_detect_9:864' if im(y+-1,x+3) > cb */
                                        /* 'fast_corner_detect_9:865' if im(y+1,x+3) > cb */
                                        /* 'fast_corner_detect_9:866' if im(y+3,x+-1) > cb */
                                        guard1 = TRUE;
                                    }
                                } else if (im_data[(y + im_sizes[0] * x) - 4] < c_b) {
                                    /* 'fast_corner_detect_9:877' elseif im(y+-1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:878' if im(y+3,x+0) > cb */
                                    if ((im_data[y + im_sizes[0] * (x + 3)] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb)) {
                                        /* 'fast_corner_detect_9:879' if im(y+1,x+3) > cb */
                                        /* 'fast_corner_detect_9:880' if im(y+3,x+-1) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:887' else */
                                        /* 'fast_corner_detect_9:884' else */
                                        /* 'fast_corner_detect_9:881' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:890' else */
                                    /* 'fast_corner_detect_9:891' if im(y+2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:892' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:896' else */
                            }
                        }
                    } else if (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) {
                        /* 'fast_corner_detect_9:900' elseif im(y+2,x+-2) < c_b */
                        /* 'fast_corner_detect_9:901' if im(y+-1,x+-3) < c_b */
                        if (im_data[(y + im_sizes[0] * x) - 4] < c_b) {
                            /* 'fast_corner_detect_9:902' if im(y+3,x+-1) > cb */
                            if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                /* 'fast_corner_detect_9:903' if im(y+-2,x+2) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] > cb)) {
                                    /* 'fast_corner_detect_9:904' if im(y+3,x+1) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:908' else */
                                    /* 'fast_corner_detect_9:905' else */
                                }
                            } else if (im_data[y + im_sizes[0] * (x + 2)] < c_b) {
                                /* 'fast_corner_detect_9:911' elseif im(y+3,x+-1) < c_b */
                                /* 'fast_corner_detect_9:912' if im(y+0,x+-3) < c_b */
                                if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb))) {
                                    /* 'fast_corner_detect_9:913' if im(y+-2,x+-2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                                        /* 'fast_corner_detect_9:915' elseif im(y+-2,x+-2) < c_b */
                                        /* 'fast_corner_detect_9:916' if im(y+1,x+-3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb))) {
                                            /* 'fast_corner_detect_9:917' if im(y+-3,x+-1) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                                /* 'fast_corner_detect_9:919' elseif im(y+-3,x+-1) < c_b */
                                                /* 'fast_corner_detect_9:920' if im(y+-3,x+0) > cb */
                                                if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (im_data[y + im_sizes[0] * (x + 4)] < c_b)))) {
                                                    /* 'fast_corner_detect_9:923' else */
                                                    /* 'fast_corner_detect_9:924' if im(y+3,x+1) < c_b */
                                                    /* 'fast_corner_detect_9:925' else */
                                                } else {
                                                    /* 'fast_corner_detect_9:922' elseif im(y+-3,x+0) < c_b */
                                                    guard1 = TRUE;
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:929' else */
                                                /* 'fast_corner_detect_9:930' if im(y+2,x+2) < c_b */
                                                if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b)) {
                                                    /* 'fast_corner_detect_9:931' if im(y+3,x+0) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:935' else */
                                                    /* 'fast_corner_detect_9:932' else */
                                                }
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:939' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:942' else */
                                        /* 'fast_corner_detect_9:943' if im(y+1,x+3) < c_b */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:944' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:948' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:951' else */
                                /* 'fast_corner_detect_9:952' if im(y+-2,x+2) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                    /* 'fast_corner_detect_9:953' if im(y+1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:954' if im(y+-2,x+-2) < c_b */
                                    /* 'fast_corner_detect_9:955' if im(y+0,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:956' if im(y+-3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:957' if im(y+-3,x+0) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:973' else */
                                    /* 'fast_corner_detect_9:970' else */
                                    /* 'fast_corner_detect_9:967' else */
                                    /* 'fast_corner_detect_9:964' else */
                                    /* 'fast_corner_detect_9:961' else */
                                    /* 'fast_corner_detect_9:958' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:977' else */
                        }
                    } else {
                        /* 'fast_corner_detect_9:980' else */
                        /* 'fast_corner_detect_9:981' if im(y+-1,x+3) < c_b */
                        if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                            /* 'fast_corner_detect_9:982' if im(y+1,x+-3) < c_b */
                            /* 'fast_corner_detect_9:983' if im(y+-1,x+-3) < c_b */
                            /* 'fast_corner_detect_9:984' if im(y+0,x+-3) < c_b */
                            /* 'fast_corner_detect_9:985' if im(y+-2,x+-2) < c_b */
                            /* 'fast_corner_detect_9:986' if im(y+-3,x+-1) < c_b */
                            guard1 = TRUE;
                        } else {
                            /* 'fast_corner_detect_9:1002' else */
                            /* 'fast_corner_detect_9:999' else */
                            /* 'fast_corner_detect_9:996' else */
                            /* 'fast_corner_detect_9:993' else */
                            /* 'fast_corner_detect_9:990' else */
                            /* 'fast_corner_detect_9:987' else */
                        }
                    }
                } else {
                    /* 'fast_corner_detect_9:1006' else */
                    /* 'fast_corner_detect_9:1007' if im(y+2,x+-2) > cb */
                    if (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) {
                        /* 'fast_corner_detect_9:1008' if im(y+3,x+0) > cb */
                        if (im_data[y + im_sizes[0] * (x + 3)] > cb) {
                            /* 'fast_corner_detect_9:1009' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                /* 'fast_corner_detect_9:1010' if im(y+1,x+-3) > cb */
                                if (im_data[(y + im_sizes[0] * x) - 2] > cb) {
                                    /* 'fast_corner_detect_9:1011' if im(y+3,x+-1) > cb */
                                    if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                        /* 'fast_corner_detect_9:1012' if im(y+1,x+3) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                            /* 'fast_corner_detect_9:1013' if im(y+2,x+2) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                                /* 'fast_corner_detect_9:1014' if im(y+3,x+1) > cb */
                                                if ((im_data[y + im_sizes[0] * (x + 4)] > cb) || (!((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb))))) {
                                                    /* 'fast_corner_detect_9:1017' else */
                                                    /* 'fast_corner_detect_9:1018' if im(y+-3,x+0) > cb */
                                                    /* 'fast_corner_detect_9:1019' if im(y+-2,x+-2) > cb */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:1015' elseif im(y+3,x+1) < c_b */
                                                    /* 'fast_corner_detect_9:1023' else */
                                                    /* 'fast_corner_detect_9:1020' else */
                                                }
                                            } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb))) {
                                                /* 'fast_corner_detect_9:1027' elseif im(y+2,x+2) < c_b */
                                                /* 'fast_corner_detect_9:1048' else */
                                                /* 'fast_corner_detect_9:1045' else */
                                            } else {
                                                /* 'fast_corner_detect_9:1029' else */
                                                /* 'fast_corner_detect_9:1030' if im(y+-3,x+-1) > cb */
                                                /* 'fast_corner_detect_9:1031' if im(y+0,x+-3) > cb */
                                                /* 'fast_corner_detect_9:1032' if im(y+3,x+1) > cb */
                                                if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                                    /* 'fast_corner_detect_9:1033' if im(y+-2,x+-2) > cb */
                                                    if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                                        guard1 = TRUE;
                                                    } else {
                                                        /* 'fast_corner_detect_9:1034' else */
                                                    }
                                                } else if ((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb))) {
                                                    /* 'fast_corner_detect_9:1037' elseif im(y+3,x+1) < c_b */
                                                    /* 'fast_corner_detect_9:1041' else */
                                                } else {
                                                    /* 'fast_corner_detect_9:1039' else */
                                                    /* 'fast_corner_detect_9:1040' if im(y+-3,x+0) > cb */
                                                    guard1 = TRUE;
                                                }
                                            }
                                        } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb))) {
                                            /* 'fast_corner_detect_9:1052' elseif im(y+1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:1086' else */
                                            /* 'fast_corner_detect_9:1083' else */
                                        } else {
                                            /* 'fast_corner_detect_9:1054' else */
                                            /* 'fast_corner_detect_9:1055' if im(y+-2,x+-2) > cb */
                                            /* 'fast_corner_detect_9:1056' if im(y+0,x+-3) > cb */
                                            /* 'fast_corner_detect_9:1057' if im(y+2,x+2) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                                /* 'fast_corner_detect_9:1058' if im(y+-1,x+-3) > cb */
                                                if (im_data[(y + im_sizes[0] * x) - 4] > cb) {
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:1059' else */
                                                }
                                            } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb)) || (!((im_data[y + im_sizes[0] * (x + 4)] > cb) || (!((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb))))))) {
                                                /* 'fast_corner_detect_9:1062' elseif im(y+2,x+2) < c_b */
                                                /* 'fast_corner_detect_9:1079' else */
                                                /* 'fast_corner_detect_9:1076' else */
                                                /* 'fast_corner_detect_9:1068' elseif im(y+3,x+1) < c_b */
                                                /* 'fast_corner_detect_9:1072' else */
                                            } else {
                                                /* 'fast_corner_detect_9:1064' else */
                                                /* 'fast_corner_detect_9:1065' if im(y+-3,x+-1) > cb */
                                                /* 'fast_corner_detect_9:1066' if im(y+-1,x+-3) > cb */
                                                /* 'fast_corner_detect_9:1067' if im(y+3,x+1) > cb */
                                                /* 'fast_corner_detect_9:1070' else */
                                                /* 'fast_corner_detect_9:1071' if im(y+-3,x+0) > cb */
                                                guard1 = TRUE;
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1090' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1093' else */
                                    /* 'fast_corner_detect_9:1094' if im(y+-2,x+2) > cb */
                                    if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb)) {
                                        /* 'fast_corner_detect_9:1095' if im(y+1,x+3) > cb */
                                        /* 'fast_corner_detect_9:1096' if im(y+2,x+2) > cb */
                                        /* 'fast_corner_detect_9:1097' if im(y+3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:1098' if im(y+3,x+1) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1111' else */
                                        /* 'fast_corner_detect_9:1108' else */
                                        /* 'fast_corner_detect_9:1105' else */
                                        /* 'fast_corner_detect_9:1102' else */
                                        /* 'fast_corner_detect_9:1099' else */
                                    }
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                /* 'fast_corner_detect_9:1115' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:1116' if im(y+0,x+-3) > cb */
                                if (im_data[(y + im_sizes[0] * x) - 3] > cb) {
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1117' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:1120' else */
                                /* 'fast_corner_detect_9:1121' if im(y+0,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb)) {
                                    /* 'fast_corner_detect_9:1122' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:1123' if im(y+3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:1124' if im(y+2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                        /* 'fast_corner_detect_9:1125' if im(y+3,x+1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                            /* 'fast_corner_detect_9:1126' if im(y+1,x+3) > cb */
                                            if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) || (!((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb))))) {
                                                /* 'fast_corner_detect_9:1129' else */
                                                /* 'fast_corner_detect_9:1130' if im(y+-2,x+-2) > cb */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1127' elseif im(y+1,x+3) < c_b */
                                                /* 'fast_corner_detect_9:1131' else */
                                            }
                                        } else if ((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb))) {
                                            /* 'fast_corner_detect_9:1135' elseif im(y+3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:1139' else */
                                        } else {
                                            /* 'fast_corner_detect_9:1137' else */
                                            /* 'fast_corner_detect_9:1138' if im(y+-3,x+0) > cb */
                                            guard1 = TRUE;
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb))) {
                                        /* 'fast_corner_detect_9:1143' elseif im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:1164' else */
                                    } else {
                                        /* 'fast_corner_detect_9:1145' else */
                                        /* 'fast_corner_detect_9:1146' if im(y+-3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:1147' if im(y+3,x+1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                            /* 'fast_corner_detect_9:1148' if im(y+-2,x+-2) > cb */
                                            if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) && (im_data[(y + im_sizes[0] * x) - 4] > cb)) {
                                                /* 'fast_corner_detect_9:1149' if im(y+-1,x+-3) > cb */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1153' else */
                                                /* 'fast_corner_detect_9:1150' else */
                                            }
                                        } else if ((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb))) {
                                            /* 'fast_corner_detect_9:1156' elseif im(y+3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:1160' else */
                                        } else {
                                            /* 'fast_corner_detect_9:1158' else */
                                            /* 'fast_corner_detect_9:1159' if im(y+-3,x+0) > cb */
                                            guard1 = TRUE;
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1174' else */
                                    /* 'fast_corner_detect_9:1171' else */
                                    /* 'fast_corner_detect_9:1168' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:1178' else */
                        }
                    } else if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b)) {
                        /* 'fast_corner_detect_9:1181' elseif im(y+2,x+-2) < c_b */
                        /* 'fast_corner_detect_9:1182' if im(y+3,x+0) < c_b */
                        /* 'fast_corner_detect_9:1183' if im(y+-2,x+-2) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                            /* 'fast_corner_detect_9:1184' if im(y+-1,x+-3) < c_b */
                            if ((im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b)) {
                                /* 'fast_corner_detect_9:1185' if im(y+1,x+3) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:1189' else */
                                /* 'fast_corner_detect_9:1186' else */
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                            /* 'fast_corner_detect_9:1192' elseif im(y+-2,x+-2) < c_b */
                            /* 'fast_corner_detect_9:1193' if im(y+0,x+-3) < c_b */
                            if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb))) {
                                /* 'fast_corner_detect_9:1194' if im(y+-3,x+-1) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                    /* 'fast_corner_detect_9:1196' elseif im(y+-3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:1197' if im(y+3,x+1) > cb */
                                    if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                    } else if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                        /* 'fast_corner_detect_9:1199' elseif im(y+3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:1200' if im(y+1,x+-3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[y + im_sizes[0] * (x + 2)] < c_b)) {
                                            /* 'fast_corner_detect_9:1201' if im(y+-1,x+-3) < c_b */
                                            /* 'fast_corner_detect_9:1202' if im(y+3,x+-1) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1209' else */
                                            /* 'fast_corner_detect_9:1206' else */
                                            /* 'fast_corner_detect_9:1203' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1212' else */
                                        /* 'fast_corner_detect_9:1213' if im(y+-3,x+0) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b)) {
                                            /* 'fast_corner_detect_9:1214' if im(y+-1,x+-3) < c_b */
                                            /* 'fast_corner_detect_9:1215' if im(y+1,x+-3) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1222' else */
                                            /* 'fast_corner_detect_9:1219' else */
                                            /* 'fast_corner_detect_9:1216' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1226' else */
                                    /* 'fast_corner_detect_9:1227' if im(y+2,x+2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[y + im_sizes[0] * (x + 2)] < c_b)) {
                                        /* 'fast_corner_detect_9:1228' if im(y+-1,x+-3) < c_b */
                                        /* 'fast_corner_detect_9:1229' if im(y+1,x+-3) < c_b */
                                        /* 'fast_corner_detect_9:1230' if im(y+3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:1231' if im(y+3,x+-1) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1244' else */
                                        /* 'fast_corner_detect_9:1241' else */
                                        /* 'fast_corner_detect_9:1238' else */
                                        /* 'fast_corner_detect_9:1235' else */
                                        /* 'fast_corner_detect_9:1232' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:1248' else */
                            }
                        } else {
                            /* 'fast_corner_detect_9:1251' else */
                            /* 'fast_corner_detect_9:1252' if im(y+1,x+3) < c_b */
                            if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[y + im_sizes[0] * (x + 2)] < c_b)) {
                                /* 'fast_corner_detect_9:1253' if im(y+-1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:1254' if im(y+0,x+-3) < c_b */
                                /* 'fast_corner_detect_9:1255' if im(y+3,x+1) < c_b */
                                /* 'fast_corner_detect_9:1256' if im(y+1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:1257' if im(y+3,x+-1) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:1273' else */
                                /* 'fast_corner_detect_9:1270' else */
                                /* 'fast_corner_detect_9:1267' else */
                                /* 'fast_corner_detect_9:1264' else */
                                /* 'fast_corner_detect_9:1261' else */
                                /* 'fast_corner_detect_9:1258' else */
                            }
                        }
                    } else {
                        /* 'fast_corner_detect_9:1280' else */
                        /* 'fast_corner_detect_9:1277' else */
                    }
                }
            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 3] < c_b) {
                /* 'fast_corner_detect_9:1284' elseif im(y+0,x+3) < c_b */
                /* 'fast_corner_detect_9:1285' if im(y+3,x+-1) > cb */
                if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                    /* 'fast_corner_detect_9:1286' if im(y+-1,x+-3) > cb */
                    if (im_data[(y + im_sizes[0] * x) - 4] > cb) {
                        /* 'fast_corner_detect_9:1287' if im(y+-3,x+-1) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                            /* 'fast_corner_detect_9:1288' if im(y+1,x+-3) > cb */
                            if (im_data[(y + im_sizes[0] * x) - 2] > cb) {
                                /* 'fast_corner_detect_9:1289' if im(y+3,x+1) > cb */
                                if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                    /* 'fast_corner_detect_9:1290' if im(y+0,x+-3) > cb */
                                    if (im_data[(y + im_sizes[0] * x) - 3] > cb) {
                                        /* 'fast_corner_detect_9:1291' if im(y+2,x+-2) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) {
                                            /* 'fast_corner_detect_9:1292' if im(y+-2,x+-2) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                                /* 'fast_corner_detect_9:1293' if im(y+3,x+0) > cb */
                                                if (im_data[y + im_sizes[0] * (x + 3)] > cb) {
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:1294' else */
                                                }
                                            } else if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb))) {
                                                /* 'fast_corner_detect_9:1297' elseif im(y+-2,x+-2) < c_b */
                                                /* 'fast_corner_detect_9:1301' else */
                                            } else {
                                                /* 'fast_corner_detect_9:1299' else */
                                                /* 'fast_corner_detect_9:1300' if im(y+1,x+3) > cb */
                                                guard1 = TRUE;
                                            }
                                        } else if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb))) {
                                            /* 'fast_corner_detect_9:1305' elseif im(y+2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:1309' else */
                                        } else {
                                            /* 'fast_corner_detect_9:1307' else */
                                            /* 'fast_corner_detect_9:1308' if im(y+-1,x+3) > cb */
                                            guard1 = TRUE;
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1313' else */
                                    }
                                } else if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                    /* 'fast_corner_detect_9:1316' elseif im(y+3,x+1) < c_b */
                                    /* 'fast_corner_detect_9:1317' if im(y+-3,x+1) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) {
                                        /* 'fast_corner_detect_9:1318' if im(y+0,x+-3) > cb */
                                        if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb)) {
                                            /* 'fast_corner_detect_9:1319' if im(y+-2,x+-2) > cb */
                                            /* 'fast_corner_detect_9:1320' if im(y+2,x+-2) > cb */
                                            /* 'fast_corner_detect_9:1321' if im(y+-3,x+0) > cb */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1331' else */
                                            /* 'fast_corner_detect_9:1328' else */
                                            /* 'fast_corner_detect_9:1325' else */
                                            /* 'fast_corner_detect_9:1322' else */
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb))) {
                                        /* 'fast_corner_detect_9:1334' elseif im(y+-3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:1342' else */
                                        /* 'fast_corner_detect_9:1339' else */
                                    } else {
                                        /* 'fast_corner_detect_9:1336' else */
                                        /* 'fast_corner_detect_9:1337' if im(y+3,x+0) > cb */
                                        /* 'fast_corner_detect_9:1338' if im(y+-3,x+0) > cb */
                                        guard1 = TRUE;
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1346' else */
                                    /* 'fast_corner_detect_9:1347' if im(y+-3,x+1) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) {
                                        /* 'fast_corner_detect_9:1348' if im(y+0,x+-3) > cb */
                                        if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb)) {
                                            /* 'fast_corner_detect_9:1349' if im(y+2,x+-2) > cb */
                                            /* 'fast_corner_detect_9:1350' if im(y+-2,x+-2) > cb */
                                            /* 'fast_corner_detect_9:1351' if im(y+-3,x+0) > cb */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1361' else */
                                            /* 'fast_corner_detect_9:1358' else */
                                            /* 'fast_corner_detect_9:1355' else */
                                            /* 'fast_corner_detect_9:1352' else */
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb)) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 3] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb))) {
                                        /* 'fast_corner_detect_9:1364' elseif im(y+-3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:1384' else */
                                        /* 'fast_corner_detect_9:1381' else */
                                        /* 'fast_corner_detect_9:1378' else */
                                        /* 'fast_corner_detect_9:1375' else */
                                        /* 'fast_corner_detect_9:1372' else */
                                    } else {
                                        /* 'fast_corner_detect_9:1366' else */
                                        /* 'fast_corner_detect_9:1367' if im(y+-3,x+0) > cb */
                                        /* 'fast_corner_detect_9:1368' if im(y+3,x+0) > cb */
                                        /* 'fast_corner_detect_9:1369' if im(y+0,x+-3) > cb */
                                        /* 'fast_corner_detect_9:1370' if im(y+2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:1371' if im(y+-2,x+-2) > cb */
                                        guard1 = TRUE;
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:1389' else */
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                            /* 'fast_corner_detect_9:1392' elseif im(y+-3,x+-1) < c_b */
                            /* 'fast_corner_detect_9:1393' if im(y+1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                /* 'fast_corner_detect_9:1394' if im(y+0,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) && (im_data[y + im_sizes[0] * (x + 3)] > cb)) {
                                    /* 'fast_corner_detect_9:1395' if im(y+3,x+1) > cb */
                                    /* 'fast_corner_detect_9:1396' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:1397' if im(y+2,x+2) > cb */
                                    /* 'fast_corner_detect_9:1398' if im(y+3,x+0) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1411' else */
                                    /* 'fast_corner_detect_9:1408' else */
                                    /* 'fast_corner_detect_9:1405' else */
                                    /* 'fast_corner_detect_9:1402' else */
                                    /* 'fast_corner_detect_9:1399' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                /* 'fast_corner_detect_9:1414' elseif im(y+1,x+3) < c_b */
                                /* 'fast_corner_detect_9:1415' if im(y+3,x+1) > cb */
                                if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                    /* 'fast_corner_detect_9:1416' if im(y+2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                        /* 'fast_corner_detect_9:1417' if im(y+-2,x+-2) > cb */
                                        if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b)) {
                                            /* 'fast_corner_detect_9:1418' if im(y+-2,x+2) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1422' else */
                                            /* 'fast_corner_detect_9:1419' else */
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b)) {
                                        /* 'fast_corner_detect_9:1425' elseif im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:1426' if im(y+-2,x+-2) < c_b */
                                        /* 'fast_corner_detect_9:1427' if im(y+-1,x+3) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1434' else */
                                        /* 'fast_corner_detect_9:1431' else */
                                        /* 'fast_corner_detect_9:1428' else */
                                    }
                                } else if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                    /* 'fast_corner_detect_9:1437' elseif im(y+3,x+1) < c_b */
                                    /* 'fast_corner_detect_9:1438' if im(y+-1,x+3) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b)) {
                                        /* 'fast_corner_detect_9:1439' if im(y+-2,x+2) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1443' else */
                                        /* 'fast_corner_detect_9:1440' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1446' else */
                                    /* 'fast_corner_detect_9:1447' if im(y+-2,x+-2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b)) {
                                        /* 'fast_corner_detect_9:1448' if im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:1449' if im(y+-1,x+3) < c_b */
                                        /* 'fast_corner_detect_9:1450' if im(y+-2,x+2) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1460' else */
                                        /* 'fast_corner_detect_9:1457' else */
                                        /* 'fast_corner_detect_9:1454' else */
                                        /* 'fast_corner_detect_9:1451' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:1464' else */
                                /* 'fast_corner_detect_9:1465' if im(y+-2,x+-2) > cb */
                                if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b)) {
                                    /* 'fast_corner_detect_9:1466' if im(y+2,x+2) > cb */
                                    /* 'fast_corner_detect_9:1467' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:1468' if im(y+3,x+1) > cb */
                                    /* 'fast_corner_detect_9:1469' if im(y+-1,x+3) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1482' else */
                                    /* 'fast_corner_detect_9:1479' else */
                                    /* 'fast_corner_detect_9:1476' else */
                                    /* 'fast_corner_detect_9:1473' else */
                                    /* 'fast_corner_detect_9:1470' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:1486' else */
                            /* 'fast_corner_detect_9:1487' if im(y+2,x+2) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                /* 'fast_corner_detect_9:1488' if im(y+-2,x+-2) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                    /* 'fast_corner_detect_9:1489' if im(y+0,x+-3) > cb */
                                    if ((im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[y + im_sizes[0] * (x + 3)] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) {
                                        /* 'fast_corner_detect_9:1490' if im(y+1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:1491' if im(y+3,x+1) > cb */
                                        /* 'fast_corner_detect_9:1492' if im(y+3,x+0) > cb */
                                        /* 'fast_corner_detect_9:1493' if im(y+2,x+-2) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1506' else */
                                        /* 'fast_corner_detect_9:1503' else */
                                        /* 'fast_corner_detect_9:1500' else */
                                        /* 'fast_corner_detect_9:1497' else */
                                        /* 'fast_corner_detect_9:1494' else */
                                    }
                                } else if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                                    /* 'fast_corner_detect_9:1509' elseif im(y+-2,x+-2) < c_b */
                                    /* 'fast_corner_detect_9:1510' if im(y+1,x+3) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1511' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1514' else */
                                    /* 'fast_corner_detect_9:1515' if im(y+1,x+3) > cb */
                                    if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) && (im_data[y + im_sizes[0] * (x + 3)] > cb)) {
                                        /* 'fast_corner_detect_9:1516' if im(y+3,x+1) > cb */
                                        /* 'fast_corner_detect_9:1517' if im(y+0,x+-3) > cb */
                                        /* 'fast_corner_detect_9:1518' if im(y+1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:1519' if im(y+2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:1520' if im(y+3,x+0) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1536' else */
                                        /* 'fast_corner_detect_9:1533' else */
                                        /* 'fast_corner_detect_9:1530' else */
                                        /* 'fast_corner_detect_9:1527' else */
                                        /* 'fast_corner_detect_9:1524' else */
                                        /* 'fast_corner_detect_9:1521' else */
                                    }
                                }
                            } else if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                /* 'fast_corner_detect_9:1540' elseif im(y+2,x+2) < c_b */
                                /* 'fast_corner_detect_9:1541' if im(y+3,x+0) < c_b */
                                /* 'fast_corner_detect_9:1542' if im(y+-3,x+0) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:1549' else */
                                /* 'fast_corner_detect_9:1546' else */
                                /* 'fast_corner_detect_9:1543' else */
                            }
                        }
                    } else if (im_data[(y + im_sizes[0] * x) - 4] < c_b) {
                        /* 'fast_corner_detect_9:1553' elseif im(y+-1,x+-3) < c_b */
                        /* 'fast_corner_detect_9:1554' if im(y+1,x+3) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                            /* 'fast_corner_detect_9:1555' if im(y+0,x+-3) < c_b */
                            if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                /* 'fast_corner_detect_9:1556' if im(y+-3,x+1) < c_b */
                                /* 'fast_corner_detect_9:1557' if im(y+-2,x+2) < c_b */
                                /* 'fast_corner_detect_9:1558' if im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:1559' if im(y+-3,x+-1) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:1572' else */
                                /* 'fast_corner_detect_9:1569' else */
                                /* 'fast_corner_detect_9:1566' else */
                                /* 'fast_corner_detect_9:1563' else */
                                /* 'fast_corner_detect_9:1560' else */
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                            /* 'fast_corner_detect_9:1575' elseif im(y+1,x+3) < c_b */
                            /* 'fast_corner_detect_9:1576' if im(y+-3,x+0) < c_b */
                            if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb))) {
                                /* 'fast_corner_detect_9:1577' if im(y+-1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                    /* 'fast_corner_detect_9:1579' elseif im(y+-1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:1580' if im(y+-3,x+-1) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb))) {
                                        /* 'fast_corner_detect_9:1581' if im(y+-3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:1582' if im(y+-2,x+-2) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                                            /* 'fast_corner_detect_9:1584' elseif im(y+-2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:1585' if im(y+-2,x+2) < c_b */
                                            if (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) {
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1586' else */
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:1589' else */
                                            /* 'fast_corner_detect_9:1590' if im(y+3,x+1) < c_b */
                                            if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1591' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1598' else */
                                        /* 'fast_corner_detect_9:1595' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1601' else */
                                    /* 'fast_corner_detect_9:1602' if im(y+2,x+-2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb)) && (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b)) && (im_data[y + im_sizes[0] * (x + 4)] > cb)) {
                                        /* 'fast_corner_detect_9:1603' if im(y+2,x+2) > cb||im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:1605' else */
                                        /* 'fast_corner_detect_9:1606' if im(y+3,x+1) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1611' else */
                                        /* 'fast_corner_detect_9:1607' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:1615' else */
                            }
                        } else {
                            /* 'fast_corner_detect_9:1618' else */
                            /* 'fast_corner_detect_9:1619' if im(y+0,x+-3) < c_b */
                            if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb))) {
                                /* 'fast_corner_detect_9:1620' if im(y+-2,x+2) < c_b */
                                /* 'fast_corner_detect_9:1621' if im(y+-2,x+-2) < c_b */
                                /* 'fast_corner_detect_9:1622' if im(y+-3,x+1) < c_b */
                                /* 'fast_corner_detect_9:1623' if im(y+-1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                    /* 'fast_corner_detect_9:1625' elseif im(y+-1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:1626' if im(y+-3,x+0) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                        /* 'fast_corner_detect_9:1627' if im(y+-3,x+-1) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1631' else */
                                        /* 'fast_corner_detect_9:1628' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1634' else */
                                    /* 'fast_corner_detect_9:1635' if im(y+2,x+-2) < c_b */
                                    if (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) {
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1636' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:1649' else */
                                /* 'fast_corner_detect_9:1646' else */
                                /* 'fast_corner_detect_9:1643' else */
                                /* 'fast_corner_detect_9:1640' else */
                            }
                        }
                    } else {
                        /* 'fast_corner_detect_9:1653' else */
                        /* 'fast_corner_detect_9:1654' if im(y+2,x+2) < c_b */
                        if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb))) {
                            /* 'fast_corner_detect_9:1655' if im(y+-2,x+-2) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                                /* 'fast_corner_detect_9:1657' elseif im(y+-2,x+-2) < c_b */
                                /* 'fast_corner_detect_9:1658' if im(y+-3,x+0) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b)) {
                                    /* 'fast_corner_detect_9:1659' if im(y+-1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:1660' if im(y+1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:1661' if im(y+-3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:1662' if im(y+-3,x+1) < c_b */
                                    /* 'fast_corner_detect_9:1663' if im(y+-2,x+2) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1679' else */
                                    /* 'fast_corner_detect_9:1676' else */
                                    /* 'fast_corner_detect_9:1673' else */
                                    /* 'fast_corner_detect_9:1670' else */
                                    /* 'fast_corner_detect_9:1667' else */
                                    /* 'fast_corner_detect_9:1664' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:1682' else */
                                /* 'fast_corner_detect_9:1683' if im(y+3,x+1) < c_b */
                                if ((im_data[y + im_sizes[0] * (x + 4)] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb))) {
                                    /* 'fast_corner_detect_9:1684' if im(y+-3,x+-1) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                        /* 'fast_corner_detect_9:1686' elseif im(y+-3,x+-1) < c_b */
                                        /* 'fast_corner_detect_9:1687' if im(y+1,x+3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b)) {
                                            /* 'fast_corner_detect_9:1688' if im(y+-2,x+2) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1692' else */
                                            /* 'fast_corner_detect_9:1689' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1695' else */
                                        /* 'fast_corner_detect_9:1696' if im(y+3,x+0) < c_b */
                                        if ((im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b)) {
                                            /* 'fast_corner_detect_9:1697' if im(y+-3,x+1) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1701' else */
                                            /* 'fast_corner_detect_9:1698' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1705' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:1709' else */
                        }
                    }
                } else if (im_data[y + im_sizes[0] * (x + 2)] < c_b) {
                    /* 'fast_corner_detect_9:1713' elseif im(y+3,x+-1) < c_b */
                    /* 'fast_corner_detect_9:1714' if im(y+-2,x+2) > cb */
                    if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                        /* 'fast_corner_detect_9:1715' if im(y+0,x+-3) > cb */
                        if (im_data[(y + im_sizes[0] * x) - 3] > cb) {
                            /* 'fast_corner_detect_9:1716' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                /* 'fast_corner_detect_9:1717' if im(y+1,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb)) {
                                    /* 'fast_corner_detect_9:1718' if im(y+-3,x+1) > cb */
                                    /* 'fast_corner_detect_9:1719' if im(y+-2,x+-2) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1726' else */
                                    /* 'fast_corner_detect_9:1723' else */
                                    /* 'fast_corner_detect_9:1720' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                /* 'fast_corner_detect_9:1729' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:1730' if im(y+1,x+-3) > cb */
                                if (im_data[(y + im_sizes[0] * x) - 2] > cb) {
                                    /* 'fast_corner_detect_9:1731' if im(y+2,x+-2) > cb */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) && (im_data[(y + im_sizes[0] * x) - 4] > cb)) {
                                        /* 'fast_corner_detect_9:1732' if im(y+-1,x+-3) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1736' else */
                                        /* 'fast_corner_detect_9:1733' else */
                                    }
                                } else if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b)) {
                                    /* 'fast_corner_detect_9:1739' elseif im(y+1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:1740' if im(y+2,x+2) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1744' else */
                                    /* 'fast_corner_detect_9:1741' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:1747' else */
                                /* 'fast_corner_detect_9:1748' if im(y+2,x+-2) > cb */
                                if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b)) {
                                    /* 'fast_corner_detect_9:1749' if im(y+2,x+2) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1753' else */
                                    /* 'fast_corner_detect_9:1750' else */
                                }
                            }
                        } else if (im_data[(y + im_sizes[0] * x) - 3] < c_b) {
                            /* 'fast_corner_detect_9:1757' elseif im(y+0,x+-3) < c_b */
                            /* 'fast_corner_detect_9:1758' if im(y+2,x+2) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                            } else if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                                /* 'fast_corner_detect_9:1760' elseif im(y+2,x+2) < c_b */
                                /* 'fast_corner_detect_9:1761' if im(y+1,x+-3) < c_b */
                                if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb))) {
                                    /* 'fast_corner_detect_9:1762' if im(y+1,x+3) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                        /* 'fast_corner_detect_9:1764' elseif im(y+1,x+3) < c_b */
                                        /* 'fast_corner_detect_9:1765' if im(y+2,x+-2) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b)) {
                                            /* 'fast_corner_detect_9:1766' if im(y+3,x+0) < c_b */
                                            /* 'fast_corner_detect_9:1767' if im(y+3,x+1) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1774' else */
                                            /* 'fast_corner_detect_9:1771' else */
                                            /* 'fast_corner_detect_9:1768' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1777' else */
                                        /* 'fast_corner_detect_9:1778' if im(y+-3,x+-1) < c_b */
                                        if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1779' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1783' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:1786' else */
                                /* 'fast_corner_detect_9:1787' if im(y+-3,x+-1) < c_b */
                                if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:1788' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:1792' else */
                            /* 'fast_corner_detect_9:1793' if im(y+-1,x+3) < c_b */
                            if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b)) {
                                /* 'fast_corner_detect_9:1794' if im(y+1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:1795' if im(y+3,x+0) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:1802' else */
                                /* 'fast_corner_detect_9:1799' else */
                                /* 'fast_corner_detect_9:1796' else */
                            }
                        }
                    } else if (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) {
                        /* 'fast_corner_detect_9:1806' elseif im(y+-2,x+2) < c_b */
                        /* 'fast_corner_detect_9:1807' if im(y+1,x+3) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                            /* 'fast_corner_detect_9:1808' if im(y+0,x+-3) < c_b */
                            if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                /* 'fast_corner_detect_9:1809' if im(y+-1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:1810' if im(y+-3,x+-1) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:1817' else */
                                /* 'fast_corner_detect_9:1814' else */
                                /* 'fast_corner_detect_9:1811' else */
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                            /* 'fast_corner_detect_9:1820' elseif im(y+1,x+3) < c_b */
                            /* 'fast_corner_detect_9:1821' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                /* 'fast_corner_detect_9:1823' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:1824' if im(y+-3,x+1) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) {
                                    /* 'fast_corner_detect_9:1825' if im(y+2,x+-2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b)) {
                                        /* 'fast_corner_detect_9:1826' if im(y+3,x+0) < c_b */
                                        /* 'fast_corner_detect_9:1827' if im(y+3,x+1) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1834' else */
                                        /* 'fast_corner_detect_9:1831' else */
                                        /* 'fast_corner_detect_9:1828' else */
                                    }
                                } else if (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) {
                                    /* 'fast_corner_detect_9:1837' elseif im(y+-3,x+1) < c_b */
                                    /* 'fast_corner_detect_9:1838' if im(y+2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                        /* 'fast_corner_detect_9:1839' if im(y+-1,x+-3) < c_b */
                                        if (im_data[(y + im_sizes[0] * x) - 4] < c_b) {
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1840' else */
                                        }
                                    } else if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                                        /* 'fast_corner_detect_9:1843' elseif im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:1844' if im(y+3,x+1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                        } else if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                            /* 'fast_corner_detect_9:1846' elseif im(y+3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:1847' if im(y+3,x+0) > cb */
                                            if ((im_data[y + im_sizes[0] * (x + 3)] > cb) || (!((im_data[y + im_sizes[0] * (x + 3)] < c_b) || ((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b))))) {
                                                /* 'fast_corner_detect_9:1850' else */
                                                /* 'fast_corner_detect_9:1851' if im(y+-3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:1856' else */
                                                /* 'fast_corner_detect_9:1853' else */
                                            } else {
                                                /* 'fast_corner_detect_9:1849' elseif im(y+3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:1852' if im(y+-3,x+-1) < c_b */
                                                guard1 = TRUE;
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:1860' else */
                                            /* 'fast_corner_detect_9:1861' if im(y+-2,x+-2) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                                /* 'fast_corner_detect_9:1862' if im(y+-3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:1863' if im(y+-3,x+-1) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1870' else */
                                                /* 'fast_corner_detect_9:1867' else */
                                                /* 'fast_corner_detect_9:1864' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1874' else */
                                        /* 'fast_corner_detect_9:1875' if im(y+-1,x+-3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                            /* 'fast_corner_detect_9:1876' if im(y+-2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:1877' if im(y+-3,x+0) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1884' else */
                                            /* 'fast_corner_detect_9:1881' else */
                                            /* 'fast_corner_detect_9:1878' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1888' else */
                                    /* 'fast_corner_detect_9:1889' if im(y+2,x+-2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b)) {
                                        /* 'fast_corner_detect_9:1890' if im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:1891' if im(y+3,x+0) < c_b */
                                        /* 'fast_corner_detect_9:1892' if im(y+3,x+1) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:1902' else */
                                        /* 'fast_corner_detect_9:1899' else */
                                        /* 'fast_corner_detect_9:1896' else */
                                        /* 'fast_corner_detect_9:1893' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:1906' else */
                                /* 'fast_corner_detect_9:1907' if im(y+0,x+-3) < c_b */
                                if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b)) {
                                    /* 'fast_corner_detect_9:1908' if im(y+2,x+-2) < c_b */
                                    /* 'fast_corner_detect_9:1909' if im(y+1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:1910' if im(y+-3,x+-1) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                        guard1 = TRUE;
                                    } else if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                        /* 'fast_corner_detect_9:1911' elseif im(y+-3,x+-1) < c_b */
                                        /* 'fast_corner_detect_9:1912' if im(y+3,x+1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                        } else if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                            /* 'fast_corner_detect_9:1914' elseif im(y+3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:1915' if im(y+2,x+2) > cb */
                                            if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) || (!((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) || (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b)))) {
                                                /* 'fast_corner_detect_9:1918' else */
                                                /* 'fast_corner_detect_9:1919' if im(y+-2,x+-2) < c_b */
                                                /* 'fast_corner_detect_9:1920' else */
                                            } else {
                                                /* 'fast_corner_detect_9:1917' elseif im(y+2,x+2) < c_b */
                                                guard1 = TRUE;
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:1924' else */
                                            /* 'fast_corner_detect_9:1925' if im(y+-2,x+-2) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b)) {
                                                /* 'fast_corner_detect_9:1926' if im(y+-3,x+1) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1930' else */
                                                /* 'fast_corner_detect_9:1927' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1934' else */
                                        /* 'fast_corner_detect_9:1935' if im(y+3,x+1) < c_b */
                                        if ((im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b)) {
                                            /* 'fast_corner_detect_9:1936' if im(y+2,x+2) < c_b */
                                            /* 'fast_corner_detect_9:1937' if im(y+3,x+0) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:1944' else */
                                            /* 'fast_corner_detect_9:1941' else */
                                            /* 'fast_corner_detect_9:1938' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1954' else */
                                    /* 'fast_corner_detect_9:1951' else */
                                    /* 'fast_corner_detect_9:1948' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:1958' else */
                            /* 'fast_corner_detect_9:1959' if im(y+0,x+-3) < c_b */
                            if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb))) {
                                /* 'fast_corner_detect_9:1960' if im(y+-1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:1961' if im(y+-2,x+-2) < c_b */
                                /* 'fast_corner_detect_9:1962' if im(y+-3,x+0) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) {
                                    /* 'fast_corner_detect_9:1964' elseif im(y+-3,x+0) < c_b */
                                    /* 'fast_corner_detect_9:1965' if im(y+-3,x+-1) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb))) {
                                        /* 'fast_corner_detect_9:1966' if im(y+-1,x+3) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                            /* 'fast_corner_detect_9:1968' elseif im(y+-1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:1969' if im(y+-3,x+1) > cb */
                                            if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) || ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b))))) {
                                                /* 'fast_corner_detect_9:1972' else */
                                                /* 'fast_corner_detect_9:1973' if im(y+2,x+2) < c_b */
                                                /* 'fast_corner_detect_9:1978' else */
                                                /* 'fast_corner_detect_9:1975' else */
                                            } else {
                                                /* 'fast_corner_detect_9:1971' elseif im(y+-3,x+1) < c_b */
                                                /* 'fast_corner_detect_9:1974' if im(y+1,x+-3) < c_b */
                                                guard1 = TRUE;
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:1982' else */
                                            /* 'fast_corner_detect_9:1983' if im(y+1,x+-3) < c_b */
                                            if (im_data[(y + im_sizes[0] * x) - 2] < c_b) {
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1984' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:1988' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:1991' else */
                                    /* 'fast_corner_detect_9:1992' if im(y+1,x+-3) < c_b */
                                    if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb))) {
                                        /* 'fast_corner_detect_9:1993' if im(y+2,x+2) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                                            /* 'fast_corner_detect_9:1995' elseif im(y+2,x+2) < c_b */
                                            /* 'fast_corner_detect_9:1996' if im(y+2,x+-2) < c_b */
                                            if (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) {
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:1997' else */
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2000' else */
                                            /* 'fast_corner_detect_9:2001' if im(y+-3,x+1) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b)) {
                                                /* 'fast_corner_detect_9:2002' if im(y+-3,x+-1) < c_b */
                                                /* 'fast_corner_detect_9:2003' if im(y+3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:2004' if im(y+2,x+-2) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2014' else */
                                                /* 'fast_corner_detect_9:2011' else */
                                                /* 'fast_corner_detect_9:2008' else */
                                                /* 'fast_corner_detect_9:2005' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2018' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:2028' else */
                                /* 'fast_corner_detect_9:2025' else */
                                /* 'fast_corner_detect_9:2022' else */
                            }
                        }
                    } else {
                        /* 'fast_corner_detect_9:2032' else */
                        /* 'fast_corner_detect_9:2033' if im(y+1,x+-3) < c_b */
                        if (im_data[(y + im_sizes[0] * x) - 2] < c_b) {
                            /* 'fast_corner_detect_9:2034' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                /* 'fast_corner_detect_9:2035' if im(y+0,x+-3) < c_b */
                                if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b)) {
                                    /* 'fast_corner_detect_9:2036' if im(y+1,x+3) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2040' else */
                                    /* 'fast_corner_detect_9:2037' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                /* 'fast_corner_detect_9:2043' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:2044' if im(y+2,x+2) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                } else if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                                    /* 'fast_corner_detect_9:2046' elseif im(y+2,x+2) < c_b */
                                    /* 'fast_corner_detect_9:2047' if im(y+2,x+-2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (!(im_data[y + im_sizes[0] * (x + 3)] > cb))) {
                                        /* 'fast_corner_detect_9:2048' if im(y+3,x+0) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 3)] < c_b) {
                                            /* 'fast_corner_detect_9:2050' elseif im(y+3,x+0) < c_b */
                                            /* 'fast_corner_detect_9:2051' if im(y+1,x+3) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                                /* 'fast_corner_detect_9:2053' elseif im(y+1,x+3) < c_b */
                                                /* 'fast_corner_detect_9:2054' if im(y+3,x+1) > cb */
                                                if ((im_data[y + im_sizes[0] * (x + 4)] > cb) || (!((im_data[y + im_sizes[0] * (x + 4)] < c_b) || ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b))))) {
                                                    /* 'fast_corner_detect_9:2057' else */
                                                    /* 'fast_corner_detect_9:2058' if im(y+-2,x+-2) < c_b */
                                                    /* 'fast_corner_detect_9:2067' else */
                                                    /* 'fast_corner_detect_9:2064' else */
                                                    /* 'fast_corner_detect_9:2061' else */
                                                } else {
                                                    /* 'fast_corner_detect_9:2056' elseif im(y+3,x+1) < c_b */
                                                    /* 'fast_corner_detect_9:2059' if im(y+-3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:2060' if im(y+0,x+-3) < c_b */
                                                    guard1 = TRUE;
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:2071' else */
                                                /* 'fast_corner_detect_9:2072' if im(y+0,x+-3) < c_b */
                                                if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b)) {
                                                    /* 'fast_corner_detect_9:2073' if im(y+-2,x+-2) < c_b */
                                                    /* 'fast_corner_detect_9:2074' if im(y+-1,x+-3) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2081' else */
                                                    /* 'fast_corner_detect_9:2078' else */
                                                    /* 'fast_corner_detect_9:2075' else */
                                                }
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2085' else */
                                            /* 'fast_corner_detect_9:2086' if im(y+-3,x+1) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                                /* 'fast_corner_detect_9:2087' if im(y+-1,x+-3) < c_b */
                                                /* 'fast_corner_detect_9:2088' if im(y+-3,x+0) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2095' else */
                                                /* 'fast_corner_detect_9:2092' else */
                                                /* 'fast_corner_detect_9:2089' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2099' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:2102' else */
                                    /* 'fast_corner_detect_9:2103' if im(y+-3,x+-1) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb))) {
                                        /* 'fast_corner_detect_9:2104' if im(y+-1,x+-3) < c_b */
                                        /* 'fast_corner_detect_9:2105' if im(y+-3,x+1) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) {
                                            /* 'fast_corner_detect_9:2107' elseif im(y+-3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:2108' if im(y+2,x+-2) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                                /* 'fast_corner_detect_9:2109' if im(y+0,x+-3) < c_b */
                                                /* 'fast_corner_detect_9:2110' if im(y+-3,x+0) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2117' else */
                                                /* 'fast_corner_detect_9:2114' else */
                                                /* 'fast_corner_detect_9:2111' else */
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2120' else */
                                            /* 'fast_corner_detect_9:2121' if im(y+3,x+0) < c_b */
                                            if ((im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb))) {
                                                /* 'fast_corner_detect_9:2122' if im(y+2,x+-2) < c_b */
                                                /* 'fast_corner_detect_9:2123' if im(y+1,x+3) > cb */
                                                if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                                    /* 'fast_corner_detect_9:2125' elseif im(y+1,x+3) < c_b */
                                                    /* 'fast_corner_detect_9:2126' if im(y+3,x+1) < c_b */
                                                    if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                                        guard1 = TRUE;
                                                    } else {
                                                        /* 'fast_corner_detect_9:2127' else */
                                                    }
                                                } else {
                                                    /* 'fast_corner_detect_9:2130' else */
                                                    /* 'fast_corner_detect_9:2131' if im(y+-2,x+-2) < c_b */
                                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b)) {
                                                        /* 'fast_corner_detect_9:2132' if im(y+0,x+-3) < c_b */
                                                        /* 'fast_corner_detect_9:2133' if im(y+3,x+1) < c_b */
                                                        guard1 = TRUE;
                                                    } else {
                                                        /* 'fast_corner_detect_9:2140' else */
                                                        /* 'fast_corner_detect_9:2137' else */
                                                        /* 'fast_corner_detect_9:2134' else */
                                                    }
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:2147' else */
                                                /* 'fast_corner_detect_9:2144' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2154' else */
                                        /* 'fast_corner_detect_9:2151' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:2158' else */
                                /* 'fast_corner_detect_9:2159' if im(y+0,x+-3) < c_b */
                                if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb))) {
                                    /* 'fast_corner_detect_9:2160' if im(y+2,x+2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                                        /* 'fast_corner_detect_9:2162' elseif im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:2163' if im(y+2,x+-2) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (!(im_data[y + im_sizes[0] * (x + 3)] > cb))) {
                                            /* 'fast_corner_detect_9:2164' if im(y+3,x+0) > cb */
                                            if (im_data[y + im_sizes[0] * (x + 3)] < c_b) {
                                                /* 'fast_corner_detect_9:2166' elseif im(y+3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:2167' if im(y+1,x+3) > cb */
                                                if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                                } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                                    /* 'fast_corner_detect_9:2169' elseif im(y+1,x+3) < c_b */
                                                    /* 'fast_corner_detect_9:2170' if im(y+3,x+1) > cb */
                                                    if ((im_data[y + im_sizes[0] * (x + 4)] > cb) || (!((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)))) {
                                                        /* 'fast_corner_detect_9:2173' else */
                                                        /* 'fast_corner_detect_9:2174' if im(y+-3,x+-1) < c_b */
                                                        /* 'fast_corner_detect_9:2175' else */
                                                    } else {
                                                        /* 'fast_corner_detect_9:2172' elseif im(y+3,x+1) < c_b */
                                                        guard1 = TRUE;
                                                    }
                                                } else {
                                                    /* 'fast_corner_detect_9:2179' else */
                                                    /* 'fast_corner_detect_9:2180' if im(y+-2,x+-2) < c_b */
                                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) && ((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b))) {
                                                        /* 'fast_corner_detect_9:2181' if im(y+-1,x+-3) < c_b */
                                                        /* 'fast_corner_detect_9:2182' if im(y+3,x+1) > cb */
                                                        /* 'fast_corner_detect_9:2184' elseif im(y+3,x+1) < c_b */
                                                        guard1 = TRUE;
                                                    } else {
                                                        /* 'fast_corner_detect_9:2194' else */
                                                        /* 'fast_corner_detect_9:2191' else */
                                                        /* 'fast_corner_detect_9:2185' else */
                                                        /* 'fast_corner_detect_9:2186' if im(y+-3,x+1) < c_b */
                                                        /* 'fast_corner_detect_9:2187' else */
                                                    }
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:2198' else */
                                                /* 'fast_corner_detect_9:2199' if im(y+-3,x+1) < c_b */
                                                if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                                    /* 'fast_corner_detect_9:2200' if im(y+-3,x+-1) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2204' else */
                                                    /* 'fast_corner_detect_9:2201' else */
                                                }
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2208' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2211' else */
                                        /* 'fast_corner_detect_9:2212' if im(y+-3,x+-1) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (!(im_data[y + im_sizes[0] * (x + 4)] > cb))) {
                                            /* 'fast_corner_detect_9:2213' if im(y+-1,x+-3) < c_b */
                                            /* 'fast_corner_detect_9:2214' if im(y+2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:2215' if im(y+3,x+1) > cb */
                                            if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                                /* 'fast_corner_detect_9:2217' elseif im(y+3,x+1) < c_b */
                                                /* 'fast_corner_detect_9:2218' if im(y+-2,x+-2) < c_b */
                                                if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) && ((im_data[y + im_sizes[0] * (x + 3)] < c_b) || (!((im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) || (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b))))) {
                                                    /* 'fast_corner_detect_9:2219' if im(y+3,x+0) > cb */
                                                    /* 'fast_corner_detect_9:2221' elseif im(y+3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:2225' else */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2228' else */
                                                    /* 'fast_corner_detect_9:2222' else */
                                                    /* 'fast_corner_detect_9:2223' if im(y+1,x+3) > cb||im(y+1,x+3) < c_b */
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:2231' else */
                                                /* 'fast_corner_detect_9:2232' if im(y+-3,x+1) > cb */
                                                if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) || ((im_data[y + im_sizes[0] * (x + 3)] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb)) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b)) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b))))) {
                                                    /* 'fast_corner_detect_9:2235' else */
                                                    /* 'fast_corner_detect_9:2236' if im(y+3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:2245' else */
                                                    /* 'fast_corner_detect_9:2241' else */
                                                } else {
                                                    /* 'fast_corner_detect_9:2234' elseif im(y+-3,x+1) < c_b */
                                                    /* 'fast_corner_detect_9:2237' if im(y+1,x+3) > cb||im(y+1,x+3) < c_b */
                                                    /* 'fast_corner_detect_9:2239' else */
                                                    /* 'fast_corner_detect_9:2240' if im(y+-2,x+-2) < c_b */
                                                    guard1 = TRUE;
                                                }
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2256' else */
                                            /* 'fast_corner_detect_9:2253' else */
                                            /* 'fast_corner_detect_9:2250' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:2260' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:2264' else */
                        }
                    }
                } else {
                    /* 'fast_corner_detect_9:2268' else */
                    /* 'fast_corner_detect_9:2269' if im(y+-3,x+0) > cb */
                    if (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) {
                        /* 'fast_corner_detect_9:2270' if im(y+-2,x+2) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                            /* 'fast_corner_detect_9:2271' if im(y+2,x+-2) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) {
                                /* 'fast_corner_detect_9:2272' if im(y+-1,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) && (im_data[(y + im_sizes[0] * x) - 3] > cb) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) {
                                    /* 'fast_corner_detect_9:2273' if im(y+-2,x+-2) > cb */
                                    /* 'fast_corner_detect_9:2274' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:2275' if im(y+-3,x+1) > cb */
                                    /* 'fast_corner_detect_9:2276' if im(y+0,x+-3) > cb */
                                    /* 'fast_corner_detect_9:2277' if im(y+-3,x+-1) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2293' else */
                                    /* 'fast_corner_detect_9:2290' else */
                                    /* 'fast_corner_detect_9:2287' else */
                                    /* 'fast_corner_detect_9:2284' else */
                                    /* 'fast_corner_detect_9:2281' else */
                                    /* 'fast_corner_detect_9:2278' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) {
                                /* 'fast_corner_detect_9:2296' elseif im(y+2,x+-2) < c_b */
                                /* 'fast_corner_detect_9:2297' if im(y+1,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb)) {
                                    /* 'fast_corner_detect_9:2298' if im(y+-1,x+3) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2302' else */
                                    /* 'fast_corner_detect_9:2299' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:2305' else */
                                /* 'fast_corner_detect_9:2306' if im(y+-1,x+3) > cb */
                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * x) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) && (im_data[(y + im_sizes[0] * x) - 3] > cb)) {
                                    /* 'fast_corner_detect_9:2307' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:2308' if im(y+-1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:2309' if im(y+-3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:2310' if im(y+0,x+-3) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2323' else */
                                    /* 'fast_corner_detect_9:2320' else */
                                    /* 'fast_corner_detect_9:2317' else */
                                    /* 'fast_corner_detect_9:2314' else */
                                    /* 'fast_corner_detect_9:2311' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:2327' else */
                        }
                    } else if (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) {
                        /* 'fast_corner_detect_9:2330' elseif im(y+-3,x+0) < c_b */
                        /* 'fast_corner_detect_9:2331' if im(y+2,x+2) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                            /* 'fast_corner_detect_9:2332' if im(y+0,x+-3) > cb */
                            if (im_data[(y + im_sizes[0] * x) - 3] > cb) {
                                /* 'fast_corner_detect_9:2333' if im(y+1,x+3) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b)) {
                                    /* 'fast_corner_detect_9:2334' if im(y+-1,x+-3) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2338' else */
                                    /* 'fast_corner_detect_9:2335' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * x) - 3] < c_b) {
                                /* 'fast_corner_detect_9:2341' elseif im(y+0,x+-3) < c_b */
                                /* 'fast_corner_detect_9:2342' if im(y+-2,x+2) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b)) {
                                    /* 'fast_corner_detect_9:2343' if im(y+-2,x+-2) < c_b */
                                    /* 'fast_corner_detect_9:2344' if im(y+-1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:2345' if im(y+-3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:2346' if im(y+-1,x+3) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2359' else */
                                    /* 'fast_corner_detect_9:2356' else */
                                    /* 'fast_corner_detect_9:2353' else */
                                    /* 'fast_corner_detect_9:2350' else */
                                    /* 'fast_corner_detect_9:2347' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:2362' else */
                                /* 'fast_corner_detect_9:2363' if im(y+1,x+3) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b)) {
                                    /* 'fast_corner_detect_9:2364' if im(y+-1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:2365' if im(y+-1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:2366' if im(y+-3,x+1) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2376' else */
                                    /* 'fast_corner_detect_9:2373' else */
                                    /* 'fast_corner_detect_9:2370' else */
                                    /* 'fast_corner_detect_9:2367' else */
                                }
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                            /* 'fast_corner_detect_9:2380' elseif im(y+2,x+2) < c_b */
                            /* 'fast_corner_detect_9:2381' if im(y+-2,x+2) < c_b */
                            if (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) {
                                /* 'fast_corner_detect_9:2382' if im(y+3,x+1) > cb */
                                if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                    /* 'fast_corner_detect_9:2383' if im(y+1,x+-3) > cb */
                                    if (im_data[(y + im_sizes[0] * x) - 2] > cb) {
                                    } else if (im_data[(y + im_sizes[0] * x) - 2] < c_b) {
                                        /* 'fast_corner_detect_9:2385' elseif im(y+1,x+-3) < c_b */
                                        /* 'fast_corner_detect_9:2386' if im(y+-1,x+3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b)) {
                                            /* 'fast_corner_detect_9:2387' if im(y+1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:2388' if im(y+-3,x+1) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:2395' else */
                                            /* 'fast_corner_detect_9:2392' else */
                                            /* 'fast_corner_detect_9:2389' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2398' else */
                                        /* 'fast_corner_detect_9:2399' if im(y+-3,x+1) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                            /* 'fast_corner_detect_9:2400' if im(y+-2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:2401' if im(y+1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:2402' if im(y+-3,x+-1) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:2412' else */
                                            /* 'fast_corner_detect_9:2409' else */
                                            /* 'fast_corner_detect_9:2406' else */
                                            /* 'fast_corner_detect_9:2403' else */
                                        }
                                    }
                                } else if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                    /* 'fast_corner_detect_9:2416' elseif im(y+3,x+1) < c_b */
                                    /* 'fast_corner_detect_9:2417' if im(y+1,x+3) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                    } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                        /* 'fast_corner_detect_9:2419' elseif im(y+1,x+3) < c_b */
                                        /* 'fast_corner_detect_9:2420' if im(y+-3,x+-1) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                            guard1 = TRUE;
                                        } else if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                            /* 'fast_corner_detect_9:2421' elseif im(y+-3,x+-1) < c_b */
                                            /* 'fast_corner_detect_9:2422' if im(y+-3,x+1) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb)) && ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) || ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b)))) {
                                                /* 'fast_corner_detect_9:2423' if im(y+-1,x+3) > cb */
                                                /* 'fast_corner_detect_9:2425' elseif im(y+-1,x+3) < c_b */
                                                /* 'fast_corner_detect_9:2428' if im(y+2,x+-2) < c_b */
                                                /* 'fast_corner_detect_9:2429' if im(y+1,x+-3) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2440' else */
                                                /* 'fast_corner_detect_9:2426' else */
                                                /* 'fast_corner_detect_9:2427' if im(y+0,x+-3) < c_b */
                                                /* 'fast_corner_detect_9:2436' else */
                                                /* 'fast_corner_detect_9:2433' else */
                                                /* 'fast_corner_detect_9:2430' else */
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2443' else */
                                            /* 'fast_corner_detect_9:2444' if im(y+3,x+0) < c_b */
                                            if ((im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b)) {
                                                /* 'fast_corner_detect_9:2445' if im(y+-1,x+3) < c_b */
                                                /* 'fast_corner_detect_9:2446' if im(y+-3,x+1) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2453' else */
                                                /* 'fast_corner_detect_9:2450' else */
                                                /* 'fast_corner_detect_9:2447' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2457' else */
                                        /* 'fast_corner_detect_9:2458' if im(y+0,x+-3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b)) {
                                            /* 'fast_corner_detect_9:2459' if im(y+-1,x+-3) < c_b */
                                            /* 'fast_corner_detect_9:2460' if im(y+-3,x+1) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:2467' else */
                                            /* 'fast_corner_detect_9:2464' else */
                                            /* 'fast_corner_detect_9:2461' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:2471' else */
                                    /* 'fast_corner_detect_9:2472' if im(y+-2,x+-2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb))) {
                                        /* 'fast_corner_detect_9:2473' if im(y+1,x+3) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                            /* 'fast_corner_detect_9:2475' elseif im(y+1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:2476' if im(y+-1,x+3) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                                /* 'fast_corner_detect_9:2478' elseif im(y+-1,x+3) < c_b */
                                                /* 'fast_corner_detect_9:2479' if im(y+-3,x+1) < c_b */
                                                if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                                    /* 'fast_corner_detect_9:2480' if im(y+-3,x+-1) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2484' else */
                                                    /* 'fast_corner_detect_9:2481' else */
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:2487' else */
                                                /* 'fast_corner_detect_9:2488' if im(y+0,x+-3) < c_b */
                                                if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (!(im_data[y + im_sizes[0] * (x + 3)] > cb)) && (!(im_data[y + im_sizes[0] * (x + 3)] < c_b)) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b)) {
                                                    /* 'fast_corner_detect_9:2489' if im(y+2,x+-2) < c_b */
                                                    /* 'fast_corner_detect_9:2490' if im(y+3,x+0) > cb||im(y+3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:2492' else */
                                                    /* 'fast_corner_detect_9:2493' if im(y+-3,x+1) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2501' else */
                                                    /* 'fast_corner_detect_9:2498' else */
                                                    /* 'fast_corner_detect_9:2494' else */
                                                }
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2505' else */
                                            /* 'fast_corner_detect_9:2506' if im(y+0,x+-3) < c_b */
                                            if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb)) && ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) || ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)))) {
                                                /* 'fast_corner_detect_9:2507' if im(y+-1,x+-3) < c_b */
                                                /* 'fast_corner_detect_9:2508' if im(y+-3,x+1) < c_b */
                                                /* 'fast_corner_detect_9:2509' if im(y+-1,x+3) > cb */
                                                /* 'fast_corner_detect_9:2511' elseif im(y+-1,x+3) < c_b */
                                                /* 'fast_corner_detect_9:2514' if im(y+-3,x+-1) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2528' else */
                                                /* 'fast_corner_detect_9:2525' else */
                                                /* 'fast_corner_detect_9:2522' else */
                                                /* 'fast_corner_detect_9:2512' else */
                                                /* 'fast_corner_detect_9:2513' if im(y+1,x+-3) < c_b */
                                                /* 'fast_corner_detect_9:2518' else */
                                                /* 'fast_corner_detect_9:2515' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2532' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:2536' else */
                            }
                        } else {
                            /* 'fast_corner_detect_9:2539' else */
                            /* 'fast_corner_detect_9:2540' if im(y+-1,x+-3) < c_b */
                            if (im_data[(y + im_sizes[0] * x) - 4] < c_b) {
                                /* 'fast_corner_detect_9:2541' if im(y+1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                    /* 'fast_corner_detect_9:2542' if im(y+0,x+-3) < c_b */
                                    if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b)) {
                                        /* 'fast_corner_detect_9:2543' if im(y+-1,x+3) < c_b */
                                        /* 'fast_corner_detect_9:2544' if im(y+-2,x+-2) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:2551' else */
                                        /* 'fast_corner_detect_9:2548' else */
                                        /* 'fast_corner_detect_9:2545' else */
                                    }
                                } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                    /* 'fast_corner_detect_9:2554' elseif im(y+1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:2555' if im(y+-2,x+2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb))) {
                                        /* 'fast_corner_detect_9:2556' if im(y+-1,x+3) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                            /* 'fast_corner_detect_9:2558' elseif im(y+-1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:2559' if im(y+-2,x+-2) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                                /* 'fast_corner_detect_9:2560' if im(y+-3,x+1) < c_b */
                                                /* 'fast_corner_detect_9:2561' if im(y+-3,x+-1) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2568' else */
                                                /* 'fast_corner_detect_9:2565' else */
                                                /* 'fast_corner_detect_9:2562' else */
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2571' else */
                                            /* 'fast_corner_detect_9:2572' if im(y+2,x+-2) < c_b */
                                            if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 3] < c_b)) {
                                                /* 'fast_corner_detect_9:2573' if im(y+0,x+-3) < c_b */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2577' else */
                                                /* 'fast_corner_detect_9:2574' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2581' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:2584' else */
                                    /* 'fast_corner_detect_9:2585' if im(y+0,x+-3) < c_b */
                                    if ((im_data[(y + im_sizes[0] * x) - 3] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb))) {
                                        /* 'fast_corner_detect_9:2586' if im(y+-2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:2587' if im(y+-2,x+-2) < c_b */
                                        /* 'fast_corner_detect_9:2588' if im(y+-3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:2589' if im(y+-1,x+3) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                            /* 'fast_corner_detect_9:2591' elseif im(y+-1,x+3) < c_b */
                                            /* 'fast_corner_detect_9:2592' if im(y+-3,x+-1) < c_b */
                                            if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2593' else */
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2596' else */
                                            /* 'fast_corner_detect_9:2597' if im(y+2,x+-2) < c_b */
                                            if (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) {
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2598' else */
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2611' else */
                                        /* 'fast_corner_detect_9:2608' else */
                                        /* 'fast_corner_detect_9:2605' else */
                                        /* 'fast_corner_detect_9:2602' else */
                                    }
                                }
                            } else {
                                /* 'fast_corner_detect_9:2615' else */
                            }
                        }
                    } else {
                        /* 'fast_corner_detect_9:2619' else */
                    }
                }
            } else {
                /* 'fast_corner_detect_9:2623' else */
                /* 'fast_corner_detect_9:2624' if im(y+0,x+-3) > cb */
                if (im_data[(y + im_sizes[0] * x) - 3] > cb) {
                    /* 'fast_corner_detect_9:2625' if im(y+-3,x+1) > cb */
                    if (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) {
                        /* 'fast_corner_detect_9:2626' if im(y+1,x+-3) > cb */
                        if ((im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * x) - 4] > cb)) {
                            /* 'fast_corner_detect_9:2627' if im(y+-1,x+-3) > cb */
                            /* 'fast_corner_detect_9:2628' if im(y+2,x+-2) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) {
                                /* 'fast_corner_detect_9:2629' if im(y+-2,x+2) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) {
                                    /* 'fast_corner_detect_9:2630' if im(y+-2,x+-2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                        /* 'fast_corner_detect_9:2631' if im(y+-3,x+-1) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                            /* 'fast_corner_detect_9:2632' if im(y+-3,x+0) > cb */
                                            if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb))))) {
                                                /* 'fast_corner_detect_9:2635' else */
                                                /* 'fast_corner_detect_9:2636' if im(y+3,x+1) > cb */
                                                /* 'fast_corner_detect_9:2637' if im(y+3,x+0) > cb */
                                                guard1 = TRUE;
                                            } else {
                                                /* 'fast_corner_detect_9:2633' elseif im(y+-3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:2641' else */
                                                /* 'fast_corner_detect_9:2638' else */
                                            }
                                        } else if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb))) {
                                            /* 'fast_corner_detect_9:2645' elseif im(y+-3,x+-1) < c_b */
                                            /* 'fast_corner_detect_9:2657' else */
                                            /* 'fast_corner_detect_9:2654' else */
                                            /* 'fast_corner_detect_9:2651' else */
                                        } else {
                                            /* 'fast_corner_detect_9:2647' else */
                                            /* 'fast_corner_detect_9:2648' if im(y+2,x+2) > cb */
                                            /* 'fast_corner_detect_9:2649' if im(y+3,x+1) > cb */
                                            /* 'fast_corner_detect_9:2650' if im(y+3,x+-1) > cb */
                                            guard1 = TRUE;
                                        }
                                    } else if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb))) {
                                        /* 'fast_corner_detect_9:2661' elseif im(y+-2,x+-2) < c_b */
                                        /* 'fast_corner_detect_9:2677' else */
                                        /* 'fast_corner_detect_9:2674' else */
                                        /* 'fast_corner_detect_9:2671' else */
                                        /* 'fast_corner_detect_9:2668' else */
                                    } else {
                                        /* 'fast_corner_detect_9:2663' else */
                                        /* 'fast_corner_detect_9:2664' if im(y+1,x+3) > cb */
                                        /* 'fast_corner_detect_9:2665' if im(y+3,x+1) > cb */
                                        /* 'fast_corner_detect_9:2666' if im(y+3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:2667' if im(y+2,x+2) > cb */
                                        guard1 = TRUE;
                                    }
                                } else if (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) {
                                    /* 'fast_corner_detect_9:2681' elseif im(y+-2,x+2) < c_b */
                                    /* 'fast_corner_detect_9:2682' if im(y+3,x+0) > cb */
                                    if ((im_data[y + im_sizes[0] * (x + 3)] > cb) && ((im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) || (!((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb)))))) {
                                        /* 'fast_corner_detect_9:2683' if im(y+1,x+3) > cb */
                                        /* 'fast_corner_detect_9:2686' else */
                                        /* 'fast_corner_detect_9:2687' if im(y+-2,x+-2) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:2692' else */
                                        /* 'fast_corner_detect_9:2684' elseif im(y+1,x+3) < c_b */
                                        /* 'fast_corner_detect_9:2688' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:2695' else */
                                    /* 'fast_corner_detect_9:2696' if im(y+3,x+-1) > cb */
                                    if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                        /* 'fast_corner_detect_9:2697' if im(y+-2,x+-2) > cb */
                                        if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                            /* 'fast_corner_detect_9:2698' if im(y+-3,x+-1) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                                /* 'fast_corner_detect_9:2699' if im(y+-3,x+0) > cb */
                                                if ((im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[y + im_sizes[0] * (x + 3)] > cb))))) {
                                                    /* 'fast_corner_detect_9:2702' else */
                                                    /* 'fast_corner_detect_9:2703' if im(y+3,x+1) > cb */
                                                    /* 'fast_corner_detect_9:2704' if im(y+3,x+0) > cb */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2700' elseif im(y+-3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:2708' else */
                                                    /* 'fast_corner_detect_9:2705' else */
                                                }
                                            } else if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb))) {
                                                /* 'fast_corner_detect_9:2712' elseif im(y+-3,x+-1) < c_b */
                                                /* 'fast_corner_detect_9:2720' else */
                                                /* 'fast_corner_detect_9:2717' else */
                                            } else {
                                                /* 'fast_corner_detect_9:2714' else */
                                                /* 'fast_corner_detect_9:2715' if im(y+2,x+2) > cb */
                                                /* 'fast_corner_detect_9:2716' if im(y+3,x+1) > cb */
                                                guard1 = TRUE;
                                            }
                                        } else if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb))) {
                                            /* 'fast_corner_detect_9:2724' elseif im(y+-2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:2736' else */
                                            /* 'fast_corner_detect_9:2733' else */
                                            /* 'fast_corner_detect_9:2730' else */
                                        } else {
                                            /* 'fast_corner_detect_9:2726' else */
                                            /* 'fast_corner_detect_9:2727' if im(y+1,x+3) > cb */
                                            /* 'fast_corner_detect_9:2728' if im(y+2,x+2) > cb */
                                            /* 'fast_corner_detect_9:2729' if im(y+3,x+1) > cb */
                                            guard1 = TRUE;
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:2740' else */
                                    }
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) {
                                /* 'fast_corner_detect_9:2744' elseif im(y+2,x+-2) < c_b */
                                /* 'fast_corner_detect_9:2745' if im(y+-3,x+-1) > cb */
                                if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb)) {
                                    /* 'fast_corner_detect_9:2746' if im(y+-1,x+3) > cb */
                                    /* 'fast_corner_detect_9:2747' if im(y+-2,x+-2) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2754' else */
                                    /* 'fast_corner_detect_9:2751' else */
                                    /* 'fast_corner_detect_9:2748' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:2757' else */
                                /* 'fast_corner_detect_9:2758' if im(y+-1,x+3) > cb */
                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb)) {
                                    /* 'fast_corner_detect_9:2759' if im(y+-2,x+2) > cb */
                                    /* 'fast_corner_detect_9:2760' if im(y+-3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:2761' if im(y+-2,x+-2) > cb */
                                    /* 'fast_corner_detect_9:2762' if im(y+-3,x+0) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2775' else */
                                    /* 'fast_corner_detect_9:2772' else */
                                    /* 'fast_corner_detect_9:2769' else */
                                    /* 'fast_corner_detect_9:2766' else */
                                    /* 'fast_corner_detect_9:2763' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:2782' else */
                            /* 'fast_corner_detect_9:2779' else */
                        }
                    } else if (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) {
                        /* 'fast_corner_detect_9:2785' elseif im(y+-3,x+1) < c_b */
                        /* 'fast_corner_detect_9:2786' if im(y+2,x+2) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                            /* 'fast_corner_detect_9:2787' if im(y+-1,x+-3) > cb */
                            if ((im_data[(y + im_sizes[0] * x) - 4] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb)) {
                                /* 'fast_corner_detect_9:2788' if im(y+3,x+-1) > cb */
                                /* 'fast_corner_detect_9:2789' if im(y+1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                    /* 'fast_corner_detect_9:2790' if im(y+1,x+-3) > cb */
                                    if ((im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) && (im_data[y + im_sizes[0] * (x + 3)] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb)) {
                                        /* 'fast_corner_detect_9:2791' if im(y+2,x+-2) > cb */
                                        /* 'fast_corner_detect_9:2792' if im(y+3,x+0) > cb */
                                        /* 'fast_corner_detect_9:2793' if im(y+3,x+1) > cb */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:2803' else */
                                        /* 'fast_corner_detect_9:2800' else */
                                        /* 'fast_corner_detect_9:2797' else */
                                        /* 'fast_corner_detect_9:2794' else */
                                    }
                                } else if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[y + im_sizes[0] * (x + 4)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb))) {
                                    /* 'fast_corner_detect_9:2806' elseif im(y+1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:2822' else */
                                    /* 'fast_corner_detect_9:2819' else */
                                    /* 'fast_corner_detect_9:2816' else */
                                    /* 'fast_corner_detect_9:2813' else */
                                } else {
                                    /* 'fast_corner_detect_9:2808' else */
                                    /* 'fast_corner_detect_9:2809' if im(y+-2,x+-2) > cb */
                                    /* 'fast_corner_detect_9:2810' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:2811' if im(y+3,x+1) > cb */
                                    /* 'fast_corner_detect_9:2812' if im(y+2,x+-2) > cb */
                                    guard1 = TRUE;
                                }
                            } else {
                                /* 'fast_corner_detect_9:2829' else */
                                /* 'fast_corner_detect_9:2826' else */
                            }
                        } else if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                            /* 'fast_corner_detect_9:2832' elseif im(y+2,x+2) < c_b */
                            /* 'fast_corner_detect_9:2833' if im(y+3,x+1) > cb */
                            if ((im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb)) {
                                /* 'fast_corner_detect_9:2834' if im(y+-3,x+-1) > cb */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:2838' else */
                                /* 'fast_corner_detect_9:2835' else */
                            }
                        } else {
                            /* 'fast_corner_detect_9:2841' else */
                            /* 'fast_corner_detect_9:2842' if im(y+-3,x+-1) > cb */
                            if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb) && (im_data[y + im_sizes[0] * (x + 3)] > cb)) {
                                /* 'fast_corner_detect_9:2843' if im(y+3,x+1) > cb */
                                /* 'fast_corner_detect_9:2844' if im(y+3,x+0) > cb */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:2851' else */
                                /* 'fast_corner_detect_9:2848' else */
                                /* 'fast_corner_detect_9:2845' else */
                            }
                        }
                    } else {
                        /* 'fast_corner_detect_9:2855' else */
                        /* 'fast_corner_detect_9:2856' if im(y+3,x+0) > cb */
                        if (im_data[y + im_sizes[0] * (x + 3)] > cb) {
                            /* 'fast_corner_detect_9:2857' if im(y+-2,x+-2) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                /* 'fast_corner_detect_9:2858' if im(y+2,x+-2) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) {
                                    /* 'fast_corner_detect_9:2859' if im(y+3,x+1) > cb */
                                    if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                        /* 'fast_corner_detect_9:2860' if im(y+1,x+-3) > cb */
                                        if ((im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[(y + im_sizes[0] * x) - 4] > cb)) {
                                            /* 'fast_corner_detect_9:2861' if im(y+-1,x+-3) > cb */
                                            /* 'fast_corner_detect_9:2862' if im(y+2,x+2) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) {
                                                /* 'fast_corner_detect_9:2863' if im(y+3,x+-1) > cb */
                                                if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2864' else */
                                                }
                                            } else if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                                                /* 'fast_corner_detect_9:2867' elseif im(y+2,x+2) < c_b */
                                                /* 'fast_corner_detect_9:2868' if im(y+-3,x+-1) > cb */
                                                if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2869' else */
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:2872' else */
                                                /* 'fast_corner_detect_9:2873' if im(y+-3,x+-1) > cb */
                                                if ((im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb)) {
                                                    /* 'fast_corner_detect_9:2874' if im(y+3,x+-1) > cb */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:2878' else */
                                                    /* 'fast_corner_detect_9:2875' else */
                                                }
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:2885' else */
                                            /* 'fast_corner_detect_9:2882' else */
                                        }
                                    } else if ((im_data[y + im_sizes[0] * (x + 4)] < c_b) || (!(im_data[(y + im_sizes[0] * (x + 3)) - 6] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 4] > cb)) || (!(im_data[(y + im_sizes[0] * x) - 2] > cb)) || (!(im_data[y + im_sizes[0] * (x + 2)] > cb)) || (!(im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb))) {
                                        /* 'fast_corner_detect_9:2888' elseif im(y+3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:2908' else */
                                        /* 'fast_corner_detect_9:2905' else */
                                        /* 'fast_corner_detect_9:2902' else */
                                        /* 'fast_corner_detect_9:2899' else */
                                        /* 'fast_corner_detect_9:2896' else */
                                    } else {
                                        /* 'fast_corner_detect_9:2890' else */
                                        /* 'fast_corner_detect_9:2891' if im(y+-3,x+0) > cb */
                                        /* 'fast_corner_detect_9:2892' if im(y+-1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:2893' if im(y+1,x+-3) > cb */
                                        /* 'fast_corner_detect_9:2894' if im(y+3,x+-1) > cb */
                                        /* 'fast_corner_detect_9:2895' if im(y+-3,x+-1) > cb */
                                        guard1 = TRUE;
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:2912' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                                /* 'fast_corner_detect_9:2915' elseif im(y+-2,x+-2) < c_b */
                                /* 'fast_corner_detect_9:2916' if im(y+-1,x+-3) > cb */
                                if ((im_data[(y + im_sizes[0] * x) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb)) {
                                    /* 'fast_corner_detect_9:2917' if im(y+1,x+3) > cb */
                                    /* 'fast_corner_detect_9:2918' if im(y+3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:2919' if im(y+2,x+-2) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2929' else */
                                    /* 'fast_corner_detect_9:2926' else */
                                    /* 'fast_corner_detect_9:2923' else */
                                    /* 'fast_corner_detect_9:2920' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:2932' else */
                                /* 'fast_corner_detect_9:2933' if im(y+1,x+3) > cb */
                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) && (im_data[(y + im_sizes[0] * x) - 4] > cb) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] > cb) && (im_data[(y + im_sizes[0] * x) - 2] > cb) && (im_data[y + im_sizes[0] * (x + 2)] > cb) && (im_data[y + im_sizes[0] * (x + 4)] > cb)) {
                                    /* 'fast_corner_detect_9:2934' if im(y+-1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:2935' if im(y+2,x+-2) > cb */
                                    /* 'fast_corner_detect_9:2936' if im(y+2,x+2) > cb */
                                    /* 'fast_corner_detect_9:2937' if im(y+1,x+-3) > cb */
                                    /* 'fast_corner_detect_9:2938' if im(y+3,x+-1) > cb */
                                    /* 'fast_corner_detect_9:2939' if im(y+3,x+1) > cb */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:2958' else */
                                    /* 'fast_corner_detect_9:2955' else */
                                    /* 'fast_corner_detect_9:2952' else */
                                    /* 'fast_corner_detect_9:2949' else */
                                    /* 'fast_corner_detect_9:2946' else */
                                    /* 'fast_corner_detect_9:2943' else */
                                    /* 'fast_corner_detect_9:2940' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:2962' else */
                        }
                    }
                } else if (im_data[(y + im_sizes[0] * x) - 3] < c_b) {
                    /* 'fast_corner_detect_9:2966' elseif im(y+0,x+-3) < c_b */
                    /* 'fast_corner_detect_9:2967' if im(y+3,x+-1) > cb */
                    if (im_data[y + im_sizes[0] * (x + 2)] > cb) {
                        /* 'fast_corner_detect_9:2968' if im(y+-1,x+3) > cb */
                        if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                        } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                            /* 'fast_corner_detect_9:2970' elseif im(y+-1,x+3) < c_b */
                            /* 'fast_corner_detect_9:2971' if im(y+1,x+-3) < c_b */
                            if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b)) {
                                /* 'fast_corner_detect_9:2972' if im(y+-3,x+1) < c_b */
                                /* 'fast_corner_detect_9:2973' if im(y+-1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:2974' if im(y+-2,x+2) < c_b */
                                /* 'fast_corner_detect_9:2975' if im(y+-3,x+-1) < c_b */
                                /* 'fast_corner_detect_9:2976' if im(y+-3,x+0) < c_b */
                                /* 'fast_corner_detect_9:2977' if im(y+-2,x+-2) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:2996' else */
                                /* 'fast_corner_detect_9:2993' else */
                                /* 'fast_corner_detect_9:2990' else */
                                /* 'fast_corner_detect_9:2987' else */
                                /* 'fast_corner_detect_9:2984' else */
                                /* 'fast_corner_detect_9:2981' else */
                                /* 'fast_corner_detect_9:2978' else */
                            }
                        } else {
                            /* 'fast_corner_detect_9:2999' else */
                            /* 'fast_corner_detect_9:3000' if im(y+2,x+-2) < c_b */
                            if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b)) {
                                /* 'fast_corner_detect_9:3001' if im(y+-2,x+2) < c_b */
                                /* 'fast_corner_detect_9:3002' if im(y+-1,x+-3) < c_b */
                                /* 'fast_corner_detect_9:3003' if im(y+-3,x+1) < c_b */
                                /* 'fast_corner_detect_9:3004' if im(y+-3,x+-1) < c_b */
                                guard1 = TRUE;
                            } else {
                                /* 'fast_corner_detect_9:3017' else */
                                /* 'fast_corner_detect_9:3014' else */
                                /* 'fast_corner_detect_9:3011' else */
                                /* 'fast_corner_detect_9:3008' else */
                                /* 'fast_corner_detect_9:3005' else */
                            }
                        }
                    } else if (im_data[y + im_sizes[0] * (x + 2)] < c_b) {
                        /* 'fast_corner_detect_9:3021' elseif im(y+3,x+-1) < c_b */
                        /* 'fast_corner_detect_9:3022' if im(y+-1,x+-3) < c_b */
                        if (im_data[(y + im_sizes[0] * x) - 4] < c_b) {
                            /* 'fast_corner_detect_9:3023' if im(y+-3,x+-1) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 2)) - 6] > cb) {
                                /* 'fast_corner_detect_9:3024' if im(y+1,x+3) > cb */
                                if (im_data[(y + im_sizes[0] * (x + 6)) - 2] > cb) {
                                } else if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                    /* 'fast_corner_detect_9:3026' elseif im(y+1,x+3) < c_b */
                                    /* 'fast_corner_detect_9:3027' if im(y+2,x+2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b)) {
                                        /* 'fast_corner_detect_9:3028' if im(y+3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:3029' if im(y+2,x+-2) < c_b */
                                        /* 'fast_corner_detect_9:3030' if im(y+1,x+-3) < c_b */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:3040' else */
                                        /* 'fast_corner_detect_9:3037' else */
                                        /* 'fast_corner_detect_9:3034' else */
                                        /* 'fast_corner_detect_9:3031' else */
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:3043' else */
                                    /* 'fast_corner_detect_9:3044' if im(y+-2,x+-2) < c_b */
                                    if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b))) {
                                        /* 'fast_corner_detect_9:3045' if im(y+2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:3046' if im(y+3,x+1) < c_b */
                                        /* 'fast_corner_detect_9:3047' if im(y+-2,x+2) < c_b */
                                        /* 'fast_corner_detect_9:3049' else */
                                        guard1 = TRUE;
                                    } else {
                                        /* 'fast_corner_detect_9:3057' else */
                                        /* 'fast_corner_detect_9:3054' else */
                                        /* 'fast_corner_detect_9:3051' else */
                                    }
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) {
                                /* 'fast_corner_detect_9:3061' elseif im(y+-3,x+-1) < c_b */
                                /* 'fast_corner_detect_9:3062' if im(y+1,x+-3) < c_b */
                                if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (!(im_data[(y + im_sizes[0] * (x + 1)) - 1] > cb))) {
                                    /* 'fast_corner_detect_9:3063' if im(y+2,x+-2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) {
                                        /* 'fast_corner_detect_9:3065' elseif im(y+2,x+-2) < c_b */
                                        /* 'fast_corner_detect_9:3066' if im(y+3,x+1) > cb */
                                        if (im_data[y + im_sizes[0] * (x + 4)] > cb) {
                                            /* 'fast_corner_detect_9:3067' if im(y+-3,x+1) > cb */
                                            if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) || (!((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) || (im_data[y + im_sizes[0] * (x + 3)] < c_b)))) {
                                                /* 'fast_corner_detect_9:3070' else */
                                                /* 'fast_corner_detect_9:3071' if im(y+3,x+0) < c_b */
                                                /* 'fast_corner_detect_9:3072' else */
                                            } else {
                                                /* 'fast_corner_detect_9:3069' elseif im(y+-3,x+1) < c_b */
                                                guard1 = TRUE;
                                            }
                                        } else if (im_data[y + im_sizes[0] * (x + 4)] < c_b) {
                                            /* 'fast_corner_detect_9:3076' elseif im(y+3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:3077' if im(y+-2,x+-2) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                            } else if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                                                /* 'fast_corner_detect_9:3079' elseif im(y+-2,x+-2) < c_b */
                                                /* 'fast_corner_detect_9:3080' if im(y+3,x+0) > cb */
                                                if ((im_data[y + im_sizes[0] * (x + 3)] > cb) || (!((im_data[y + im_sizes[0] * (x + 3)] < c_b) || ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b))))) {
                                                    /* 'fast_corner_detect_9:3083' else */
                                                    /* 'fast_corner_detect_9:3084' if im(y+-3,x+1) < c_b */
                                                    /* 'fast_corner_detect_9:3089' else */
                                                    /* 'fast_corner_detect_9:3086' else */
                                                } else {
                                                    /* 'fast_corner_detect_9:3082' elseif im(y+3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:3085' if im(y+-3,x+0) < c_b */
                                                    guard1 = TRUE;
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:3093' else */
                                                /* 'fast_corner_detect_9:3094' if im(y+1,x+3) < c_b */
                                                if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b)) {
                                                    /* 'fast_corner_detect_9:3095' if im(y+2,x+2) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:3099' else */
                                                    /* 'fast_corner_detect_9:3096' else */
                                                }
                                            }
                                        } else {
                                            /* 'fast_corner_detect_9:3103' else */
                                            /* 'fast_corner_detect_9:3104' if im(y+-3,x+1) > cb */
                                            if (im_data[(y + im_sizes[0] * (x + 4)) - 6] > cb) {
                                            } else if (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) {
                                                /* 'fast_corner_detect_9:3106' elseif im(y+-3,x+1) < c_b */
                                                /* 'fast_corner_detect_9:3107' if im(y+-2,x+-2) < c_b */
                                                if ((im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                                    /* 'fast_corner_detect_9:3108' if im(y+-3,x+0) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:3112' else */
                                                    /* 'fast_corner_detect_9:3109' else */
                                                }
                                            } else {
                                                /* 'fast_corner_detect_9:3115' else */
                                                /* 'fast_corner_detect_9:3116' if im(y+3,x+0) < c_b */
                                                if ((im_data[y + im_sizes[0] * (x + 3)] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b)) {
                                                    /* 'fast_corner_detect_9:3117' if im(y+-3,x+0) < c_b */
                                                    /* 'fast_corner_detect_9:3118' if im(y+-2,x+-2) < c_b */
                                                    guard1 = TRUE;
                                                } else {
                                                    /* 'fast_corner_detect_9:3125' else */
                                                    /* 'fast_corner_detect_9:3122' else */
                                                    /* 'fast_corner_detect_9:3119' else */
                                                }
                                            }
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:3130' else */
                                        /* 'fast_corner_detect_9:3131' if im(y+-1,x+3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                            /* 'fast_corner_detect_9:3132' if im(y+-3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:3133' if im(y+-2,x+2) < c_b */
                                            /* 'fast_corner_detect_9:3134' if im(y+-2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:3135' if im(y+-3,x+0) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:3148' else */
                                            /* 'fast_corner_detect_9:3145' else */
                                            /* 'fast_corner_detect_9:3142' else */
                                            /* 'fast_corner_detect_9:3139' else */
                                            /* 'fast_corner_detect_9:3136' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:3152' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:3155' else */
                                /* 'fast_corner_detect_9:3156' if im(y+2,x+2) < c_b */
                                if (im_data[(y + im_sizes[0] * (x + 5)) - 1] < c_b) {
                                    /* 'fast_corner_detect_9:3157' if im(y+-2,x+-2) > cb */
                                    if (im_data[(y + im_sizes[0] * (x + 1)) - 5] > cb) {
                                        /* 'fast_corner_detect_9:3158' if im(y+1,x+3) < c_b */
                                        if (im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) {
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:3159' else */
                                        }
                                    } else if (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) {
                                        /* 'fast_corner_detect_9:3162' elseif im(y+-2,x+-2) < c_b */
                                        /* 'fast_corner_detect_9:3163' if im(y+1,x+-3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b)) {
                                            /* 'fast_corner_detect_9:3164' if im(y+3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:3165' if im(y+2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:3166' if im(y+3,x+0) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:3176' else */
                                            /* 'fast_corner_detect_9:3173' else */
                                            /* 'fast_corner_detect_9:3170' else */
                                            /* 'fast_corner_detect_9:3167' else */
                                        }
                                    } else {
                                        /* 'fast_corner_detect_9:3179' else */
                                        /* 'fast_corner_detect_9:3180' if im(y+1,x+3) < c_b */
                                        if ((im_data[(y + im_sizes[0] * (x + 6)) - 2] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[y + im_sizes[0] * (x + 4)] < c_b) && (im_data[y + im_sizes[0] * (x + 3)] < c_b)) {
                                            /* 'fast_corner_detect_9:3181' if im(y+1,x+-3) < c_b */
                                            /* 'fast_corner_detect_9:3182' if im(y+2,x+-2) < c_b */
                                            /* 'fast_corner_detect_9:3183' if im(y+3,x+1) < c_b */
                                            /* 'fast_corner_detect_9:3184' if im(y+3,x+0) < c_b */
                                            guard1 = TRUE;
                                        } else {
                                            /* 'fast_corner_detect_9:3197' else */
                                            /* 'fast_corner_detect_9:3194' else */
                                            /* 'fast_corner_detect_9:3191' else */
                                            /* 'fast_corner_detect_9:3188' else */
                                            /* 'fast_corner_detect_9:3185' else */
                                        }
                                    }
                                } else {
                                    /* 'fast_corner_detect_9:3201' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:3205' else */
                        }
                    } else {
                        /* 'fast_corner_detect_9:3208' else */
                        /* 'fast_corner_detect_9:3209' if im(y+-2,x+2) < c_b */
                        if ((im_data[(y + im_sizes[0] * (x + 5)) - 5] < c_b) && (im_data[(y + im_sizes[0] * x) - 2] < c_b)) {
                            /* 'fast_corner_detect_9:3210' if im(y+1,x+-3) < c_b */
                            /* 'fast_corner_detect_9:3211' if im(y+-1,x+3) > cb */
                            if (im_data[(y + im_sizes[0] * (x + 6)) - 4] > cb) {
                                /* 'fast_corner_detect_9:3212' if im(y+2,x+-2) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b)) {
                                    /* 'fast_corner_detect_9:3213' if im(y+-1,x+-3) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:3217' else */
                                    /* 'fast_corner_detect_9:3214' else */
                                }
                            } else if (im_data[(y + im_sizes[0] * (x + 6)) - 4] < c_b) {
                                /* 'fast_corner_detect_9:3220' elseif im(y+-1,x+3) < c_b */
                                /* 'fast_corner_detect_9:3221' if im(y+-3,x+1) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                    /* 'fast_corner_detect_9:3222' if im(y+-1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:3223' if im(y+-2,x+-2) < c_b */
                                    /* 'fast_corner_detect_9:3224' if im(y+-3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:3225' if im(y+-3,x+0) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:3238' else */
                                    /* 'fast_corner_detect_9:3235' else */
                                    /* 'fast_corner_detect_9:3232' else */
                                    /* 'fast_corner_detect_9:3229' else */
                                    /* 'fast_corner_detect_9:3226' else */
                                }
                            } else {
                                /* 'fast_corner_detect_9:3241' else */
                                /* 'fast_corner_detect_9:3242' if im(y+2,x+-2) < c_b */
                                if ((im_data[(y + im_sizes[0] * (x + 1)) - 1] < c_b) && (im_data[(y + im_sizes[0] * (x + 4)) - 6] < c_b) && (im_data[(y + im_sizes[0] * x) - 4] < c_b) && (im_data[(y + im_sizes[0] * (x + 2)) - 6] < c_b) && (im_data[(y + im_sizes[0] * (x + 1)) - 5] < c_b) && (im_data[(y + im_sizes[0] * (x + 3)) - 6] < c_b)) {
                                    /* 'fast_corner_detect_9:3243' if im(y+-3,x+1) < c_b */
                                    /* 'fast_corner_detect_9:3244' if im(y+-1,x+-3) < c_b */
                                    /* 'fast_corner_detect_9:3245' if im(y+-3,x+-1) < c_b */
                                    /* 'fast_corner_detect_9:3246' if im(y+-2,x+-2) < c_b */
                                    /* 'fast_corner_detect_9:3247' if im(y+-3,x+0) < c_b */
                                    guard1 = TRUE;
                                } else {
                                    /* 'fast_corner_detect_9:3263' else */
                                    /* 'fast_corner_detect_9:3260' else */
                                    /* 'fast_corner_detect_9:3257' else */
                                    /* 'fast_corner_detect_9:3254' else */
                                    /* 'fast_corner_detect_9:3251' else */
                                    /* 'fast_corner_detect_9:3248' else */
                                }
                            }
                        } else {
                            /* 'fast_corner_detect_9:3270' else */
                            /* 'fast_corner_detect_9:3267' else */
                        }
                    }
                } else {
                    /* 'fast_corner_detect_9:3274' else */
                }
            }
            if (guard1 == TRUE) {
                /* 'fast_corner_detect_9:3278' nc = nc + 1; */
                nc++;
                /*  			if nc > length(cs)													 */
                /*  				cs(length(cs)*2,1) = 0;											 */
                /*  			end																	 */
                /* 'fast_corner_detect_9:3282' cs(nc,1) = x; */
                cs[nc] = (uint8_T)(x + 4);
                /* 'fast_corner_detect_9:3283' cs(nc,2) = y; */
                cs[5000 + nc] = (uint8_T)(y - 2);
            }
        }
    }
    /* 'fast_corner_detect_9:3286' coords = cs([1:nc],:); */
    if (1 > nc + 1) {
        nc = -1;
    }
    coords_sizes[0] = nc + 1;
    coords_sizes[1] = 2;
    for (loop_ub = 0; loop_ub < 2; loop_ub++) {
        for (x = 0; x <= nc; x++) {
            coords_data[x + coords_sizes[0] * loop_ub] = (real_T)cs[x + 5000 * loop_ub];
        }
    }
}
/* End of code generation (fast_corner_detect_9.c) */
