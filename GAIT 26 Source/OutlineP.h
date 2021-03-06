/************
The GAIT Tool was developed at the National Geospatial-Intelligence Agency (NGA) in collaboration with the   
Institute for Defense Analyses (IDA) under a Government contract. The government is releasing this software   
to increase the impact of government investments by providing developers with the opportunity to take things   
in new directions. The software use, modification, and distribution rights are stipulated within the MIT license.

As-is warranty is covered within the MIT license but it's re-stated here for clarity: The GAIT tool contains several   
software components. The software is provided "as is," and no warranty, express or implied, including but not limited   
to the implied warranties of merchantability and fitness for particular purpose or arising by statute or otherwise in   
law or from a course of dealing or usage in trade, is made with respect to the accuracy or functioning of the software.   
Neither NGA or IDA are liable for any claims, losses, or damages arising from or connected with the use of the software.   
The user's sole and exclusive remedy is to stop using the software.
************/
/* OutlineP.h: Private header file for the Outline widget. */
/* Copyright © 1994 Torgeir Veimo. */
/* See the README file for copyright details. */

#ifndef OUTLINEP_H
#define OUTLINEP_H

#include "Outline.h"
#include <Xm/ManagerP.h>

typedef struct _XmOutlineClassPart {
    int    empty;
} XmOutlineClassPart;
        
typedef struct _XmOutlineClassRec {
    CoreClassPart        core_class;
    CompositeClassPart   composite_class;
    ConstraintClassPart  constraint_class;
    XmManagerClassPart   manager_class;
    XmOutlineClassPart   outline_class;
}  XmOutlineClassRec;
    
typedef struct {
    Bool draw_outline;     /* Draw outline? */
    Dimension indentation; /* Indentation for child widgets. */
    Dimension spacing;      /* Space between children. */
    Dimension marginWidth, marginHeight;
} XmOutlinePart;
    
typedef struct _XmOutlineRec {
    CorePart        core;
    CompositePart   composite;
    ConstraintPart  constraint;
    XmManagerPart   manager;
    XmOutlinePart   outline;
}  XmOutlineRec;

#endif /* OUTLINEP_H */

