/************
GAIT (Geospatial Analysis Integrity Tool) is a geospatial data validation tool developed by the Institute for Defense Analyses (IDA) for the National Geospatial-Intelligence Agency (NGA).  

This source code was used to generate GAIT 26 executable software in accordance with Amendment 6 to Task Order DH-8-3815 under Contract HQ0034-14-D-0001.

IDA is furnishing this item "as is". IDA was not tasked or funded to generate developer documentation or to provide support for this source code. IDA does not provide any warranty of the item whatsoever, whether express, implied, or statutory, including, but not limited to, any warranty of fitness for a particular purpose or any warranty that the contents of the item will be error-free. In no event shall NGA or IDA be held liable for damages arising, directly or indirectly, from the use of this source code. 

This material may be reproduced by the U.S. Government pursuant to its unlimited use rights under DFARS 252.227-7014 [Feb 2014].

The Institute for Defense Analyses (IDA) is a Federally Funded Research and Development Center that provides scientific and technical expertise on issues important to national security to the Office of the Secretary of Defense, Joint Staff, Unified Commands, and Defense Agencies. 

� 2017 Institute for Defense Analyses
  4850 Mark Center Drive
  Alexandria, Virginia 22311-1882
  703.845-2000
  www.ida.org
************/


#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "share.h"
#include "shapefil.h"

#define NIL  &sentinel 
#define DuplicateSearchLimit 5000


extern double RegionSize;
extern double Xtranslation;
extern double Ytranslation;

extern int CIRCLESIZE;
extern struct SCCdata * SCCtable;
extern FILE *unsortout;
extern FILE *errtypeinout;
extern FILE *CDFout;
extern char * IDheaderfile;
extern char * IDdatafile;

extern int SzD;
extern int SzI;
extern int SzUC;
extern int SzShort;
extern int SzL;
extern int NumberOfModels;
extern int TotalSCC; /** is short list number ***/
extern int SACfull;  /** == long list entries ***/
extern struct ModelNames * MdlNames;
extern struct CrossWalk  *CrsWlk;
extern int INscc_loop;
extern double PI;
extern int DRAW_NOW;

extern double *GL_magnitudes;
extern int    *GL_instances;
extern int    *GL_condnums;
extern char  **GL_IDs;
extern char  **GL_errnums;
extern char  **GL_geoms;


extern int CDFREPORT;
extern int SLASHTYPE;

/*** below defined in moregeomchecks.c ***/
extern void FwriteDynamicInfo(int keyval, int Cnumber, int PtUsed, int MagUsed, int NumObjects,
                                  char *msg, double magnitude, double pointX, double pointY, double pointZ,
                                  int index1, char gform1, int Lindex1, double localid1, int IDN1,
                                  double radius1, double height1,
                                  double *x1, double *y1, double *z1, int numverts1,
                                  int index2, char gform2, int Lindex2, double localid2, int IDN2,
                                  double radius2, double height2,
                                  double *x2, double *y2, double *z2, int numverts2);

struct ErrorTable ErrorLookup[CONDITION_ARRAY_SIZE];
struct CloneErrorTable *CloneErrorLookup=NULL; /* will malloc and realloc */

extern void not_while_running(Widget w,char *message,int mode,char *title,int type);
extern Widget drawing_a;
extern int GetCloneIndex(int clonenum,int errnum);
extern void ProcessSF(char *ignore_shapefile);
extern double RadiansToDegrees(double radianmeasure);
extern char * ParseGAITgeometry(unsigned char geom, int CaseFlag);
extern int FindMinMaxSensitivities(int checktype, double *min1, double *max1, double *min2, double *max2, int *numT);
extern int file_endianness;
extern char indirectory[500];

int ConsultPreviouslyIgnored = 1;
char IgnoredLineFile[1000];
char IgnoredPointFile[1000];
char IgnoredLineFile2[1000];
char IgnoredPointFile2[1000];

struct SEEITconditionObject
{
  int keyval;
  int Cnumber; /* not an index! */
  int msglen;
  char *errmsg;
  char keepit;
  double magnitude;
  double px,py,pz;
  int Lindex1, Lindex2;
  double LocalID1, LocalID2;
  int idn1, idn2;
  int ECC1, ECC2;
  int SIDlen1, SIDlen2;
  char * SID1, *SID2;
  char gform1, gform2;
  double radius1, radius2;
  double height1, height2;
  int numverts1, numverts2;
  double *x1, *x2;
  double *y1, *y2;
  double *z1, *z2;
} GenericErr, GE2;




struct IndexEntry
{
  long int fileposn;
  long int endfileposn;
  double magnitude;
  int Cindex;
  unsigned char isduplicate;
  struct IndexEntry * next;
} *CListRoot, *CListLast;

struct ConditionSort
{
  int kv;
  rb_red_blk_tree * RB_Entry;
} *RegularConditions, *CloneConditions;

rb_red_blk_node * RB_newObjNode;
rb_red_blk_tree * RB_ObjTree = NULL;
rb_red_blk_tree * RB_ObjSortTree = NULL;
struct RB_ObjErrList
{
  double magnitude;
  int ecc_index;
  int Lindex2;
  int keyval;
  int instanceNum;
  int specificNum;
  struct RB_ObjErrList *next;
};
struct RB_ObjList
{
  char geom;
  int ecc_index;
  int Lindex1;
  int numerrs;
  double id;
  double sortid;
  struct RB_ObjErrList * lastentry;
  struct RB_ObjErrList * errs;
};
struct RB_ListOfObjects
{
  struct RB_ObjList * o;
  struct RB_ListOfObjects * next;
};

struct RB_ObjErrList *oelc;
struct RB_ObjList *olc;
struct RB_ListOfObjects *loo;



struct ConditionObjectForRW_DC
{
  int keyval;
  int Cnumber; /* not an index! */
  int msglen;
  int errmsgMax;
  char *errmsg;
  double magnitude;
  double px,py,pz;
  int idn1, idn2;
  int ECC1, ECC2;
  int SIDlen1Max, SIDlen2Max;
  int SIDlen1, SIDlen2;
  char * SID1, *SID2;
  char gform1, gform2;
  int Lindex1, Lindex2;
  double localID1, localID2;
  double radius1, radius2;
  double height1, height2;
  int MaxVerts1, MaxVerts2;
  int numverts1, numverts2;
  double *x1, *x2;
  double *y1, *y2;
  double *z1, *z2;
} *Current_GE, *Last_GE, *GE3, *GE4;
struct ConditionObjectDuplicateForRW_DC
{
  int keyval;
  int Cnumber; 
  double magnitude;
  double px,py,pz;
  int idn1, idn2;
  int ECC1, ECC2;
  char gform1, gform2;
  unsigned int Lindex1, Lindex2;
  double localID1, localID2;
  double radius1, radius2;
  double height1, height2;
  int numverts1, numverts2;
  double *x1, *x2;
  double *y1, *y2;
  double *z1, *z2;
  struct ConditionObjectDuplicateForRW_DC * next;
} *GE_DUP_Root, *GE_DUP1, *GE_DUP2;
int SzCO_DC = sizeof(struct ConditionObjectDuplicateForRW_DC);

struct NonMagnitudeDupRemoval
   {
   double px, py, pz;
   double LID1, LID2;
   int KV, Cindex;
   struct NonMagnitudeDupRemoval * next;
   } *NMDRroot, *NMDRc, *NMDRn;

struct CheckGroupNames CGN[NUMCHECKGROUPS];

struct clonestocount
   {
   int cindex;
   int count;
   struct clonestocount *next;
   };
struct CountCondByObj
   {
   int count;
   struct clonestocount *c;
   } CCBY[CONDITION_DEFINITIONS + 2];
int SzCTC = sizeof(struct clonestocount);
int SzNMDR = sizeof(struct NonMagnitudeDupRemoval);


struct IG_features
{
   int Instance;
   int Cnumber;
   int Record;
   int NV1;
   int NV2;
   char GT1;
   char GT2;
   char * IDstr1;
   char * IDstr2;
   double Magnitude;
   double *Px;
   double *Py;
   double *X1;
   double *Y1;
   double *X2;
   double *Y2;
   struct IG_features * next;
} *IGNn, *IGNc, *IGNp;
struct IG_features *IGN[CONDITION_DEFINITIONS+5];









int ETF_DblCompGE(const void * a, const void * b)
{
  if( * (double *) a > * (double *) b)
    return(1);
  else if( * (double *) a < * (double *) b)
    return(-1);
  else
    return(0);
}

int ETF_DblCompLE(const void * a, const void * b)
{
  if( * (double *) a < * (double *) b)
    return(1);
  else if( * (double *) a > * (double *) b)
    return(-1);
  else
    return(0);
}



int ETF_IntCompLE(const void * a, const void * b)
{
  if( * (int *) a < * (int *) b)
    return(1);
  else if( * (int *) a > * (int *) b)
    return(-1);
  else
    return(0);
}




void ETF_IntPrint(const void * a)
{
  printf("%d", * (int *) a);
}

void ETF_InfoPrint(void * a)
{
  return;
}

void ETF_InfoDest(void * a)
{
  return;
  /**** here, want to free memory, but not in this usage ****
   *
   * example follows where tree->info points to objects of type ConditionCollection:
   struct ConditionCollection *C;
   
   C = (struct ConditionCollection *) a;
   free(C->x);
   free(C->y);
   free(C->z);
   free(C);
   
   return;
  **********/
}

void ETF_ObjInfoDest(void * e)
{
  struct RB_ObjList *o;
  struct RB_ObjErrList *ec, *en;
  
  o = (struct RB_ObjList *) e;
  ec = o->errs;
  while(ec != NULL)
    {
      en = ec;
      ec = ec->next;
      free(en);
    }
  free(o);
  
  return;
}


void ETF_ObjCondListDest(void * e)
{
  struct RB_ListOfObjects *o, *n;
  o = (struct RB_ListOfObjects *) e;
  while(o != NULL)
    {
      n = o;
      o = o->next;
      free(n);
    }
}



void ETF_IntDest(void * a)
{
  free((int *) a);
  return;
}

void ETF_DblDest(void * a)
{
  /**free((double *) a);  **/
  return;
}


void ETF_ObjDblDest(void * a)
{
  free((double *) a);
  return;
}


void ETF_Assert(int assertion, char* error) {
  if(!assertion) {
    printf("Assertion Failed: %s\n",error);
    exit(-1);
  }
}

void * ETF_SafeMalloc(size_t size) {
  void * result;
  
  if ( (result = malloc(size)) ) {
    return(result);
  } else {
    printf("memory overflow: malloc failed in ETF_SafeMalloc.");
    printf("  Exiting Program.\n");
    exit(-1);
    return(0);
  }
}




rb_red_blk_tree* ETF_RBTreeCreate( int (*CompFunc) (const void*,const void*),
				   void (*DestFunc) (void*),
				   void (*InfoDestFunc) (void*),
				   void (*PrintFunc) (const void*),
				   void (*PrintInfo)(void*)) {
  rb_red_blk_tree* newTree;
  rb_red_blk_node* temp;
  
  newTree=(rb_red_blk_tree*) ETF_SafeMalloc(sizeof(rb_red_blk_tree));
  newTree->Compare=  CompFunc;
  newTree->DestroyKey= DestFunc;
  newTree->PrintKey= PrintFunc;
  newTree->PrintInfo= PrintInfo;
  newTree->DestroyInfo= InfoDestFunc;
  
  temp=newTree->nil= (rb_red_blk_node*) ETF_SafeMalloc(sizeof(rb_red_blk_node));
  temp->parent=temp->left=temp->right=temp;
  temp->red=0;
  temp->key=0;
  temp=newTree->root= (rb_red_blk_node*) ETF_SafeMalloc(sizeof(rb_red_blk_node));
  temp->parent=temp->left=temp->right=newTree->nil;
  temp->key=0;
  temp->red=0;
  return(newTree);
}

void ETF_LeftRotate(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;
  
  y=x->right;
  x->right=y->left;
  
  if (y->left != nil) y->left->parent=x;
  
  y->parent=x->parent;
  
  if( x == x->parent->left) {
    x->parent->left=y;
  } else {
    x->parent->right=y;
  }
  y->left=x;
  x->parent=y;
  
}

void ETF_RightRotate(rb_red_blk_tree* tree, rb_red_blk_node* y) {
  rb_red_blk_node* x;
  rb_red_blk_node* nil=tree->nil;
  
  x=y->left;
  y->left=x->right;
  
  if (nil != x->right)  x->right->parent=y;
  x->parent=y->parent;
  if( y == y->parent->left) {
    y->parent->left=x;
  } else {
    y->parent->right=x;
  }
  x->right=y;
  y->parent=x;
  
}


void ETF_TreeInsertHelp(rb_red_blk_tree* tree, rb_red_blk_node* z) {
  rb_red_blk_node* x;
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;
  
  z->left=z->right=nil;
  y=tree->root;
  x=tree->root->left;
  while( x != nil) {
    y=x;
    if (1 == tree->Compare(x->key,z->key)) {
      x=x->left;
    } else {
      x=x->right;
    }
  }
  z->parent=y;
  if ( (y == tree->root) ||
       (1 == tree->Compare(y->key,z->key))) {
    y->left=z;
  } else {
    y->right=z;
  }
  
}


rb_red_blk_node * ETF_RBTreeInsert(rb_red_blk_tree* tree, void* key, void* info) {
  rb_red_blk_node * y;
  rb_red_blk_node * x;
  rb_red_blk_node * newNode;
  
  x=(rb_red_blk_node*) ETF_SafeMalloc(sizeof(rb_red_blk_node));
  x->key=key;
  x->info=info;
  
  
  ETF_TreeInsertHelp(tree,x);
  newNode=x;
  x->red=1;
  while(x->parent->red) {
    if (x->parent == x->parent->parent->left) {
      y=x->parent->parent->right;
      if (y->red) {
        x->parent->red=0;
        y->red=0;
        x->parent->parent->red=1;
        x=x->parent->parent;
      } else {
        if (x == x->parent->right) {
          x=x->parent;
          ETF_LeftRotate(tree,x);
        }
        x->parent->red=0;
        x->parent->parent->red=1;
        ETF_RightRotate(tree,x->parent->parent);
      }
    } else {
      y=x->parent->parent->left;
      if (y->red) {
        x->parent->red=0;
        y->red=0;
        x->parent->parent->red=1;
        x=x->parent->parent;
      } else {
        if (x == x->parent->left) {
          x=x->parent;
          ETF_RightRotate(tree,x);
        }
        x->parent->red=0;
        x->parent->parent->red=1;
        ETF_LeftRotate(tree,x->parent->parent);
      }
    }
  }
  tree->root->left->red=0;
  return(newNode);
}


rb_red_blk_node* ETF_TreeSuccessor(rb_red_blk_tree* tree,rb_red_blk_node* x) {
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;
  
  if (nil != (y = x->right))
  {
    while(y->left != nil)
    {
      y=y->left;
    }
    return(y);
  } else {
    y=x->parent;
    while(x == y->right) {
      x=y;
      y=y->parent;
    }
    if (y == root) return(nil);
    return(y);
  }
}


rb_red_blk_node* ETF_TreePredecessor(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;
  
  if (nil != (y = x->left)) {
    while(y->right != nil) {
      y=y->right;
    }
    return(y);
  } else {
    y=x->parent;
    while(x == y->left) {
      if (y == root) return(nil);
      x=y;
      y=y->parent;
    }
    return(y);
  }
}


void ETF_InorderTreePrint(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;
  if (x != tree->nil) {
    ETF_InorderTreePrint(tree,x->left);
    printf("info=");
    tree->PrintInfo(x->info);
    printf("  key=");
    tree->PrintKey(x->key);
    printf("  l->key=");
    if( x->left == nil) printf("NULL"); else tree->PrintKey(x->left->key);
    printf("  r->key=");
    if( x->right == nil) printf("NULL"); else tree->PrintKey(x->right->key);
    printf("  p->key=");
    if( x->parent == root) printf("NULL"); else tree->PrintKey(x->parent->key);
    printf("  red=%i\n",x->red);
    ETF_InorderTreePrint(tree,x->right);
  }
}



void ETF_TreeDestHelper(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* nil=tree->nil;
  if (x != nil) {
    ETF_TreeDestHelper(tree,x->left);
    ETF_TreeDestHelper(tree,x->right);

    tree->DestroyKey(x->key);

    tree->DestroyInfo(x->info);

    free(x);
  }
}



void ETF_RBTreeDestroy(rb_red_blk_tree* tree) {
  ETF_TreeDestHelper(tree,tree->root->left);
  free(tree->root);
  free(tree->nil);
  free(tree);
}



void ETF_Tree_Obj_DestHelper(rb_red_blk_tree* tree, rb_red_blk_node* x)
{
  rb_red_blk_node* nil=tree->nil;
  if (x != nil)
    {
      ETF_TreeDestHelper(tree,x->left);
      ETF_TreeDestHelper(tree,x->right);
      tree->DestroyInfo(x->info);
      free(x->key);
      free(x);
    }     
}



void ETF_RB_Obj_TreeDestroy(rb_red_blk_tree* tree) {
  ETF_Tree_Obj_DestHelper(tree,tree->root->left);
  free(tree->root);
  free(tree->nil);
  free(tree); 
} 



rb_red_blk_node* ETF_RB_Dbl_ExactQuery(rb_red_blk_tree* tree, void* q) {
  rb_red_blk_node* x=tree->root->left;
  rb_red_blk_node* nil=tree->nil;
  int compVal;
  if (x == nil) return(0);
  compVal=tree->Compare(x->key,(double*) q);
  while(0 != compVal) {
    if (1 == compVal) {
      x=x->left; 
    } else {  
      x=x->right;
    }
    if ( x == nil) return(0);
    compVal=tree->Compare(x->key,(double*) q);
  }
  return(x);  
} 



rb_red_blk_node* ETF_RB_Int_ExactQuery(rb_red_blk_tree* tree, void* q) {
  rb_red_blk_node* x=tree->root->left;
  rb_red_blk_node* nil=tree->nil;
  int compVal;
  if (x == nil) return(0);
  compVal=tree->Compare(x->key,(int*) q);
  while(0 != compVal) {
    if (1 == compVal) {
      x=x->left;
    } else {
      x=x->right;
    }
    if ( x == nil) return(0);
    compVal=tree->Compare(x->key,(int*) q);
  }
  return(x);
}





rb_red_blk_node* ETF_RBExactQuery(rb_red_blk_tree* tree, void* q) {
  rb_red_blk_node* x=tree->root->left;
  rb_red_blk_node* nil=tree->nil;
  int compVal;
  if (x == nil) return(0);
  compVal=tree->Compare(x->key,(int*) q);
  while(0 != compVal) {
    if (1 == compVal) {
      x=x->left;
    } else {
      x=x->right;
    }
    if ( x == nil) return(0);
    compVal=tree->Compare(x->key,(int*) q);
  }
  return(x);
}



void ETF_RBDeleteFixUp(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* root=tree->root->left;
  rb_red_blk_node* w;
  
  while( (!x->red) && (root != x)) {
    if (x == x->parent->left) {
      w=x->parent->right;
      if (w->red) {
        w->red=0;
        x->parent->red=1;
        ETF_LeftRotate(tree,x->parent);
        w=x->parent->right;
      }
      if ( (!w->right->red) && (!w->left->red) ) {
        w->red=1;
        x=x->parent;
      } else {
        if (!w->right->red) {
          w->left->red=0;
          w->red=1;
          ETF_RightRotate(tree,w);
          w=x->parent->right;
        }
        w->red=x->parent->red;
        x->parent->red=0;
        w->right->red=0;
        ETF_LeftRotate(tree,x->parent);
        x=root;
      }
    } else {
      w=x->parent->left;
      if (w->red) {
        w->red=0;
        x->parent->red=1;
        ETF_RightRotate(tree,x->parent);
        w=x->parent->left;
      }
      if ( (!w->right->red) && (!w->left->red) ) {
        w->red=1;
        x=x->parent;
      } else {
        if (!w->left->red) {
          w->right->red=0;
          w->red=1;
          ETF_LeftRotate(tree,w);
          w=x->parent->left;
        }
        w->red=x->parent->red;
        x->parent->red=0;
        w->left->red=0;
        ETF_RightRotate(tree,x->parent);
        x=root;
      }
    }
  }
  x->red=0;
  
}



void ETF_RBDelete(rb_red_blk_tree* tree, rb_red_blk_node* z){
  rb_red_blk_node* y;
  rb_red_blk_node* x;
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;
  
  y= ((z->left == nil) || (z->right == nil)) ? z : ETF_TreeSuccessor(tree,z);
  x= (y->left == nil) ? y->right : y->left;
  if (root == (x->parent = y->parent)) { /* assignment of y->p to x->p is intentional */
    root->left=x;
  } else {
    if (y == y->parent->left) {
      y->parent->left=x;
    } else {
      y->parent->right=x;
    }
  }
  if (y != z) {
    
    if (!(y->red)) ETF_RBDeleteFixUp(tree,x);
    
    tree->DestroyKey(z->key);
    tree->DestroyInfo(z->info);
    y->left=z->left;
    y->right=z->right;
    y->parent=z->parent;
    y->red=z->red;
    z->left->parent=z->right->parent=y;
    if (z == z->parent->left) {
      z->parent->left=y;
    } else {
      z->parent->right=y;
    }
    free(z);
  } else {
    tree->DestroyKey(y->key);
    tree->DestroyInfo(y->info);
    if (!(y->red)) ETF_RBDeleteFixUp(tree,x);
    free(y);
  }
  
}



void RetrieveConditionTreeInorder( rb_red_blk_tree* tree, rb_red_blk_node* x)
{
  if (x != tree->nil)
    {
      RetrieveConditionTreeInorder(tree,x->left);
      
      if(CListRoot == NULL)
	{
	  CListRoot = (struct  IndexEntry *) (x->info);
	  CListRoot->next = NULL;
	  CListLast = CListRoot;
	}
      else
	{  
	  CListLast->next = (struct  IndexEntry *) (x->info);
	  CListLast = CListLast->next;
	  CListLast->next = NULL;
	}  
      
      RetrieveConditionTreeInorder(tree,x->right);
    }     
}        




void AddToObjectSortedConditionTree(int numconditions, struct RB_ObjList * ObjIn)
{
  int *RB_newKey;
  
  RB_newKey = (int *) (malloc(SzI));
  if(RB_newKey == NULL)
    {
      printf("available memory has been exhausted\n");
      exit(-1);
    }
  *RB_newKey = numconditions;
  if(RB_ObjSortTree == NULL)
    {
      RB_ObjSortTree = ETF_RBTreeCreate(ETF_IntCompLE, ETF_IntDest, ETF_ObjCondListDest, ETF_IntPrint,ETF_InfoPrint);
      loo = (struct RB_ListOfObjects * ) (malloc(sizeof(struct RB_ListOfObjects)));
      if(loo == NULL)
	{
	  printf("memory exhausted during object - condition assembly\n");
	  exit(-1);
	}
      loo->o = (struct RB_ObjList *) ObjIn;
      loo->next = NULL;
      
      ETF_RBTreeInsert(RB_ObjSortTree, RB_newKey, loo);
    }
  else if((RB_newObjNode = ETF_RB_Int_ExactQuery(RB_ObjSortTree,RB_newKey)))
    {
      loo = (struct RB_ListOfObjects * ) (malloc(sizeof(struct RB_ListOfObjects)));
      if(loo == NULL)
	{
	  printf("memory exhausted during object - condition assembly\n");
	  exit(-1);
	}
      loo->o = (struct RB_ObjList *) ObjIn;
      loo->next = (struct RB_ListOfObjects *) RB_newObjNode->info;
      RB_newObjNode->info = loo;
      free(RB_newKey);
    }
  else
    {
      loo = (struct RB_ListOfObjects * ) (malloc(sizeof(struct RB_ListOfObjects)));
      if(loo == NULL)
	{
	  printf("memory exhausted during object - condition assembly\n");
	  exit(-1);
	}
      loo->o = (struct RB_ObjList *) ObjIn;
      loo->next = NULL;
      
      ETF_RBTreeInsert(RB_ObjSortTree, RB_newKey, loo);
    }
  return;
}



void ReversePrintedListOrder(struct RB_ObjErrList *elist, FILE *fp)
{

   if(elist == NULL)
      return;
   ReversePrintedListOrder(elist->next, fp);

   SEEIT_fwrite_int(&elist->keyval,fp);
   SEEIT_fwrite_int(&elist->ecc_index,fp);
   SEEIT_fwrite_int(&elist->instanceNum,fp);
   SEEIT_fwrite_int(&elist->specificNum,fp);

   return;
}





void PrintSortedObjConditionTreeInorder( rb_red_blk_tree* tree, rb_red_blk_node* x, int * OW, int OPP, FILE *fp, int *OMF)
{
  int /**kv,**/maxnumb, numberwritten;
  struct RB_ObjList *otoprint;
  struct RB_ObjErrList *ectoprint;
  int writeFP, seekamount;
  long int fileposn;
  struct sorthelper
    {
    int kv;
    int instanceNum;
    int numb;
    struct RB_ObjErrList *starthere;
    struct sorthelper *next;
    } *SHroot, *sc, *pc;
   
  SHroot = NULL;
  
  if (x != tree->nil)
    {
      PrintSortedObjConditionTreeInorder(tree,x->left, OW, OPP, fp, OMF);
      
      
      loo = (struct RB_ListOfObjects * ) x->info;
      while(loo != NULL)
	{
          writeFP = *OW % OPP;
          if(writeFP == 0)
             {
             writeFP = *OW / OPP;
             fileposn = ftell(fp);
             seekamount = (3 * SzI) + (writeFP * SzL);
             fseek(fp,seekamount,SEEK_SET);
             SEEIT_fwrite_long(&fileposn,fp);
             seekamount = 0;
             fseek(fp,seekamount,SEEK_END);
             }
	  otoprint =  (struct RB_ObjList *) loo->o;
	  fwrite(&otoprint->geom,1,1,fp);
          SEEIT_fwrite_double(&otoprint->sortid, fp);
          SEEIT_fwrite_double(&otoprint->id, fp);
          SEEIT_fwrite_int(&otoprint->ecc_index,fp);
          SEEIT_fwrite_int(&otoprint->Lindex1,fp);
	  SEEIT_fwrite_int(&otoprint->numerrs,fp);

          OMF[CrsWlk[otoprint->Lindex1].crossindex] += 1;


          *OW += 1; 
	  
	  ectoprint = otoprint->errs;
          while(ectoprint != NULL)
             {
             if((SHroot == NULL) || (SHroot->kv != ectoprint->keyval) ||
                    (SHroot->instanceNum != ectoprint->instanceNum))
                {
                sc = (struct sorthelper *) (malloc(sizeof(struct sorthelper)));
                if(sc == NULL)
                   {
                   printf("available memory has been exhauseted during object-based condition sorting\n");
                   exit(-1);
                   }
                sc->kv = ectoprint->keyval;
                sc->instanceNum = ectoprint->instanceNum;
                sc->numb = 1;
                sc->starthere = ectoprint;
                sc->next = SHroot;
                SHroot = sc;
                }
             else if(SHroot->kv == ectoprint->keyval)
                {
                SHroot->numb += 1;
                }
             ectoprint = ectoprint->next;
             }

          if(SHroot->next == NULL)   /** only one kind of condition for this object ***/
             {
             SEEIT_fwrite_int(&SHroot->kv, fp);
             SEEIT_fwrite_int(&SHroot->numb,fp);
             ectoprint = otoprint->errs;
             SEEIT_fwrite_int(&ectoprint->instanceNum,fp);
             while(ectoprint != NULL)
                {
                SEEIT_fwrite_double(&ectoprint->magnitude,fp);
                SEEIT_fwrite_int(&ectoprint->ecc_index,fp);
                SEEIT_fwrite_int(&ectoprint->Lindex2,fp);
                SEEIT_fwrite_int(&ectoprint->specificNum,fp);
              
                ectoprint = ectoprint->next;
                }
             }
          else
             {
             maxnumb = 0;
             numberwritten = 0;
             while(numberwritten < otoprint->numerrs)
                {
                maxnumb = 0;
                pc = NULL;
                sc = SHroot;
                while(sc != NULL)
                   {
                   if((sc->numb > 0) && (sc->numb > maxnumb))
                      {
                      maxnumb = sc->numb;
                      pc = sc;
                      }
                   sc = sc->next;
                   }
                if(pc != NULL)
                   {
                   SEEIT_fwrite_int(&pc->kv, fp);
                   SEEIT_fwrite_int(&pc->numb,fp);
                   ectoprint = pc->starthere;
                   SEEIT_fwrite_int(&ectoprint->instanceNum,fp);
                   while((ectoprint->keyval == pc->kv) && (ectoprint->instanceNum == pc->instanceNum))
                      {
                      SEEIT_fwrite_double(&ectoprint->magnitude,fp);
                      SEEIT_fwrite_int(&ectoprint->ecc_index,fp);
                      SEEIT_fwrite_int(&ectoprint->Lindex2,fp);
                      SEEIT_fwrite_int(&ectoprint->specificNum,fp);

                      ++numberwritten;

                      ectoprint = ectoprint->next;
                      if(ectoprint == NULL)
                         break;
                      }
                   pc->numb = -1;
                   }
                else
                   break;
                }
             }
          sc = SHroot;
          while(sc != NULL)
             {
             pc = sc;
             sc = sc->next;
             free(pc);
             }
          SHroot = NULL;


	  loo = loo->next;
	}
      
      PrintSortedObjConditionTreeInorder(tree,x->right, OW, OPP, fp, OMF);
    }
}






void PrintObjConditionTreeInorder( rb_red_blk_tree* tree, rb_red_blk_node* x)
{
  struct RB_ObjList *otoprint;
  
  if (x != tree->nil)
    {
      PrintObjConditionTreeInorder(tree,x->left);
      
      otoprint =  (struct RB_ObjList *) x->info;
      
      AddToObjectSortedConditionTree((otoprint->numerrs * 10000)+(10000 - otoprint->ecc_index), otoprint);
      PrintObjConditionTreeInorder(tree,x->right);
    }
}



/*** if a new object is placed in the tree, return 1, o/w, return zero ***/
int AddToObjectConditionTree(double localid, int sortid, int keyval, int Cindex, int ECCindex1, int ECCindex2,
                              double magnitude, int Lindex1, int Lindex2,  int SpecificCnumber, char geom, int * O, int * T)
{
  double *RB_newKey;
  struct clonestocount *ctcp, *ctcn, *ctcc;
  int newobjadded = 0;

  if(geom == G_GRIDPT)
     return(0);

  if(Cindex == 0)
     {
     CCBY[keyval].count += 1;
     }
  else
     {
     if(CCBY[keyval].c == NULL)
        {
        ctcn = (struct clonestocount *) (malloc(SzCTC));
        if(ctcn == NULL)
           {
           printf("memory has been exhausted during condition-by-object processing\n");
           printf("execution terminating\n");
           exit(-1);
           }
        else
           {
           ctcn->count = 1;
           ctcn->cindex = Cindex;
           ctcn->next = NULL;
           CCBY[keyval].c = ctcn;
           }
        }
     else
        {
        if(CCBY[keyval].c->cindex == Cindex)
           CCBY[keyval].c->count += 1;
        else if(CCBY[keyval].c->cindex > Cindex)
           {
           ctcn = (struct clonestocount *) (malloc(SzCTC));
           if(ctcn == NULL)
              {
              printf("memory has been exhausted during condition-by-object processing\n");
              printf("execution terminating\n");
              exit(-1);
              }
           else
              {
              ctcn->count = 1;
              ctcn->cindex = Cindex;
              ctcn->next = CCBY[keyval].c;
              CCBY[keyval].c = ctcn;
              }
           }
        else  /*** root clone index must be less than the clone index to count and possibly insert ***/
           {
           ctcp = ctcn = CCBY[keyval].c;
           while((ctcp != NULL) && (ctcp->cindex < Cindex))
              {
              ctcn = ctcp;
              ctcp = ctcp->next;
              } /** this will break when find == Cindex or all on list are < new Cindex ***/
           if(ctcp == NULL) /** must be largest Cindex seen so far - add it to end of list ***/
              {
              ctcp = (struct clonestocount *) (malloc(SzCTC));
              if(ctcp == NULL)
                 {
                 printf("memory has been exhausted during condition-by-object processing\n");
                 printf("execution terminating\n");
                 exit(-1);
                 }
              else
                 {
                 ctcp->count = 1;
                 ctcp->cindex = Cindex;
                 ctcp->next = NULL;
                 ctcn->next = ctcp;
                 }
              }
           else if(ctcp->cindex == Cindex) /** Cindex already in the list so bump its counter ***/
              {
              ctcp->count += 1;
              }
           else /** ctcn->cindex < Cindex and ctcp->cindex > Cindex, so insert Cindex between those two ***/
              {
              ctcc = (struct clonestocount *) (malloc(SzCTC));
              if(ctcc == NULL)
                 {
                 printf("memory has been exhausted during condition-by-object processing\n");
                 printf("execution terminating\n");
                 exit(-1);
                 }
              else
                 {
                 ctcc->count = 1;
                 ctcc->cindex = Cindex;
                 ctcc->next = ctcp;
                 ctcn->next = ctcc;
                 }
              }
           }
        }
     }
  
  RB_newKey = (double *) (malloc(SzD));
  if(RB_newKey == NULL)
    {
      printf("availanle memory has been exhausted\n");
      exit(-1);
    }
  *RB_newKey = (double) sortid;
  if(RB_ObjTree == NULL)
    {
      RB_ObjTree = ETF_RBTreeCreate(ETF_DblCompLE, ETF_ObjDblDest, ETF_ObjInfoDest, ETF_IntPrint,ETF_InfoPrint);
      oelc = (struct RB_ObjErrList * ) (malloc(sizeof(struct RB_ObjErrList)));
      olc = (struct RB_ObjList * ) (malloc(sizeof(struct RB_ObjList)));
      if(olc == NULL)
	{
	  printf("memory exhausted during object - condition assembly\n");
	  exit(-1);
	}
      olc->geom = geom;
      olc->Lindex1 = Lindex1;
      olc->id = localid;
      olc->sortid = (double) sortid;
      olc->ecc_index = ECCindex1;
      olc->numerrs = 1;
      olc->errs = oelc;
      olc->lastentry = oelc;
      
      oelc->keyval = keyval;
      oelc->ecc_index = ECCindex2;
      oelc->Lindex2 = Lindex2;
      oelc->magnitude = magnitude;
      oelc->instanceNum = Cindex;
      oelc->specificNum = SpecificCnumber;
      oelc->next = NULL;
      ETF_RBTreeInsert(RB_ObjTree, RB_newKey, olc);
      *O = 1;
      newobjadded = 1;
      *T = 1;
    }
  else if((RB_newObjNode = ETF_RB_Dbl_ExactQuery(RB_ObjTree,RB_newKey)))
    {
      oelc = (struct RB_ObjErrList * ) (malloc(sizeof(struct RB_ObjErrList)));
      olc = (struct RB_ObjList * ) RB_newObjNode->info;
      oelc->keyval = keyval;
      oelc->ecc_index = ECCindex2;
      oelc->Lindex2 = Lindex2;
      oelc->magnitude = magnitude;
      oelc->instanceNum = Cindex;
      oelc->specificNum = SpecificCnumber;
      olc->lastentry->next = oelc;
      olc->lastentry = oelc;
      oelc->next = NULL;
      olc->numerrs = olc->numerrs + 1;
      *T += 1;
      free(RB_newKey);
    }
  else
    {
      oelc = (struct RB_ObjErrList * ) (malloc(sizeof(struct RB_ObjErrList)));
      olc = (struct RB_ObjList * ) (malloc(sizeof(struct RB_ObjList)));
      if(olc == NULL)
	{
	  printf("memory exhausted during object - condition assembly\n");
	  exit(-1);
	}
      olc->geom = geom;
      olc->ecc_index = ECCindex1;
      olc->Lindex1 = Lindex1;
      olc->id = localid;
      olc->sortid = (double) sortid;
      olc->numerrs = 1;
      olc->errs = oelc;
      olc->lastentry = oelc;
      
      oelc->keyval = keyval;
      oelc->ecc_index = ECCindex2;
      oelc->Lindex2 = Lindex2;
      oelc->magnitude = magnitude;
      oelc->instanceNum = Cindex;
      oelc->specificNum = SpecificCnumber;
      oelc->next = NULL;
      *O += 1;
      newobjadded = 1;
      *T += 1;
      ETF_RBTreeInsert(RB_ObjTree, RB_newKey, olc);
    }
  return(newobjadded);
}




void Free_GE_DUP_List(void)
{
  GE_DUP1 = GE_DUP_Root;
  while(GE_DUP1 != NULL)
    {
      GE_DUP2 = GE_DUP1;
      GE_DUP1 = GE_DUP1->next;
      
      if(GE_DUP2->x1 != NULL)
        free(GE_DUP2->x1);
      if(GE_DUP2->y1 != NULL)
        free(GE_DUP2->y1);
      if(GE_DUP2->z1 != NULL)
        free(GE_DUP2->z1);
      if(GE_DUP2->x2 != NULL)
        free(GE_DUP2->x2);
      if(GE_DUP2->y2 != NULL)
        free(GE_DUP2->y2);
      if(GE_DUP2->z2 != NULL)
        free(GE_DUP2->z2);
      
      free(GE_DUP2);
    }
  GE_DUP_Root = NULL;
}



int SameValueWithinTolerance(double v1, double v2,double tolerance)
{
double d;
  if(v1 > v2)
     d = v1 - v2;
  else
     d = v2 - v1;

  if(d < tolerance)
     return(1);
  else
     return(0);
}




double DC_FreadFwriteDynamicInfo(int keyval, int writeflag, int *ObjsWritten, int *Duplicate, FILE *infile, FILE *outfile)
{
  int i;
  int PtUsed;
  int MagUsed;
  int NumObjects;
  double returnval;
  int isDup;
  int DSL;

  *Duplicate = 0;

  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);

  if(Current_GE == GE3)
    {
      Current_GE = GE4;
    }
  else
    {
      Current_GE = GE3;
    }

  Current_GE->keyval = keyval;

  Current_GE->numverts1 = 0;
  Current_GE->msglen = 0;
  Current_GE->gform1 = 0;
  Current_GE->idn1 = 0;
  Current_GE->radius1 = 0;
  Current_GE->height1 = 0;
  Current_GE->px = Current_GE->py = Current_GE->pz = 0;
  Current_GE->numverts2 = 0;
  Current_GE->gform2 = 0;
  Current_GE->idn2 = 0;
  Current_GE->radius2 = 0;
  Current_GE->height2 = 0;
  Current_GE->errmsg = NULL;

  SEEIT_fread_int(&Current_GE->Cnumber,infile);
  SEEIT_fread_int(&PtUsed,infile);
  SEEIT_fread_int(&MagUsed,infile);
  SEEIT_fread_int(&NumObjects,infile);

  *ObjsWritten = NumObjects;
/*** stopped here ***/

  SEEIT_fread_int(&Current_GE->msglen,infile);
  if(Current_GE->msglen > 0)
     {
     if(Current_GE->msglen > Current_GE->errmsgMax)
        {
        Current_GE->errmsgMax = Current_GE->msglen + 5;
        if(Current_GE->errmsg != NULL)
           Current_GE->errmsg = (char *) (realloc(Current_GE->errmsg,Current_GE->errmsgMax));
        else
           Current_GE->errmsg = (char *) (malloc(Current_GE->errmsgMax));
        if(Current_GE->errmsg == NULL)
           {
           printf("memory has been exhausted during Dynamic Information duplicate inspection, read/write operation\n terminating now\n");
           exit(-1);
           }
        }
     fread(&Current_GE->errmsg[0],1,Current_GE->msglen,infile);
     Current_GE->errmsg[Current_GE->msglen] = '\0';
     }

  Current_GE->magnitude = 0.0;
  if(MagUsed > 0)
     SEEIT_fread_double(&Current_GE->magnitude,infile);

  if(PtUsed > 0)
     {
     SEEIT_fread_double(&Current_GE->px, infile);
     SEEIT_fread_double(&Current_GE->py, infile);
     SEEIT_fread_double(&Current_GE->pz, infile);
     }

  if(NumObjects > 0)
     {
     SEEIT_fread_int(&Current_GE->idn1,infile);

     SEEIT_fread_int(&Current_GE->SIDlen1,infile);
     if(Current_GE->SIDlen1 > Current_GE->SIDlen1Max)
       {
       Current_GE->SID1 = (char *) (realloc(Current_GE->SID1, Current_GE->SIDlen1 + 1));
       if(Current_GE->SID1 == NULL)
          {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (DC_MagnitudeAndTwoObjects 1)\n",keyval);
          exit(-1);
          }
       Current_GE->SIDlen1Max = Current_GE->SIDlen1 + 1;
       }


     fread(&Current_GE->SID1[0],1,Current_GE->SIDlen1,infile);
     Current_GE->SID1[Current_GE->SIDlen1] = '\0';

     SEEIT_fread_int(&Current_GE->ECC1,infile);

     fread(&Current_GE->gform1,1,1,infile);

     SEEIT_fread_int(&Current_GE->Lindex1,infile);

     SEEIT_fread_double(&Current_GE->localID1,infile);
     SEEIT_fread_double(&Current_GE->radius1,infile);

     SEEIT_fread_double(&Current_GE->height1,infile);

     SEEIT_fread_int(&Current_GE->numverts1, infile);

     if(Current_GE->numverts1 > Current_GE->MaxVerts1)
       {
         Current_GE->MaxVerts1 = Current_GE->numverts1 + 1;
         Current_GE->x1 = (double *) (realloc(Current_GE->x1, SzD * Current_GE->numverts1 + 1));
         Current_GE->y1 = (double *) (realloc(Current_GE->y1, SzD * Current_GE->numverts1 + 1));
         Current_GE->z1 = (double *) (realloc(Current_GE->z1, SzD * Current_GE->numverts1 + 1));
         if(Current_GE->z1 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue (DC_MagnitudeAndTwoObjects 2)\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
       }
   
     for (i=0; i<Current_GE->numverts1; i++)
        {
        SEEIT_fread_double(&Current_GE->x1[i], infile);
        SEEIT_fread_double(&Current_GE->y1[i], infile);
        SEEIT_fread_double(&Current_GE->z1[i], infile);
        }
     }

  /** second object ***/
  if(NumObjects > 1)
     {
     SEEIT_fread_int(&Current_GE->idn2,infile);

     SEEIT_fread_int(&Current_GE->SIDlen2,infile);
     if(Current_GE->SIDlen2 > Current_GE->SIDlen2Max)
        {
        Current_GE->SID2 = (char *) (realloc(Current_GE->SID2, Current_GE->SIDlen2 + 1));
        if(Current_GE->SID2 == NULL)
           {
           printf("System allocation memory has been exhausted during SEE-IT condition read\n");
           printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
           printf("   execution cannot continue\n");
           printf("error keyvalue is %d (DC_MagnitudeAndTwoObjects 3)\n",keyval);
           exit(-1);
           }
        Current_GE->SIDlen2Max = Current_GE->SIDlen2 + 1;
        }

     fread(&Current_GE->SID2[0],1,Current_GE->SIDlen2,infile);
     Current_GE->SID2[Current_GE->SIDlen2] = '\0';

     SEEIT_fread_int(&Current_GE->ECC2,infile);

     fread(&Current_GE->gform2,1,1,infile);

     SEEIT_fread_int(&Current_GE->Lindex2,infile);

     SEEIT_fread_double(&Current_GE->localID2,infile);

     SEEIT_fread_double(&Current_GE->radius2,infile);

     SEEIT_fread_double(&Current_GE->height2,infile);

     SEEIT_fread_int(&Current_GE->numverts2, infile);
     if(Current_GE->numverts2 > Current_GE->MaxVerts2)
        {
        Current_GE->MaxVerts2 = Current_GE->numverts2 + 1;
        Current_GE->x2 = (double *) (realloc(Current_GE->x2, SzD * Current_GE->numverts2 + 1));
        Current_GE->y2 = (double *) (realloc(Current_GE->y2, SzD * Current_GE->numverts2 + 1));
        Current_GE->z2 = (double *) (realloc(Current_GE->z2, SzD * Current_GE->numverts2 + 1));
        if(Current_GE->z2 == NULL)
           {
           printf("System allocation memory has been exhausted during SEE-IT condition read\n");
           printf("   execution cannot continue (DC_MagnitudeAndTwoObjects 4)\n");
           printf("error keyvalue is %d\n",keyval);
           exit(-1);
           }
        }
     for (i=0; i<Current_GE->numverts2; i++)
        {
        SEEIT_fread_double(&Current_GE->x2[i], infile);
        SEEIT_fread_double(&Current_GE->y2[i], infile);
        SEEIT_fread_double(&Current_GE->z2[i], infile);
        }
     }

  if(writeflag > 0)
    {
      isDup = 0;
      if(Last_GE != NULL)
        {
          if((Last_GE->keyval == Current_GE->keyval) && (Last_GE->Cnumber == Current_GE->Cnumber) && (Last_GE->magnitude == Current_GE->magnitude))
            {
              if(isDup == 0)
                {
                  DSL = 0;
                  GE_DUP1 = GE_DUP_Root;
                  while(GE_DUP1 != NULL)
                    {
                      if((GE_DUP1->keyval == Current_GE->keyval) && (GE_DUP1->magnitude == Current_GE->magnitude))
                        {
                          if((GE_DUP1->Cnumber == Current_GE->Cnumber) &&
                                  (GE_DUP1->x1[0] == Current_GE->x1[0]) && (GE_DUP1->y1[0] == Current_GE->y1[0]) &&
                                  (GE_DUP1->x2[0] == Current_GE->x2[0]) && (GE_DUP1->y2[0] == Current_GE->y2[0]))
                            {
                              if((GE_DUP1->localID1 == Current_GE->localID1) && (GE_DUP1->localID2 == Current_GE->localID2))
                                 isDup = 2;
                              else if((GE_DUP1->localID2 == Current_GE->localID1) && (GE_DUP1->localID1 == Current_GE->localID2))
                                 isDup = 2;
                            }
                        }
                      if(isDup > 0)
                        {
                        break;
                        }
                      GE_DUP1 = GE_DUP1->next;
                      DSL++;
                      if(DSL > DuplicateSearchLimit)
                        {
                          GE_DUP1 = NULL;
                          break;
                        }
                    }
                } /*** end first isDup == 0 ***/

                  if(isDup  == 0)
                    {
                      if(GE_DUP_Root == NULL)
                         {
                         GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
                         if(GE_DUP1 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->x1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->y1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->z1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->x2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->y2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->z2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         if(GE_DUP1->z2 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->keyval = Last_GE->keyval;
                         GE_DUP1->magnitude = Last_GE->magnitude;
                         GE_DUP1->idn1 = Last_GE->idn1;
                         GE_DUP1->idn2 = Last_GE->idn2;
                         GE_DUP1->Lindex1 = Last_GE->Lindex1;
                         GE_DUP1->Lindex2 = Last_GE->Lindex2;
                         GE_DUP1->localID1 = Last_GE->localID1;
                         GE_DUP1->localID2 = Last_GE->localID2;
                         GE_DUP1->Cnumber = Last_GE->Cnumber;
                         GE_DUP1->numverts1 = Last_GE->numverts1;
                         GE_DUP1->numverts2 = Last_GE->numverts2;
                         for (i=0; i<Last_GE->numverts1; i++)
                           {
                             GE_DUP1->x1[i] = Last_GE->x1[i];
                             GE_DUP1->y1[i] = Last_GE->y1[i];
                             GE_DUP1->z1[i] = Last_GE->z1[i];
                           }
                         for (i=0; i<Last_GE->numverts2; i++)
                           {
                             GE_DUP1->x2[i] = Last_GE->x2[i];
                             GE_DUP1->y2[i] = Last_GE->y2[i];
                             GE_DUP1->z2[i] = Last_GE->z2[i];
                           }
                         GE_DUP_Root = GE_DUP1;
                         GE_DUP_Root->next = NULL;
                         }
                      GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
                      if(GE_DUP1 == NULL)
                        {
                          printf("available system memory has been consumed during duplicate removal process\n");
                          printf("GAIT processing must terminate\n");
                          exit(-1);
                        }
                      GE_DUP1->x1 = (double *) (malloc(SzD * Current_GE->numverts1));
                      GE_DUP1->y1 = (double *) (malloc(SzD * Current_GE->numverts1));
                      GE_DUP1->z1 = (double *) (malloc(SzD * Current_GE->numverts1));
                      GE_DUP1->x2 = (double *) (malloc(SzD * Current_GE->numverts2));
                      GE_DUP1->y2 = (double *) (malloc(SzD * Current_GE->numverts2));
                      GE_DUP1->z2 = (double *) (malloc(SzD * Current_GE->numverts2));
                      if(GE_DUP1->z2 == NULL)
                        {
                          printf("available system memory has been consumed during duplicate removal process\n");
                          printf("GAIT processing must terminate\n");
                          exit(-1);
                        }
                      GE_DUP1->keyval = Current_GE->keyval;
                      GE_DUP1->magnitude = Current_GE->magnitude;
                      GE_DUP1->idn1 = Current_GE->idn1;
                      GE_DUP1->idn2 = Current_GE->idn2;
                      GE_DUP1->Lindex1 = Current_GE->Lindex1;
                      GE_DUP1->Lindex2 = Current_GE->Lindex2;
                      GE_DUP1->localID1 = Current_GE->localID1;
                      GE_DUP1->localID2 = Current_GE->localID2;
                      GE_DUP1->Cnumber = Current_GE->Cnumber;
                      GE_DUP1->numverts1 = Current_GE->numverts1;
                      GE_DUP1->numverts2 = Current_GE->numverts2;
                      for (i=0; i<Current_GE->numverts1; i++)
                        {
                          GE_DUP1->x1[i] = Current_GE->x1[i];
                          GE_DUP1->y1[i] = Current_GE->y1[i];
                          GE_DUP1->z1[i] = Current_GE->z1[i];
                        }
                      for (i=0; i<Current_GE->numverts2; i++)
                        {
                          GE_DUP1->x2[i] = Current_GE->x2[i];
                          GE_DUP1->y2[i] = Current_GE->y2[i];
                          GE_DUP1->z2[i] = Current_GE->z2[i];
                        }
                      GE_DUP1->next = GE_DUP_Root;
                      GE_DUP_Root = GE_DUP1;
                    }
                 /*** end first isDup == 0 ***/
            } /*** end if((Last_GE->keyval == Current_GE->keyval) && (Last_GE->magnitude == Current_GE->magnitude)) ***/
          else if(GE_DUP_Root != NULL) /*** have found a new magnitude, so no need to worry about dups here ... ****/
            {
              Free_GE_DUP_List();
            }
        } /** end if Last_GE != NULL ***/

      if(isDup < 1)  /** then write the data out **/
        {
          Last_GE = Current_GE;

          SEEIT_fwrite_int(&keyval,outfile);
          SEEIT_fwrite_int(&Current_GE->Cnumber,outfile);

          SEEIT_fwrite_int(&PtUsed,outfile);
          SEEIT_fwrite_int(&MagUsed,outfile);
          SEEIT_fwrite_int(&NumObjects,outfile);

          SEEIT_fwrite_int(&Current_GE->msglen,outfile);
          if(Current_GE->msglen > 0)
             fwrite(&Current_GE->errmsg[0],1,Current_GE->msglen,outfile);

          if(MagUsed > 0)
             SEEIT_fwrite_double(&Current_GE->magnitude,outfile);

         if(PtUsed > 0)
            {
            SEEIT_fwrite_double(&Current_GE->px, outfile);
            SEEIT_fwrite_double(&Current_GE->py, outfile);
            SEEIT_fwrite_double(&Current_GE->pz, outfile);
            }
          if(NumObjects > 0)
             {
             SEEIT_fwrite_int(&Current_GE->idn1,outfile);
             SEEIT_fwrite_int(&Current_GE->SIDlen1,outfile);
             fwrite(&Current_GE->SID1[0],1,Current_GE->SIDlen1,outfile);
             SEEIT_fwrite_int(&Current_GE->ECC1,outfile);
             fwrite(&Current_GE->gform1,1,1,outfile);
             SEEIT_fwrite_int(&Current_GE->Lindex1,outfile);
             SEEIT_fwrite_double(&Current_GE->localID1,outfile);
             SEEIT_fwrite_double(&Current_GE->radius1,outfile);
             SEEIT_fwrite_double(&Current_GE->height1,outfile);
             SEEIT_fwrite_int(&Current_GE->numverts1, outfile);
             for (i=0; i<Current_GE->numverts1; i++)
                {
                SEEIT_fwrite_double(&Current_GE->x1[i], outfile);
                SEEIT_fwrite_double(&Current_GE->y1[i], outfile);
                SEEIT_fwrite_double(&Current_GE->z1[i], outfile);
                }
             }
          /** second object ***/
          if(NumObjects > 1)
             {
             SEEIT_fwrite_int(&Current_GE->idn2,outfile);
             SEEIT_fwrite_int(&Current_GE->SIDlen2,outfile);
             fwrite(&Current_GE->SID2[0],1,Current_GE->SIDlen2,outfile);
             SEEIT_fwrite_int(&Current_GE->ECC2,outfile);
             fwrite(&Current_GE->gform2,1,1,outfile);
             SEEIT_fwrite_int(&Current_GE->Lindex2,outfile);
             SEEIT_fwrite_double(&Current_GE->localID2,outfile);
             SEEIT_fwrite_double(&Current_GE->radius2,outfile);
             SEEIT_fwrite_double(&Current_GE->height2,outfile);
             SEEIT_fwrite_int(&Current_GE->numverts2, outfile);
             for (i=0; i<Current_GE->numverts2; i++)
                {
                SEEIT_fwrite_double(&Current_GE->x2[i], outfile);
                SEEIT_fwrite_double(&Current_GE->y2[i], outfile);
                SEEIT_fwrite_double(&Current_GE->z2[i], outfile);
                }
             }

          if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
            {
              fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\",\"%s\"\n",
                      keyval,
                      Current_GE->Cnumber + 1,
                      Current_GE->magnitude,
                      GetECCLabel(Current_GE->ECC1),
                      ParseGAITgeometry(Current_GE->gform1,2),
                      Current_GE->SID1,
                      GetECCLabel(Current_GE->ECC2),
                      ParseGAITgeometry(Current_GE->gform2,2),
                      Current_GE->SID2,
                      ParseErrType(keyval));
            }
        }
      else
        {
          *Duplicate = 1;
          Last_GE = Current_GE;
        }
    } /*** end if writeflag ***/

  returnval = Current_GE->magnitude;

  return(returnval);
}








void DC_FreadFwritePointAndObject(int keyval, int writeflag, int *Duplicate, FILE *infile, FILE *outfile)
{
  int i;
  int numverts;
  int sl1, sl2;
  int isDup;
  
  *Duplicate = 0;
  
  if(infile == NULL)
    return;
  if((writeflag > 0) && (outfile == NULL))
    return;
  
  if(Current_GE == GE3)
    {
      Current_GE = GE4;
    }
  else
    {
      Current_GE = GE3;
    }
  
  Current_GE->keyval = keyval;
  
  /**** first, just read in the object ***/
  Current_GE->numverts1 = 0;
  Current_GE->gform1 = 0;
  Current_GE->idn1 = 0;
  Current_GE->radius1 = 0;
  Current_GE->height1 = 0;
  Current_GE->px = Current_GE->py = Current_GE->pz = 0;
  Current_GE->numverts2 = 0;
  Current_GE->gform2 = 0;
  Current_GE->idn2 = 0;
  Current_GE->radius2 = 0;
  Current_GE->height2 = 0;
  Current_GE->msglen = 0;
  Current_GE->errmsg = NULL;
  
  SEEIT_fread_int(&Current_GE->Cnumber,infile);
  
  SEEIT_fread_double(&Current_GE->px, infile);
  SEEIT_fread_double(&Current_GE->py, infile);
  SEEIT_fread_double(&Current_GE->pz, infile);
  
  SEEIT_fread_int(&Current_GE->idn2,infile);
  SEEIT_fread_int(&sl1,infile);
  ++sl1;
  if(sl1 > Current_GE->SIDlen2Max)
    {
      Current_GE->SID2 = (char *) (realloc(Current_GE->SID2, sl1));
      if(Current_GE->SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen2);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d (DC_PointAndObject 1)\n",keyval);
	  exit(-1);
	}
      Current_GE->SIDlen2Max = sl1;
    }
  Current_GE->SIDlen2 = sl1 - 1;
  fread(&Current_GE->SID2[0],1,Current_GE->SIDlen2,infile);
  Current_GE->SID2[Current_GE->SIDlen2] = '\0';
  
  SEEIT_fread_int(&Current_GE->idn1,infile);
  
  SEEIT_fread_int(&sl2,infile);
  ++sl2;
  if(sl2 > Current_GE->SIDlen1Max)
    {
      Current_GE->SID1 = (char *) (realloc(Current_GE->SID1, sl2));
      if(Current_GE->SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d (DC_PointAndObject 2)\n",keyval);
	  exit(-1);
	}
      Current_GE->SIDlen1Max = sl2;
    }
  Current_GE->SIDlen1 = sl2 - 1;
  
  fread(&Current_GE->SID1[0],1,Current_GE->SIDlen1,infile);
  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC1,infile);
  
  fread(&Current_GE->gform1,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex1,infile);
  
  SEEIT_fread_double(&Current_GE->localID1,infile);
  
  SEEIT_fread_double(&Current_GE->radius1,infile);
  
  SEEIT_fread_double(&Current_GE->height1,infile);
  
  SEEIT_fread_int(&numverts, infile);
  if(numverts > Current_GE->MaxVerts1)
    {
      Current_GE->MaxVerts1 = numverts;
      Current_GE->x1 = (double *) (realloc(Current_GE->x1, SzD * Current_GE->MaxVerts1));
      Current_GE->y1 = (double *) (realloc(Current_GE->y1, SzD * Current_GE->MaxVerts1));
      Current_GE->z1 = (double *) (realloc(Current_GE->z1, SzD * Current_GE->MaxVerts1));
      if(Current_GE->z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
    }
  Current_GE->numverts1 = numverts;
  for (i=0; i<Current_GE->numverts1; i++)
    {
      SEEIT_fread_double(&Current_GE->x1[i], infile);
      SEEIT_fread_double(&Current_GE->y1[i], infile);
      SEEIT_fread_double(&Current_GE->z1[i], infile);
    }
  
  if(writeflag > 0)
    {
      isDup = 0;
      if(Last_GE != NULL) /*** then, have read a preceeding object - check for duplication before write ***/
	{
	  if((Last_GE->keyval == Current_GE->keyval) &&
	     (Last_GE->Cnumber == Current_GE->Cnumber) && (Last_GE->px == Current_GE->px) && 
	     (Last_GE->py == Current_GE->py) && (Last_GE->pz == Current_GE->pz) &&
	     (Last_GE->idn1 == Current_GE->idn1) && (Last_GE->idn2 == Current_GE->idn2) &&
	     (Last_GE->numverts1 == Current_GE->numverts1))
	    {
	      for (i=0; i<Current_GE->numverts1; i++)
		{
		  if((Last_GE->x1[i] != Current_GE->x1[i]) || (Last_GE->y1[i] != Current_GE->y1[i]) ||(Last_GE->z1[i] != Current_GE->z1[i]))
		    break;
		}
	      if(i == Current_GE->numverts1)  /**** must be a duplicate, so don't write it out ****/
		isDup = 1;
	    }
	}
      if(isDup == 0)
	{
	  Last_GE = Current_GE;
	  
	  SEEIT_fwrite_int(&keyval,outfile);
	  SEEIT_fwrite_int(&Current_GE->Cnumber,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->px, outfile);
	  SEEIT_fwrite_double(&Current_GE->py, outfile);
	  SEEIT_fwrite_double(&Current_GE->pz, outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->idn2,outfile);
	  SEEIT_fwrite_int(&Current_GE->SIDlen2,outfile);
	  
	  Current_GE->SID2[Current_GE->SIDlen2] = '\0';
	  fwrite(&Current_GE->SID2[0],1,Current_GE->SIDlen2,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->idn1,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->SIDlen1,outfile);
	  
	  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
	  fwrite(&Current_GE->SID1[0],1,Current_GE->SIDlen1,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->ECC1,outfile);
	  
	  fwrite(&Current_GE->gform1,1,1,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->Lindex1,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->localID1,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->radius1,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->height1,outfile);
	  SEEIT_fwrite_int(&Current_GE->numverts1, outfile);
	  for (i=0; i<Current_GE->numverts1; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z1[i], outfile);
	    }
	  if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	    {
	      Current_GE->gform2 = 0;
	      fprintf(CDFout,"%d,%d,,\"%s\",\"%s\",\"%s\",%lf,%lf,%lf,,,,\"%s\"\n",
		      keyval,
		      Current_GE->Cnumber + 1,
		      GetECCLabel(Current_GE->ECC1),
		      ParseGAITgeometry(Current_GE->gform1,2),
		      Current_GE->SID1,
		      Current_GE->px,
		      Current_GE->py,
		      Current_GE->pz,
		      ParseErrType(keyval));
	    }
	}
      else
	{
	  Last_GE = Current_GE;
	  *Duplicate = 1;
	}
    }
  
  return;
}




double DC_FreadFwriteObjectAndMagnitude(int keyval, int writeflag, int *Duplicate, FILE *infile, FILE *outfile)
{
  double returnval;
  int i;
  int sl1;
  int isDup;
  int numverts;
  int DSL;
  
  *Duplicate = 0;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  if(Current_GE == GE3)
    {
      Current_GE = GE4;
    }
  else
    {
      Current_GE = GE3;
    }
  
  Current_GE->keyval = keyval;
  
  /**** first, just read in the object ***/
  Current_GE->numverts1 = 0;
  Current_GE->gform1 = 0; 
  Current_GE->idn1 = 0;
  Current_GE->radius1 = 0;
  Current_GE->height1 = 0;
  Current_GE->px = Current_GE->py = Current_GE->pz = 0;
  Current_GE->numverts2 = 0;
  Current_GE->gform2 = 0; 
  Current_GE->idn2 = 0;
  Current_GE->radius2 = 0;
  Current_GE->height2 = 0;
  Current_GE->msglen = 0;
  Current_GE->errmsg = NULL;
  
  SEEIT_fread_int(&Current_GE->Cnumber,infile);
  
  SEEIT_fread_double(&Current_GE->magnitude, infile);
  
  SEEIT_fread_int(&Current_GE->idn1,infile);
  
  SEEIT_fread_int(&sl1,infile);
  ++sl1;

  if(sl1 > Current_GE->SIDlen1Max)
    {
      Current_GE->SID1 = (char *) (realloc(Current_GE->SID1, sl1));
      if(Current_GE->SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d (DC_ObjectAndMagnitude 1)\n",keyval);
	  exit(-1);
	}
      Current_GE->SIDlen1Max = sl1;
    }
  Current_GE->SIDlen1 = sl1 - 1;
  fread(&Current_GE->SID1[0],1,Current_GE->SIDlen1,infile);
  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC1,infile);
  
  fread(&Current_GE->gform1,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex1,infile);
  
  SEEIT_fread_double(&Current_GE->localID1,infile);
  
  SEEIT_fread_double(&Current_GE->radius1,infile);
  
  SEEIT_fread_double(&Current_GE->height1,infile);
  
  SEEIT_fread_int(&numverts, infile);
  if(numverts > Current_GE->MaxVerts1)
    {
      Current_GE->MaxVerts1 = numverts;
      Current_GE->x1 = (double *) (realloc(Current_GE->x1, SzD * Current_GE->MaxVerts1));
      Current_GE->y1 = (double *) (realloc(Current_GE->y1, SzD * Current_GE->MaxVerts1));
      Current_GE->z1 = (double *) (realloc(Current_GE->z1, SzD * Current_GE->MaxVerts1));
      if(Current_GE->z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("Memory request for %d vertices (x,y, and z) has failed\n",numverts);
	  printf("   execution cannot continue (DC_ObjectAndMagnitude 2)\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
    }
  Current_GE->numverts1 = numverts;
  for (i=0; i<Current_GE->numverts1; i++)
    {
      SEEIT_fread_double(&Current_GE->x1[i], infile);
      SEEIT_fread_double(&Current_GE->y1[i], infile);
      SEEIT_fread_double(&Current_GE->z1[i], infile);
    }
  
  if(writeflag > 0)
    {
      isDup = 0;
      if(Last_GE != NULL)
	{
	  if((Last_GE->keyval == Current_GE->keyval) && (Last_GE->magnitude == Current_GE->magnitude))
	    {
	      if((Last_GE->idn1 == Current_GE->idn1) && 
		 (Last_GE->Cnumber == Current_GE->Cnumber) && (Last_GE->numverts1 == Current_GE->numverts1))
		{
		  for (i=0; i<Current_GE->numverts1; i++)
		    {
		      if((Last_GE->x1[i] != Current_GE->x1[i]) || (Last_GE->y1[i] != Current_GE->y1[i]) ||(Last_GE->z1[i] != Current_GE->z1[i]))
			{
			  break;
			}
		    }
		  if(i == Current_GE->numverts1)  /**** must be a duplicate, so don't write it out ****/
		    {
		      isDup = 1;
		    }
		}
	      if(isDup == 0) /**if(GE_DUP_Root != NULL) ***/
		{
		  GE_DUP1 = GE_DUP_Root;
		  DSL = 0;
		  while(GE_DUP1 != NULL)
		    {
		      if((GE_DUP1->keyval == Current_GE->keyval) && (GE_DUP1->magnitude == Current_GE->magnitude))
			{
			  if((GE_DUP1->idn1 == Current_GE->idn1) &&
			     (GE_DUP1->Cnumber == Current_GE->Cnumber) && (GE_DUP1->numverts1 == Current_GE->numverts1))
			    {
			      for (i=0; i<Current_GE->numverts1; i++)
				{
				  if((GE_DUP1->x1[i] != Current_GE->x1[i]) || (GE_DUP1->y1[i] != Current_GE->y1[i]) ||(GE_DUP1->z1[i] != Current_GE->z1[i]))
				    {
				      break;
				    }
				}
			      if(i == Current_GE->numverts1)  /**** must be a duplicate, so don't write it out ****/
				{
				  isDup = 1;
				  break;
				}
			    }
			  if(isDup > 0)
			    break;
			}
		      GE_DUP1 = GE_DUP1->next;
		      DSL++;
		      if(DSL > DuplicateSearchLimit)
			{
			  GE_DUP1 = NULL;
			  break;
			}
		    }
                 }
                  if(isDup == 0)
		    {
                    if(GE_DUP_Root == NULL)
                      {
                         GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
                         if(GE_DUP1 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->x1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->y1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->z1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->x2 = NULL;
                         GE_DUP1->y2 = NULL;
                         GE_DUP1->z2 = NULL;
                         if(GE_DUP1->z1 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->keyval = Last_GE->keyval;
                         GE_DUP1->magnitude = Last_GE->magnitude;
                         GE_DUP1->idn1 = Last_GE->idn1;
                         GE_DUP1->Cnumber = Last_GE->Cnumber;
                         GE_DUP1->numverts1 = Last_GE->numverts1;
                         for (i=0; i<Last_GE->numverts1; i++)
                           {
                             GE_DUP1->x1[i] = Last_GE->x1[i];
                             GE_DUP1->y1[i] = Last_GE->y1[i];
                             GE_DUP1->z1[i] = Last_GE->z1[i];
                           }
                         GE_DUP1->next = NULL;
                         GE_DUP_Root = GE_DUP1;
                      }
		      GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
		      if(GE_DUP1 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->x1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->y1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->z1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->x2 = NULL;
		      GE_DUP1->y2 = NULL;
		      GE_DUP1->z2 = NULL;
		      if(GE_DUP1->z1 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->keyval = Current_GE->keyval;
		      GE_DUP1->magnitude = Current_GE->magnitude;
		      GE_DUP1->idn1 = Current_GE->idn1;
		      GE_DUP1->Cnumber = Current_GE->Cnumber;
		      GE_DUP1->numverts1 = Current_GE->numverts1;
		      for (i=0; i<Current_GE->numverts1; i++)
			{
			  GE_DUP1->x1[i] = Current_GE->x1[i];
			  GE_DUP1->y1[i] = Current_GE->y1[i];
			  GE_DUP1->z1[i] = Current_GE->z1[i];
			}
		      GE_DUP1->next = GE_DUP_Root;
		      GE_DUP_Root = GE_DUP1;
		    }
	    }
	  else if(GE_DUP_Root != NULL) /*** have found a new magnitude, so no need to worry about dups here ... ****/
	    {
	      Free_GE_DUP_List();
	    }
	}
      
      if(isDup < 1)
	{
	  Last_GE = Current_GE;
	  
	  SEEIT_fwrite_int(&keyval,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->Cnumber,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->magnitude, outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->idn1,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->SIDlen1,outfile);
	  fwrite(&Current_GE->SID1[0],1,Current_GE->SIDlen1,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->ECC1,outfile);
	  
	  fwrite(&Current_GE->gform1,1,1,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->Lindex1,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->localID1,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->radius1,outfile);
	  
	  SEEIT_fwrite_double(&Current_GE->height1,outfile);
	  
	  SEEIT_fwrite_int(&Current_GE->numverts1, outfile);
	  for (i=0; i<Current_GE->numverts1; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z1[i], outfile);
	    }
         if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
            {
              fprintf(CDFout,"%d,%d,%.20lf,\"%s\",\"%s\",\"%s\",\"%s\"\n",
                      keyval,
                      Current_GE->Cnumber + 1,
                      Current_GE->magnitude,
                      GetECCLabel(Current_GE->ECC1),
                      ParseGAITgeometry(Current_GE->gform1,2),
                      Coordinate2DtoString(Current_GE->x1[0],Current_GE->y1[0]),
                      ParseErrType(keyval));
            }

	}
      else
	{
	  GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
	  if(GE_DUP1 == NULL)
	    {
	      printf("available system memory has been consumed during duplicate removal process\n");
	      printf("GAIT processing must terminate\n");
	      exit(-1);
	    }
	  GE_DUP1->x1 = (double *) (malloc(SzD * Last_GE->numverts1));
	  GE_DUP1->y1 = (double *) (malloc(SzD * Last_GE->numverts1));
	  GE_DUP1->z1 = (double *) (malloc(SzD * Last_GE->numverts1));
	  GE_DUP1->x2 = NULL;
	  GE_DUP1->y2 = NULL;
	  GE_DUP1->z2 = NULL;
	  if(GE_DUP1->z1 == NULL)
	    {
	      printf("available system memory has been consumed during duplicate removal process\n");
	      printf("GAIT processing must terminate\n");
	      exit(-1);
	    }
	  GE_DUP1->keyval = Last_GE->keyval;
	  GE_DUP1->magnitude = Last_GE->magnitude;
	  GE_DUP1->idn1 = Last_GE->idn1;
	  GE_DUP1->Cnumber = Last_GE->Cnumber;
	  GE_DUP1->numverts1 = Last_GE->numverts1;
	  for (i=0; i<Last_GE->numverts1; i++)
	    {
	      GE_DUP1->x1[i] = Last_GE->x1[i];
	      GE_DUP1->y1[i] = Last_GE->y1[i];
	      GE_DUP1->z1[i] = Last_GE->z1[i];
	    }
	  GE_DUP1->next = GE_DUP_Root;
	  GE_DUP_Root = GE_DUP1;
	  Last_GE = Current_GE;
	  *Duplicate = 1;
	}
    }
  
  returnval = Current_GE->magnitude;
  
  return(returnval);
}





double DC_FreadFwritePointEdgeAndMagnitude(int keyval, int writeflag, int *Duplicate, FILE *infile, FILE *outfile)
{
  int i;
  double returnval;
  int isDup;
  int DSL;
  
  *Duplicate = 0;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  if(Current_GE == GE3)
    {
      Current_GE = GE4;
    }
  else
    {
      Current_GE = GE3;
    }
  
  Current_GE->keyval = keyval;
  
  Current_GE->numverts1 = 0;
  Current_GE->gform1 = 0;
  Current_GE->idn1 = 0;
  Current_GE->radius1 = 0;
  Current_GE->height1 = 0;
  Current_GE->px = Current_GE->py = Current_GE->pz = 0;
  Current_GE->numverts2 = 0;
  Current_GE->gform2 = 0;
  Current_GE->idn2 = 0;
  Current_GE->radius2 = 0;
  Current_GE->height2 = 0;
  Current_GE->msglen = 0;
  Current_GE->errmsg = NULL;
  
  SEEIT_fread_int(&Current_GE->Cnumber,infile);
  
  SEEIT_fread_double(&Current_GE->magnitude,infile);
  
  SEEIT_fread_int(&Current_GE->idn1,infile);
  
  SEEIT_fread_int(&Current_GE->SIDlen1,infile);
  
  if(Current_GE->SIDlen1 > Current_GE->SIDlen1Max)
    {
      Current_GE->SID1 = (char *) (realloc(Current_GE->SID1, Current_GE->SIDlen1 + 1));
      if(Current_GE->SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (DC_PointEdgeAndMagnitude 1)\n",keyval);
          exit(-1);
        }
      Current_GE->SIDlen1Max = Current_GE->SIDlen1 + 1;
    }
  
  fread(&Current_GE->SID1[0],1,Current_GE->SIDlen1,infile);
  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC1,infile);
  
  fread(&Current_GE->gform1,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex1,infile);
  
  SEEIT_fread_double(&Current_GE->localID1,infile);
  
  Current_GE->numverts1 = 1;  /** no need for a realloc here, there is > 1 alloc already as a min ***/
  /** first object, the 2D point ***/
  SEEIT_fread_double(&Current_GE->x1[0], infile);
  SEEIT_fread_double(&Current_GE->y1[0], infile);
  SEEIT_fread_double(&Current_GE->z1[0], infile); 
  
  
  /** second object, the edge end-points ***/
  
  SEEIT_fread_int(&Current_GE->idn2,infile);
  
  SEEIT_fread_int(&Current_GE->SIDlen2,infile);
  
  if(Current_GE->SIDlen2 > Current_GE->SIDlen2Max)
    {
      Current_GE->SID2 = (char *) (realloc(Current_GE->SID2, Current_GE->SIDlen2 + 1));
      if(Current_GE->SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen2);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (DC_PointEdgeAndMagnitude 2)\n",keyval);
          exit(-1);
        }
      Current_GE->SIDlen1Max = Current_GE->SIDlen1 + 1;
    }
  
  fread(&Current_GE->SID2[0],1,Current_GE->SIDlen2,infile);
  Current_GE->SID2[Current_GE->SIDlen2] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC2,infile);
  
  fread(&Current_GE->gform2,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex2,infile);
  
  SEEIT_fread_double(&Current_GE->localID2,infile);
  
  Current_GE->numverts2 = 2;  /** no need for read of this field, always have 2 vertices **/
  /** similarly, no need to check for realloc as will have more than 2 spaces as min ***/
  for (i=0; i<Current_GE->numverts2; i++)
    {
      SEEIT_fread_double(&Current_GE->x2[i], infile);
      SEEIT_fread_double(&Current_GE->y2[i], infile);
      SEEIT_fread_double(&Current_GE->z2[i], infile);
    }
  
  if(writeflag > 0)
    {
      isDup = 0;
      if(Last_GE != NULL)
        {
	  if((Last_GE->keyval == Current_GE->keyval) && (Last_GE->magnitude == Current_GE->magnitude))
	    {
	      if((Last_GE->idn1 == Current_GE->idn1) && (Last_GE->idn2 == Current_GE->idn2) &&
                 (Last_GE->Cnumber == Current_GE->Cnumber) && (Last_GE->numverts1 == Current_GE->numverts1) &&
                 (Last_GE->x1[0] == Current_GE->x1[0]) && (Last_GE->y1[0] == Current_GE->y1[0]) && (Last_GE->z1[0] == Current_GE->z1[0]))
		{
		  for (i=0; i<Current_GE->numverts2; i++)
		    {
		      if((Last_GE->x2[i] != Current_GE->x2[i]) || (Last_GE->y2[i] != Current_GE->y2[i]) ||(Last_GE->z2[i] != Current_GE->z2[i]))
			break;
		    }
		  if(i == Current_GE->numverts2)  /**** must be a duplicate, so don't write it out ****/
		    isDup = 1;
		}
	      if(isDup == 0)
		{
		  GE_DUP1 = GE_DUP_Root;
		  DSL = 0;
		  while(GE_DUP1 != NULL)
		    {
		      if((GE_DUP1->keyval == Current_GE->keyval) && (GE_DUP1->magnitude == Current_GE->magnitude))
			{
			  if((GE_DUP1->idn1 == Current_GE->idn1) && (GE_DUP1->idn2 == Current_GE->idn2) &&
			     (GE_DUP1->Cnumber == Current_GE->Cnumber) && (GE_DUP1->numverts1 == Current_GE->numverts1) &&
			     (GE_DUP1->x1[0] == Current_GE->x1[0]) && (GE_DUP1->y1[0] == Current_GE->y1[0]) && (GE_DUP1->z1[0] == Current_GE->z1[0]))
			    {
			      for (i=0; i<Current_GE->numverts2; i++)
				{
				  if((GE_DUP1->x2[i] != Current_GE->x2[i]) || (GE_DUP1->y2[i] != Current_GE->y2[i]) ||(GE_DUP1->z2[i] != Current_GE->z2[i]))
				    break;
				}
			      if(i == Current_GE->numverts2)  /**** must be a duplicate, so don't write it out ****/
				isDup = 1;
			    }
			  if(isDup > 0)
			    break;
			}
		      GE_DUP1 = GE_DUP1->next;
		      DSL++;
		      if(DSL > DuplicateSearchLimit)
			{
			  GE_DUP1 = NULL;
			  break;
			}
		    }
                 }
		  if(isDup == 0)
		    {
                     if(GE_DUP_Root == NULL)
                        {
                         GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
                         if(GE_DUP1 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->x1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->y1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->z1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->x2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->y2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->z2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         if(GE_DUP1->z2 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->keyval = Last_GE->keyval;
                         GE_DUP1->magnitude = Last_GE->magnitude;
                         GE_DUP1->idn1 = Last_GE->idn1;
                         GE_DUP1->idn2 = Last_GE->idn2;
                         GE_DUP1->Cnumber = Last_GE->Cnumber;
                         GE_DUP1->numverts1 = Last_GE->numverts1;
                         GE_DUP1->numverts2 = Last_GE->numverts2;
                         for (i=0; i<Last_GE->numverts1; i++)
                           {
                             GE_DUP1->x1[i] = Last_GE->x1[i];
                             GE_DUP1->y1[i] = Last_GE->y1[i];
                             GE_DUP1->z1[i] = Last_GE->z1[i];
                           }
                         for (i=0; i<Last_GE->numverts2; i++)
                           {
                             GE_DUP1->x2[i] = Last_GE->x2[i];
                             GE_DUP1->y2[i] = Last_GE->y2[i];
                                GE_DUP1->z2[i] = Last_GE->z2[i];
                           }
                         GE_DUP1->next = NULL;
                         GE_DUP_Root = GE_DUP1;
                        }
		      GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
		      if(GE_DUP1 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->x1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->y1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->z1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->x2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      GE_DUP1->y2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      GE_DUP1->z2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      if(GE_DUP1->z2 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->keyval = Current_GE->keyval;
		      GE_DUP1->magnitude = Current_GE->magnitude;
		      GE_DUP1->idn1 = Current_GE->idn1;
		      GE_DUP1->idn2 = Current_GE->idn2;
		      GE_DUP1->Cnumber = Current_GE->Cnumber;
		      GE_DUP1->numverts1 = Current_GE->numverts1;
		      GE_DUP1->numverts2 = Current_GE->numverts2;
		      for (i=0; i<Current_GE->numverts1; i++)
			{
			  GE_DUP1->x1[i] = Current_GE->x1[i];
			  GE_DUP1->y1[i] = Current_GE->y1[i];
			  GE_DUP1->z1[i] = Current_GE->z1[i];
			}
		      for (i=0; i<Current_GE->numverts2; i++)
			{
			  GE_DUP1->x2[i] = Current_GE->x2[i];
			  GE_DUP1->y2[i] = Current_GE->y2[i];
			  GE_DUP1->z2[i] = Current_GE->z2[i];
			}
		      GE_DUP1->next = GE_DUP_Root;
		      GE_DUP_Root = GE_DUP1;
		    }
		 /*** end first isDup == 0 ***/
	    }
	  else if(GE_DUP_Root != NULL) /*** have found a new magnitude, so no need to worry about dups here ... ****/
	    {
	      Free_GE_DUP_List();
	    }
        }
      if(isDup < 1)  /** then write the data out **/
        {
	  Last_GE = Current_GE;
	  
	  SEEIT_fwrite_int(&keyval,outfile);
	  SEEIT_fwrite_int(&Current_GE->Cnumber,outfile);
	  SEEIT_fwrite_double(&Current_GE->magnitude,outfile);
	  SEEIT_fwrite_int(&Current_GE->idn1,outfile);
	  SEEIT_fwrite_int(&Current_GE->SIDlen1,outfile);
	  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
	  fwrite(&Current_GE->SID1[0],1,Current_GE->SIDlen1,outfile);
	  SEEIT_fwrite_int(&Current_GE->ECC1,outfile);
	  fwrite(&Current_GE->gform1,1,1,outfile);
	  SEEIT_fwrite_int(&Current_GE->Lindex1,outfile);
	  SEEIT_fwrite_double(&Current_GE->localID1,outfile);
	  for (i=0; i<Current_GE->numverts1; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z1[i], outfile); 
	    }
	  
	  /** second object ***/
	  SEEIT_fwrite_int(&Current_GE->idn2,outfile);
	  SEEIT_fwrite_int(&Current_GE->SIDlen2,outfile);
	  Current_GE->SID2[Current_GE->SIDlen2] = '\0';
	  fwrite(&Current_GE->SID2[0],1,Current_GE->SIDlen2,outfile);
	  SEEIT_fwrite_int(&Current_GE->ECC2,outfile);
	  fwrite(&Current_GE->gform2,1,1,outfile);
	  SEEIT_fwrite_int(&Current_GE->Lindex2,outfile);
	  SEEIT_fwrite_double(&Current_GE->localID2,outfile);
	  for (i=0; i<Current_GE->numverts2; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x2[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y2[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z2[i], outfile);
	    }
	  
	  if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	    {
	      fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\"\n",
		      keyval,
		      Current_GE->Cnumber + 1,
		      Current_GE->magnitude,
		      GetECCLabel(Current_GE->ECC1),
		      ParseGAITgeometry(Current_GE->gform1,2),
		      Current_GE->SID1,
		      GetECCLabel(Current_GE->ECC2),
		      ParseGAITgeometry(Current_GE->gform2,2),
		      Current_GE->SID2,
		      ParseErrType(keyval));
	    }
        }
      else
        {
	  Last_GE = Current_GE;
	  *Duplicate = 1;
        }
    }
  
  
  returnval = Current_GE->magnitude;
  
  return(returnval);
}




double DC_FreadFwritePointObjectAndMagnitude(int keyval, int writeflag, int *Duplicate, FILE *infile, FILE *outfile)
{
  int i;
  double returnval;
  int isDup;
  int DSL;
  
  *Duplicate = 0;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  if(Current_GE == GE3)
    {
      Current_GE = GE4;
    }
  else
    {
      Current_GE = GE3;
    }
  
  Current_GE->keyval = keyval;
  
  Current_GE->numverts1 = 0;
  Current_GE->gform1 = 0;
  Current_GE->idn1 = 0;
  Current_GE->radius1 = 0;
  Current_GE->height1 = 0;
  Current_GE->px = Current_GE->py = Current_GE->pz = 0;
  Current_GE->numverts2 = 0;
  Current_GE->gform2 = 0;
  Current_GE->idn2 = 0;
  Current_GE->radius2 = 0;
  Current_GE->height2 = 0;
  Current_GE->msglen = 0;
  Current_GE->errmsg = NULL;
  
  SEEIT_fread_int(&Current_GE->Cnumber,infile);
  
  SEEIT_fread_double(&Current_GE->magnitude,infile);
  
  SEEIT_fread_int(&Current_GE->idn1,infile);
  
  SEEIT_fread_int(&Current_GE->SIDlen1,infile);
  
  if(Current_GE->SIDlen1 > Current_GE->SIDlen1Max)
    {
      Current_GE->SID1 = (char *) (realloc(Current_GE->SID1, Current_GE->SIDlen1 + 1));
      if(Current_GE->SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (DC_PointObjectAndMagnitude 1)\n",keyval);
          exit(-1);
        }
      Current_GE->SIDlen1Max = Current_GE->SIDlen1 + 1;
    }
  
  fread(&Current_GE->SID1[0],1,Current_GE->SIDlen1,infile);
  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC1,infile);
  
  fread(&Current_GE->gform1,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex1,infile);
  
  SEEIT_fread_double(&Current_GE->localID1,infile);
  
  Current_GE->numverts1 = 1;  /** no need for a realloc here, there is > 1 alloc already as a min ***/
  /** first object, the 2D point ***/
  SEEIT_fread_double(&Current_GE->x1[0], infile);
  SEEIT_fread_double(&Current_GE->y1[0], infile);
  SEEIT_fread_double(&Current_GE->z1[0], infile);
  
  
  /** second object, the edge end-points ***/
  
  SEEIT_fread_int(&Current_GE->idn2,infile);
  
  SEEIT_fread_int(&Current_GE->SIDlen2,infile);
  
  if(Current_GE->SIDlen2 > Current_GE->SIDlen2Max)
    {
      Current_GE->SID2 = (char *) (realloc(Current_GE->SID2, Current_GE->SIDlen2 + 1));
      if(Current_GE->SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen2);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (DC_PointObjectAndMagnitude 2)\n",keyval);
          exit(-1);
        }
      Current_GE->SIDlen1Max = Current_GE->SIDlen1 + 1;
    }
  
  fread(&Current_GE->SID2[0],1,Current_GE->SIDlen2,infile);
  Current_GE->SID2[Current_GE->SIDlen2] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC2,infile);
  
  fread(&Current_GE->gform2,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex2,infile);
  
  SEEIT_fread_double(&Current_GE->localID2,infile);
  
  SEEIT_fread_int(&Current_GE->numverts2,infile);
  
  if(Current_GE->numverts2 > Current_GE->MaxVerts2)
    {
      Current_GE->MaxVerts2 = Current_GE->numverts2 + 1;
      Current_GE->x2 = (double *) (realloc(Current_GE->x2, SzD * Current_GE->numverts2 + 1));
      Current_GE->y2 = (double *) (realloc(Current_GE->y2, SzD * Current_GE->numverts2 + 1));
      Current_GE->z2 = (double *) (realloc(Current_GE->z2, SzD * Current_GE->numverts2 + 1));
      if(Current_GE->z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue (DC_PointObjectAndMagnitude 4)\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
    }
  
  for (i=0; i<Current_GE->numverts2; i++)
    {
      SEEIT_fread_double(&Current_GE->x2[i], infile);
      SEEIT_fread_double(&Current_GE->y2[i], infile);
      SEEIT_fread_double(&Current_GE->z2[i], infile);
    }
  
  if(writeflag > 0)
    {
      isDup = 0;
      if(Last_GE != NULL)
        {
	  if((Last_GE->keyval == Current_GE->keyval) && (Last_GE->magnitude == Current_GE->magnitude))
	    {
	      if((Last_GE->idn1 == Current_GE->idn1) && (Last_GE->idn2 == Current_GE->idn2) &&
		 (Last_GE->Cnumber == Current_GE->Cnumber) && (Last_GE->numverts1 == Current_GE->numverts1) &&
		 (Last_GE->x1[0] == Current_GE->x1[0]) && (Last_GE->y1[0] == Current_GE->y1[0]) && (Last_GE->z1[0] == Current_GE->z1[0]))
		{
		  for (i=0; i<Current_GE->numverts2; i++)
		    {
		      if((Last_GE->x2[i] != Current_GE->x2[i]) || (Last_GE->y2[i] != Current_GE->y2[i]) ||(Last_GE->z2[i] != Current_GE->z2[i]))
			break;
		    }
		  if(i == Current_GE->numverts2)  /**** must be a duplicate, so don't write it out ****/
		    isDup = 1;
		}
	      if(isDup == 0)
		{
		  DSL = 0;
		  GE_DUP1 = GE_DUP_Root;
		  while(GE_DUP1 != NULL)
		    {
		      if((GE_DUP1->keyval == Current_GE->keyval) && (GE_DUP1->magnitude == Current_GE->magnitude))
			{
			  if((GE_DUP1->idn1 == Current_GE->idn1) && (GE_DUP1->idn2 == Current_GE->idn2) &&
			     (GE_DUP1->Cnumber == Current_GE->Cnumber) && (GE_DUP1->numverts1 == Current_GE->numverts1) &&
			     (GE_DUP1->x1[0] == Current_GE->x1[0]) && (GE_DUP1->y1[0] == Current_GE->y1[0]) && (GE_DUP1->z1[0] == Current_GE->z1[0]))
			    {
			      for (i=0; i<Current_GE->numverts2; i++)
				{
				  if((GE_DUP1->x2[i] != Current_GE->x2[i]) || (GE_DUP1->y2[i] != Current_GE->y2[i]) ||(GE_DUP1->z2[i] != Current_GE->z2[i]))
				    break;
				}
			      if(i == Current_GE->numverts2)  /**** must be a duplicate, so don't write it out ****/
				isDup = 1;
			    }
			  if(isDup > 0)
			    break;
			}
		      GE_DUP1 = GE_DUP1->next;
		      DSL++;
		      if(DSL > DuplicateSearchLimit)
			{
			  GE_DUP1 = NULL;
			  break;
			}
		    }
                 }
		  if(isDup == 0)
		    {
                      if(GE_DUP_Root == NULL)
                        {
                         GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
                         if(GE_DUP1 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->x1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->y1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->z1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->x2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->y2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->z2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         if(GE_DUP1->z2 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->keyval = Last_GE->keyval;
                         GE_DUP1->magnitude = Last_GE->magnitude;
                         GE_DUP1->idn1 = Last_GE->idn1;
                         GE_DUP1->idn2 = Last_GE->idn2;
                         GE_DUP1->Cnumber = Last_GE->Cnumber;
                         GE_DUP1->numverts1 = Last_GE->numverts1;
                         GE_DUP1->numverts2 = Last_GE->numverts2;
                         for (i=0; i<Last_GE->numverts1; i++)
                           {
                             GE_DUP1->x1[i] = Last_GE->x1[i];
                             GE_DUP1->y1[i] = Last_GE->y1[i];
                             GE_DUP1->z1[i] = Last_GE->z1[i];
                           }
                         for (i=0; i<Last_GE->numverts2; i++)
                           {
                             GE_DUP1->x2[i] = Last_GE->x2[i];
                             GE_DUP1->y2[i] = Last_GE->y2[i];
                             GE_DUP1->z2[i] = Last_GE->z2[i];
                           }
                         GE_DUP1->next = NULL;
                         GE_DUP_Root = GE_DUP1;
                        }
		      GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
		      if(GE_DUP1 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->x1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->y1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->z1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->x2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      GE_DUP1->y2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      GE_DUP1->z2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      if(GE_DUP1->z2 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->keyval = Current_GE->keyval;
		      GE_DUP1->magnitude = Current_GE->magnitude;
		      GE_DUP1->idn1 = Current_GE->idn1;
		      GE_DUP1->idn2 = Current_GE->idn2;
		      GE_DUP1->Cnumber = Current_GE->Cnumber;
		      GE_DUP1->numverts1 = Current_GE->numverts1;
		      GE_DUP1->numverts2 = Current_GE->numverts2;
		      for (i=0; i<Current_GE->numverts1; i++)
			{
			  GE_DUP1->x1[i] = Current_GE->x1[i];
			  GE_DUP1->y1[i] = Current_GE->y1[i];
			  GE_DUP1->z1[i] = Current_GE->z1[i];
			}
		      for (i=0; i<Current_GE->numverts2; i++)
			{
			  GE_DUP1->x2[i] = Current_GE->x2[i];
			  GE_DUP1->y2[i] = Current_GE->y2[i];
			  GE_DUP1->z2[i] = Current_GE->z2[i];
			}
		      GE_DUP1->next = GE_DUP_Root;
		      GE_DUP_Root = GE_DUP1;
		    }
	    }
	  else if(GE_DUP_Root != NULL) /*** have found a new magnitude, so no need to worry about dups here ... ****/
	    {
	      Free_GE_DUP_List();
	    }
        }
      if(isDup < 1)  /** then write the data out **/
        {
	  Last_GE = Current_GE;
	  
	  SEEIT_fwrite_int(&keyval,outfile);
	  SEEIT_fwrite_int(&Current_GE->Cnumber,outfile);
	  SEEIT_fwrite_double(&Current_GE->magnitude,outfile);
	  SEEIT_fwrite_int(&Current_GE->idn1,outfile);
	  SEEIT_fwrite_int(&Current_GE->SIDlen1,outfile);
	  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
	  fwrite(&Current_GE->SID1[0],1,Current_GE->SIDlen1,outfile);
	  SEEIT_fwrite_int(&Current_GE->ECC1,outfile);
	  fwrite(&Current_GE->gform1,1,1,outfile);
	  SEEIT_fwrite_int(&Current_GE->Lindex1,outfile);
	  SEEIT_fwrite_double(&Current_GE->localID1,outfile);
	  for (i=0; i<Current_GE->numverts1; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z1[i], outfile);
	    }
	  
	  /** second object ***/
	  SEEIT_fwrite_int(&Current_GE->idn2,outfile);
	  SEEIT_fwrite_int(&Current_GE->SIDlen2,outfile);
	  Current_GE->SID2[Current_GE->SIDlen2] = '\0';
	  fwrite(&Current_GE->SID2[0],1,Current_GE->SIDlen2,outfile);
	  SEEIT_fwrite_int(&Current_GE->ECC2,outfile);
	  fwrite(&Current_GE->gform2,1,1,outfile);
	  SEEIT_fwrite_int(&Current_GE->Lindex2,outfile);
	  SEEIT_fwrite_double(&Current_GE->localID2,outfile);
	  SEEIT_fwrite_int(&Current_GE->numverts2, outfile);
	  for (i=0; i<Current_GE->numverts2; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x2[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y2[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z2[i], outfile);
	    }
	  
	  if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	    {
	      fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\"\n",
		      keyval,
		      Current_GE->Cnumber + 1,
		      Current_GE->magnitude,
		      GetECCLabel(Current_GE->ECC1),
		      ParseGAITgeometry(Current_GE->gform1,2),
		      Current_GE->SID1,
		      GetECCLabel(Current_GE->ECC2),
		      ParseGAITgeometry(Current_GE->gform2,2),
		      Current_GE->SID2,
		      ParseErrType(keyval));
	    }
        }
      else
        {
	  Last_GE = Current_GE;
	  
	  *Duplicate = 1;
        }
    }


  if((keyval == ENCONNECT) || (keyval == AREAKINK) || (keyval == INCLSLIVER) || (keyval == FEATBRIDGE) ||
     /***(keyval == LAINTNOEND) || ***/ (keyval == BADENCON) || (keyval == LENOCOVERA) )
     Current_GE->ECC2 = -1;
  
  
  returnval = Current_GE->magnitude;
  
  return(returnval);
}








double DC_FreadFwriteMagnitudeAndTwoObjects(int keyval, int writeflag, int *Duplicate, FILE *infile, FILE *outfile)
{
  int i;
  double returnval;
  int isDup;
  int DSL;
  
  *Duplicate = 0;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  if(Current_GE == GE3)
    {
      Current_GE = GE4;
    }
  else
    {
      Current_GE = GE3;
    }
  
  Current_GE->keyval = keyval;
  
  Current_GE->numverts1 = 0;
  Current_GE->gform1 = 0;
  Current_GE->idn1 = 0;
  Current_GE->radius1 = 0;
  Current_GE->height1 = 0;
  Current_GE->px = Current_GE->py = Current_GE->pz = 0;
  Current_GE->numverts2 = 0;
  Current_GE->gform2 = 0;
  Current_GE->idn2 = 0;
  Current_GE->radius2 = 0;
  Current_GE->height2 = 0;
  Current_GE->msglen = 0;
  Current_GE->errmsg = NULL;
  
  SEEIT_fread_int(&Current_GE->Cnumber,infile);
  
  SEEIT_fread_double(&Current_GE->magnitude,infile);
  
  SEEIT_fread_int(&Current_GE->idn1,infile);
  
  SEEIT_fread_int(&Current_GE->SIDlen1,infile);
  if(Current_GE->SIDlen1 > Current_GE->SIDlen1Max)
    {
      Current_GE->SID1 = (char *) (realloc(Current_GE->SID1, Current_GE->SIDlen1 + 1));
      if(Current_GE->SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (DC_MagnitudeAndTwoObjects 1)\n",keyval);
          exit(-1);
        }
      Current_GE->SIDlen1Max = Current_GE->SIDlen1 + 1;
    }
  
  
  fread(&Current_GE->SID1[0],1,Current_GE->SIDlen1,infile);
  Current_GE->SID1[Current_GE->SIDlen1] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC1,infile);
  
  fread(&Current_GE->gform1,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex1,infile);
  
  SEEIT_fread_double(&Current_GE->localID1,infile);
  
  SEEIT_fread_double(&Current_GE->radius1,infile);
  
  SEEIT_fread_double(&Current_GE->height1,infile);
  
  SEEIT_fread_int(&Current_GE->numverts1, infile);
  
  if(Current_GE->numverts1 > Current_GE->MaxVerts1)
    {
      Current_GE->MaxVerts1 = Current_GE->numverts1 + 1;
      Current_GE->x1 = (double *) (realloc(Current_GE->x1, SzD * Current_GE->numverts1 + 1));
      Current_GE->y1 = (double *) (realloc(Current_GE->y1, SzD * Current_GE->numverts1 + 1));
      Current_GE->z1 = (double *) (realloc(Current_GE->z1, SzD * Current_GE->numverts1 + 1));
      if(Current_GE->z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue (DC_MagnitudeAndTwoObjects 2)\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
    }
  
  for (i=0; i<Current_GE->numverts1; i++)
    {
      SEEIT_fread_double(&Current_GE->x1[i], infile);
      SEEIT_fread_double(&Current_GE->y1[i], infile);
      SEEIT_fread_double(&Current_GE->z1[i], infile);
    }
  
  /** second object ***/
  
  SEEIT_fread_int(&Current_GE->idn2,infile);
  
  SEEIT_fread_int(&Current_GE->SIDlen2,infile);
  if(Current_GE->SIDlen2 > Current_GE->SIDlen2Max)
    {
      Current_GE->SID2 = (char *) (realloc(Current_GE->SID2, Current_GE->SIDlen2 + 1));
      if(Current_GE->SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",Current_GE->SIDlen1);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (DC_MagnitudeAndTwoObjects 3)\n",keyval);
          exit(-1);
        }
      Current_GE->SIDlen2Max = Current_GE->SIDlen2 + 1;
    }
  
  fread(&Current_GE->SID2[0],1,Current_GE->SIDlen2,infile);
  Current_GE->SID2[Current_GE->SIDlen2] = '\0';
  
  SEEIT_fread_int(&Current_GE->ECC2,infile);
  
  fread(&Current_GE->gform2,1,1,infile);
  
  SEEIT_fread_int(&Current_GE->Lindex2,infile);
  
  SEEIT_fread_double(&Current_GE->localID2,infile);
  
  SEEIT_fread_double(&Current_GE->radius2,infile);
  
  SEEIT_fread_double(&Current_GE->height2,infile);
  
  SEEIT_fread_int(&Current_GE->numverts2, infile);
  if(Current_GE->numverts2 > Current_GE->MaxVerts2)
    {
      Current_GE->MaxVerts2 = Current_GE->numverts2 + 1;
      Current_GE->x2 = (double *) (realloc(Current_GE->x2, SzD * Current_GE->numverts2 + 1));
      Current_GE->y2 = (double *) (realloc(Current_GE->y2, SzD * Current_GE->numverts2 + 1));
      Current_GE->z2 = (double *) (realloc(Current_GE->z2, SzD * Current_GE->numverts2 + 1));
      if(Current_GE->z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue (DC_MagnitudeAndTwoObjects 4)\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
    }
  for (i=0; i<Current_GE->numverts2; i++)
    {
      SEEIT_fread_double(&Current_GE->x2[i], infile);
      SEEIT_fread_double(&Current_GE->y2[i], infile);
      SEEIT_fread_double(&Current_GE->z2[i], infile);
    }
  
  if(writeflag > 0)
    {
      isDup = 0;
      if(Last_GE != NULL)
        {
	  if((Last_GE->keyval == Current_GE->keyval) && (Last_GE->Cnumber == Current_GE->Cnumber) && (Last_GE->magnitude == Current_GE->magnitude))
	    {
if((Current_GE->gform1 == C_GRID) &&
     ((Current_GE->keyval == LODELEVDIF) || (Current_GE->keyval == MASKZERO) ||(Current_GE->keyval == MASKCONSTANT) ||
       (Current_GE->keyval == MASKCONFLICT) ||
       (Current_GE->keyval == MASKCONF2) || (Current_GE->keyval == GRIDEXACTDIF) ||
      (Current_GE->keyval == MASKMONO) || (Current_GE->keyval == MASKSHOREL) ||(Current_GE->keyval == CLAMP_SEG)))
isDup = 0;
else
{
/****************** don't do this for CNODE_ZBUST since want all the incidents that may occur at difference vertices******/
/****************** of the same two features **********************/
              if((Current_GE->keyval != CNODE_ZBUST) &&  (Current_GE->keyval != LEZ_PROX_3D) &&
                   (Current_GE->keyval != AUNDERSHTA) && (Current_GE->keyval !=AOVERSHTA) &&
                   (Last_GE->localID1 == Current_GE->localID1) && (Last_GE->localID2 == Current_GE->localID2))
                 isDup = 1;
              else if((Current_GE->keyval != CNODE_ZBUST) && (Current_GE->keyval != LEZ_PROX_3D) &&
                   (Current_GE->keyval != AUNDERSHTA) && (Current_GE->keyval !=AOVERSHTA) &&
                         (Last_GE->localID2 == Current_GE->localID1) && (Last_GE->localID1 == Current_GE->localID2))
                 isDup = 1;
              else if(((Current_GE->keyval == AUNDERSHTA) || (Current_GE->keyval ==AOVERSHTA)) &&
                        (Last_GE->localID1 == Current_GE->localID1) && (Last_GE->localID2 == Current_GE->localID2) &&
                         (Last_GE->x1[0] == Current_GE->x1[0]) && (Last_GE->y1[0] == Current_GE->y1[0]) &&
                         (Last_GE->x2[0] == Current_GE->x2[0]) && (Last_GE->y2[0] == Current_GE->y2[0]))
                 isDup = 1;
              else if(((Current_GE->keyval == AUNDERSHTA) || (Current_GE->keyval ==AOVERSHTA)) &&
                        (Last_GE->localID1 == Current_GE->localID1) && (Last_GE->localID2 == Current_GE->localID2) &&
                         ((((Last_GE->x1[0] == Current_GE->x1[0]) && (Last_GE->y1[0] == Current_GE->y1[0])) ||
                         ((Last_GE->x1[0] == Current_GE->x1[1]) && (Last_GE->y1[0] == Current_GE->y1[1]))) ||
                          (((Last_GE->x1[1] == Current_GE->x1[0]) && (Last_GE->y1[1] == Current_GE->y1[0])) ||
                         ((Last_GE->x1[1] == Current_GE->x1[1]) && (Last_GE->y1[1] == Current_GE->y1[1])))))
                 isDup = 1;
	      if(isDup == 0)
		{
		  DSL = 0;
		  GE_DUP1 = GE_DUP_Root;
		  while(GE_DUP1 != NULL)
		    {
		      if((GE_DUP1->keyval == Current_GE->keyval) && (GE_DUP1->magnitude == Current_GE->magnitude))
			{
                          if((GE_DUP1->Cnumber == Current_GE->Cnumber) &&
                                  (GE_DUP1->x1[0] == Current_GE->x1[0]) && (GE_DUP1->y1[0] == Current_GE->y1[0]) &&
                                  (GE_DUP1->x2[0] == Current_GE->x2[0]) && (GE_DUP1->y2[0] == Current_GE->y2[0]))
                            {
                              if((GE_DUP1->localID1 == Current_GE->localID1) && (GE_DUP1->localID2 == Current_GE->localID2))
                                 isDup = 2;
                              else if((GE_DUP1->localID2 == Current_GE->localID1) && (GE_DUP1->localID1 == Current_GE->localID2))
                                 isDup = 2;
                            }
			}
		      if(isDup > 0)
                        {
			break;
                        }
		      GE_DUP1 = GE_DUP1->next;
		      DSL++;
		      if(DSL > DuplicateSearchLimit)
			{
			  GE_DUP1 = NULL;
			  break;
			}
		    }
                } /*** end first isDup == 0 ***/
}
		  if(isDup  == 0)
		    {
                      if(GE_DUP_Root == NULL)
                         {
                         GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
                         if(GE_DUP1 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->x1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->y1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->z1 = (double *) (malloc(SzD * Last_GE->numverts1));
                         GE_DUP1->x2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->y2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         GE_DUP1->z2 = (double *) (malloc(SzD * Last_GE->numverts2));
                         if(GE_DUP1->z2 == NULL)
                           {
                             printf("available system memory has been consumed during duplicate removal process\n");
                             printf("GAIT processing must terminate\n");
                             exit(-1);
                           }
                         GE_DUP1->keyval = Last_GE->keyval;
                         GE_DUP1->magnitude = Last_GE->magnitude;
                         GE_DUP1->idn1 = Last_GE->idn1;
                         GE_DUP1->idn2 = Last_GE->idn2;
                         GE_DUP1->Lindex1 = Last_GE->Lindex1;
                         GE_DUP1->Lindex2 = Last_GE->Lindex2;
                         GE_DUP1->localID1 = Last_GE->localID1;
                         GE_DUP1->localID2 = Last_GE->localID2;
                         GE_DUP1->Cnumber = Last_GE->Cnumber;
                         GE_DUP1->numverts1 = Last_GE->numverts1;
                         GE_DUP1->numverts2 = Last_GE->numverts2;
                         for (i=0; i<Last_GE->numverts1; i++)
                           {
                             GE_DUP1->x1[i] = Last_GE->x1[i];
                             GE_DUP1->y1[i] = Last_GE->y1[i];
                             GE_DUP1->z1[i] = Last_GE->z1[i];
                           }
                         for (i=0; i<Last_GE->numverts2; i++)
                           {
                             GE_DUP1->x2[i] = Last_GE->x2[i];
                             GE_DUP1->y2[i] = Last_GE->y2[i];
                             GE_DUP1->z2[i] = Last_GE->z2[i];
                           }
                         GE_DUP_Root = GE_DUP1;
                         GE_DUP_Root->next = NULL;
                         }
		      GE_DUP1 = (struct ConditionObjectDuplicateForRW_DC *) (malloc(SzCO_DC));
		      if(GE_DUP1 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->x1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->y1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->z1 = (double *) (malloc(SzD * Current_GE->numverts1));
		      GE_DUP1->x2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      GE_DUP1->y2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      GE_DUP1->z2 = (double *) (malloc(SzD * Current_GE->numverts2));
		      if(GE_DUP1->z2 == NULL)
			{
			  printf("available system memory has been consumed during duplicate removal process\n");
			  printf("GAIT processing must terminate\n");
			  exit(-1);
			}
		      GE_DUP1->keyval = Current_GE->keyval;
		      GE_DUP1->magnitude = Current_GE->magnitude;
		      GE_DUP1->idn1 = Current_GE->idn1;
		      GE_DUP1->idn2 = Current_GE->idn2;
		      GE_DUP1->Lindex1 = Current_GE->Lindex1;
		      GE_DUP1->Lindex2 = Current_GE->Lindex2;
		      GE_DUP1->localID1 = Current_GE->localID1;
		      GE_DUP1->localID2 = Current_GE->localID2;
		      GE_DUP1->Cnumber = Current_GE->Cnumber;
		      GE_DUP1->numverts1 = Current_GE->numverts1;
		      GE_DUP1->numverts2 = Current_GE->numverts2;
		      for (i=0; i<Current_GE->numverts1; i++)
			{
			  GE_DUP1->x1[i] = Current_GE->x1[i];
			  GE_DUP1->y1[i] = Current_GE->y1[i];
			  GE_DUP1->z1[i] = Current_GE->z1[i];
			}
		      for (i=0; i<Current_GE->numverts2; i++)
			{
			  GE_DUP1->x2[i] = Current_GE->x2[i];
			  GE_DUP1->y2[i] = Current_GE->y2[i];
			  GE_DUP1->z2[i] = Current_GE->z2[i];
			}
		      GE_DUP1->next = GE_DUP_Root;
		      GE_DUP_Root = GE_DUP1;
		    }
		 /*** end first isDup == 0 ***/
	    } /*** end if((Last_GE->keyval == Current_GE->keyval) && (Last_GE->magnitude == Current_GE->magnitude)) ***/
	  else if(GE_DUP_Root != NULL) /*** have found a new magnitude, so no need to worry about dups here ... ****/
	    {
	      Free_GE_DUP_List();
	    }
        }

      if(isDup < 1)  /** then write the data out **/
        {
	  Last_GE = Current_GE;
	  
	  SEEIT_fwrite_int(&keyval,outfile);
	  SEEIT_fwrite_int(&Current_GE->Cnumber,outfile);
	  SEEIT_fwrite_double(&Current_GE->magnitude,outfile);
	  SEEIT_fwrite_int(&Current_GE->idn1,outfile);
	  SEEIT_fwrite_int(&Current_GE->SIDlen1,outfile);
	  fwrite(&Current_GE->SID1[0],1,Current_GE->SIDlen1,outfile);
	  SEEIT_fwrite_int(&Current_GE->ECC1,outfile);
	  fwrite(&Current_GE->gform1,1,1,outfile);
	  SEEIT_fwrite_int(&Current_GE->Lindex1,outfile);
	  SEEIT_fwrite_double(&Current_GE->localID1,outfile);
	  SEEIT_fwrite_double(&Current_GE->radius1,outfile);
	  SEEIT_fwrite_double(&Current_GE->height1,outfile);
	  SEEIT_fwrite_int(&Current_GE->numverts1, outfile);
	  for (i=0; i<Current_GE->numverts1; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y1[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z1[i], outfile);
	    }
	  
	  /** second object ***/
	  SEEIT_fwrite_int(&Current_GE->idn2,outfile);
	  SEEIT_fwrite_int(&Current_GE->SIDlen2,outfile);
	  fwrite(&Current_GE->SID2[0],1,Current_GE->SIDlen2,outfile);
	  SEEIT_fwrite_int(&Current_GE->ECC2,outfile);
	  fwrite(&Current_GE->gform2,1,1,outfile);
	  SEEIT_fwrite_int(&Current_GE->Lindex2,outfile);
	  SEEIT_fwrite_double(&Current_GE->localID2,outfile);
	  SEEIT_fwrite_double(&Current_GE->radius2,outfile);
	  SEEIT_fwrite_double(&Current_GE->height2,outfile);
	  SEEIT_fwrite_int(&Current_GE->numverts2, outfile);
	  for (i=0; i<Current_GE->numverts2; i++)
	    {
	      SEEIT_fwrite_double(&Current_GE->x2[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->y2[i], outfile);
	      SEEIT_fwrite_double(&Current_GE->z2[i], outfile);
	    }
	  
	  if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	    {
	      fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\",\"%s\"\n",
		      keyval,
		      Current_GE->Cnumber + 1,
		      Current_GE->magnitude,
		      GetECCLabel(Current_GE->ECC1),
		      ParseGAITgeometry(Current_GE->gform1,2),
		      Current_GE->SID1,
		      GetECCLabel(Current_GE->ECC2),
		      ParseGAITgeometry(Current_GE->gform2,2),
		      Current_GE->SID2,
		      ParseErrType(keyval));
	    }
        }
      else
        {
	  *Duplicate = 1;
	  Last_GE = Current_GE;
        }
    }
  
  
  returnval = Current_GE->magnitude;
  
  return(returnval);
}








void FreeGenericErrorAllocations(void)
{
  
  if(GenericErr.msglen > 0)
    free(GenericErr.errmsg);
  GenericErr.msglen = 0;
  GenericErr.errmsg = NULL;
  
  if(GenericErr.SIDlen1 > 0)
    free(GenericErr.SID1);
  GenericErr.SIDlen1 = 0;
  GenericErr.SID1 = NULL;
  
  if(GenericErr.SIDlen2 > 0)
    free(GenericErr.SID2);
  GenericErr.SIDlen2 = 0;
  GenericErr.SID2 = NULL;
  
  if(GenericErr.numverts1 > 0)
    {
      free(GenericErr.x1);
      free(GenericErr.y1);
      free(GenericErr.z1);
    }
  GenericErr.x1 = NULL;
  GenericErr.y1 = NULL;
  GenericErr.z1 = NULL;
  GenericErr.numverts1 = 0;
  
  if(GenericErr.numverts2 > 0)
    {
      free(GenericErr.x2);
      free(GenericErr.y2);
      free(GenericErr.z2);
    }
  GenericErr.x2 = NULL;
  GenericErr.y2 = NULL;
  GenericErr.z2 = NULL;
  GenericErr.numverts2 = 0;
}



char * ParseErrName(int errname)
{
  if(errname==LELINEPROX)    {return "LELINEPROX";}
  else if(errname==EN_EN_PROX)    {return "EN_EN_PROX";}
  else if(errname==LUNDERSHTL)    {return "LUNDERSHTL";}
  else if(errname==LUSHTL_CLEAN)  {return "LUSHTL_CLEAN";}
  else if(errname==LVUSHTL)       {return "LVUSHTL";}
  else if(errname==VUSHTL_CLEAN)  {return "VUSHTL_CLEAN";}
  else if(errname==LVOSHTL)       {return "LVOSHTL";}
  else if(errname==LOVERSHTL)     {return "LOVERSHTL";}
  else if(errname==LUNDERSHTA)    {return "LUNDERSHTA";}
  else if(errname==LOVERSHTA)     {return "LOVERSHTA";}
  else if(errname==LUSHTL_DF)     {return "LUSHTL_DF";}
  else if(errname==LOSHTL_DF)     {return "LOSHTL_DF";}
  else if(errname==LAPROX)        {return "LAPROX";}
  else if(errname==LASLIVER)      {return "LASLIVER";}
  else if(errname==LSLICEA)       {return "LSLICEA";}
  else if(errname==LLSLIVER)      {return "LLSLIVER";}
  else if(errname==AUNDERSHTA)    {return "AUNDERSHTA";}
  else if(errname==AOVERSHTA)     {return "AOVERSHTA";}
  else if(errname==L_UNM_A)       {return "L_UNM_A";}
  else if(errname==LSAME_UNM_A)   {return "LSAME_UNM_A";}
  else if(errname==LUNM_ACRS_A)   {return "LUNM_ACRS_A";}
  else if(errname==LUNMA_ACRS_A)  {return "LUNMA_ACRS_A";}
  else if(errname==LRNGE_UNM_LAT) {return "LRNGE_UNM_LAT";}
  else if(errname==LRNGE_UNM_LON) {return "LRNGE_UNM_LON";}
  else if(errname==LE_A_UNM_LAT) {return "LE_A_UNM_LAT";}
  else if(errname==LE_A_UNM_LON) {return "LE_A_UNM_LON";}
  else if(errname==ARNGE_UNM_LAT) {return "ARNGE_UNM_LAT";}
  else if(errname==ARNGE_UNM_LON) {return "ARNGE_UNM_LON";}
  else if(errname==LHANG_LON)     {return "LHANG_LON";}
  else if(errname==LHANG_LAT)     {return "LHANG_LAT";}
  else if(errname==AHANG_LON)     {return "AHANG_LON";}
  else if(errname==AHANG_LAT)     {return "AHANG_LAT";}
  else if(errname==AUNM_ACRS_A)   {return "AUNM_ACRS_A";}
  else if(errname==LGEOM_UNM_LAT) {return "LGEOM_UNM_LAT";}
  else if(errname==LGEOM_UNM_LON) {return "LGEOM_UNM_LON";}
  else if(errname==AGEOM_UNM_LAT) {return "AGEOM_UNM_LAT";}
  else if(errname==AGEOM_UNM_LON) {return "AGEOM_UNM_LON";}
  else if(errname==AUNM_ATTR_A)   {return "AUNM_ATTR_A";}
  else if(errname==LUNM_ATTR_A)   {return "LUNM_ATTR_A";}
  else if(errname==LLMULTINT)     {return "LLMULTINT";}
  else if(errname==LOC_MULTINT)   {return "LOC_MULTINT";}
  else if(errname==L2D_L3D_MATCH) {return "L2D_L3D_MATCH";}
  else if(errname==LEZ_PROX_3D)   {return "LEZ_PROX_3D";}
  else if(errname==CNODE_ZBUST)   {return "CNODE_ZBUST";}
  else if(errname==LVPROX)        {return "LVPROX";}    
  else if(errname==PLPROX)        {return "PLPROX";}    
  else if(errname==PSHOOTL)       {return "PSHOOTL";}
  else if(errname==ENCONNECT)     {return "ENCONNECT";}
  else if(errname==BADENCON)      {return "BADENCON";}
  else if(errname==ENCONFAIL)     {return "ENCONFAIL";}
  else if(errname==NOENDCON)      {return "NOENDCON";}
  else if(errname==BOTHENDCON)    {return "BOTHENDCON";}
  else if(errname==LENOCOVERL)    {return "LENOCOVERL";}
  else if(errname==NOLCOVLE)      {return "NOLCOVLE";}
  else if(errname==FEATNOTCUT)    {return "FEATNOTCUT";}
  else if(errname==EXTRA_NET)     {return "EXTRA_NET";}
  else if(errname==INTRA_NET)     {return "INTRA_NET";}
  else if(errname==CREATENET)     {return "CREATENET";}
  else if(errname==FAILMERGEL)    {return "FAILMERGEL";}
  else if(errname==FAILMERGEL2)   {return "FAILMERGEL2";}
  else if(errname==FAILMERGEA)    {return "FAILMERGEA";}
  else if(errname==FAILMERGEA2)   {return "FAILMERGEA2";}
  else if(errname==PLLPROXFAIL)   {return "PLLPROXFAIL";}
  else if(errname==PTPTPROX)      {return "PTPTPROX";}
  else if(errname==PLPROXEX)      {return "PLPROXEX";}
  else if(errname==PUNDERSHTA)    {return "PUNDERSHTA";}
  else if(errname==POVERSHTA)     {return "POVERSHTA";}
  else if(errname==PLPFAIL)       {return "PLPFAIL";}   
  else if(errname==ATTRERR)       {return "ATTRERR";}   
  else if(errname==VVTERR1WAY)    {return "VVTERR1WAY";}
  else if(errname==VVTERR2WAY)    {return "VVTERR2WAY";}
  else if(errname==VVTERR3WAY)    {return "VVTERR3WAY";}
  else if(errname==HIGHLIGHTED)   {return "HIGHLIGHTED";}
  else if(errname==RPTD_ATTR)     {return "RPTD_ATTR";}
  else if(errname==ATTR_PAIR)     {return "ATTR_PAIR";}
  else if(errname==ATTR_UNEXP)    {return "ATTR_UNEXP";}
  else if(errname==ATTR_MISSING)  {return "ATTR_MISSING";}
  else if(errname==ATTR_DT)       {return "ATTR_DT";}
  else if(errname==ATTR_RNG)      {return "ATTR_RNG";}
  else if(errname==ATTR_PICK)     {return "ATTR_PICK";}
  else if(errname==ATTR_META)     {return "ATTR_META";}
  else if(errname==ATTR_VVT)      {return "ATTR_VVT";}
  else if(errname==ATTR_RNULL)    {return "ATTR_RNULL";}
  else if(errname==OVERC)         {return "OVERC";}     
  else if(errname==ISOTURN)       {return "ISOTURN";}
  else if(errname==KINK)          {return "KINK";}   
  else if(errname==Z_KINK)        {return "Z_KINK";}
  else if(errname==INTERNALKINK)  {return "INTERNALKINK";}
  else if(errname==CONTEXT_KINK)  {return "CONTEXT_KINK";}
  else if(errname==AREAKINK)      {return "AREAKINK";}
  else if(errname==L_A_KINK)      {return "L_A_KINK";}
  else if(errname==KICKBACK)      {return "KICKBACK";}
  else if(errname==INCLSLIVER)    {return "INCLSLIVER";}
  else if(errname==SEGLEN)        {return "SEGLEN";}
  else if(errname==LONGSEG)       {return "LONGSEG";}
  else if(errname==FEATBRIDGE)    {return "FEATBRIDGE";}
  else if(errname==ZUNCLOSED)     {return "ZUNCLOSED";}
  else if(errname==AREAUNCLOSED)  {return "AREAUNCLOSED";}
  else if(errname==SMALLAREA)     {return "SMALLAREA";}
  else if(errname==SMLCUTOUT)     {return "SMLCUTOUT";}
  else if(errname==BIGAREA)       {return "BIGAREA";}
  else if(errname==NOT_FLAT)      {return "NOT_FLAT";}
  else if(errname==CLAMP_NFLAT)   {return "CLAMP_NFLAT";}
  else if(errname==CLAMP_DIF)     {return "CLAMP_DIF";}
  else if(errname==PERIMLEN)      {return "PERIMLEN";}
  else if(errname==SHORTFEAT)     {return "SHORTFEAT";}
  else if(errname==PC_SLOPE)      {return "PC_SLOPE";}
  else if(errname==LONGFEAT)      {return "LONGFEAT";}
  else if(errname==CALC_AREA)     {return "CALC_AREA";}
  else if(errname==COVERFAIL)     {return "COVERFAIL";}
  else if(errname==LAINT)         {return "LAINT";}      
  else if(errname==LACUTFAIL)     {return "LACUTFAIL";}
  else if(errname==ISOLATEDA)     {return "ISOLATEDA";}
  else if(errname==NETISOA)       {return "NETISOA"; }
  else if(errname==ANETISOA)      {return "ANETISOA"; }
  else if(errname==NETISOFEAT)    {return "NETISOFEAT"; }
  else if(errname==LODELEVDIF)    {return "LODELEVDIF";}
  else if(errname==GRIDEXACTDIF)  {return "GRIDEXACTDIF";}
  else if(errname==MASKZERO)      {return "MASKZERO";}
  else if(errname==MASKCONSTANT)  {return "MASKCONSTANT";}
  else if(errname==MASKEDIT_0)    {return "MASKEDIT_0";}
  else if(errname==MASKEDIT_1)    {return "MASKEDIT_1";}
  else if(errname==KERNELSTATS)   {return "KERNELSTATS";}
  else if(errname==BILINSTATS)    {return "BILINSTATS";}
  else if(errname==MASKSHOREL)    {return "MASKSHOREL";}
  else if(errname==MASKCONFLICT)  {return "MASKCONFLICT";}
  else if(errname==MASKCONF2)     {return "MASKCONF2";}
  else if(errname==MASKMONO)      {return "MASKMONO";}
  else if(errname==BREAKLINE)     {return "BREAKLINE";}
  else if(errname==CLAMP_SEG)     {return "CLAMP_SEG";}
  else if(errname==AREAINTAREA)   {return "AREAINTAREA";}
  else if(errname==PART_ISF)      {return "PART_ISF";}
  else if(errname==CUT_INT)       {return "CUT_INT";}
  else if(errname==ACOVERA)       {return "ACOVERA";}
  else if(errname==AINSIDEHOLE)   {return "AINSIDEHOLE";}
  else if(errname==AOVERLAPA)     {return "AOVERLAPA";}
  else if(errname==LLINT)         {return "LLINT";}     
  else if(errname==BADFEATCUT)    {return "BADFEATCUT";}
  else if(errname==LLNONODEINT)   {return "LLNONODEINT";}
  else if(errname==NONODEOVLP)    {return "NONODEOVLP";}
  else if(errname==LLNOENDINT)    {return "LLNOENDINT";}
  else if(errname==LLINTAWAY)     {return "LLINTAWAY";}
  else if(errname==LLINTNOEND)    {return "LLINTNOEND";}
  else if(errname==LLNOINT)       {return "LLNOINT";}
  else if(errname==LFNOINT)       {return "LFNOINT";}
  else if(errname==LLIEX)         {return "LLIEX";}     
  else if(errname==LAIEX)         {return "LAIEX";}
  else if(errname==LOUTSIDEA)     {return "LOUTSIDEA";}  
  else if(errname==LLAINT)        {return "LLAINT";}
  else if(errname==L_NOTL_AINT)   {return "L_NOTL_AINT";}
  else if(errname==PTINREGION)    {return "PTINREGION";}
  else if(errname==PTINPROPER)    {return "PTINPROPER";}
  else if(errname==PTOSIDEREGION) {return "PTOSIDEREGION";}
  else if(errname==OBJECTWITHOUT) {return "OBJECTWITHOUT";}
  else if(errname==OBJ_WO_TWO)    {return "OBJ_WO_TWO";}
  else if(errname==AWITHOUTA)     {return "AWITHOUTA";}
  else if(errname==FSFAIL)        {return "FSFAIL";}
  else if(errname==PSHAREFAIL)    {return "PSHAREFAIL";}
  else if(errname==NOCOINCIDE)    {return "NOCOINCIDE";}
  else if(errname==LNOCOVERLA)    {return "LNOCOVERLA";}
  else if(errname==CONFLATE)      {return "CONFLATE";}
  else if(errname==CONF_STATS)      {return "CONF_STATS";}
  else if(errname==LSPANFAIL)     {return "LSPANFAIL";}
  else if(errname==LNOCOV2A)      {return "LNOCOV2A";}
  else if(errname==ISOLINE)       {return "ISOLINE";}
  else if(errname==LINSIDEA)      {return "LINSIDEA";}
  else if(errname==LSEGCOVERA)    {return "LSEGCOVERA";}
  else if(errname==LEINSIDEA)     {return "LEINSIDEA";}
  else if(errname==LEAON_NOTIN)   {return "LEAON_NOTIN";}
  else if(errname==MULTIDFEAT)    {return "MULTIDFEAT";}
  else if(errname==MULTISENTINEL) {return "MULTISENTINEL";}
  else if(errname==LENOCOVERP)    {return "LENOCOVERP";}
  else if(errname==LAINTNOEND)    {return "LAINTNOEND";}
  else if(errname==LENOCOVERA)    {return "LENOCOVERA";}
  else if(errname==PNOCOVERLE)    {return "PNOCOVERLE";}
  else if(errname==PNOCOV2LEA)    {return "PNOCOV2LEA";}
  else if(errname==PNOCOVERLV)    {return "PNOCOVERLV";}
  else if(errname==ANOCOVERLA)    {return "ANOCOVERLA";}
  else if(errname==QUALANOCOVLA)  {return "QUALANOCOVLA";}
  else if(errname==ANOCOVERA)     {return "ANOCOVERA";}
  else if(errname==OVERUNDER)     {return "OVERUNDER";}
  else if(errname== AMCOVAFAIL)   {return "AMCOVAFAIL";}
  else if(errname==CUTOUT)        {return "CUTOUT";}
  else if(errname==PORTRAYF)      {return "PORTRAYF";}
  else if(errname==TPORTRAYF)     {return "TPORTRAYF";}
  else if(errname==COLINEAR)      {return "COLINEAR";}
  else if(errname==GSPIKE)        {return "GSPIKE";}     
  else if(errname==AVGSPIKE)      {return "AVGSPIKE";}
  else if(errname==GSHELF)        {return "GSHELF";}
  else if(errname==LOSMINHGT)     {return "LOSMINHGT";}
  else if(errname==FLOWSTEP)      {return "FLOWSTEP";}
  else if(errname==WATERMMU)      {return "WATERMMU";}
  else if(errname==RAISEDPC)      {return "RAISEDPC";}
  else if(errname==PT_GRID_DIF)   {return "PT_GRID_DIF";}
  else if(errname==GRID_STD_DEV)  {return "GRID_STD_DEV";}
  else if(errname==ELEVGT)        {return "ELEVGT";}     
  else if(errname==ELEVLT)        {return "ELEVLT";}     
  else if(errname==CONNECTFAIL)   {return "CONNECTFAIL";}
  else if(errname==FEATOUTSIDE)   {return "FEATOUTSIDE";}
  else if(errname==BNDRYUNDERSHT) {return "BNDRYUNDERSHT";}
  else if(errname==LBNDUSHT)      {return "LBNDUSHT";}
  else if(errname==OSIDE_LAT)     {return "OSIDE_LAT";}
  else if(errname==OSIDE_LON)     {return "OSIDE_LON";}
  else if(errname==MULTIPARTL)    {return "MULTIPARTL";}
  else if(errname==MULTIPARTA)    {return "MULTIPARTA";}
  else if(errname==MULTIPARTP)    {return "MULTIPARTP";}
  else if(errname==ELEVEQ)        {return "ELEVEQ";}     
  else if(errname==ELEVEQOPEN)    {return "ELEVEQOPEN";}
  else if(errname==ELEVADJCHANGE) {return "ELEVADJCHANGE";}
  else if(errname==FEATSPIKE)     {return "FEATSPIKE";}
  else if(errname==LJOINSLOPEDC)  {return "LJOINSLOPEDC";}
  else if(errname==CLAMP_JOINSDC) {return "CLAMP_JOINSDC";}
  else if(errname==SLOPEDIRCH)    {return "SLOPEDIRCH";}
  else if(errname==CLAMP_SDC)     {return "CLAMP_SDC";}
  else if(errname==G_DUPS)        {return "G_DUPS";}   
  else if(errname==C_DUPS)        {return "C_DUPS";}    
  else if(errname==SAMEID)        {return "SAMEID";}
  else if(errname==SAMEID_CDUP)   {return "SAMEID_CDUP";}
  else if(errname==SAMEID_GDUP)   {return "SAMEID_GDUP";}
  else if(errname==ANY_SAMEID)    {return "ANY_SAMEID";}
  else if(errname==V_DUPS)        {return "V_DUPS";}    
  else if(errname==LOOPS)         {return "LOOPS";}
  else if(errname==P_O_LOOP)      {return "P_O_LOOP";}
  else if(errname==ENDPTINT)      {return "ENDPTINT";}
  else if(errname==LATTRCHNG)     {return "LATTRCHNG";}
  else if(errname==DUPLICATESEG)  {return "DUPLICATESEG";}
  else if(errname==SHARE3SEG)     {return "SHARE3SEG";}
  else if(errname==COINCIDEFAIL)  {return "COINCIDEFAIL";}
  else if(errname==SHARESEG)      {return "SHARESEG";}
  else if(errname==LLI_ANGLE)     {return "LLI_ANGLE";}
  else if(errname==SHAREPERIM)    {return "SHAREPERIM";}
  else if(errname==SLIVER)        {return "SLIVER";}
  else if(errname==FACESIZE)      {return "FACESIZE";}
  
  if(NGA_TYPE==1)
    {
      printf("ParseErrName got bad number %d\n",errname);
      exit(-1);
    }

  
  
  if(errname==NARROW)        {return "NARROW";}    
  else if(errname==SMALLOBJ)      {return "SMALLOBJ";}  
  else if(errname==HSLOPE)        {return "HSLOPE";}    
  else if(errname==VERTSLOPE)     {return "VERTSLOPE";} 
  else if(errname==VTEAR)         {return "VTEAR";}     
  else if(errname==HTEAR)         {return "HTEAR";}     
  else if(errname==TVERT)         {return "TVERT";}     
  else if(errname==LMINT)         {return "LMINT";}     
  else if(errname==LSPINT)        {return "LSPINT";}    
  else if(errname==LSPIEXP)       {return "LSPIEXP";}   
  else if(errname==POLYINAREA)    {return "POLYINAREA";}
  else if(errname==POLYOSIDEAREA) {return "POLYOSIDEAREA";}
  else if(errname==POLYINTPOLY)   {return "POLYINTPOLY";}
  else if(errname==POLYINTAREA)   {return "POLYINTAREA";}
  else
    {
      printf("ParseErrName got bad number %d\n",errname);
      exit(-1);
    } 
  
  return "junk";
}


int ParseErrNumber(char *errname,int line, char *exefile)
{
  char message[1000];
  extern int batch_mode;
  
  if(!strcmp(errname,"G_DUPS"))             {return G_DUPS;}
  else if(!strcmp(errname,"C_DUPS"))        {return C_DUPS;}
  else if(!strcmp(errname,"SAMEID"))        {return SAMEID;}
  else if(!strcmp(errname,"SAMEID_CDUP"))   {return SAMEID_CDUP;}
  else if(!strcmp(errname,"SAMEID_GDUP"))   {return SAMEID_GDUP;}
  else if(!strcmp(errname,"ANY_SAMEID"))    {return ANY_SAMEID;}
  else if(!strcmp(errname,"V_DUPS"))        {return V_DUPS;}
  else if(!strcmp(errname,"ATTRERR"))       {return ATTRERR;}
  else if(!strcmp(errname,"VVTERR1WAY"))    {return VVTERR1WAY;}
  else if(!strcmp(errname,"VVTERR2WAY"))    {return VVTERR2WAY;}
  else if(!strcmp(errname,"VVTERR3WAY"))    {return VVTERR3WAY;}
  else if(!strcmp(errname,"HIGHLIGHTED"))   {return HIGHLIGHTED;}
  else if(!strcmp(errname,"RPTD_ATTR"))     {return RPTD_ATTR;}
  else if(!strcmp(errname,"ATTR_PAIR"))     {return ATTR_PAIR;}
  else if(!strcmp(errname,"ATTR_UNEXP"))    {return ATTR_UNEXP;}
  else if(!strcmp(errname,"ATTR_MISSING"))  {return ATTR_MISSING;}
  else if(!strcmp(errname,"ATTR_DT"))       {return ATTR_DT;}
  else if(!strcmp(errname,"ATTR_RNG"))      {return ATTR_RNG;}
  else if(!strcmp(errname,"ATTR_PICK"))     {return ATTR_PICK;}
  else if(!strcmp(errname,"ATTR_META"))     {return ATTR_META;}
  else if(!strcmp(errname,"ATTR_VVT"))      {return ATTR_VVT;}
  else if(!strcmp(errname,"ATTR_RNULL"))    {return ATTR_RNULL;}
  else if(!strcmp(errname,"OVERC"))         {return OVERC;}
  else if(!strcmp(errname,"LLINT"))         {return LLINT;}
  else if(!strcmp(errname,"BADFEATCUT"))    {return BADFEATCUT;}
  else if(!strcmp(errname,"LLNONODEINT"))   {return LLNONODEINT;}
  else if(!strcmp(errname,"NONODEOVLP"))    {return NONODEOVLP;}
  else if(!strcmp(errname,"LLNOENDINT"))    {return LLNOENDINT;}
  else if(!strcmp(errname,"LLINTAWAY"))     {return LLINTAWAY;}
  else if(!strcmp(errname,"LLINTNOEND"))    {return LLINTNOEND;}
  else if(!strcmp(errname,"LLNOINT"))       {return LLNOINT;}
  else if(!strcmp(errname,"LFNOINT"))       {return LFNOINT;}
  else if(!strcmp(errname,"LLIEX"))         {return LLIEX;}
  else if(!strcmp(errname,"LAIEX"))         {return LAIEX;}
  else if(!strcmp(errname,"LVPROX"))        {return LVPROX;}
  else if(!strcmp(errname,"LELINEPROX"))    {return LELINEPROX;}
  else if(!strcmp(errname,"EN_EN_PROX"))    {return EN_EN_PROX;}
  else if(!strcmp(errname,"LUNDERSHTL"))    {return LUNDERSHTL;}
  else if(!strcmp(errname,"LUSHTL_CLEAN"))  {return LUSHTL_CLEAN;}
  else if(!strcmp(errname,"LVUSHTL"))       {return LVUSHTL;}
  else if(!strcmp(errname,"VUSHTL_CLEAN"))  {return VUSHTL_CLEAN;}
  else if(!strcmp(errname,"LVOSHTL"))       {return LVOSHTL;}
  else if(!strcmp(errname,"LOVERSHTL"))     {return LOVERSHTL;}
  else if(!strcmp(errname,"LUNDERSHTA"))    {return LUNDERSHTA;}
  else if(!strcmp(errname,"LOVERSHTA"))     {return LOVERSHTA;}
  else if(!strcmp(errname,"LUSHTL_DF"))     {return LUSHTL_DF;}
  else if(!strcmp(errname,"LOSHTL_DF"))     {return LOSHTL_DF;}
  else if(!strcmp(errname,"LAPROX"))        {return LAPROX;}
  else if(!strcmp(errname,"LASLIVER"))      {return LASLIVER;}
  else if(!strcmp(errname,"LSLICEA"))       {return LSLICEA;}
  else if(!strcmp(errname,"LLSLIVER"))      {return LLSLIVER;}
  else if(!strcmp(errname,"AUNDERSHTA"))    {return AUNDERSHTA;}
  else if(!strcmp(errname,"AOVERSHTA"))     {return AOVERSHTA;}
  else if(!strcmp(errname,"L_UNM_A"))       {return L_UNM_A;}
  else if(!strcmp(errname,"LSAME_UNM_A"))   {return LSAME_UNM_A;}
  else if(!strcmp(errname,"LUNM_ACRS_A"))   {return LUNM_ACRS_A;}
  else if(!strcmp(errname,"LUNMA_ACRS_A"))  {return LUNMA_ACRS_A;}
  else if(!strcmp(errname,"LRNGE_UNM_LAT")) {return LRNGE_UNM_LAT;}
  else if(!strcmp(errname,"LRNGE_UNM_LON")) {return LRNGE_UNM_LON;}
  else if(!strcmp(errname,"LE_A_UNM_LAT")) {return LE_A_UNM_LAT;}
  else if(!strcmp(errname,"LE_A_UNM_LON")) {return LE_A_UNM_LON;}
  else if(!strcmp(errname,"ARNGE_UNM_LAT")) {return ARNGE_UNM_LAT;}
  else if(!strcmp(errname,"ARNGE_UNM_LON")) {return ARNGE_UNM_LON;}
  else if(!strcmp(errname,"LHANG_LON"))     {return LHANG_LON;}
  else if(!strcmp(errname,"LHANG_LAT"))     {return LHANG_LAT;}
  else if(!strcmp(errname,"AHANG_LON"))     {return AHANG_LON;}
  else if(!strcmp(errname,"AHANG_LAT"))     {return AHANG_LAT;}
  else if(!strcmp(errname,"AUNM_ACRS_A"))   {return AUNM_ACRS_A;}
  else if(!strcmp(errname,"LGEOM_UNM_LAT")) {return LGEOM_UNM_LAT;}
  else if(!strcmp(errname,"LGEOM_UNM_LON")) {return LGEOM_UNM_LON;}
  else if(!strcmp(errname,"AGEOM_UNM_LAT")) {return AGEOM_UNM_LAT;}
  else if(!strcmp(errname,"AUNM_ATTR_A"))   {return AUNM_ATTR_A;}
  else if(!strcmp(errname,"LUNM_ATTR_A"))   {return LUNM_ATTR_A;}
  else if(!strcmp(errname,"AGEOM_UNM_LON")) {return AGEOM_UNM_LON;}
  else if(!strcmp(errname,"LLMULTINT"))     {return LLMULTINT;}
  else if(!strcmp(errname,"LOC_MULTINT"))   {return LOC_MULTINT;}
  else if(!strcmp(errname,"L2D_L3D_MATCH")) {return L2D_L3D_MATCH;}
  else if(!strcmp(errname,"LEZ_PROX_3D"))   {return LEZ_PROX_3D;}
  else if(!strcmp(errname,"CNODE_ZBUST"))   {return CNODE_ZBUST;}
  else if(!strcmp(errname,"PLPROX"))        {return PLPROX;}
  else if(!strcmp(errname,"PSHOOTL"))       {return PSHOOTL;}
  else if(!strcmp(errname,"ENCONNECT"))     {return ENCONNECT;}
  else if(!strcmp(errname,"BADENCON"))      {return BADENCON;}
  else if(!strcmp(errname,"ENCONFAIL"))     {return ENCONFAIL;}
  else if(!strcmp(errname,"NOENDCON"))      {return NOENDCON;}
  else if(!strcmp(errname,"BOTHENDCON"))    {return BOTHENDCON;}
  else if(!strcmp(errname,"LENOCOVERL"))    {return LENOCOVERL;}
  else if(!strcmp(errname,"NOLCOVLE"))    {return NOLCOVLE;}
  else if(!strcmp(errname,"FEATNOTCUT"))    {return FEATNOTCUT;}
  else if(!strcmp(errname,"EXTRA_NET"))     {return EXTRA_NET;}
  else if(!strcmp(errname,"INTRA_NET"))     {return INTRA_NET;} 
  else if(!strcmp(errname,"CREATENET"))     {return CREATENET;}
  else if(!strcmp(errname,"FAILMERGEL"))    {return FAILMERGEL;}
  else if(!strcmp(errname,"FAILMERGEL2"))   {return FAILMERGEL2;}
  else if(!strcmp(errname,"FAILMERGEA"))    {return FAILMERGEA;}
  else if(!strcmp(errname,"FAILMERGEA2"))   {return FAILMERGEA2;}
  else if(!strcmp(errname,"PLLPROXFAIL"))   {return PLLPROXFAIL;}
  else if(!strcmp(errname,"PLPFAIL"))       {return PLPFAIL;}
  else if(!strcmp(errname,"ISOTURN"))       {return ISOTURN;}
  else if(!strcmp(errname,"KINK"))          {return KINK;}
  else if(!strcmp(errname,"Z_KINK"))        {return Z_KINK;}
  else if(!strcmp(errname,"INTERNALKINK"))  {return INTERNALKINK;}
  else if(!strcmp(errname,"CONTEXT_KINK"))  {return CONTEXT_KINK;}
  else if(!strcmp(errname,"AREAKINK"))      {return AREAKINK;}
  else if(!strcmp(errname,"L_A_KINK"))      {return L_A_KINK;}
  else if(!strcmp(errname,"KICKBACK"))      {return KICKBACK; }
  else if(!strcmp(errname,"INCLSLIVER"))    {return INCLSLIVER;}
  else if(!strcmp(errname,"LJOINSLOPEDC"))  {return LJOINSLOPEDC;}
  else if(!strcmp(errname,"CLAMP_JOINSDC")) {return CLAMP_JOINSDC;}
  else if(!strcmp(errname,"SLOPEDIRCH"))    {return SLOPEDIRCH;}
  else if(!strcmp(errname,"CLAMP_SDC"))     {return CLAMP_SDC;}
  else if(!strcmp(errname,"GSPIKE"))        {return GSPIKE;}
  else if(!strcmp(errname,"AVGSPIKE"))      {return AVGSPIKE;}
  else if(!strcmp(errname,"LOSMINHGT"))     {return LOSMINHGT;}
  else if(!strcmp(errname,"GSHELF"))        {return GSHELF;}
  else if(!strcmp(errname,"FLOWSTEP"))      {return FLOWSTEP;}
  else if(!strcmp(errname,"WATERMMU"))      {return WATERMMU;}
  else if(!strcmp(errname,"RAISEDPC"))      {return RAISEDPC;}
  else if(!strcmp(errname,"PT_GRID_DIF"))   {return PT_GRID_DIF;}
  else if(!strcmp(errname,"GRID_STD_DEV"))  {return GRID_STD_DEV;}
  else if(!strcmp(errname,"ELEVGT"))        {return ELEVGT;}
  else if(!strcmp(errname,"ELEVLT"))        {return ELEVLT;}
  else if(!strcmp(errname,"CONNECTFAIL"))   {return CONNECTFAIL;}
  else if(!strcmp(errname,"FEATOUTSIDE"))   {return FEATOUTSIDE;}
  else if(!strcmp(errname,"BNDRYUNDERSHT")) {return BNDRYUNDERSHT;}
  else if(!strcmp(errname,"LBNDUSHT"))      {return LBNDUSHT;}
  else if(!strcmp(errname,"OSIDE_LAT"))     {return OSIDE_LAT;}
  else if(!strcmp(errname,"OSIDE_LON"))     {return OSIDE_LON;}
  else if(!strcmp(errname,"MULTIPARTL"))    {return MULTIPARTL;}
  else if(!strcmp(errname,"MULTIPARTA"))    {return MULTIPARTA;}
  else if(!strcmp(errname,"MULTIPARTP"))    {return MULTIPARTP;}
  else if(!strcmp(errname,"ELEVEQ"))        {return ELEVEQ;}
  else if(!strcmp(errname,"ELEVEQOPEN"))    {return ELEVEQOPEN;}
  else if(!strcmp(errname,"LAINT"))         {return LAINT;}
  else if(!strcmp(errname,"LACUTFAIL"))     {return LACUTFAIL;}
  else if(!strcmp(errname,"LOUTSIDEA"))     {return LOUTSIDEA;}
  else if(!strcmp(errname,"LLAINT"))        {return LLAINT;}
  else if(!strcmp(errname,"ISOLATEDA"))     {return ISOLATEDA;}
  else if(!strcmp(errname,"NETISOA"))       {return NETISOA; }
  else if(!strcmp(errname,"ANETISOA"))       {return ANETISOA; }
  else if(!strcmp(errname,"NETISOFEAT"))    {return NETISOFEAT; }
  else if(!strcmp(errname,"L_NOTL_AINT"))   {return L_NOTL_AINT;}
  else if(!strcmp(errname,"PTINREGION"))    {return PTINREGION;}
  else if(!strcmp(errname,"PTINPROPER"))    {return PTINPROPER;}
  else if(!strcmp(errname,"PTOSIDEREGION")) {return PTOSIDEREGION;}
  else if(!strcmp(errname,"PTPTPROX"))      {return PTPTPROX;}
  else if(!strcmp(errname,"PLPROXEX"))      {return PLPROXEX;}
  else if(!strcmp(errname,"PUNDERSHTA"))    {return PUNDERSHTA;}
  else if(!strcmp(errname,"POVERSHTA"))     {return POVERSHTA;}
  else if(!strcmp(errname,"ELEVADJCHANGE")) {return ELEVADJCHANGE;}
  else if(!strcmp(errname,"FEATSPIKE"))     {return FEATSPIKE;}
  else if(!strcmp(errname,"LODELEVDIF"))    {return LODELEVDIF;}
  else if(!strcmp(errname,"GRIDEXACTDIF"))  {return GRIDEXACTDIF;}
  else if(!strcmp(errname,"MASKZERO"))      {return MASKZERO;}
  else if(!strcmp(errname,"MASKCONSTANT"))  {return MASKCONSTANT;}
  else if(!strcmp(errname,"MASKEDIT_0"))    {return MASKEDIT_0;}
  else if(!strcmp(errname,"MASKEDIT_1"))    {return MASKEDIT_1;}
  else if(!strcmp(errname,"KERNELSTATS"))   {return KERNELSTATS;}
  else if(!strcmp(errname,"BILINSTATS"))    {return BILINSTATS;}
  else if(!strcmp(errname,"MASKSHOREL"))    {return MASKSHOREL;}
  else if(!strcmp(errname,"MASKCONFLICT"))  {return MASKCONFLICT;}
  else if(!strcmp(errname,"MASKCONF2"))     {return MASKCONF2;}
  else if(!strcmp(errname,"MASKMONO"))      {return MASKMONO;}
  else if(!strcmp(errname,"BREAKLINE"))     {return BREAKLINE;}
  else if(!strcmp(errname,"CLAMP_SEG"))     {return CLAMP_SEG;}
  else if(!strcmp(errname,"OBJECTWITHOUT")) {return OBJECTWITHOUT;}
  else if(!strcmp(errname,"OBJ_WO_TWO"))    {return OBJ_WO_TWO;}
  else if(!strcmp(errname,"AWITHOUTA"))     {return AWITHOUTA;}
  else if(!strcmp(errname,"FSFAIL"))        {return FSFAIL;}
  else if(!strcmp(errname,"PSHAREFAIL"))    {return PSHAREFAIL;}
  else if(!strcmp(errname,"NOCOINCIDE"))    {return NOCOINCIDE;}
  else if(!strcmp(errname,"LNOCOVERLA"))    {return LNOCOVERLA;}
  else if(!strcmp(errname,"CONFLATE"))      {return CONFLATE;}
  else if(!strcmp(errname,"CONF_STATS"))    {return CONF_STATS;}
  else if(!strcmp(errname,"LSPANFAIL"))     {return LSPANFAIL;}
  else if(!strcmp(errname,"LNOCOV2A"))      {return LNOCOV2A;}
  else if(!strcmp(errname,"ISOLINE"))       {return ISOLINE;}
  else if(!strcmp(errname,"LINSIDEA"))      {return LINSIDEA;}
  else if(!strcmp(errname,"LSEGCOVERA"))    {return LSEGCOVERA;}
  else if(!strcmp(errname,"LEINSIDEA"))     {return LEINSIDEA;}
  else if(!strcmp(errname,"LEAON_NOTIN"))   {return LEAON_NOTIN;}
  else if(!strcmp(errname,"MULTIDFEAT"))    {return MULTIDFEAT;}
  else if(!strcmp(errname,"MULTISENTINEL")) {return MULTISENTINEL;}
  else if(!strcmp(errname,"LENOCOVERP"))    {return LENOCOVERP;}
  else if(!strcmp(errname,"LAINTNOEND"))    {return LAINTNOEND;}
  else if(!strcmp(errname,"LENOCOVERA"))    {return LENOCOVERA;}
  else if(!strcmp(errname,"PNOCOVERLE"))    {return PNOCOVERLE;}
  else if(!strcmp(errname,"PNOCOV2LEA"))    {return PNOCOV2LEA;}
  else if(!strcmp(errname,"PNOCOVERLV"))    {return PNOCOVERLV;}
  else if(!strcmp(errname,"ANOCOVERLA"))    {return ANOCOVERLA;}
  else if(!strcmp(errname,"QUALANOCOVLA"))  {return QUALANOCOVLA;}
  else if(!strcmp(errname,"ANOCOVERA"))     {return ANOCOVERA;}
  else if(!strcmp(errname,"OVERUNDER"))     {return OVERUNDER;}
  else if(!strcmp(errname,"AMCOVAFAIL"))    {return AMCOVAFAIL;}
  else if(!strcmp(errname,"CUTOUT"))        {return CUTOUT;}
  else if(!strcmp(errname,"PORTRAYF"))      {return PORTRAYF;}
  else if(!strcmp(errname,"TPORTRAYF"))     {return TPORTRAYF;}
  else if(!strcmp(errname,"COLINEAR"))      {return COLINEAR;}
  else if(!strcmp(errname,"AREAINTAREA"))   {return AREAINTAREA;}
  else if(!strcmp(errname,"PART_ISF"))      {return PART_ISF;}
  else if(!strcmp(errname,"CUT_INT"))       {return CUT_INT; }
  else if(!strcmp(errname,"ACOVERA"))       {return ACOVERA;}
  else if(!strcmp(errname,"AINSIDEHOLE"))   {return AINSIDEHOLE;}
  else if(!strcmp(errname,"AOVERLAPA"))     {return AOVERLAPA;}
  else if(!strcmp(errname,"ZUNCLOSED"))     {return ZUNCLOSED;}
  else if(!strcmp(errname,"AREAUNCLOSED"))  {return AREAUNCLOSED;}
  else if(!strcmp(errname,"SMALLAREA"))     {return SMALLAREA;}
  else if(!strcmp(errname,"SMLCUTOUT"))     {return SMLCUTOUT;}
  else if(!strcmp(errname,"BIGAREA"))       {return BIGAREA;}
  else if(!strcmp(errname,"NOT_FLAT"))      {return NOT_FLAT;}
  else if(!strcmp(errname,"CLAMP_NFLAT"))   {return CLAMP_NFLAT;}
  else if(!strcmp(errname,"CLAMP_DIF"))     {return CLAMP_DIF;}
  else if(!strcmp(errname,"SEGLEN"))        {return SEGLEN;}
  else if(!strcmp(errname,"LONGSEG"))       {return LONGSEG;}
  else if(!strcmp(errname,"FEATBRIDGE"))    {return FEATBRIDGE;}
  else if(!strcmp(errname,"PERIMLEN"))      {return PERIMLEN;}
  else if(!strcmp(errname,"SHORTFEAT"))     {return SHORTFEAT;}
  else if(!strcmp(errname,"PC_SLOPE"))      {return PC_SLOPE;}
  else if(!strcmp(errname,"LONGFEAT"))      {return LONGFEAT;}
  else if(!strcmp(errname,"CALC_AREA"))     {return CALC_AREA;}
  else if(!strcmp(errname,"COVERFAIL"))     {return COVERFAIL;}
  else if(!strcmp(errname,"LOOPS"))         {return LOOPS;}
  else if(!strcmp(errname,"P_O_LOOP"))      {return P_O_LOOP;}
  else if(!strcmp(errname,"ENDPTINT"))      {return ENDPTINT;}
  else if(!strcmp(errname,"LATTRCHNG"))     {return LATTRCHNG;}
  else if(!strcmp(errname,"DUPLICATESEG"))  {return DUPLICATESEG;}
  else if(!strcmp(errname,"SHARE3SEG"))     {return SHARE3SEG;}
  else if(!strcmp(errname,"COINCIDEFAIL"))  {return COINCIDEFAIL;}
  else if(!strcmp(errname,"SHARESEG"))      {return SHARESEG;}
  else if(!strcmp(errname,"LLI_ANGLE"))     {return LLI_ANGLE;}
  else if(!strcmp(errname,"SHAREPERIM"))    {return SHAREPERIM;}
  else if(!strcmp(errname,"SLIVER"))        {return SLIVER;}
  else if(!strcmp(errname,"FACESIZE"))      {return FACESIZE;}
  else if(!strncmp(errname,"GIFDGIFDGIFD",12)) {return -10;}  /* we are done with execution options */
  else if(!strncmp(errname,"ATTRATTRATTR",12)) {return -10;}  /* we are done with execution options */
  else if(!strncmp(errname,"EDCSEDCSEDCS",12)) {return -10;}  /* we are done with execution options */
  
  if(NGA_TYPE==1)
    {
      if(batch_mode==1)
	{
	  printf("Bad input in file %s, line %d\n",exefile,line);
	  printf("expected an error name, received: %s\n",errname);
	  exit(-1);
	}
      
      sprintf(message,"\
Bad input in file %s, line %d:\n\
Expected an error name, received: %s.\n\
Terminating load of this file at the location of this error.",exefile,line,errname);
      not_while_running(drawing_a,message,1366,"Bad Execution Options File",1);
      
      
      return -1;
    }
  
  
  if(!strcmp(errname,"NARROW"))        {return NARROW;}
  else if(!strcmp(errname,"SMALLOBJ"))      {return SMALLOBJ;}
  else if(!strcmp(errname,"HSLOPE"))        {return HSLOPE;}
  else if(!strcmp(errname,"VERTSLOPE"))     {return VERTSLOPE;}
  else if(!strcmp(errname,"VTEAR"))         {return VTEAR;}
  else if(!strcmp(errname,"HTEAR"))         {return HTEAR;}
  else if(!strcmp(errname,"TVERT"))         {return TVERT;}
  else if(!strcmp(errname,"LMINT"))         {return LMINT;}
  else if(!strcmp(errname,"LSPINT"))        {return LSPINT;}
  else if(!strcmp(errname,"LSPIEXP"))       {return LSPIEXP;}
  else if(!strcmp(errname,"POLYINAREA"))    {return POLYINAREA;}
  else if(!strcmp(errname,"POLYOSIDEAREA")) {return POLYOSIDEAREA;}
  else if(!strcmp(errname,"POLYINTPOLY"))   {return POLYINTPOLY;}
  else if(!strcmp(errname,"POLYINTAREA"))   {return POLYINTAREA;}
  
  else
    {
      if(batch_mode==1)
	{
	  printf("Bad input in file %s, line %d\n",exefile,line);
	  printf("expected an error name, received: %s\n",errname);
	  exit(-1);
	}
      else
	{
	  if(NGA_TYPE==0)
	    {
	      if(!strncmp(errname,"EDCSEDCSEDCS",12))
		{
		  return -10;  /* we are done with execution options */
		}
	    }
	  
	  sprintf(message,"\
Bad input in file %s, line %d:\n\
Expected an error name, received: %s.\n\
Terminating load of this file at the location of this error.",exefile,line,errname);
	  not_while_running(drawing_a,message,1367,"Bad Execution Options File",1);
	  
	  
	}
    } 
  return -1;
}


char * ParseErrType(int i)
{
  static char returnname[100];
  int j;
  
  if(i < CONDITION_ARRAY_SIZE)
    {
      strcpy(returnname,ErrorLookup[i].name);
      j = 0;
      while(returnname[j] != '\0')
	{
	  if(returnname[j] < 32)
            returnname[j] = ' ';
	  ++j;
	}
    }
  else
    strcpy(returnname,"Unknown Condition Type");
  
  return(returnname);
}





void DrawIndividualConditionObject(struct ConditionList *C)
{
  long int seekposn;
  int filenumber,namelength, fileopen;
  double stripseekposn;
  extern char * arealsin;
  char * ain;
  extern struct ArealAsRead ArealFromFile;
  extern int NeedAreaCalculation;
  FILE *fp;
  extern int ReadArealFromFile(FILE * arealfile, double decimalplaces);
  
  
  if(ErrorLookup[C->CONDITION_TYPE].draw_wholeareal  < 1)
    return;
  
  fileopen = 0;
  filenumber = -1;
  ain = NULL;
  if(C->num_areals >= 1)
    {
      seekposn = (long int) C->areal1.FileAndPosn;
      stripseekposn = C->areal1.FileAndPosn - (double) seekposn;
      if(stripseekposn < 0.11)
	{
	  fp = fopen(arealsin,"rb");
	  fileopen = 1;
	  filenumber = 0;
	}
      else if(stripseekposn < 0.21)
	{
	  namelength = strlen(arealsin) + 3;
	  ain = (char *) malloc(namelength);
	  if(ain == NULL)
            {
	      printf("available memory has been exhausted during specific condition review\n");
	      exit(-1);
            }
	  sprintf(ain,"%s1",arealsin);
	  fp = fopen(ain,"rb");
	  fileopen = 1;
	  filenumber = 1;
	}
      else if(stripseekposn < 0.31)
	{
	  namelength = strlen(arealsin) + 3;
	  ain = (char *) malloc(namelength);
	  if(ain == NULL)
            {
	      printf("available memory has been exhausted during specific condition review\n");
	      exit(-1);
            }
	  sprintf(ain,"%s2",arealsin);
	  fp = fopen(ain,"rb");
	  fileopen = 1;
	  filenumber = 2;
	}
      else if(stripseekposn < 0.41)
	{
	  namelength = strlen(arealsin) + 3;
	  ain = (char *) malloc(namelength);
	  if(ain == NULL)
            {
	      printf("available memory has been exhausted during specific condition review\n");
	      exit(-1);
            }
	  sprintf(ain,"%s3",arealsin);
	  fp = fopen(ain,"rb");
	  fileopen = 1;
	  filenumber = 3;
	}
      if(fileopen > 0)
	{
	  fseek(fp,seekposn,SEEK_SET);
	  NeedAreaCalculation = 0;
	  ReadArealFromFile(fp,12.0);
	  
	  MAPdrawpoly(ArealFromFile.numverts,ArealFromFile.x,ArealFromFile.y,
		      1, 0, 1,1,DRAW_NOW);
	}
    }
  
  if(C->num_areals > 1)
    {
      seekposn = (long int) C->areal2.FileAndPosn;
      stripseekposn = C->areal2.FileAndPosn - (double) seekposn;
      if(stripseekposn < 0.11)
	{
	  if(filenumber != 0)
            {
	      if(fileopen > 0)
		fclose(fp);
	      fp = fopen(arealsin,"rb");
	      fileopen = 1;
	      filenumber = 0;
            }
	}
      else if(stripseekposn < 0.21)
	{
	  if(filenumber != 1)
            {
	      if(fileopen > 0)
		fclose(fp);
	      namelength = strlen(arealsin) + 3;
	      ain = (char *) malloc(namelength);
	      if(ain == NULL)
		{
		  printf("available memory has been exhausted during specific condition review\n");
		  exit(-1);
		}
	      sprintf(ain,"%s1",arealsin);
	      fp = fopen(ain,"rb");
	      fileopen = 1;
	      filenumber = 1;
            }
	}
      else if(stripseekposn < 0.31)
	{
	  if(filenumber != 2)
            {
	      if(fileopen > 0)
		fclose(fp);
	      namelength = strlen(arealsin) + 3;
	      ain = (char *) malloc(namelength);
	      if(ain == NULL)
		{
		  printf("available memory has been exhausted during specific condition review\n");
		  exit(-1);
		}
	      sprintf(ain,"%s2",arealsin);
	      fp = fopen(ain,"rb");
	      fileopen = 1;
	      filenumber = 2;
            }
	}
      else if(stripseekposn < 0.41)
	{
	  if(filenumber != 3)
            {
	      if(fileopen > 0)
		fclose(fp);
	      namelength = strlen(arealsin) + 3;
	      ain = (char *) malloc(namelength);
	      if(ain == NULL)
		{
		  printf("available memory has been exhausted during specific condition review\n");
		  exit(-1);
		}
	      sprintf(ain,"%s3",arealsin);
	      fp = fopen(ain,"rb");
	      fileopen = 1;
	      filenumber = 3;
            }
	}
      
      if(fileopen > 0)
        {
          fseek(fp,seekposn,SEEK_SET);
          NeedAreaCalculation = 0;
          ReadArealFromFile(fp,12.0);
	  
          MAPdrawpoly(ArealFromFile.numverts,ArealFromFile.x,ArealFromFile.y,
                      1, 0, 1,1,DRAW_NOW);
        }
    }
  if(fileopen)
    {
      fclose(fp);
      
      if(ain != NULL)
	free(ain);
    }
}





int CheckNMDRlist(double PX, double PY, double PZ, int KV, int Cindex, double LID1, double LID2)
{
int answer = 0;

   if(NMDRroot == NULL)
      {
      NMDRroot = (struct NonMagnitudeDupRemoval *) (malloc(SzNMDR));
      if(NMDRroot == NULL)
         {
         printf("available system memory has been exhausted during condition duplicate removal - terminating now\n");
         exit(-1);
         }
      NMDRroot->KV = KV;
      NMDRroot->Cindex = Cindex;
      NMDRroot->LID1 = LID1;
      NMDRroot->LID2 = LID2;
      NMDRroot->px = PX;
      NMDRroot->py = PY;
      NMDRroot->pz = PZ;
      NMDRroot->next = NULL;
      }
   else if(NMDRroot->LID1 != LID1)
      { 
      NMDRroot->KV = KV;
      NMDRroot->Cindex = Cindex;
      NMDRroot->LID1 = LID1;
      NMDRroot->LID2 = LID2;
      NMDRroot->px = PX;
      NMDRroot->py = PY;
      NMDRroot->pz = PZ;
      }
   else
      {
      NMDRc = NMDRroot;
      while(NMDRc != NULL)
         {
         if(NMDRc->LID1 == LID1)
            {
            if((NMDRc->KV == KV) && (NMDRc->Cindex == Cindex) && (NMDRc->LID1 == LID1) && (NMDRc->LID2 == LID2) &&
               (NMDRc->px == PX) && (NMDRc->py == PY) && (NMDRc->pz == PZ))
               {
               answer = 1;
               break;
               }
            }
         else
            {
            break;
            }
         NMDRc = NMDRc->next;
         }
      if(NMDRc == NULL)
         {
         NMDRn = (struct NonMagnitudeDupRemoval *) (malloc(SzNMDR));
         if(NMDRn == NULL)
            {
            printf("available system memory has been exhausted during condition duplicate removal - terminating now\n");
            exit(-1);
            }
         NMDRn->KV = KV;
         NMDRn->Cindex = Cindex;
         NMDRn->LID1 = LID1;
         NMDRn->LID2 = LID2;
         NMDRn->px = PX;
         NMDRn->py = PY;
         NMDRn->pz = PZ;
         NMDRn->next = NMDRroot;
         NMDRroot = NMDRn;
         }
      else
         {
         NMDRc->KV = KV;
         NMDRc->Cindex = Cindex;
         NMDRc->LID1 = LID1;
         NMDRc->LID2 = LID2;
         NMDRc->px = PX;
         NMDRc->py = PY;
         NMDRc->pz = PZ;
         }
      }

   return(answer);
}






int FreadFwriteDynamicInfo(int keyval, int writeflag, FILE *infile, FILE *outfile,
     double LPX, double LPY, double LPZ, int LastKV, int LastCindex, double LastLID1, double LastLID2)
{
  int i;
  int PtUsed;
  int MagUsed;
  int NumObjects;
  int duplicatefound;
  int answer = 1;

  if(infile == NULL)
    return(0);
  if((writeflag > 0) && (outfile == NULL))
    return(0);

  GenericErr.keyval = keyval;

  if(writeflag > 0)
    {
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      SEEIT_fread_int(&PtUsed,infile);
      SEEIT_fread_int(&MagUsed,infile);
      SEEIT_fread_int(&NumObjects,infile);


      SEEIT_fread_int(&GenericErr.msglen,infile);
      if(GenericErr.msglen > 0)
        {
        GenericErr.errmsg = (char *) (malloc(GenericErr.msglen + 2));
        fread(&GenericErr.errmsg[0],1,GenericErr.msglen,infile);
        GenericErr.errmsg[GenericErr.msglen] = '\0';
        }
      GenericErr.magnitude = 0.0;
      if(MagUsed > 0)
         SEEIT_fread_double(&GenericErr.magnitude,infile);
      if(PtUsed > 0)
         {
         SEEIT_fread_double(&GenericErr.px, infile);
         SEEIT_fread_double(&GenericErr.py, infile);
         SEEIT_fread_double(&GenericErr.pz, infile);
         }

      if(NumObjects > 0)
         {
         SEEIT_fread_int(&GenericErr.idn1,infile);

         SEEIT_fread_int(&GenericErr.SIDlen1,infile);

         GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
         if(GenericErr.SID1 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
         GenericErr.SID1[GenericErr.SIDlen1] = '\0';

         SEEIT_fread_int(&GenericErr.ECC1,infile);

         fread(&GenericErr.gform1,1,1,infile);

         SEEIT_fread_int(&GenericErr.Lindex1,infile);

         SEEIT_fread_double(&GenericErr.LocalID1,infile);

         SEEIT_fread_double(&GenericErr.radius1,infile);

         SEEIT_fread_double(&GenericErr.height1,infile);

         SEEIT_fread_int(&GenericErr.numverts1, infile);
         GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
         GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
         GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
         if(GenericErr.z1 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         for (i=0; i<GenericErr.numverts1; i++)
            {
            SEEIT_fread_double(&GenericErr.x1[i], infile);
            SEEIT_fread_double(&GenericErr.y1[i], infile);
            SEEIT_fread_double(&GenericErr.z1[i], infile);
            }
         }

      /** second object ***/

      if(NumObjects > 1)
         {
         SEEIT_fread_int(&GenericErr.idn2,infile);

         SEEIT_fread_int(&GenericErr.SIDlen2,infile);

         GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
         if(GenericErr.SID2 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
         GenericErr.SID2[GenericErr.SIDlen2] = '\0';

         SEEIT_fread_int(&GenericErr.ECC2,infile);

         fread(&GenericErr.gform2,1,1,infile);
         SEEIT_fread_int(&GenericErr.Lindex2,infile);

         SEEIT_fread_double(&GenericErr.LocalID2,infile);

         SEEIT_fread_double(&GenericErr.radius2,infile);

         SEEIT_fread_double(&GenericErr.height2,infile);

         SEEIT_fread_int(&GenericErr.numverts2, infile);
         GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
         GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
         GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
         if(GenericErr.z2 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         for (i=0; i<GenericErr.numverts2; i++)
            {
            SEEIT_fread_double(&GenericErr.x2[i], infile);
            SEEIT_fread_double(&GenericErr.y2[i], infile);
            SEEIT_fread_double(&GenericErr.z2[i], infile);
            }
         }


      duplicatefound = 0;
      if(PtUsed == 0)
         {
         if(NumObjects > 1)
            {
            if(CheckNMDRlist(-1.0,-1.0,-1.0,keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
               {
               duplicatefound = 1;
               }
            }
         else
            {
            if(CheckNMDRlist(-1.0,-1.0,-1.0,keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID1) > 0)
               {
               duplicatefound = 1;
               }
            }
         }
      else
         {
         if(NumObjects > 1)
            {
            if(CheckNMDRlist(GenericErr.px,GenericErr.py,GenericErr.pz,
                    keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
               {
               duplicatefound = 1;
               }
            }
         else
            {
            if(CheckNMDRlist(GenericErr.px,GenericErr.py,GenericErr.pz,
                    keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
               {
               duplicatefound = 1;
               }
            }
         }
          
      if(duplicatefound == 0)
         {
         SEEIT_fwrite_int(&keyval,outfile);

         SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
 SEEIT_fwrite_int(&PtUsed,outfile);
 SEEIT_fwrite_int(&MagUsed,outfile);
 SEEIT_fwrite_int(&NumObjects,outfile);
         SEEIT_fwrite_int(&GenericErr.msglen,outfile);
         if(GenericErr.msglen > 0)
           {
           fwrite(&GenericErr.errmsg[0],1,GenericErr.msglen,outfile);
           }
         if(MagUsed > 0)
            SEEIT_fwrite_double(&GenericErr.magnitude,outfile);

         if(PtUsed > 0)
            {
            SEEIT_fwrite_double(&GenericErr.px, outfile);
            SEEIT_fwrite_double(&GenericErr.py, outfile);
            SEEIT_fwrite_double(&GenericErr.pz, outfile);
            }

         if(NumObjects > 0)
            {
            SEEIT_fwrite_int(&GenericErr.idn1,outfile);

            SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);

            GenericErr.SID1[GenericErr.SIDlen1] = '\0';
            fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);

            SEEIT_fwrite_int(&GenericErr.ECC1,outfile);

            fwrite(&GenericErr.gform1,1,1,outfile);

            SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);

            SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);

            SEEIT_fwrite_double(&GenericErr.radius1,outfile);
            SEEIT_fwrite_double(&GenericErr.height1,outfile);

            SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
            for (i=0; i<GenericErr.numverts1; i++)
               {
               SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
               SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
               SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
               }
            }

      /** second object ***/

         if(NumObjects > 1)
            {
            SEEIT_fwrite_int(&GenericErr.idn2,outfile);

            SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);

            GenericErr.SID2[GenericErr.SIDlen2] = '\0';
            fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);

            SEEIT_fwrite_int(&GenericErr.ECC2,outfile);

            GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

            fwrite(&GenericErr.gform2,1,1,outfile);

            SEEIT_fwrite_int(&GenericErr.Lindex2,outfile);

            SEEIT_fwrite_double(&GenericErr.LocalID2,outfile);

            SEEIT_fwrite_double(&GenericErr.radius2,outfile);

            SEEIT_fwrite_double(&GenericErr.height2,outfile);

            SEEIT_fwrite_int(&GenericErr.numverts2, outfile);
            for (i=0; i<GenericErr.numverts2; i++)
               {
               SEEIT_fwrite_double(&GenericErr.x2[i], outfile);
               SEEIT_fwrite_double(&GenericErr.y2[i], outfile);
               SEEIT_fwrite_double(&GenericErr.z2[i], outfile);
               }
            }
         }
       else
         answer = 0;
    }
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      GenericErr.magnitude = 0;

      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      SEEIT_fread_int(&PtUsed,infile);
      SEEIT_fread_int(&MagUsed,infile);
      SEEIT_fread_int(&NumObjects,infile);
      SEEIT_fread_int(&GenericErr.msglen,infile);
      if(GenericErr.msglen > 0)
        {
        GenericErr.errmsg = (char *) (malloc(GenericErr.msglen + 2));
        fread(&GenericErr.errmsg[0],1,GenericErr.msglen,infile);
        GenericErr.errmsg[GenericErr.msglen] = '\0';
        }
      if(MagUsed > 0)
         SEEIT_fread_double(&GenericErr.magnitude,infile);

      if(PtUsed > 0)
         {
         SEEIT_fread_double(&GenericErr.px, infile);
         SEEIT_fread_double(&GenericErr.py, infile);
         SEEIT_fread_double(&GenericErr.pz, infile);
         }

      if(NumObjects > 0)
         {
         SEEIT_fread_int(&GenericErr.idn1,infile);

         SEEIT_fread_int(&GenericErr.SIDlen1,infile);

         GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
         if(GenericErr.SID1 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
         GenericErr.SID1[GenericErr.SIDlen1] = '\0';

         SEEIT_fread_int(&GenericErr.ECC1,infile);

         if(MagUsed == 0)
            GenericErr.magnitude = ((double) GenericErr.ECC1) / (double) (INscc_loop + 2);

         fread(&GenericErr.gform1,1,1,infile);

         SEEIT_fread_int(&GenericErr.Lindex1,infile);

         SEEIT_fread_double(&GenericErr.LocalID1,infile);

         SEEIT_fread_double(&GenericErr.radius1,infile);

         SEEIT_fread_double(&GenericErr.height1,infile);

         SEEIT_fread_int(&GenericErr.numverts1, infile);
         GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
         GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
         GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
         if(GenericErr.z1 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         for (i=0; i<GenericErr.numverts1; i++)
            {
            SEEIT_fread_double(&GenericErr.x1[i], infile);
            SEEIT_fread_double(&GenericErr.y1[i], infile);
            SEEIT_fread_double(&GenericErr.z1[i], infile);
            }
         }

      /** second object ***/
      if(NumObjects > 1)
         {
         SEEIT_fread_int(&GenericErr.idn2,infile);

         SEEIT_fread_int(&GenericErr.SIDlen2,infile);

         GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
         if(GenericErr.SID2 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
         GenericErr.SID2[GenericErr.SIDlen2] = '\0';

         SEEIT_fread_int(&GenericErr.ECC2,infile);
         if(MagUsed == 0)
            GenericErr.magnitude = ((double) GenericErr.ECC1 +  (double) GenericErr.ECC2) / (double) (INscc_loop + 2);

         fread(&GenericErr.gform2,1,1,infile);

         SEEIT_fread_int(&GenericErr.Lindex2,infile);

         SEEIT_fread_double(&GenericErr.LocalID2,infile);

         SEEIT_fread_double(&GenericErr.radius2,infile);

         SEEIT_fread_double(&GenericErr.height2,infile);

         SEEIT_fread_int(&GenericErr.numverts2, infile);
         GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
         GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
         GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
         if(GenericErr.z2 == NULL)
            {
            printf("System allocation memory has been exhausted during SEE-IT condition read\n");
            printf("   execution cannot continue\n");
            printf("error keyvalue is %d\n",keyval);
            exit(-1);
            }
         for (i=0; i<GenericErr.numverts2; i++)
            {
            SEEIT_fread_double(&GenericErr.x2[i], infile);
            SEEIT_fread_double(&GenericErr.y2[i], infile);
            SEEIT_fread_double(&GenericErr.z2[i], infile);
            }
         }

    }

  return(answer);
}







int FreadFwriteObject(int keyval, int writeflag, FILE *infile, FILE *outfile, int LastKV, int LastCindex, double LastLID)
{
  int i;
  int duplicatefound;
  int answer = 1;
  
  if(infile == NULL)
    return(0);
  if((writeflag > 0) && (outfile == NULL))
    return(0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fread_int(&GenericErr.Cnumber,infile);

      SEEIT_fread_int(&GenericErr.idn1,infile);

      SEEIT_fread_int(&GenericErr.SIDlen1,infile);

      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1 + 1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';

      SEEIT_fread_int(&GenericErr.ECC1,infile);

      fread(&GenericErr.gform1,1,1,infile);

      fread(&GenericErr.keepit,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex1,infile);

      GenericErr.magnitude = (double) GenericErr.ECC1;

      SEEIT_fread_double(&GenericErr.LocalID1,infile);

      SEEIT_fread_double(&GenericErr.radius1,infile);

      SEEIT_fread_double(&GenericErr.height1,infile);

      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
        }

      duplicatefound = 0;
      if(keyval == LNOCOV2A)
         {
         if(CheckNMDRlist(0,0,0,keyval,GenericErr.Cnumber,GenericErr.LocalID1,0) > 0)
            {
            duplicatefound = 1;
            }
         }
      else if(GenericErr.numverts1 > 1)
         {
         if(CheckNMDRlist(GenericErr.x1[0],GenericErr.y1[0],GenericErr.x1[1],keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.y1[1]) > 0)
            {
            duplicatefound = 1;
            }
         }
      else
         {
         if(CheckNMDRlist(GenericErr.x1[0],GenericErr.y1[0],GenericErr.z1[0],keyval,GenericErr.Cnumber,GenericErr.LocalID1,-1) > 0)
            {
            duplicatefound = 1;
            }
         }
      if(duplicatefound == 0)
         {
         SEEIT_fwrite_int(&keyval,outfile);
      
         SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
      
         SEEIT_fwrite_int(&GenericErr.idn1,outfile);
      
         SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);
      
         GenericErr.SID1[GenericErr.SIDlen1] = '\0';
         fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);
      
         SEEIT_fwrite_int(&GenericErr.ECC1,outfile);
      
         fwrite(&GenericErr.gform1,1,1,outfile);
      
         fwrite(&GenericErr.keepit,1,1,outfile);
      
         SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);

         GenericErr.magnitude = (double) GenericErr.ECC1;

         SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);
      
         SEEIT_fwrite_double(&GenericErr.radius1,outfile);
      
         SEEIT_fwrite_double(&GenericErr.height1,outfile);
      
         SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
         for (i=0; i<GenericErr.numverts1; i++)
	   {
	     SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
	     SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
	     SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
	   }
         if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	   {
	     fprintf(CDFout,"%d,%d,,\"%s\",\"%s\",\"%s\",,,,,,,\"%s\"\n",
		     keyval,
		     GenericErr.Cnumber + 1,
		     GetECCLabel(GenericErr.ECC1),
		     ParseGAITgeometry(GenericErr.gform1,2),
		     GenericErr.SID1,
		     ParseErrType(keyval));
	   }
        }
    else
        answer = 0;
    }
  
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1 + 1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      
      fread(&GenericErr.gform1,1,1,infile);
      
      fread(&GenericErr.keepit,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);

      GenericErr.magnitude = (double) GenericErr.ECC1;
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
    }
  return(answer);
}



int FreadFwriteTwoObjects(int keyval, int writeflag, FILE *infile, FILE *outfile, int LastKV, int LastCindex, double LastLID1, double LastLID2)
{
  int i;
  int duplicatefound;
  int answer = 1;
  
  if(infile == NULL)
    return(0);
  if((writeflag > 0) && (outfile == NULL))
    return(0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fread_int(&GenericErr.Cnumber,infile);

      SEEIT_fread_int(&GenericErr.idn1,infile);

      SEEIT_fread_int(&GenericErr.SIDlen1,infile);

      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';

      SEEIT_fread_int(&GenericErr.ECC1,infile);

      fread(&GenericErr.gform1,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex1,infile);

      SEEIT_fread_double(&GenericErr.LocalID1,infile);

      SEEIT_fread_double(&GenericErr.radius1,infile);

      SEEIT_fread_double(&GenericErr.height1,infile);

      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
        }

      /** second object ***/
      SEEIT_fread_int(&GenericErr.idn2,infile);

      SEEIT_fread_int(&GenericErr.SIDlen2,infile);

      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';

      SEEIT_fread_int(&GenericErr.ECC2,infile);

      GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

      fread(&GenericErr.gform2,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex2,infile);

      SEEIT_fread_double(&GenericErr.LocalID2,infile);

      SEEIT_fread_double(&GenericErr.radius2,infile);

      SEEIT_fread_double(&GenericErr.height2,infile);

      SEEIT_fread_int(&GenericErr.numverts2, infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts2; i++)
        {
          SEEIT_fread_double(&GenericErr.x2[i], infile);
          SEEIT_fread_double(&GenericErr.y2[i], infile);
          SEEIT_fread_double(&GenericErr.z2[i], infile);
        }

      duplicatefound = 0;
      if(keyval == FEATNOTCUT)
         {
         if(CheckNMDRlist(GenericErr.x2[0], GenericErr.y2[0],GenericErr.z2[0],
                  keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
            {
            duplicatefound = 1;
            }
         }
      else
         {
         if(CheckNMDRlist(-1.0,-1.0,-1.0,keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
            {
            duplicatefound = 1;
            }
         }
      if(duplicatefound == 0)
         {
         SEEIT_fwrite_int(&keyval,outfile);

         SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);

         SEEIT_fwrite_int(&GenericErr.idn1,outfile);

         SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);

         GenericErr.SID1[GenericErr.SIDlen1] = '\0';
         fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);

         SEEIT_fwrite_int(&GenericErr.ECC1,outfile);

         fwrite(&GenericErr.gform1,1,1,outfile);

         SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);

         SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);

         SEEIT_fwrite_double(&GenericErr.radius1,outfile);

         SEEIT_fwrite_double(&GenericErr.height1,outfile);

         SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
        
         for (i=0; i<GenericErr.numverts1; i++)
           {
             SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
             SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
             SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
           }

         /** second object ***/
         SEEIT_fwrite_int(&GenericErr.idn2,outfile);

         SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);

         GenericErr.SID2[GenericErr.SIDlen2] = '\0';
         fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);

         SEEIT_fwrite_int(&GenericErr.ECC2,outfile);

         fwrite(&GenericErr.gform2,1,1,outfile);
   
         SEEIT_fwrite_int(&GenericErr.Lindex2,outfile);

         SEEIT_fwrite_double(&GenericErr.LocalID2,outfile);

         GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

         SEEIT_fwrite_double(&GenericErr.radius2,outfile);

         SEEIT_fwrite_double(&GenericErr.height2,outfile);

         SEEIT_fwrite_int(&GenericErr.numverts2, outfile);
        
         for (i=0; i<GenericErr.numverts2; i++)
           {
             SEEIT_fwrite_double(&GenericErr.x2[i], outfile);
             SEEIT_fwrite_double(&GenericErr.y2[i], outfile);
             SEEIT_fwrite_double(&GenericErr.z2[i], outfile);
           }

         if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
           {
             fprintf(CDFout,"%d,%d,,\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\",\"%s\"\n",
                  keyval,
                  GenericErr.Cnumber + 1,
                  GetECCLabel(GenericErr.ECC1),
                  ParseGAITgeometry(GenericErr.gform1,2),
                  GenericErr.SID1,
                  GetECCLabel(GenericErr.ECC2),
                  ParseGAITgeometry(GenericErr.gform2,2),
                  GenericErr.SID2,
                  ParseErrType(keyval));
           }
         }
       else
         answer = 0;
    }

  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
      
      /** second object ***/
      SEEIT_fread_int(&GenericErr.idn2,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);

      GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);
      
      fread(&GenericErr.gform2,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      
      SEEIT_fread_double(&GenericErr.radius2,infile);
      
      SEEIT_fread_double(&GenericErr.height2,infile);
      
      SEEIT_fread_int(&GenericErr.numverts2, infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts2; i++)
	{
	  SEEIT_fread_double(&GenericErr.x2[i], infile);
	  SEEIT_fread_double(&GenericErr.y2[i], infile);
	  SEEIT_fread_double(&GenericErr.z2[i], infile);
	}
      
    }
  
  return(answer);
}


void FreadFwriteObjectAndMessage(int keyval, int writeflag, FILE *infile, FILE *outfile)
{
  int i;
  
  if(infile == NULL)
    return;
  if((writeflag > 0) && (outfile == NULL))
    return;
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fwrite_int(&keyval,outfile);
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      SEEIT_fwrite_int(&GenericErr.idn1,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   system could not satisfy memory request for %d bytes of memory (id str length\n",GenericErr.SIDlen1+1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      SEEIT_fwrite_int(&GenericErr.ECC1,outfile);

      GenericErr.magnitude = (double) GenericErr.ECC1;
      
      fread(&GenericErr.gform1,1,1,infile);
      fwrite(&GenericErr.gform1,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      SEEIT_fwrite_double(&GenericErr.radius1,outfile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      SEEIT_fwrite_double(&GenericErr.height1,outfile);
      
      SEEIT_fread_int(&GenericErr.msglen,infile);
      SEEIT_fwrite_int(&GenericErr.msglen,outfile);
      
      GenericErr.errmsg = (char *) (malloc(GenericErr.msglen));
      if(GenericErr.errmsg == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   system could not satisfy memory request for %d bytes of memory (message length)\n",GenericErr.msglen);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      
      fread(&GenericErr.errmsg[0],1,GenericErr.msglen,infile);
      fwrite(&GenericErr.errmsg[0],1,GenericErr.msglen,outfile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   system could not satisfy memory request for %d bytes of memory (vertices)\n",GenericErr.numverts1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	  
	  SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
	}
      
      if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	{
	  fprintf(CDFout,"%d,%d,,\"%s\",\"%s\",\"%s\",,,,,,,\"%s %s\"\n",
		  keyval,
		  GenericErr.Cnumber + 1,
		  GetECCLabel(GenericErr.ECC1),
		  ParseGAITgeometry(GenericErr.gform1,2),
		  GenericErr.SID1,
		  ParseErrType(keyval),
		  GenericErr.errmsg);
	}
      
    }
  
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   system could not satisfy memory request for %d bytes of memory (2: ID str length)\n",GenericErr.SIDlen1+1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);

      GenericErr.magnitude = (double) GenericErr.ECC1;
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      
      SEEIT_fread_int(&GenericErr.msglen,infile);
      GenericErr.errmsg = (char *) (malloc(GenericErr.msglen));
      if(GenericErr.errmsg == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   system could not satisfy memory request for %d bytes of memory (2: message length)\n",GenericErr.msglen);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      
      fread(&GenericErr.errmsg[0],1,GenericErr.msglen,infile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   system could not satisfy memory request for %d bytes of memory (2: vertices)\n",GenericErr.numverts1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
      
    }
  
  return;
}



double FreadFwriteObjectAndMagnitude(int keyval, int writeflag, FILE *infile, FILE *outfile)
{
  int i;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fwrite_int(&keyval,outfile);
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      SEEIT_fwrite_double(&GenericErr.magnitude,outfile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      SEEIT_fwrite_int(&GenericErr.idn1,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      SEEIT_fwrite_int(&GenericErr.ECC1,outfile);
      
      fread(&GenericErr.gform1,1,1,infile);
      fwrite(&GenericErr.gform1,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      SEEIT_fwrite_double(&GenericErr.radius1,outfile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      SEEIT_fwrite_double(&GenericErr.height1,outfile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	  
	  SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
	}
      

      if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	{
	  fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",,,,,,,\"%s\"\n",
		  keyval,
		  GenericErr.Cnumber + 1,
		  GenericErr.magnitude,
		  GetECCLabel(GenericErr.ECC1),
		  ParseGAITgeometry(GenericErr.gform1,2),
		  GenericErr.SID1,
		  ParseErrType(keyval));
	}
      
    }
  
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
      
    }
  
  return(GenericErr.magnitude);
}



double FreadFwritePointEdgeAndMagnitude(int keyval, int writeflag, FILE *infile, FILE *outfile)
{
  int i;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fwrite_int(&keyval,outfile);
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      SEEIT_fwrite_double(&GenericErr.magnitude,outfile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      SEEIT_fwrite_int(&GenericErr.idn1,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      SEEIT_fwrite_int(&GenericErr.ECC1,outfile);
      
      fread(&GenericErr.gform1,1,1,infile);
      fwrite(&GenericErr.gform1,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);
      
      GenericErr.numverts1 = 1;  /** no need for  a read here ***/
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
	  
	  SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
          SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
	}
      
      /** second object ***/
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      SEEIT_fwrite_int(&GenericErr.idn2,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);
      SEEIT_fwrite_int(&GenericErr.ECC2,outfile);
      
      fread(&GenericErr.gform2,1,1,infile);
      fwrite(&GenericErr.gform2,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex2,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID2,outfile);
      
      GenericErr.numverts2 = 2;  /** no need for read of this field, always have 2 vertices **/
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts2; i++)
	{
	  SEEIT_fread_double(&GenericErr.x2[i], infile);
	  SEEIT_fread_double(&GenericErr.y2[i], infile);
	  SEEIT_fread_double(&GenericErr.z2[i], infile);
	  
	  SEEIT_fwrite_double(&GenericErr.x2[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.y2[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.z2[i], outfile);
	}
      
      if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	{
	  fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\"\n",
		  keyval,
		  GenericErr.Cnumber + 1,
		  GenericErr.magnitude,
		  GetECCLabel(GenericErr.ECC1),
		  ParseGAITgeometry(GenericErr.gform1,2),
		  GenericErr.SID1,
		  GetECCLabel(GenericErr.ECC2),
		  ParseGAITgeometry(GenericErr.gform2,2),
		  GenericErr.SID2,
		  ParseErrType(keyval));
	}
      
    }
  
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      GenericErr.numverts1 = 1;  /** no need for a read here ***/
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
      
      /** second object ***/
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);
      
      fread(&GenericErr.gform2,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      
      GenericErr.numverts2 = 2;  /** no need for read of this field, always have 2 vertices **/
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts2; i++)
	{
	  SEEIT_fread_double(&GenericErr.x2[i], infile);
	  SEEIT_fread_double(&GenericErr.y2[i], infile);
	  SEEIT_fread_double(&GenericErr.z2[i], infile);
	}
    }
  
  return(GenericErr.magnitude);
}




double FreadFwritePointObjectAndMagnitude(int keyval, int writeflag, FILE *infile, FILE *outfile)
{
  int i;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fwrite_int(&keyval,outfile);
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      SEEIT_fwrite_double(&GenericErr.magnitude,outfile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      SEEIT_fwrite_int(&GenericErr.idn1,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      SEEIT_fwrite_int(&GenericErr.ECC1,outfile);
      
      fread(&GenericErr.gform1,1,1,infile);
      fwrite(&GenericErr.gform1,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);
      
      GenericErr.numverts1 = 1;  /** no need for  a read here ***/
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
	  
          SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
          SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
          SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
        }
      
      /** second object ***/
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      SEEIT_fwrite_int(&GenericErr.idn2,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);
      SEEIT_fwrite_int(&GenericErr.ECC2,outfile);
      
      fread(&GenericErr.gform2,1,1,infile);
      fwrite(&GenericErr.gform2,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex2,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID2,outfile);
      
      SEEIT_fread_int(&GenericErr.numverts2,infile);
      SEEIT_fwrite_int(&GenericErr.numverts2,outfile);
      
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts2; i++)
        {
          SEEIT_fread_double(&GenericErr.x2[i], infile);
          SEEIT_fread_double(&GenericErr.y2[i], infile);
          SEEIT_fread_double(&GenericErr.z2[i], infile);
	  
          SEEIT_fwrite_double(&GenericErr.x2[i], outfile);
          SEEIT_fwrite_double(&GenericErr.y2[i], outfile);
          SEEIT_fwrite_double(&GenericErr.z2[i], outfile);
        }
      
      if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
        {
          fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\"\n",
                  keyval,
                  GenericErr.Cnumber + 1,
                  GenericErr.magnitude,
                  GetECCLabel(GenericErr.ECC1),
                  ParseGAITgeometry(GenericErr.gform1,2),
                  GenericErr.SID1,
                  GetECCLabel(GenericErr.ECC2),
                  ParseGAITgeometry(GenericErr.gform2,2),
                  GenericErr.SID2,
                  ParseErrType(keyval));
        }
      
    }
  
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      GenericErr.numverts1 = 1;  /** no need for a read here ***/
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
        }
      
      /** second object ***/
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);
      
      fread(&GenericErr.gform2,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      
      SEEIT_fread_int(&GenericErr.numverts2,infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts2; i++)
        {
          SEEIT_fread_double(&GenericErr.x2[i], infile);
          SEEIT_fread_double(&GenericErr.y2[i], infile);
          SEEIT_fread_double(&GenericErr.z2[i], infile);
        }
    }

  if((keyval == ENCONNECT) || (keyval == AREAKINK) || (keyval == INCLSLIVER) || (keyval == FEATBRIDGE) ||
      (keyval == CLAMP_NFLAT) || (keyval == ZUNCLOSED) || (keyval == NOT_FLAT) || (keyval == BADENCON) ||
      (keyval == CLAMP_DIF) || (keyval == AREAUNCLOSED) || (keyval == LENOCOVERA))
     GenericErr.ECC2 = -1;
  
  return(GenericErr.magnitude);
}





double FreadFwriteMagnitudeAndTwoObjects(int keyval, int writeflag, FILE *infile, FILE *outfile)
{
  int i;
  
  if(infile == NULL)
    return(0.0);
  if((writeflag > 0) && (outfile == NULL))
    return(0.0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fwrite_int(&keyval,outfile);
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      SEEIT_fwrite_double(&GenericErr.magnitude,outfile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      SEEIT_fwrite_int(&GenericErr.idn1,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("   memory requested for obj 1 ID string of length %d\n",GenericErr.SIDlen1);
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      SEEIT_fwrite_int(&GenericErr.ECC1,outfile);
      
      fread(&GenericErr.gform1,1,1,infile);
      fwrite(&GenericErr.gform1,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      SEEIT_fwrite_double(&GenericErr.radius1,outfile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      SEEIT_fwrite_double(&GenericErr.height1,outfile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("  memory requested for first object's %d double (x,y,z) vertices\n",GenericErr.numverts1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	  
	  SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
	}
      
      /** second object ***/
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      SEEIT_fwrite_int(&GenericErr.idn2,outfile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("   memory requested for obj 2 ID string of length %d\n",GenericErr.SIDlen2);
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);
      SEEIT_fwrite_int(&GenericErr.ECC2,outfile);
      
      fread(&GenericErr.gform2,1,1,infile);
      fwrite(&GenericErr.gform2,1,1,outfile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile);
      SEEIT_fwrite_int(&GenericErr.Lindex2,outfile);
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      SEEIT_fwrite_double(&GenericErr.LocalID2,outfile);
      
      SEEIT_fread_double(&GenericErr.radius2,infile);
      SEEIT_fwrite_double(&GenericErr.radius2,outfile);
      
      SEEIT_fread_double(&GenericErr.height2,infile);
      SEEIT_fwrite_double(&GenericErr.height2,outfile);
      
      SEEIT_fread_int(&GenericErr.numverts2, infile);
      SEEIT_fwrite_int(&GenericErr.numverts2, outfile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("  memory requested for second object's %d double (x,y,z) vertices\n",GenericErr.numverts2);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts2; i++)
	{
	  SEEIT_fread_double(&GenericErr.x2[i], infile);
	  SEEIT_fread_double(&GenericErr.y2[i], infile);
	  SEEIT_fread_double(&GenericErr.z2[i], infile);
	  
	  SEEIT_fwrite_double(&GenericErr.x2[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.y2[i], outfile);
	  SEEIT_fwrite_double(&GenericErr.z2[i], outfile);
	}
      
      if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	{
	  fprintf(CDFout,"%d,%d,%lf,\"%s\",\"%s\",\"%s\",,,,\"%s\",\"%s\",\"%s\",\"%s\"\n",
		  keyval,
		  GenericErr.Cnumber + 1,
		  GenericErr.magnitude,
		  GetECCLabel(GenericErr.ECC1),
		  ParseGAITgeometry(GenericErr.gform1,2),
		  GenericErr.SID1,
		  GetECCLabel(GenericErr.ECC2),
		  ParseGAITgeometry(GenericErr.gform2,2),
		  GenericErr.SID2,
		  ParseErrType(keyval));
	}
    }
  
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_double(&GenericErr.magnitude,infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   memory requested for obj 1 ID string of length %d\n",GenericErr.SIDlen1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile); 
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("  memory requested for first object's %d double (x,y,z) vertices\n",GenericErr.numverts1);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
      
      /** second object ***/
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   memory requested for obj 2 ID string of length %d\n",GenericErr.SIDlen2);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);
      
      fread(&GenericErr.gform2,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile); 
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      
      SEEIT_fread_double(&GenericErr.radius2,infile);
      
      SEEIT_fread_double(&GenericErr.height2,infile);
      
      SEEIT_fread_int(&GenericErr.numverts2, infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("  memory requested for second object's %d double (x,y,z) vertices\n",GenericErr.numverts2);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts2; i++)
	{
	  SEEIT_fread_double(&GenericErr.x2[i], infile);
	  SEEIT_fread_double(&GenericErr.y2[i], infile);
	  SEEIT_fread_double(&GenericErr.z2[i], infile);
	}
      
    }
  
  return(GenericErr.magnitude);
}





int FreadFwritePointAndObject(int keyval, int writeflag, FILE *infile, FILE *outfile,
                double LPX, double LPY, double LPZ, int LastKV, int LastCindex, double LastLID1)
{
  int i;
  int duplicatefound;
  int answer = 1;
  
  if(infile == NULL)
    return(0);
  if((writeflag > 0) && (outfile == NULL))
    return(0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fread_int(&GenericErr.Cnumber,infile);

      SEEIT_fread_double(&GenericErr.px, infile);
      SEEIT_fread_double(&GenericErr.py, infile);
      SEEIT_fread_double(&GenericErr.pz, infile);

      SEEIT_fread_int(&GenericErr.idn2,infile);
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);

      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   attempt to allocate %d bytes\n",GenericErr.SIDlen2);
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d (PointAndObject 1)\n",keyval);
          exit(-1);
        } 
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';

      SEEIT_fread_int(&GenericErr.idn1,infile); 

      SEEIT_fread_int(&GenericErr.SIDlen1,infile);

      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';

      SEEIT_fread_int(&GenericErr.ECC1,infile);

      GenericErr.magnitude = (double) GenericErr.ECC1;

      fread(&GenericErr.gform1,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex1,infile);

      SEEIT_fread_double(&GenericErr.LocalID1,infile);

      SEEIT_fread_double(&GenericErr.radius1,infile);

      SEEIT_fread_double(&GenericErr.height1,infile);

      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
        }

      duplicatefound = 0;
      if(CheckNMDRlist(GenericErr.px,GenericErr.py,GenericErr.pz,keyval,GenericErr.Cnumber,GenericErr.LocalID1,-1.0) > 0)
         {
         duplicatefound = 1;
         }
      if(duplicatefound == 0)
         {
         SEEIT_fwrite_int(&keyval,outfile);
      
         SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
      
         SEEIT_fwrite_double(&GenericErr.px, outfile);
         SEEIT_fwrite_double(&GenericErr.py, outfile);
         SEEIT_fwrite_double(&GenericErr.pz, outfile);
      
         SEEIT_fwrite_int(&GenericErr.idn2,outfile);
         SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);
      
         GenericErr.SID2[GenericErr.SIDlen2] = '\0';
         fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);
      
         SEEIT_fwrite_int(&GenericErr.idn1,outfile);
         
         SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);
      
         GenericErr.SID1[GenericErr.SIDlen1] = '\0';
         fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);
      
         SEEIT_fwrite_int(&GenericErr.ECC1,outfile);

         GenericErr.magnitude = (double) GenericErr.ECC1;
      
         fwrite(&GenericErr.gform1,1,1,outfile);
      
         SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);
      
         SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);
      
         SEEIT_fwrite_double(&GenericErr.radius1,outfile);
      
         SEEIT_fwrite_double(&GenericErr.height1,outfile);
      
         SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
         for (i=0; i<GenericErr.numverts1; i++)
	   {
	     SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
	     SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
	     SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
	   }
         if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
	   {
	     GenericErr.gform2 = 0;
	     fprintf(CDFout,"%d,%d,,\"%s\",\"%s\",\"%s\",%lf,%lf,%lf,,,,\"%s\"\n",
		  keyval,
		  GenericErr.Cnumber + 1,
		  GetECCLabel(GenericErr.ECC1),
		  ParseGAITgeometry(GenericErr.gform1,2),
		  GenericErr.SID1,
		  GenericErr.px,
		  GenericErr.py,
		  GenericErr.pz,
		  ParseErrType(keyval));
	   }
         }
       else
         answer = 0;
    }
  
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_double(&GenericErr.px, infile);
      SEEIT_fread_double(&GenericErr.py, infile);
      SEEIT_fread_double(&GenericErr.pz, infile);
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   attempt to allocate %d bytes\n",GenericErr.SIDlen2);
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d (PointAndObject 1)\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);

      GenericErr.magnitude = (double) GenericErr.ECC1;
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
    }
  
  return(answer);
}






int FreadFwriteMsgMagPointObjects(int keyval, int writeflag, FILE *infile, FILE *outfile,
     double LPX, double LPY, double LPZ, int LastKV, int LastCindex, double LastLID1, double LastLID2)
{
  int i;
  int duplicatefound;
  int answer = 1;

  if(infile == NULL)
    return(0);
  if((writeflag > 0) && (outfile == NULL))
    return(0);

  GenericErr.keyval = keyval;

  if(writeflag > 0)
    {
      SEEIT_fread_int(&GenericErr.Cnumber,infile);

      SEEIT_fread_int(&GenericErr.msglen,infile);
      if(GenericErr.msglen > 0)
        {
        GenericErr.errmsg = (char *) (malloc(GenericErr.msglen + 2));
        fread(&GenericErr.errmsg[0],1,GenericErr.msglen,infile);
        GenericErr.errmsg[GenericErr.msglen] = '\0';
        }
      SEEIT_fread_double(&GenericErr.magnitude,infile);

      SEEIT_fread_double(&GenericErr.px, infile);
      SEEIT_fread_double(&GenericErr.py, infile);
      SEEIT_fread_double(&GenericErr.pz, infile);

      SEEIT_fread_int(&GenericErr.idn1,infile);

      SEEIT_fread_int(&GenericErr.SIDlen1,infile);

      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';

      SEEIT_fread_int(&GenericErr.ECC1,infile);

      fread(&GenericErr.gform1,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex1,infile);

      SEEIT_fread_double(&GenericErr.LocalID1,infile);

      SEEIT_fread_double(&GenericErr.radius1,infile);

      SEEIT_fread_double(&GenericErr.height1,infile);

      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
        }

      /** second object ***/

      SEEIT_fread_int(&GenericErr.idn2,infile);

      SEEIT_fread_int(&GenericErr.SIDlen2,infile);

      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';

      SEEIT_fread_int(&GenericErr.ECC2,infile);

      GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

      fread(&GenericErr.gform2,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex2,infile);

      SEEIT_fread_double(&GenericErr.LocalID2,infile);

      SEEIT_fread_double(&GenericErr.radius2,infile);

      SEEIT_fread_double(&GenericErr.height2,infile);

      SEEIT_fread_int(&GenericErr.numverts2, infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts2; i++)
        {
          SEEIT_fread_double(&GenericErr.x2[i], infile);
          SEEIT_fread_double(&GenericErr.y2[i], infile);
          SEEIT_fread_double(&GenericErr.z2[i], infile);
        }


      duplicatefound = 0;
      if((keyval == AGEOM_UNM_LAT) ||
        (keyval == AGEOM_UNM_LON))
         {
         if(CheckNMDRlist(-1.0,-1.0,-1.0,keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
            {
            duplicatefound = 1;
            }
         }
      else
         {
         if(CheckNMDRlist(GenericErr.px,GenericErr.py,GenericErr.pz,keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
            {
            duplicatefound = 1;
            }
         }
      if(duplicatefound == 0)
         {
         SEEIT_fwrite_int(&keyval,outfile);

         SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);
         SEEIT_fwrite_int(&GenericErr.msglen,outfile);
         if(GenericErr.msglen > 0)
           {
           fwrite(&GenericErr.errmsg[0],1,GenericErr.msglen,outfile);
           }
         SEEIT_fwrite_double(&GenericErr.magnitude,outfile);

         SEEIT_fwrite_double(&GenericErr.px, outfile);
         SEEIT_fwrite_double(&GenericErr.py, outfile);
         SEEIT_fwrite_double(&GenericErr.pz, outfile);

         SEEIT_fwrite_int(&GenericErr.idn1,outfile);

         SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);

         GenericErr.SID1[GenericErr.SIDlen1] = '\0';
         fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);

         SEEIT_fwrite_int(&GenericErr.ECC1,outfile);

         fwrite(&GenericErr.gform1,1,1,outfile);

         SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);

         SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);

         SEEIT_fwrite_double(&GenericErr.radius1,outfile);

         SEEIT_fwrite_double(&GenericErr.height1,outfile);

         SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
         for (i=0; i<GenericErr.numverts1; i++)
           {
             SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
             SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
             SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
           }

      /** second object ***/

         SEEIT_fwrite_int(&GenericErr.idn2,outfile);

         SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);

         GenericErr.SID2[GenericErr.SIDlen2] = '\0';
         fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);

         SEEIT_fwrite_int(&GenericErr.ECC2,outfile);

         GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

         fwrite(&GenericErr.gform2,1,1,outfile);

         SEEIT_fwrite_int(&GenericErr.Lindex2,outfile);

         SEEIT_fwrite_double(&GenericErr.LocalID2,outfile);

         SEEIT_fwrite_double(&GenericErr.radius2,outfile);

         SEEIT_fwrite_double(&GenericErr.height2,outfile);

         SEEIT_fwrite_int(&GenericErr.numverts2, outfile);
         for (i=0; i<GenericErr.numverts2; i++)
           {
             SEEIT_fwrite_double(&GenericErr.x2[i], outfile);
             SEEIT_fwrite_double(&GenericErr.y2[i], outfile);
             SEEIT_fwrite_double(&GenericErr.z2[i], outfile);
           }
         if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
           {
             fprintf(CDFout,"%d,%d,,\"%s\",\"%s\",\"%s\",%lf,%lf,%lf,\"%s\",\"%s\",\"%s\",\"%s\"\n",
                  keyval,
                  GenericErr.Cnumber + 1,
                  GetECCLabel(GenericErr.ECC1),
                  ParseGAITgeometry(GenericErr.gform1,2),
                  GenericErr.SID1,
                  GenericErr.px,
                  GenericErr.py,
                  GenericErr.pz,
                  GetECCLabel(GenericErr.ECC2),
                  ParseGAITgeometry(GenericErr.gform2,2),
                  GenericErr.SID2,
                  ParseErrType(keyval));
           }
         }
       else
         answer = 0;
    }
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;

      SEEIT_fread_int(&GenericErr.Cnumber,infile);

      SEEIT_fread_int(&GenericErr.msglen,infile);
      if(GenericErr.msglen > 0)
        {
        GenericErr.errmsg = (char *) (malloc(GenericErr.msglen + 2));
        fread(&GenericErr.errmsg[0],1,GenericErr.msglen,infile);
        GenericErr.errmsg[GenericErr.msglen] = '\0';
        }
      SEEIT_fread_double(&GenericErr.magnitude,infile);

      SEEIT_fread_double(&GenericErr.px, infile);
      SEEIT_fread_double(&GenericErr.py, infile);
      SEEIT_fread_double(&GenericErr.pz, infile);

      SEEIT_fread_int(&GenericErr.idn1,infile);

      SEEIT_fread_int(&GenericErr.SIDlen1,infile);

      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';

      SEEIT_fread_int(&GenericErr.ECC1,infile);

      fread(&GenericErr.gform1,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex1,infile);

      SEEIT_fread_double(&GenericErr.LocalID1,infile);

      SEEIT_fread_double(&GenericErr.radius1,infile);

      SEEIT_fread_double(&GenericErr.height1,infile);

      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
        }

      /** second object ***/

      SEEIT_fread_int(&GenericErr.idn2,infile);

      SEEIT_fread_int(&GenericErr.SIDlen2,infile);

      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';

      SEEIT_fread_int(&GenericErr.ECC2,infile);

      GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

      fread(&GenericErr.gform2,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex2,infile);

      SEEIT_fread_double(&GenericErr.LocalID2,infile);

      SEEIT_fread_double(&GenericErr.radius2,infile);

      SEEIT_fread_double(&GenericErr.height2,infile);

      SEEIT_fread_int(&GenericErr.numverts2, infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts2; i++)
        {
          SEEIT_fread_double(&GenericErr.x2[i], infile);
          SEEIT_fread_double(&GenericErr.y2[i], infile);
          SEEIT_fread_double(&GenericErr.z2[i], infile);
        }

    }

  return(answer);
}





int FreadFwritePointAndTwoObjects(int keyval, int writeflag, FILE *infile, FILE *outfile, 
     double LPX, double LPY, double LPZ, int LastKV, int LastCindex, double LastLID1, double LastLID2)
{
  int i;
  int duplicatefound;
  int answer = 1;
  
  if(infile == NULL)
    return(0);
  if((writeflag > 0) && (outfile == NULL))
    return(0);
  
  GenericErr.keyval = keyval;
  
  if(writeflag > 0)
    {
      SEEIT_fread_int(&GenericErr.Cnumber,infile);

      SEEIT_fread_double(&GenericErr.px, infile);
      SEEIT_fread_double(&GenericErr.py, infile);
      SEEIT_fread_double(&GenericErr.pz, infile);

      SEEIT_fread_int(&GenericErr.idn1,infile);

      SEEIT_fread_int(&GenericErr.SIDlen1,infile);

      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';

      SEEIT_fread_int(&GenericErr.ECC1,infile);

      fread(&GenericErr.gform1,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex1,infile);

      SEEIT_fread_double(&GenericErr.LocalID1,infile);

      SEEIT_fread_double(&GenericErr.radius1,infile);

      SEEIT_fread_double(&GenericErr.height1,infile);

      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts1; i++)
        {
          SEEIT_fread_double(&GenericErr.x1[i], infile);
          SEEIT_fread_double(&GenericErr.y1[i], infile);
          SEEIT_fread_double(&GenericErr.z1[i], infile);
        }

      /** second object ***/

      SEEIT_fread_int(&GenericErr.idn2,infile);

      SEEIT_fread_int(&GenericErr.SIDlen2,infile);

      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';

      SEEIT_fread_int(&GenericErr.ECC2,infile);

      GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

      fread(&GenericErr.gform2,1,1,infile);

      SEEIT_fread_int(&GenericErr.Lindex2,infile);

      SEEIT_fread_double(&GenericErr.LocalID2,infile);

      SEEIT_fread_double(&GenericErr.radius2,infile);

      SEEIT_fread_double(&GenericErr.height2,infile);

      SEEIT_fread_int(&GenericErr.numverts2, infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
        {
          printf("System allocation memory has been exhausted during SEE-IT condition read\n");
          printf("   execution cannot continue\n");
          printf("error keyvalue is %d\n",keyval);
          exit(-1);
        }
      for (i=0; i<GenericErr.numverts2; i++)
        {
          SEEIT_fread_double(&GenericErr.x2[i], infile);
          SEEIT_fread_double(&GenericErr.y2[i], infile);
          SEEIT_fread_double(&GenericErr.z2[i], infile);
        }


      duplicatefound = 0;
      if(CheckNMDRlist(GenericErr.px,GenericErr.py,GenericErr.pz,keyval,GenericErr.Cnumber,GenericErr.LocalID1,GenericErr.LocalID2) > 0)
         {
         duplicatefound = 1;
         }
      if(duplicatefound == 0)
         {
         SEEIT_fwrite_int(&keyval,outfile);

         SEEIT_fwrite_int(&GenericErr.Cnumber,outfile);

         SEEIT_fwrite_double(&GenericErr.px, outfile);
         SEEIT_fwrite_double(&GenericErr.py, outfile);
         SEEIT_fwrite_double(&GenericErr.pz, outfile);

         SEEIT_fwrite_int(&GenericErr.idn1,outfile);

         SEEIT_fwrite_int(&GenericErr.SIDlen1,outfile);

         GenericErr.SID1[GenericErr.SIDlen1] = '\0';
         fwrite(&GenericErr.SID1[0],1,GenericErr.SIDlen1,outfile);

         SEEIT_fwrite_int(&GenericErr.ECC1,outfile);
   
         fwrite(&GenericErr.gform1,1,1,outfile);

         SEEIT_fwrite_int(&GenericErr.Lindex1,outfile);

         SEEIT_fwrite_double(&GenericErr.LocalID1,outfile);

         SEEIT_fwrite_double(&GenericErr.radius1,outfile);

         SEEIT_fwrite_double(&GenericErr.height1,outfile);

         SEEIT_fwrite_int(&GenericErr.numverts1, outfile);
         for (i=0; i<GenericErr.numverts1; i++)
           {
             SEEIT_fwrite_double(&GenericErr.x1[i], outfile);
             SEEIT_fwrite_double(&GenericErr.y1[i], outfile);
             SEEIT_fwrite_double(&GenericErr.z1[i], outfile);
           }

      /** second object ***/

         SEEIT_fwrite_int(&GenericErr.idn2,outfile);

         SEEIT_fwrite_int(&GenericErr.SIDlen2,outfile);
   
         GenericErr.SID2[GenericErr.SIDlen2] = '\0';
         fwrite(&GenericErr.SID2[0],1,GenericErr.SIDlen2,outfile);

         SEEIT_fwrite_int(&GenericErr.ECC2,outfile);

         GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);

         fwrite(&GenericErr.gform2,1,1,outfile);

         SEEIT_fwrite_int(&GenericErr.Lindex2,outfile);

         SEEIT_fwrite_double(&GenericErr.LocalID2,outfile);

         SEEIT_fwrite_double(&GenericErr.radius2,outfile);

         SEEIT_fwrite_double(&GenericErr.height2,outfile);

         SEEIT_fwrite_int(&GenericErr.numverts2, outfile);
         for (i=0; i<GenericErr.numverts2; i++)
           {
             SEEIT_fwrite_double(&GenericErr.x2[i], outfile);
             SEEIT_fwrite_double(&GenericErr.y2[i], outfile);
             SEEIT_fwrite_double(&GenericErr.z2[i], outfile);
           }
         if(CDFREPORT > 0) /** cdf format is err numb, magnitude, ECL1, Geom1, SID1, Pt LocX, Y, Z, ECL2, Geom2, SID2, Err Desc  **/
           {
             fprintf(CDFout,"%d,%d,,\"%s\",\"%s\",\"%s\",%lf,%lf,%lf,\"%s\",\"%s\",\"%s\",\"%s\"\n",
                  keyval,
                  GenericErr.Cnumber + 1,
                  GetECCLabel(GenericErr.ECC1),
                  ParseGAITgeometry(GenericErr.gform1,2),
                  GenericErr.SID1,
                  GenericErr.px,
                  GenericErr.py,
                  GenericErr.pz,
                  GetECCLabel(GenericErr.ECC2),
                  ParseGAITgeometry(GenericErr.gform2,2),
                  GenericErr.SID2,
                  ParseErrType(keyval));
           }
         }
       else
         answer = 0;
    }
  else /*** no write required, just read error objects ***/
    {
      GenericErr.numverts1 = 0;
      GenericErr.gform1 = 0;
      GenericErr.idn1 = 0;
      GenericErr.radius1 = 0;
      GenericErr.height1 = 0;
      GenericErr.px = GenericErr.py = GenericErr.pz = 0;
      GenericErr.numverts2 = 0;
      GenericErr.gform2 = 0;
      GenericErr.idn2 = 0;
      GenericErr.radius2 = 0;
      GenericErr.height2 = 0;
      GenericErr.msglen = 0;
      GenericErr.errmsg = NULL;
      
      SEEIT_fread_int(&GenericErr.Cnumber,infile);
      
      SEEIT_fread_double(&GenericErr.px, infile);
      SEEIT_fread_double(&GenericErr.py, infile);
      SEEIT_fread_double(&GenericErr.pz, infile);
      
      SEEIT_fread_int(&GenericErr.idn1,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen1,infile);
      
      GenericErr.SID1 = (char *) (malloc(GenericErr.SIDlen1+1));
      if(GenericErr.SID1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID1[0],1,GenericErr.SIDlen1,infile);
      GenericErr.SID1[GenericErr.SIDlen1] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC1,infile);
      
      fread(&GenericErr.gform1,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex1,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID1,infile);
      
      SEEIT_fread_double(&GenericErr.radius1,infile);
      
      SEEIT_fread_double(&GenericErr.height1,infile);
      
      SEEIT_fread_int(&GenericErr.numverts1, infile);
      GenericErr.x1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.y1 = (double *) (malloc(SzD * GenericErr.numverts1));
      GenericErr.z1 = (double *) (malloc(SzD * GenericErr.numverts1));
      if(GenericErr.z1 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts1; i++)
	{
	  SEEIT_fread_double(&GenericErr.x1[i], infile);
	  SEEIT_fread_double(&GenericErr.y1[i], infile);
	  SEEIT_fread_double(&GenericErr.z1[i], infile);
	}
      
      /** second object ***/
      
      SEEIT_fread_int(&GenericErr.idn2,infile);
      
      SEEIT_fread_int(&GenericErr.SIDlen2,infile);
      
      GenericErr.SID2 = (char *) (malloc(GenericErr.SIDlen2+1));
      if(GenericErr.SID2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      fread(&GenericErr.SID2[0],1,GenericErr.SIDlen2,infile);
      GenericErr.SID2[GenericErr.SIDlen2] = '\0';
      
      SEEIT_fread_int(&GenericErr.ECC2,infile);

      GenericErr.magnitude = ((double) GenericErr.ECC1) +  (double) GenericErr.ECC2 / (double) (INscc_loop + 2);
      
      fread(&GenericErr.gform2,1,1,infile);
      
      SEEIT_fread_int(&GenericErr.Lindex2,infile);
      
      SEEIT_fread_double(&GenericErr.LocalID2,infile);
      
      SEEIT_fread_double(&GenericErr.radius2,infile);
      
      SEEIT_fread_double(&GenericErr.height2,infile);
      
      SEEIT_fread_int(&GenericErr.numverts2, infile);
      GenericErr.x2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.y2 = (double *) (malloc(SzD * GenericErr.numverts2));
      GenericErr.z2 = (double *) (malloc(SzD * GenericErr.numverts2));
      if(GenericErr.z2 == NULL)
	{
	  printf("System allocation memory has been exhausted during SEE-IT condition read\n");
	  printf("   execution cannot continue\n");
	  printf("error keyvalue is %d\n",keyval);
	  exit(-1);
	}
      for (i=0; i<GenericErr.numverts2; i++)
	{
	  SEEIT_fread_double(&GenericErr.x2[i], infile);
	  SEEIT_fread_double(&GenericErr.y2[i], infile);
	  SEEIT_fread_double(&GenericErr.z2[i], infile);
	}
      
    }
  
  return(answer);
}





void ReadWriteObjConditions(FILE *fin, FILE *fout, int dimension, int *CheckArray, int *InstanceArray)
{
int ObjsWithErrs;
int TtlObjErrs;
int Orig_ObjsWithErrs;
int Orig_TtlObjErrs;
int FPinFile;
long int fileposn;
int ecc_index1, ecc_index2;
double PosnAndFile,SortID;
double magnitude;
int Lindex1, Lindex2;
char geom;
int *Obj_Mdl_Flags;
long int OMFposn;
int NumbObjErrs,NumbInstanceErrors;
int keyval, instanceNumb, specificNumb;
int i,j,k,o,noe,nie;
int ObjsWritten, ObjsPerPage, FPtoWrite;
int KeepCondition;
int LookupIndex;
int template_entries;
int zeroval = 0;
struct clonestocount *ctc, *ctcp;


  Obj_Mdl_Flags = (int * ) (malloc(SzI * (NumberOfModels + 2)));
  if(Obj_Mdl_Flags == NULL)
     {
     printf("all available memory has been consumed, execution must terminate now\n");
     exit(-1);
     }

  for(i=0; i<=CONDITION_DEFINITIONS; i++)
     {
     CCBY[i].count = 0;
     CCBY[i].c = NULL;
     }


  ObjsWithErrs = 0; 
  TtlObjErrs = 0;

  RB_ObjTree = NULL;
  RB_ObjSortTree = NULL;


  SEEIT_fread_int(&Orig_ObjsWithErrs,fin);
  SEEIT_fread_int(&Orig_TtlObjErrs,fin);
  SEEIT_fread_int(&FPinFile,fin);
  for(i=0; i<FPinFile; i++)
     SEEIT_fread_long(&fileposn,fin);

  for(i=0; i<NumberOfModels; i++)
     {
     SEEIT_fread_int(&Obj_Mdl_Flags[i], fin); /** really just skipping over the old settings here ***/
     Obj_Mdl_Flags[i] = 0;  /*** reset these as they will be calculated using filters ***/
     }

  SEEIT_fread_int(&template_entries,fin);
  for(i=0; i<template_entries; i++)
     {
     SEEIT_fread_int(&j,fin);
     SEEIT_fread_int(&k,fin);
     SEEIT_fread_int(&o,fin);
     }

  for(o=0; o<Orig_ObjsWithErrs; o++)
     {
     fread(&geom,1,1,fin);
     SEEIT_fread_double(&SortID,fin);
     SEEIT_fread_double(&PosnAndFile,fin);
     SEEIT_fread_int(&ecc_index1,fin);
     SEEIT_fread_int(&Lindex1,fin);
     SEEIT_fread_int(&NumbObjErrs,fin);
     noe = 1;
     do
        {
        SEEIT_fread_int(&keyval,fin);
        SEEIT_fread_int(&NumbInstanceErrors,fin);
        SEEIT_fread_int(&instanceNumb,fin);
        for(nie=0; nie<NumbInstanceErrors; nie++)
           {
           SEEIT_fread_double(&magnitude,fin);
           SEEIT_fread_int(&ecc_index2,fin);
           SEEIT_fread_int(&Lindex2,fin);
           SEEIT_fread_int(&specificNumb,fin);
           KeepCondition = 1;

           if((Lindex1 >= 0) && (MdlNames[CrsWlk[Lindex1].crossindex].inout2 > 0))
              {
              KeepCondition = 0;
              }
           if((Lindex2 >= 0) && (MdlNames[CrsWlk[Lindex2].crossindex].inout2 > 0))
              {
              KeepCondition = 0;
              }

           if(KeepCondition > 0)  /*** now fileter by check, instance, and condition number ***/
              {
              if(instanceNumb > 0)   /*** is a clone condition ***/
                 {
                 LookupIndex = GetCloneIndex(instanceNumb, keyval);
                 if(CloneErrorLookup[LookupIndex].filterout > 0)
                    KeepCondition = 0;
                 }
              else /*** is a root-level condition ***/
                 {
                 if(ErrorLookup[keyval].filterout > 0)
                    KeepCondition = 0;
                 }
              }
      
           if(KeepCondition > 0)
              {
							  
			   AddToObjectConditionTree(PosnAndFile,(int)SortID,keyval,instanceNumb,ecc_index1,ecc_index2,magnitude,
                                      Lindex1, Lindex2, specificNumb, geom, &ObjsWithErrs, &TtlObjErrs);
              }
           }
        noe += NumbInstanceErrors;
        } while(noe <= NumbObjErrs);
     }

   if(ObjsWithErrs > 0)
      {
      SEEIT_fwrite_int(&ObjsWithErrs,fout);
      SEEIT_fwrite_int(&TtlObjErrs,fout);
      printf("writing %d features with %d ttl conditions to file \n",ObjsWithErrs,TtlObjErrs);

      ObjsWritten = 0;
      ObjsPerPage = 25;

      FPtoWrite = ObjsWithErrs / ObjsPerPage;
      i = ObjsWithErrs % ObjsPerPage;
      if(i > 0)
         FPtoWrite += 1;

      SEEIT_fwrite_int(&FPtoWrite,fout);
      fileposn = 0;
      for(i=0; i<FPtoWrite; i++)
         {
         SEEIT_fwrite_long(&fileposn,fout);
         }

      OMFposn = ftell(fout);

      for(i=0; i<NumberOfModels; i++)
         SEEIT_fwrite_int(&Obj_Mdl_Flags[i], fout);

      template_entries = 0;
      for(i=0; i<=CONDITION_DEFINITIONS; i++)
         {
         if(CCBY[i].count > 0)
            {
            template_entries += 1;
            }
         if(CCBY[i].c != NULL)
            {
            ctc = CCBY[i].c;
            while(ctc != NULL)
               {
               if(ctc->count > 0)
                  {
                  template_entries += 1;
                  }
               ctc = ctc->next;
               }
            }
         }
      SEEIT_fwrite_int(&template_entries,fout);
      for(i=0; i<=CONDITION_DEFINITIONS; i++)
         {
         if(CCBY[i].count > 0)
            {
            SEEIT_fwrite_int(&i,fout);
            SEEIT_fwrite_int(&zeroval,fout);
            SEEIT_fwrite_int(&CCBY[i].count,fout);
            }
         if(CCBY[i].c != NULL)
            {
            ctc = CCBY[i].c;
            while(ctc != NULL)
               {
               SEEIT_fwrite_int(&i,fout);
               SEEIT_fwrite_int(&ctc->cindex,fout);
               SEEIT_fwrite_int(&ctc->count,fout);
               ctcp = ctc;
               ctc = ctc->next;
               free(ctcp);
               }
            CCBY[i].c = NULL;
            CCBY[i].count = 0;
            }
         }



      if(RB_ObjTree != NULL)
        {
          PrintObjConditionTreeInorder(RB_ObjTree,RB_ObjTree->root->left);
          PrintSortedObjConditionTreeInorder(RB_ObjSortTree, RB_ObjSortTree->root->left, &ObjsWritten, ObjsPerPage, fout, Obj_Mdl_Flags);

          ETF_RB_Obj_TreeDestroy(RB_ObjTree);

          ETF_RB_Obj_TreeDestroy(RB_ObjSortTree);
        }
      }


  fseek(fout, OMFposn, SEEK_SET);
  for(i=0; i<NumberOfModels; i++)
     SEEIT_fwrite_int(&Obj_Mdl_Flags[i], fout);
  free(Obj_Mdl_Flags);

   return;

}







char * AddToCompString(int *currentsize, int * strlimit, char * ans, char * StrToAdd)
{
int CharsToAdd;
static char * copyover;

   CharsToAdd = strlen(StrToAdd);
   *currentsize += CharsToAdd;
   if(*currentsize >= *strlimit)
      {
      *strlimit = *strlimit + CharsToAdd + 1000;
      copyover = (char *) malloc(*strlimit);
      if(copyover == NULL)
         {
         printf("memory has been exhausted during comparison of condition reports\n terminating now\n");
         exit(-1);
         }
      else
         {
         strcpy(copyover, ans);
         strcat(copyover,StrToAdd);
         free(ans);
         return(copyover);
         }
      }
   else
      {
      strcat(ans,StrToAdd);

      return(ans);
      }

   return "ERROR";
}


int ConsultLK2file(char *smfilename, int *KDFlags)
{
  char * KeepDismissFileName;
  int i,j, kdflag;
  FILE *kdin;
  int totalentries, ttlignore, GroupID;
  int *errnums,*instances,*numbers;
  long int *fileposns;
  char tval;
  
  
  ttlignore = 0;
  j = strlen(smfilename);
  KeepDismissFileName = (char *) (malloc(j + 5));
  if(KeepDismissFileName == NULL)
    return(ttlignore);
  
  strcpy(KeepDismissFileName,smfilename);
  
  strcat(KeepDismissFileName,".lk2");
  
  kdin = fopen(KeepDismissFileName,"rb");
  if(kdin == NULL)
    return(ttlignore);
  
  free(KeepDismissFileName);
  
  SEEIT_fread_int(&totalentries,kdin);
  
  if(totalentries == 0)
    {
      fclose(kdin);
      return(ttlignore);
    }
  
  errnums   = (int *) malloc(SzI * totalentries);
  instances = (int *) malloc(SzI * totalentries);
  numbers   = (int *) malloc(SzI * totalentries);
  fileposns = (long int *) malloc(SzL * totalentries);
  
  for(i=0;i<totalentries;i++)
    {
      SEEIT_fread_int (&errnums  [i], kdin);
      SEEIT_fread_int (&instances[i], kdin);
      SEEIT_fread_int (&numbers  [i], kdin);
      SEEIT_fread_long(&fileposns[i], kdin);
    }
  

  for(i=0;i<totalentries;i++)
    {
      fseek(kdin,fileposns[i],SEEK_SET);
      
      for(j=1; j<=numbers[i]; j++)
	{
	  fread(&tval,1,1,kdin);
	  SEEIT_fread_int(&GroupID,kdin);
	  kdflag = (int) tval;
	  
	  if(kdflag == 0)
	    {
		  KDFlags[errnums[i]] += 1;
		  ttlignore += 1;

            }
	}
    }
  
  free(errnums);
  free(instances);
  free(numbers);
  free(fileposns);
  fclose(kdin);
  return(ttlignore);
}




char * CompareInspectionResults(char *f1name, char *f2name,
                               char *f1type, char *f2type, int verbosity)
{
FILE *f1, *f2;
char lineout[1000];
char lo2[1000];
char proj1[1000];
char proj2[1000];
char *P1PointFeatures = NULL;
char *P1LineFeatures = NULL;
char *P1AreaFeatures = NULL;
char *P1ElevPosts = NULL;
char *P2PointFeatures = NULL;
char *P2LineFeatures = NULL;
char *P2AreaFeatures = NULL;
char *P2ElevPosts = NULL;
char *answerstring = NULL;
int TtlPts1, TtlA1, TtlL1, TtlEP1;
int TtlPts2, TtlA2, TtlL2, TtlEP2;
int answerstrlimit, answerstrcurrent;
extern int SmryMagic;
unsigned char CS1, CS2;
double MinXC1, MaxXC1, MinYC1, MaxYC1;
double MinXC2, MaxXC2, MinYC2, MaxYC2;
double minX1, minY1, maxX1,maxY1;
double minX2, minY2, maxX2,maxY2;
long int polys1, gpoint1, ptverts1, lineverts1, areaverts1;
long int polys2, gpoint2, ptverts2, lineverts2, areaverts2;
double A_inspected1, L_inspected1, P_inspected1, G_inspected1;
double A_inspected2, L_inspected2, P_inspected2, G_inspected2;
double tdbl;
int grids1, grids2, sindex1, lindex1, sindex2, lindex2;
double RSize1, RSize2;
double MinFXC1, MinFYC1, MaxFXC1, MaxFYC1;
double MinFXC2, MinFYC2, MaxFXC2, MaxFYC2;
int XExtent1, YExtent1, XExtent2, YExtent2;
double XTrans1, YTrans1, XTrans2, YTrans2;
int magic1, magic2, save_endian;
char FE1, FE2;
double dbltemp;
long int time_dif;
struct FCODEdetails
   {
   char *geom;
   char *ECCcode;
   char *ECCname;
   int number;
   };
struct ByTemplate 
   { 
   int keyval;
   int e_count;
   int ign_count;
   int numactive;
   int usemagnitude;
   int numthreshold;
   char *uom;
   double t1min;
   double t1max;
   double t2min;
   double t2max;
   int bigisworse;
   double cond_min;
   double cond_max;
   int numFCODES;
   struct FCODEdetails * FCD;
   } *OMF1, *OMF2;
struct Results
   {
   char *project;
   char *fromcoord;
   char *tocoord;
   long inspect_time;
   int numconditions;
   int numignore;
   int numinstances;
   int numobjects;
   int numelevobj;
   int numelevp;
   } Meta1, Meta2;
int answer = 1;
int i,j,k,ii,jj,kk,arraysize,ti1,sl,sl1,sl2;
int col1start,col2start;
int SecPerHr = 3600;
int SecPerDay = 86400;
int daydiff, hrdiff, mindiff;
time_t Time1, Time2;
struct OrderNames
   {
   int OneOrTwo;
   int index;
   int pairindex;
   struct OrderNames * next;
   } *ONroot, *ONc, *ONp, *ONn;
int SzON = sizeof(struct OrderNames);

int KDFlags[CONDITION_ARRAY_SIZE];



   answerstrlimit = 10000;
   answerstrcurrent = 0;

   save_endian = file_endianness;
   
   answerstring = (char *) (malloc(answerstrlimit));
   if(answerstring == NULL)
      {
      printf("memory has been exhausted during comparison of condition reports\n terminating now\n");
      exit(-1);
      }

   f1 = f2 = NULL;

   if(f1name != NULL)
      {
      f1 = fopen(f1name,"rb");
printf("opening %s for condit rpt comparison\n",f1name);
      i = strlen(f1name);
      while(i > 0)
         {
         if(f1name[i] == '.')
            {
            f1name[i] = '\0';
            break;
            }
         --i;
         }
      }
   if(f1 == NULL)
    {
    sprintf(answerstring,"Unable to open %s for use in generating comparison\nThis file is required\n",f1name);
    return(answerstring);
    }

   if(f2name != NULL)
      {
      f2 = fopen(f2name,"rb");
      i = strlen(f2name);
      while(i > 0)
         {
         if(f2name[i] == '.')
            {
            f2name[i] = '\0';
            break;
            }
         --i;
         }
      }

   fread(&FE1,1,1,f1);  /** file endianness for this condreport ***/
   file_endianness = (int) FE1;

   SEEIT_fread_int(&magic1,f1); /** use as a magic number ***/
   fread(&CS1,SzUC,1,f1); /** cood syst type ***/
   SEEIT_fread_double(&MinXC1,f1);   /***MinXcoord***/
   SEEIT_fread_double(&MinYC1,f1);  /***MinYcoord***/
   SEEIT_fread_double(&MaxXC1,f1);  /***MaxXcoord***/
   SEEIT_fread_double(&MaxYC1,f1);  /***MaxYcoord***/
   SEEIT_fread_double(&minX1,f1);  /***minX***/
   SEEIT_fread_double(&minY1,f1);  /***minY***/
   SEEIT_fread_double(&maxX1,f1);  /***maxX***/
   SEEIT_fread_double(&maxY1,f1);  /***maxY ***/
   SEEIT_fread_long(&polys1,f1);     /***ttlpolys***/
   SEEIT_fread_long(&gpoint1,f1);     /***ttlgridpoints***/
   SEEIT_fread_long(&ptverts1,f1);     /***pointverts***/
   SEEIT_fread_long(&lineverts1,f1);    /***lineverts***/
   SEEIT_fread_long(&areaverts1,f1);    /***arealverts***/
   SEEIT_fread_int(&grids1,f1);     /****numgrids***/
   SEEIT_fread_int(&sindex1,f1);     /****si1***/
   SEEIT_fread_int(&lindex1,f1);      /****si2***/
   SEEIT_fread_double(&RSize1,f1);       /****LRegionSize ****/
   SEEIT_fread_double(&MinFXC1,f1);       /****MinXcoord ****/
   SEEIT_fread_double(&MinFYC1,f1);       /****MinYcoord ****/
   SEEIT_fread_double(&MaxFXC1,f1);       /****MaxXcoord ****/
   SEEIT_fread_double(&MaxFYC1,f1);       /****MaxYcoord ****/
   SEEIT_fread_int(&XExtent1,f1);      /****XindexExtent***/
   SEEIT_fread_int(&YExtent1,f1);      /****YindexExtent***/
   SEEIT_fread_double(&XTrans1,f1);       /****Xtranslation ****/
   SEEIT_fread_double(&YTrans1,f1);       /****Ytranslation ****/

   if(magic1 != SmryMagic)
      {
      fclose(f1);
      sprintf(answerstring,"The file: %s is not the correct type for generating a condition report comparison\n",f1name);
      file_endianness = save_endian;
      return(answerstring);
      }

   if(f2 != NULL)
      {
      fread(&FE2,1,1,f2);  /** file endianness for this condreport ***/
      file_endianness = (int) FE2;

      SEEIT_fread_int(&magic2,f2); /** use as a magic number ***/
      fread(&CS2,SzUC,1,f2); /** cood syst type ***/
      SEEIT_fread_double(&MinXC2,f2);   /***MinXcoord***/
      SEEIT_fread_double(&MinYC2,f2);  /***MinYcoord***/
      SEEIT_fread_double(&MaxXC2,f2);  /***MaxXcoord***/
      SEEIT_fread_double(&MaxYC2,f2);  /***MaxYcoord***/
      SEEIT_fread_double(&minX2,f2);  /***minX***/
      SEEIT_fread_double(&minY2,f2);  /***minY***/
      SEEIT_fread_double(&maxX2,f2);  /***maxX***/
      SEEIT_fread_double(&maxY2,f2);  /***maxY ***/
      SEEIT_fread_long(&polys2,f2);     /***ttlpolys***/
      SEEIT_fread_long(&gpoint2,f2);     /***ttlgridpoints***/
      SEEIT_fread_long(&ptverts2,f2);     /***pointverts***/
      SEEIT_fread_long(&lineverts2,f2);    /***lineverts***/
      SEEIT_fread_long(&areaverts2,f2);    /***arealverts***/
      SEEIT_fread_int(&grids2,f2);     /****numgrids***/
      SEEIT_fread_int(&sindex2,f2);     /****si1***/
      SEEIT_fread_int(&lindex2,f2);      /****si2***/
      SEEIT_fread_double(&RSize2,f2);       /****LRegionSize ****/
      SEEIT_fread_double(&MinFXC2,f2);       /****MinXcoord ****/
      SEEIT_fread_double(&MinFYC2,f2);       /****MinYcoord ****/
      SEEIT_fread_double(&MaxFXC2,f2);       /****MaxXcoord ****/
      SEEIT_fread_double(&MaxFYC2,f2);       /****MaxYcoord ****/
      SEEIT_fread_int(&XExtent2,f2);      /****XindexExtent***/
      SEEIT_fread_int(&YExtent2,f2);      /****YindexExtent***/
      SEEIT_fread_double(&XTrans2,f2);       /****Xtranslation ****/
      SEEIT_fread_double(&YTrans2,f2);       /****Ytranslation ****/

      if(magic2 != SmryMagic)
         {
         fclose(f2);
         sprintf(answerstring,"The file: %s is not the correct type for generating a condition report comparison\n",f2name);
         file_endianness = save_endian;
         return(answerstring);
         }
      }

   if((f2 != NULL) && (answer != -2))
      {
      if(CS1 != CS2) answer = -3;
      if(MinXC1 != MinXC2) 
         {
         dbltemp = MinXC1 - MinXC2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }
      if(MinYC1 != MinYC2) 
         {
         dbltemp = MinYC1 - MinYC2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }
      if(MaxXC1 != MaxXC2)
         {
         dbltemp = MaxXC1 - MaxXC2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }
      if(MaxYC1 != MaxYC2)
         {
         dbltemp = MaxYC1 - MaxYC2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }

      if(minX1 != minX2)
         {
         dbltemp = minX1 - minX2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }
      if(maxX1 != maxX2)
         {
         dbltemp = maxX1 - maxX2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }
      if(minY1 != minY2)
         {
         dbltemp = minY1 - minY2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }
      if(maxY1 != maxY2)
         {
         dbltemp = maxY1 - maxY2;
         if((0.11 > dbltemp) && (-0.11 < dbltemp))
            answer = -3;
         }

      if(polys1 != polys2) answer = -3;
      if(gpoint1 != gpoint2) answer = -3;
      if(ptverts1 != ptverts2) answer = -3;
      if(lineverts1 != lineverts2) answer = -3;
      if(areaverts1 != areaverts2) answer = -3;
      if(grids1 != grids2) answer = -3;
      }

  OMF1 = (struct ByTemplate * ) (malloc(sizeof(struct ByTemplate) * (CONDITION_DEFINITIONS + 2)));
  if(OMF1 == NULL)
     {
     printf("all available memory has been consumed, execution must terminate now\n");
     exit(-1);
     }
  if((f2 != NULL) && (answer != -2))
     {
     OMF2 = (struct ByTemplate * ) (malloc(sizeof(struct ByTemplate) * (CONDITION_DEFINITIONS + 2)));
     if(OMF2 == NULL)
        {
        printf("all available memory has been consumed, execution must terminate now\n");
        exit(-1);
        }
     }
   else
     OMF2 = NULL;



   file_endianness = (int) FE1;

   SEEIT_fread_int(&ti1,f1);
   Meta1.project = (char *) (malloc(ti1 + 2));
   fread(&Meta1.project[0],ti1,1,f1);
   Meta1.project[ti1] = '\0';

   SEEIT_fread_int(&ti1,f1);

   Meta1.fromcoord = (char *) (malloc(ti1 + 2));
   fread(&Meta1.fromcoord[0],ti1,1,f1);
   Meta1.fromcoord[ti1] = '\0';
   SEEIT_fread_int(&ti1,f1);

   Meta1.tocoord = (char *) (malloc(ti1 + 2));
   fread(&Meta1.tocoord[0],ti1,1,f1);
   Meta1.tocoord[ti1] = '\0';
   SEEIT_fread_long(&Meta1.inspect_time,f1);

   SEEIT_fread_int(&ti1,f1);
   P1PointFeatures = (char *) (malloc(ti1 + 2));
   fread(&P1PointFeatures[0],ti1,1,f1);
   P1PointFeatures[ti1] = '\0';

   SEEIT_fread_int(&ti1,f1);
   P1LineFeatures = (char *) (malloc(ti1 + 2));
   fread(&P1LineFeatures[0],ti1,1,f1);
   P1LineFeatures[ti1] = '\0';

   SEEIT_fread_int(&ti1,f1);
   P1AreaFeatures = (char *) (malloc(ti1 + 2));
   fread(&P1AreaFeatures[0],ti1,1,f1);
   P1AreaFeatures[ti1] = '\0';

   SEEIT_fread_int(&ti1,f1);
   P1ElevPosts = (char *) (malloc(ti1 + 2));
   fread(&P1ElevPosts[0],ti1,1,f1);
   P1ElevPosts[ti1] = '\0';
   SEEIT_fread_double(&A_inspected1,f1); /** area features inspected ***/
   SEEIT_fread_double(&L_inspected1,f1); /** line features inspected ***/
   SEEIT_fread_double(&P_inspected1,f1); /** point features inspected ***/
   SEEIT_fread_double(&G_inspected1,f1); /** grid features inspected ***/

   for(i=0; i<= CONDITION_DEFINITIONS; i++)
      {
      OMF1[i].keyval = -1; /** int  **/
      OMF1[i].e_count = 0; /** int  **/
      OMF1[i].ign_count = 0; /** int **/
      OMF1[i].numactive = 0; /** int  **/
      OMF1[i].usemagnitude = -1; /*** int ***/
      OMF1[i].numthreshold = -1; /*** int ***/
      OMF1[i].uom = NULL; /** char *  ***/
      OMF1[i].t1min = 0.0; /*** double ***/
      OMF1[i].t1max = 0.0; /*** double ***/
      OMF1[i].t2min = 0.0; /*** double ***/
      OMF1[i].t2max = 0.0; /*** double ***/
      OMF1[i].bigisworse = -1; /** int  **/
      OMF1[i].cond_min = 0.0; /*** double ***/
      OMF1[i].cond_max = 0.0; /*** double ***/
      OMF1[i].numFCODES = -1;  /*** int ***/
      OMF1[i].FCD = NULL; /*** struct FCODEdetails * ***/
      }

   for(arraysize = 0; arraysize < CONDITION_DEFINITIONS; arraysize++)
      {
      SEEIT_fread_int(&i,f1);  /**** this is the keyval from file ***/
      if(i > CONDITION_DEFINITIONS)
         {
         sprintf(answerstring,"Error in condition report comparison - condition key value is higher than curent recognized limit\n");
         printf("place 1: key values read %d Condition Definitions limit %d\n",i,CONDITION_DEFINITIONS);
         file_endianness = save_endian;
         return(answerstring);
         }
      OMF1[i].keyval = i;
      SEEIT_fread_int(&OMF1[i].e_count,f1);
      
      SEEIT_fread_int(&OMF1[i].numactive,f1);
      SEEIT_fread_int(&OMF1[i].usemagnitude,f1);
      SEEIT_fread_int(&OMF1[i].numthreshold,f1);
      SEEIT_fread_int(&j,f1); /** strlen for uom string ***/
      OMF1[i].uom = (char *) (malloc(j + 2));
      fread(&OMF1[i].uom[0],j,1,f1);
      OMF1[i].uom[j] = '\0';
      SEEIT_fread_double(&OMF1[i].t1min,f1);
      SEEIT_fread_double(&OMF1[i].t1max,f1);
      SEEIT_fread_double(&OMF1[i].t2min,f1);
      SEEIT_fread_double(&OMF1[i].t2max,f1);
      if(OMF1[i].e_count > 0)
         {
         SEEIT_fread_int(&OMF1[i].bigisworse,f1);
         SEEIT_fread_double(&OMF1[i].cond_min,f1);
         SEEIT_fread_double(&OMF1[i].cond_max,f1);
         SEEIT_fread_int(&j,f1);  /*** number of FCODE entries that follow ***/
         OMF1[i].numFCODES = j;
         OMF1[i].FCD = (struct FCODEdetails *) (malloc(sizeof(struct FCODEdetails) * (j+1)));
         if(OMF1[i].FCD == NULL)
            {
            printf("available memory has been consumed during condition report comarison data read operation\n");
            printf("execution must now terminate\n");
            exit(-1);
            }
         for(k=0; k<j; k++)
            {
            SEEIT_fread_int(&ti1,f1);
            OMF1[i].FCD[k].geom = (char *) (malloc(ti1+1));
            fread(&OMF1[i].FCD[k].geom[0],ti1,1,f1);
            OMF1[i].FCD[k].geom[ti1] = '\0';
   
            if(NGA_TYPE == 1)
               {
               SEEIT_fread_int(&ti1,f1);
               OMF1[i].FCD[k].ECCcode = (char *) (malloc(ti1+1));
               fread(&OMF1[i].FCD[k].ECCcode[0],ti1,1,f1);
               OMF1[i].FCD[k].ECCcode[ti1] = '\0';
               }
            else
               {
               OMF1[i].FCD[k].ECCcode = (char *) (malloc(2));
               strcpy(OMF1[i].FCD[k].ECCcode," ");
               }
   
            SEEIT_fread_int(&ti1,f1);
            OMF1[i].FCD[k].ECCname = (char *) (malloc(ti1+1));
            fread(&OMF1[i].FCD[k].ECCname[0],ti1,1,f1);
               OMF1[i].FCD[k].ECCname[ti1] = '\0';
   
            SEEIT_fread_int(&OMF1[i].FCD[k].number,f1);
            }
         }
      } /*** end for arraysize ***/

   SEEIT_fread_int(&Meta1.numconditions,f1);

   for(i=0; i<CONDITION_DEFINITIONS; i++)
      {
      KDFlags[i] = 0;
      }

   if(Meta1.numconditions > 0)
      Meta1.numignore =  ConsultLK2file(f1name, KDFlags);
   else
      Meta1.numignore = 0;
   if(Meta1.numignore > 0)
      {
      for(i=0; i<CONDITION_DEFINITIONS; i++)
         {
         OMF1[i].ign_count = KDFlags[i];
         }
      }

   SEEIT_fread_int(&Meta1.numobjects,f1);
   SEEIT_fread_int(&Meta1.numelevobj,f1);
   SEEIT_fread_int(&Meta1.numelevp,f1);
   Meta1.numobjects -= Meta1.numelevobj;
   SEEIT_fread_int(&Meta1.numinstances,f1); /** this is the value written as variable I_Applied **/
   
   file_endianness = save_endian;



   if(OMF2 != NULL)
      {
      file_endianness = (int) FE2;

      SEEIT_fread_int(&ti1,f2);
      Meta2.project = (char *) (malloc(ti1 + 2));
      fread(&Meta2.project[0],ti1,1,f2);
      Meta2.project[ti1] = '\0';

      SEEIT_fread_int(&ti1,f2);
      Meta2.fromcoord = (char *) (malloc(ti1 + 2));
      fread(&Meta2.fromcoord[0],ti1,1,f2);
      Meta2.fromcoord[ti1] = '\0';
      SEEIT_fread_int(&ti1,f2);
      Meta2.tocoord = (char *) (malloc(ti1 + 2));
      fread(&Meta2.tocoord[0],ti1,1,f2);
      Meta2.tocoord[ti1] = '\0';
      SEEIT_fread_long(&Meta2.inspect_time,f2);

      Time1 = (time_t) Meta1.inspect_time;
      Time2 = (time_t) Meta2.inspect_time;

      if(Time2 > Time1)
         time_dif = Time2 - Time1;
      else
         time_dif = Time1 - Time2;

      daydiff = hrdiff = mindiff = 0;
      if(time_dif > SecPerDay)
         {
         daydiff = time_dif / SecPerDay;
         time_dif -= (daydiff * SecPerDay);
         }

      if(time_dif > SecPerHr)
         {
         hrdiff = time_dif / SecPerHr;
         time_dif -= (hrdiff * SecPerHr);
         }

      if(time_dif > 60)
         {
         mindiff = time_dif / 60;
         time_dif -= (mindiff * 60);
         }


      SEEIT_fread_int(&ti1,f2);
      P2PointFeatures = (char *) (malloc(ti1 + 2));
      fread(&P2PointFeatures[0],ti1,1,f2);
      P2PointFeatures[ti1] = '\0';
   
      SEEIT_fread_int(&ti1,f2);
      P2LineFeatures = (char *) (malloc(ti1 + 2));
      fread(&P2LineFeatures[0],ti1,1,f2);
      P2LineFeatures[ti1] = '\0';
   
      SEEIT_fread_int(&ti1,f2);
      P2AreaFeatures = (char *) (malloc(ti1 + 2));
      fread(&P2AreaFeatures[0],ti1,1,f2);
      P2AreaFeatures[ti1] = '\0';
   
      SEEIT_fread_int(&ti1,f2);
      P2ElevPosts = (char *) (malloc(ti1 + 2));
      fread(&P2ElevPosts[0],ti1,1,f2);
      P2ElevPosts[ti1] = '\0';


      SEEIT_fread_double(&A_inspected2,f2); /** area features inspected ***/
      SEEIT_fread_double(&L_inspected2,f2); /** line features inspected ***/
      SEEIT_fread_double(&P_inspected2,f2); /** point features inspected ***/
      SEEIT_fread_double(&G_inspected2,f2); /** grid features inspected ***/


      for(i=0; i<= CONDITION_DEFINITIONS; i++)
         {
         OMF2[i].keyval = -1; /** int  **/
         OMF2[i].e_count = 0; /** int  **/
         OMF2[i].ign_count = 0; /*** int ***/
         OMF2[i].numactive = 0; /** int  **/
         OMF2[i].usemagnitude = -1; /*** int ***/
         OMF2[i].numthreshold = -1; /*** int ***/
         OMF2[i].uom = NULL; /** char *  ***/
         OMF2[i].t1min = 0.0; /*** double ***/
         OMF2[i].t1max = 0.0; /*** double ***/
         OMF2[i].t2min = 0.0; /*** double ***/
         OMF2[i].t2max = 0.0; /*** double ***/
         OMF2[i].bigisworse = -1; /** int  **/
         OMF2[i].cond_min = 0.0; /*** double ***/
         OMF2[i].cond_max = 0.0; /*** double ***/
         OMF2[i].numFCODES = -1;  /*** int ***/
         OMF2[i].FCD = NULL; /*** struct FCODEdetails * ***/
         }

      for(arraysize = 0; arraysize < CONDITION_DEFINITIONS; arraysize++)
         {
         SEEIT_fread_int(&i,f2);  /**** this is the keyval from file ***/
/***printf("keyval just read is %d\n",i);***/
         if(i > CONDITION_DEFINITIONS)
            {
            sprintf(answerstring,"Error in condition report comparison - condition key value is higher than curent limit\n");
            printf("place 2: key values read %d Condition Definitions limit %d\n",i,CONDITION_DEFINITIONS);
            file_endianness = save_endian;
            return(answerstring);
            }
         OMF2[i].keyval = i;
         SEEIT_fread_int(&OMF2[i].e_count,f2);
      
         SEEIT_fread_int(&OMF2[i].numactive,f2);
         SEEIT_fread_int(&OMF2[i].usemagnitude,f2);
         SEEIT_fread_int(&OMF2[i].numthreshold,f2);
         SEEIT_fread_int(&j,f2); /** strlen for uom string ***/
         OMF2[i].uom = (char *) (malloc(j + 2));
         fread(&OMF2[i].uom[0],j,1,f2);
         OMF2[i].uom[j] = '\0';
         SEEIT_fread_double(&OMF2[i].t1min,f2);
         SEEIT_fread_double(&OMF2[i].t1max,f2);
         SEEIT_fread_double(&OMF2[i].t2min,f2);
         SEEIT_fread_double(&OMF2[i].t2max,f2);

         if(OMF2[i].e_count > 0)
            {
            SEEIT_fread_int(&OMF2[i].bigisworse,f2);
            SEEIT_fread_double(&OMF2[i].cond_min,f2);
            SEEIT_fread_double(&OMF2[i].cond_max,f2);
            SEEIT_fread_int(&j,f2);  /*** number of FCODE entries that follow ***/
            OMF2[i].numFCODES = j;
            OMF2[i].FCD = (struct FCODEdetails *) (malloc(sizeof(struct FCODEdetails) * (j+1)));
            if(OMF2[i].FCD == NULL)
               {
               printf("available memory has been consumed during condition report comarison data read operation\n");
               printf("execution must now terminate\n");
               exit(-1);
               }
            for(k=0; k<j; k++)
               {
               SEEIT_fread_int(&ti1,f2);
               OMF2[i].FCD[k].geom = (char *) (malloc(ti1+1));
               fread(&OMF2[i].FCD[k].geom[0],ti1,1,f2);
               OMF2[i].FCD[k].geom[ti1] = '\0';
   
               if(NGA_TYPE == 1)
                  {
                  SEEIT_fread_int(&ti1,f2);
                  OMF2[i].FCD[k].ECCcode = (char *) (malloc(ti1+1));
                  fread(&OMF2[i].FCD[k].ECCcode[0],ti1,1,f2);
                  OMF2[i].FCD[k].ECCcode[ti1] = '\0';
                  }
               else
                  {
                  OMF2[i].FCD[k].ECCcode = (char *) (malloc(2));
                  strcpy(OMF2[i].FCD[k].ECCcode," ");
                  }

   
               SEEIT_fread_int(&ti1,f2);
               OMF2[i].FCD[k].ECCname = (char *) (malloc(ti1+1));
               fread(&OMF2[i].FCD[k].ECCname[0],ti1,1,f2);
               OMF2[i].FCD[k].ECCname[ti1] = '\0';

               SEEIT_fread_int(&OMF2[i].FCD[k].number,f2);
               }
            }
         } /*** end for arraysize ***/

      SEEIT_fread_int(&Meta2.numconditions,f2);

      for(i=0; i<CONDITION_DEFINITIONS; i++)
         {
         KDFlags[i] = 0;
         }

      if(Meta2.numconditions > 0)
         Meta2.numignore =  ConsultLK2file(f2name, KDFlags);
      else
         Meta2.numignore = 0;

      if(Meta2.numignore > 0)
         {
         for(i=0; i<CONDITION_DEFINITIONS; i++)
            {
            OMF2[i].ign_count = KDFlags[i];
            }
         }

      SEEIT_fread_int(&Meta2.numobjects,f2);
      SEEIT_fread_int(&Meta2.numelevobj,f2);
      SEEIT_fread_int(&Meta2.numelevp,f2);
      Meta2.numobjects -= Meta2.numelevobj;
      SEEIT_fread_int(&Meta2.numinstances,f2); /** written from I_Applied variable value ***/


      }

   file_endianness = save_endian;

   col1start = 40;
   col2start = 67;

   answerstring[0] = '\0';

   if(OMF2 != NULL)  /*** do the comparison at this point ***/
      {
      strcpy(lineout,"   (Note: lines beginning with '*' denote a difference in the compared data)\n\n");
      answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
      /**sprintf(outfile,"%sReportsCompared.txt",outdirectory);
      fout = fopen(outfile,"wt");**/
      if(verbosity >= 0)
         {
         jj = 0;
         while(Meta1.project[jj] >= ' ')
            ++jj;
         kk = 0;
         while(Meta2.project[kk] >= ' ')
            ++kk;
         if(strcmp(&Meta1.project[jj],&Meta2.project[kk]) == 0)
            {
            sprintf(lineout,"First Analysis:\n  %s\n",Meta1.project);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            if(strcmp(f1type,f2type) == 0)
               sprintf(lineout,"  inspection performed: %s\n\n",f1type);
            else
               sprintf(lineout,"*  inspection performed: %s\n\n",f1type);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            
            sprintf(lineout,"Second Analysis:\n  %s\n",Meta2.project);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            if(strcmp(f1type,f2type) == 0)
               sprintf(lineout,"  inspection performed: %s\n\n",f2type);
            else
               sprintf(lineout,"*  inspection performed: %s\n\n",f2type);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }
         else
            {
            sprintf(lineout,"* First Analysis:\n  %s\n",Meta1.project);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            if(strcmp(f1type,f2type) == 0)
               sprintf(lineout,"  inspection performed: %s\n\n",f1type);
            else
               sprintf(lineout,"*  inspection performed: %s\n\n",f1type);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

            sprintf(lineout,"* Second Analysis:\n  %s\n",Meta2.project);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            if(strcmp(f1type,f2type) == 0)
               sprintf(lineout,"  inspection performed: %s\n\n",f2type);
            else
               sprintf(lineout,"*  inspection performed: %s\n\n",f2type);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }
         if(answer < -2)
            {
            sprintf(lineout,"*These projects have different extents and / or counts of features\n");
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }
         else
            {
            sprintf(lineout," These projects have the same extents and number of features\n");
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }

         if((daydiff == 0) && (hrdiff == 0) && (mindiff == 0) && (time_dif == 0))
            {
            sprintf(lineout," First analysis date / time:   %s",ctime(&Meta1.inspect_time));
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            sprintf(lineout," Second analysis date / time:   %s",ctime(&Meta2.inspect_time));
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }
         else
            {
            sprintf(lineout," First analysis date / time:   %s",ctime(&Meta1.inspect_time));
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            sprintf(lineout," Second analysis date / time:   %s",ctime(&Meta2.inspect_time));
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            sprintf(lineout,"  Time between analyses: %d days %d hours %d minutes %ld seconds\n",daydiff,hrdiff,mindiff,time_dif);
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }

         sprintf(lineout,"Condition reports:\n    ");
         strcat(lineout,f1name);
         strcat(lineout,"\n    ");
         strcat(lineout,f2name);
         strcat(lineout,"\n");
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


         for(sl = 0; sl <= col1start; sl++)
            lineout[sl] = ' ';
         sl2 = strlen(f1name);
         --sl2;
         while((sl2 >= 0) && (f1name[sl2] != '\\') && (f1name[sl2] != '/'))
            --sl2;
         strcpy(&lineout[col1start],&f1name[sl2+1]);
         sl2 = strlen(lineout);
         if(sl2 < (col2start -1))
            {
            for(sl = sl2; sl < col2start; sl++)
               lineout[sl] = ' ';
            sl = strlen(f2name);
            while((sl >= 0) && (f2name[sl] != '\\') && (f2name[sl] != '/'))
               --sl;
            strcpy(&lineout[col2start],&f2name[sl+1]);
            strcat(lineout,"\n");
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }
         else
            {
            strcat(lineout,"\n");
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            for(sl = 0; sl < col2start; sl++)
               lineout[sl] = ' ';
            sl = strlen(f2name);
            while((sl >= 0) && (f2name[sl] != '\\') && (f2name[sl] != '/'))
               --sl;
            strcpy(&lineout[col2start],&f2name[sl+1]);
            strcat(lineout,"\n");
            answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            }

         if(strcmp(P1PointFeatures,P2PointFeatures) == 0)
            {
            strcpy(lineout," Point features in project: ");
            }
         else
            {
            strcpy(lineout,"*Point features in project: ");
            }
         sscanf(P1PointFeatures,"%d",&sl1);
         TtlPts1 = sl1;
         sscanf(P2PointFeatures,"%d",&sl1);
         TtlPts2 = sl1;
         sprintf(proj1,"%s",P1PointFeatures);
         sprintf(proj2,"%s",P2PointFeatures);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(strcmp(P1LineFeatures,P2LineFeatures) == 0)
            {
            strcpy(lineout," Line features in project: ");
            }
         else
            {
            strcpy(lineout,"*Line features in project: ");
            }
         sscanf(P1LineFeatures,"%d",&sl1);
         TtlL1 = sl1;
         sscanf(P2LineFeatures,"%d",&sl1);
         TtlL2 = sl1;
         sprintf(proj1,"%s",P1LineFeatures);
         sprintf(proj2,"%s",P2LineFeatures);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(strcmp(P1AreaFeatures,P2AreaFeatures) == 0)
            {
            strcpy(lineout," Area features in project: ");
            }
         else
            {
            strcpy(lineout,"*Area features in project: ");
            }
         sscanf(P1AreaFeatures,"%d",&sl1);
         TtlA1 = sl1;
         sscanf(P2AreaFeatures,"%d",&sl1);
         TtlA2 = sl1;
         sprintf(proj1,"%s",P1AreaFeatures);
         sprintf(proj2,"%s",P2AreaFeatures);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         sl1 = TtlPts1 + TtlL1 + TtlA1;
         sl2 = TtlPts2 + TtlL2 + TtlA2;
         if(sl1 == sl2)
            strcpy(lineout," Total Features in project: ");
         else
            strcpy(lineout,"*Total Features in project: ");
         sprintf(proj1,"%d",sl1);
         sprintf(proj2,"%d",sl2);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(strcmp(P1ElevPosts,P2ElevPosts) == 0)
            {
            strcpy(lineout," Elevation posts in project: ");
            }
         else
            {
            strcpy(lineout,"*Elevation posts in project: ");
            }
         sprintf(proj1,"%s",P1ElevPosts);
         sprintf(proj2,"%s",P2ElevPosts);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
         
         if(P_inspected1 == P_inspected2)
            {
            strcpy(lineout," Point features inspected: ");
            }
         else
            {
            strcpy(lineout,"*Point features inspected: ");
            }
         sscanf(P1PointFeatures,"%d",&sl1);
         TtlPts1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (P_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",P_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sscanf(P2PointFeatures,"%d",&sl1);
         TtlPts2 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (P_inspected2) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj2,"%.0lf (%.1lf%%)",P_inspected2,dbltemp);
            }
         else
            strcpy(proj2,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(L_inspected1 == L_inspected2)
            {
            strcpy(lineout," Line features inspected: ");
            }
         else
            {
            strcpy(lineout,"*Line features inspected: ");
            }
         sscanf(P1LineFeatures,"%d",&sl1);
         TtlL1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (L_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",L_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sscanf(P2LineFeatures,"%d",&sl1);
         TtlL2 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (L_inspected2) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj2,"%.0lf (%.1lf%%)",L_inspected2,dbltemp);
            }
         else
            strcpy(proj2,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(A_inspected1 == A_inspected2)
            {
            strcpy(lineout," Area features inspected: ");
            }
         else
            {
            strcpy(lineout,"*Area features inspected: ");
            }
         sscanf(P1AreaFeatures,"%d",&sl1);
         TtlA1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (A_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",A_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sscanf(P2AreaFeatures,"%d",&sl1);
         TtlA2 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (A_inspected2) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj2,"%.0lf (%.1lf%%)",A_inspected2,dbltemp);
            }
         else
            strcpy(proj2,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(G_inspected1 == G_inspected2)
            {
            strcpy(lineout," Elevation posts inspected: ");
            }
         else
            {
            strcpy(lineout,"*Elevation posts inspected: ");
            }
         sscanf(P1ElevPosts,"%d",&sl1);
         TtlEP1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (G_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",G_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sscanf(P2ElevPosts,"%d",&sl1);
         TtlEP2 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (G_inspected2) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj2,"%.0lf (%.1lf%%)",G_inspected2,dbltemp);
            }
         else
            strcpy(proj2,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(strcmp(Meta1.fromcoord,Meta2.fromcoord) == 0)
            {
            strcpy(lineout," Inspection area S/W start location: ");
            sprintf(proj1,"%s",Meta1.fromcoord);
            sprintf(proj2,"%s",Meta2.fromcoord);
            }
         else
            {
            strcpy(lineout,"*Inspection area S/W start location: ");
            sprintf(proj1,"%s",Meta1.fromcoord);
            sprintf(proj2,"%s",Meta2.fromcoord);
            }
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(strcmp(Meta1.tocoord,Meta2.tocoord) == 0)
            {
            strcpy(lineout," Inspection area N/E end location: ");
            }
         else
            {
            strcpy(lineout,"*Inspection area N/E end location: ");
            }
         sprintf(proj1,"%s",Meta1.tocoord);
         sprintf(proj2,"%s",Meta2.tocoord);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


         if(Meta1.numinstances == Meta2.numinstances)
            {
            strcpy(lineout," Inspection instances applied: ");
            }
         else
            {
            strcpy(lineout,"*Inspection instances applied: ");
            }
         sprintf(proj1,"%d",Meta1.numinstances);
         sprintf(proj2,"%d",Meta2.numinstances);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(Meta1.numconditions == Meta2.numconditions)
            {
            strcpy(lineout," Conditions identified: ");
            }
         else
            {
            strcpy(lineout,"*Conditions identified: ");
            }
         sprintf(proj1,"%d",Meta1.numconditions);
         sprintf(proj2,"%d",Meta2.numconditions);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(Meta1.numignore == Meta2.numignore)
            {
            strcpy(lineout," Marked to IGNORE: ");
            }
         else
            {
            strcpy(lineout,"*Marked to IGNORE: ");
            }
         sprintf(proj1,"%d",Meta1.numignore);
         sprintf(proj2,"%d",Meta2.numignore);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(Meta1.numignore == Meta2.numignore)
            {
            strcpy(lineout," Marked to RETAIN: ");
            }
         else
            {
            strcpy(lineout,"*Marked to RETAIN: ");
            }
         sprintf(proj1,"%d",Meta1.numconditions - Meta1.numignore);
         sprintf(proj2,"%d",Meta2.numconditions - Meta2.numignore);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


         if(Meta1.numobjects == Meta2.numobjects)
            {
            strcpy(lineout," Features with conditions: ");
            }
         else
            {
            strcpy(lineout,"*Features with conditions: ");
            }
         tdbl = P_inspected1 + L_inspected1 + A_inspected1;
         if(tdbl > 0)
            {
            dbltemp = ((double) Meta1.numobjects) / tdbl;
            dbltemp *= 100.0;
            sprintf(proj1,"%d (%.2lf%%)",Meta1.numobjects,dbltemp);
            }
         else
            strcpy(proj1, "N/A");

         tdbl = P_inspected2 + L_inspected2 + A_inspected2;
         if(tdbl > 0)
            {
            dbltemp = ((double) Meta2.numobjects) / tdbl;
            dbltemp *= 100.0;
            sprintf(proj2,"%d (%.2lf%%)",Meta2.numobjects,dbltemp);
            }
         else
            strcpy(proj2, "N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         if(Meta1.numelevp == Meta2.numelevp)
            {
            strcpy(lineout," Elevation posts with conditions: ");
            }
         else
            {
            strcpy(lineout,"*Elevation posts with conditions: ");
            }
         if(TtlEP1 > 0)
            {
            dbltemp = ((double) Meta1.numelevp) / ((double) TtlEP1);
            dbltemp *= 100.0;
            sprintf(proj1,"%d (%.2lf%%)",Meta1.numelevp,dbltemp);
            }
         else
            strcpy(proj1, "N/A");

         if(TtlEP2 > 0)
            {
            dbltemp = ((double) Meta2.numelevp) / ((double) TtlEP2);
            dbltemp *= 100.0;
            sprintf(proj2,"%d (%.2lf%%)",Meta2.numelevp,dbltemp);
            }
         else
            strcpy(proj2, "N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col2start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col2start],proj2);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


         if(verbosity > 1)
            {
            if((Meta1.numconditions > 0) || (Meta2.numconditions > 0))
               {
               sprintf(lineout,"\n  Results by inspection type:\n");
               answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
               }
            for(i=0; i<= CONDITION_DEFINITIONS; i++)
               {
               if((OMF1[i].e_count > 0) || (OMF2[i].e_count > 0))
                  {
                  sprintf(lineout,"  ");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                  j = 0;
                  while(ErrorLookup[i].name[j] != '\0')
                     {
                     if(ErrorLookup[i].name[j] >= ' ')
                        {
                        sprintf(lineout,"%c",ErrorLookup[i].name[j]);
                        answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                        }
                     else
                        {
                        sprintf(lineout," ");
                        answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                        }
                     ++j;
                     }
     
                  sprintf(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                  if(OMF1[i].e_count == OMF2[i].e_count)
                     {
                     strcpy(lineout,"     Conditions identified: ");
                     }
                  else
                     {
                     strcpy(lineout,"*    Conditions identified: ");
                     }
                  if(OMF1[i].numactive > 0)
                     sprintf(proj1,"%d",OMF1[i].e_count);
                  else
                     strcpy(proj1,"N/A");
                  if(OMF2[i].numactive > 0)
                     sprintf(proj2,"%d",OMF2[i].e_count);
                  else
                     strcpy(proj2,"N/A");
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col1start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col1start],proj1);
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col2start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col2start],proj2);
                  strcat(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                  if(OMF1[i].ign_count == OMF2[i].ign_count)
                     {
                     strcpy(lineout,"     Marked to IGNORE: ");
                     }
                  else
                     {
                     strcpy(lineout,"*    Marked to IGNORE: ");
                     }
                  if(OMF1[i].numactive > 0)
                     sprintf(proj1,"%d",OMF1[i].ign_count);
                  else
                     strcpy(proj1,"N/A");
                  if(OMF2[i].numactive > 0)
                     sprintf(proj2,"%d",OMF2[i].ign_count);
                  else
                     strcpy(proj2,"N/A");
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col1start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col1start],proj1);
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col2start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col2start],proj2);
                  strcat(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                  if(OMF1[i].ign_count == OMF2[i].ign_count)
                     {
                     strcpy(lineout,"     Marked to RETAIN: ");
                     }
                  else
                     {
                     strcpy(lineout,"*    Marked to RETAIN: ");
                     }
                  if(OMF1[i].numactive > 0)
                     sprintf(proj1,"%d",OMF1[i].e_count - OMF1[i].ign_count);
                  else
                     strcpy(proj1,"N/A");
                  if(OMF2[i].numactive > 0)
                     sprintf(proj2,"%d",OMF2[i].e_count - OMF2[i].ign_count);
                  else
                     strcpy(proj2,"N/A");
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col1start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col1start],proj1);
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col2start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col2start],proj2);
                  strcat(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


                  if(verbosity > 2)
                     {
                     if(OMF1[i].numactive == OMF2[i].numactive)
                        {
                        strcpy(lineout,"     Active inspection instances: ");
                        }
                     else
                        {
                        strcpy(lineout,"*    Active inspection instances: ");
                        }
                     sprintf(proj1,"%d",OMF1[i].numactive);
                     sprintf(proj2,"%d",OMF2[i].numactive);
                     sl2 = strlen(lineout);
                     for(sl = sl2; sl < col1start; sl++)
                        lineout[sl] = ' ';
                     strcpy(&lineout[col1start],proj1);
                     sl2 = strlen(lineout);
                     for(sl = sl2; sl < col2start; sl++)
                        lineout[sl] = ' ';
                     strcpy(&lineout[col2start],proj2);
                     strcat(lineout,"\n");
                     answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


                     if(ErrorLookup[i].numthresholds > 0)
                        {
                        if(ErrorLookup[i].numthresholds >= 1)
                           {
                           if(OMF1[i].t1min == OMF1[i].t1max)
                              {
                              if(OMF2[i].numactive == 0)
                                 {
                                 sprintf(lineout,"*    Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t1min);
                                 strcpy(proj2,"N/A");
                                 }
                              else if((OMF2[i].t1min == OMF2[i].t1max) && (OMF1[i].t1min == OMF2[i].t1min))
                                 {
                                 sprintf(lineout,"     Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t1min);
                                 sprintf(proj2,"%lf",OMF2[i].t1min);
                                 }
                              else if(OMF2[i].t1min == OMF2[i].t1max)
                                 {
                                 sprintf(lineout,"*    Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t1min);
                                 sprintf(proj2,"%lf",OMF2[i].t1min);
                                 }
                              else
                                 {
                                 sprintf(lineout,"*    Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t1min);
                                 sprintf(proj2,"%lf to %lf", OMF2[i].t1min, OMF2[i].t1max);
                                 }
                              }
                           else
                              {
                              if(OMF2[i].numactive == 0)
                                 {
                                 sprintf(lineout,"*    Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t1min, OMF1[i].t1max);
                                 strcpy(proj2,"N/A");
                                 }
                              else if((OMF1[i].t1min == OMF2[i].t1min) && (OMF1[i].t1max == OMF2[i].t1max))
                                 {
                                 sprintf(lineout,"     Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t1min, OMF1[i].t1max);
                                 sprintf(proj2,"%lf to %lf", OMF2[i].t1min, OMF2[i].t1max);
                                 }
                              else if(OMF2[i].t1min == OMF2[i].t1max)
                                 {
                                 sprintf(lineout,"*    Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t1min, OMF1[i].t1max);
                                 sprintf(proj2,"%lf", OMF2[i].t1min);
                                 }
                              else
                                 {
                                 sprintf(lineout,"*    Primary tolerance (UOM: %s): ",OMF1[i].uom);
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t1min, OMF1[i].t1max);
                                 sprintf(proj2,"%lf to %lf", OMF2[i].t1min, OMF2[i].t1max);
                                 }
                              }
                           sl2 = strlen(lineout);
                           for(sl = sl2; sl < col1start; sl++)
                              lineout[sl] = ' ';
                           strcpy(&lineout[col1start],proj1);
                           sl2 = strlen(lineout);
                           for(sl = sl2; sl < col2start; sl++)
                              lineout[sl] = ' ';
                           strcpy(&lineout[col2start],proj2);
                           strcat(lineout,"\n");
                           answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                           }
   
                        if(ErrorLookup[i].numthresholds > 1)
                           {
                           if(OMF1[i].t2min == OMF1[i].t2max)
                              {
                              if(OMF2[i].numactive == 0)
                                 {
                                 strcpy(lineout,"*    Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t2min);
                                 strcpy(proj2,"N/A");
                                 }
                              else if((OMF2[i].t2min == OMF2[i].t2max) && (OMF1[i].t2min == OMF2[i].t2min))
                                 {
                                 strcpy(lineout,"     Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t2min);
                                 sprintf(proj2,"%lf",OMF2[i].t2min);
                                 }
                              else if(OMF2[i].t2min == OMF2[i].t2max)
                                 {
                                 strcpy(lineout,"*    Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t2min);
                                 sprintf(proj2,"%lf",OMF2[i].t2min);
                                 }
                              else
                                 {
                                 strcpy(lineout,"*    Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf",OMF1[i].t2min);
                                 sprintf(proj2,"%lf to %lf", OMF2[i].t2min, OMF2[i].t2max);
                                 }
                              }
                           else
                              {
                              if(OMF2[i].numactive == 0)
                                 {
                                 strcpy(lineout,"*    Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t2min, OMF1[i].t2max);
                                 strcpy(proj2,"N/A");
                                 }
                              else if((OMF1[i].t2min == OMF2[i].t2min) && (OMF1[i].t2max == OMF2[i].t2max))
                                 {
                                 strcpy(lineout,"     Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t2min, OMF1[i].t2max);
                                 sprintf(proj2,"%lf to %lf", OMF2[i].t2min, OMF2[i].t2max);
                                 }
                              else if(OMF2[i].t2min == OMF2[i].t2max)
                                 {
                                 strcpy(lineout,"*    Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t2min, OMF1[i].t2max);
                                 sprintf(proj2,"%lf", OMF2[i].t2min);
                                 }
                              else
                                 {
                                 strcpy(lineout,"*    Secondary tolerance: ");
                                 if(OMF1[i].numactive == 0)
                                    strcpy(proj1,"N/A");
                                 else
                                    sprintf(proj1,"%lf to %lf", OMF1[i].t2min, OMF1[i].t2max);
                                 sprintf(proj2,"%lf to %lf", OMF2[i].t2min, OMF2[i].t2max);
                                 }
                              }
                           sl2 = strlen(lineout);
                           for(sl = sl2; sl < col1start; sl++)
                              lineout[sl] = ' ';
                           strcpy(&lineout[col1start],proj1);
                           sl2 = strlen(lineout);
                           for(sl = sl2; sl < col2start; sl++)
                              lineout[sl] = ' ';
                           strcpy(&lineout[col2start],proj2);
                           strcat(lineout,"\n");
                           answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                           }
                        }


                     if(OMF1[i].usemagnitude > 0)
                        {
                        if((SameValueWithinTolerance(OMF1[i].cond_min, OMF2[i].cond_min, 0.00001) > 0) &&
                              (SameValueWithinTolerance(OMF1[i].cond_max, OMF2[i].cond_max,0.00001) > 0))
                           {
                           sprintf(lineout,"     Condition range: ");
                           }
                        else
                           {
                           sprintf(lineout,"*    Condition range: ");
                           }
                        if(OMF1[i].numactive == 0)
                           strcpy(proj1,"N/A");
                        else
                           sprintf(proj1,"%lf to %lf",OMF1[i].cond_min,OMF1[i].cond_max);
                        if(OMF2[i].e_count > 0)
                           sprintf(proj2,"%lf to %lf",OMF2[i].cond_min,OMF2[i].cond_max);
                        else
                           strcpy(proj2,"N/A");
                        sl2 = strlen(lineout);
                        for(sl = sl2; sl < col1start; sl++)
                           lineout[sl] = ' ';
                        strcpy(&lineout[col1start],proj1);
                        sl2 = strlen(lineout);
                        for(sl = sl2; sl < col2start; sl++)
                           lineout[sl] = ' ';
                        strcpy(&lineout[col2start],proj2);
                        strcat(lineout,"\n");
                        answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                        }
                     if(verbosity > 3)
                        {
                        ONroot = NULL;
                        if((OMF1[i].numFCODES > 0) || (OMF2[i].numFCODES > 0))
                           {
                           for(j=0; j< OMF1[i].numFCODES; j++)
                              {
                              ONc = (struct OrderNames *) (malloc(SzON));
                              if(ONc == NULL)
                                 {
                                 printf("all available memory has been used - terminating now!\n");
                                 exit(-1);
                                 }
                              ONc->OneOrTwo = 1;
                              ONc->index = j;
                              ONc->pairindex = -1;
                              ONc->next = NULL;
                              if(ONroot == NULL)
                                 ONroot = ONc;
                              else
                                 {
                                 ONn = ONp = ONroot;
                                 while(ONn != NULL)
                                    {
                                    sl1 = strcmp(OMF1[i].FCD[ONn->index].ECCname, OMF1[i].FCD[ONc->index].ECCname);
                                    if(sl1 < 0)
                                       {
                                       ONp = ONn;
                                       ONn = ONn->next;
                                       }
                                    else if(sl1 >= 0)
                                       {
                                       if(sl1 == 0)
                                          {
                                          if(strcmp(OMF1[i].FCD[ONn->index].geom, OMF1[i].FCD[ONc->index].geom) == 0)
                                             {
                                             OMF1[i].FCD[ONn->index].number += OMF1[i].FCD[ONc->index].number;
                                             OMF1[i].FCD[ONc->index].number = 0;
                                             free(ONc);
                                             }
                                          else
                                             {
                                             if(ONn == ONroot)
                                                {
                                                ONc->next = ONroot;
                                                ONroot = ONc;
                                                }
                                             else
                                                {
                                                ONp->next = ONc;
                                                ONc->next = ONn;
                                                }
                                             }
                                          }
                                       else
                                          {
                                          if(ONn == ONroot)
                                             {
                                             ONc->next = ONroot;
                                             ONroot = ONc;
                                             }
                                          else
                                             {
                                             ONp->next = ONc;
                                             ONc->next = ONn;
                                             }
                                          }
                                       break;
                                       }
                                    }
                                 if(ONn == NULL)
                                    {
                                    ONp->next = ONc;
                                    ONc->next = NULL;
                                    }
                                 }
                              }
                           for(j=0; j< OMF2[i].numFCODES; j++)
                              {
                              ONc = (struct OrderNames *) (malloc(SzON));
                              if(ONc == NULL)
                                 {
                                 printf("all available memory has been used - terminating now!\n");
                                 exit(-1);
                                 }
                              ONc->OneOrTwo = 2;
                              ONc->index = j;
                              ONc->pairindex = -1;
                              ONc->next = NULL;
                              if(ONroot == NULL)
                                 ONroot = ONc;
                              else
                                 {
                                 ONn = ONp = ONroot;
                                 while(ONn != NULL)
                                    {
                                    if(ONn->OneOrTwo == 1)
                                       {
                                       sl1 = strcmp(OMF1[i].FCD[ONn->index].ECCname, OMF2[i].FCD[ONc->index].ECCname);
                                       if(sl1 == 0)
                                          {
                                          sl1 = strcmp(OMF1[i].FCD[ONn->index].geom, OMF2[i].FCD[ONc->index].geom);
                                          if(sl1 != 0)
                                             sl1 = -10;
                                          }
                                       }
                                    else
                                       {
                                       sl1 = strcmp(OMF2[i].FCD[ONn->index].ECCname, OMF2[i].FCD[ONc->index].ECCname);
                                       if(sl1 == 0)
                                          {
                                          sl1 = strcmp(OMF2[i].FCD[ONn->index].geom, OMF2[i].FCD[ONc->index].geom);
                                          if(sl1 != 0)
                                             sl1 = -10;
                                          }
                                       }
                                    if(sl1 < 0)
                                       {
                                       ONp = ONn;
                                       ONn = ONn->next;
                                       }
                                    else if(sl1 == 0)
                                       {
                                       if(ONn->OneOrTwo == 1)
                                          {
                                          if(ONn->pairindex == -1)
                                             ONn->pairindex = j;
                                          else
                                             {
                                             OMF2[i].FCD[ONn->pairindex].number += OMF2[i].FCD[ONc->index].number;
                                             OMF2[i].FCD[ONc->index].number = 0;
                                             }
                                          }
                                       else
                                          {
                                          OMF2[i].FCD[ONn->pairindex].number += OMF2[i].FCD[ONc->index].number;
                                          OMF2[i].FCD[ONc->index].number = 0;
                                          }
                                       free(ONc);
                                       break;
                                       }
                                    else if(sl1 > 0)
                                       {
                                       if(ONn == ONroot)
                                          {
                                          ONc->next = ONroot;
                                          ONroot = ONc;
                                          }
                                       else
                                          {
                                          ONp->next = ONc;
                                          ONc->next = ONn;
                                          }
                                       break;
                                       }
                                    }
                                 if(ONn == NULL)
                                    {
                                    ONp->next = ONc;
                                    ONc->next = NULL;
                                    }
                                 }
                              }
                           }
                           


                        if(ONroot != NULL)
                           {
                           ONc = ONroot;
                           while(ONc != NULL)
                              {
                              j = ONc->index;
                              k = ONc->pairindex;
                              if(ONc->OneOrTwo == 1)
                                 {
                                 if(k >= 0)
                                    {
                                    if(OMF1[i].FCD[j].number == OMF2[i].FCD[k].number)
                                       {
                                       sprintf(lineout,"       %s %s %s:",OMF1[i].FCD[j].geom,OMF1[i].FCD[j].ECCcode, OMF1[i].FCD[j].ECCname);
                                       }
                                    else
                                       {
                                       sprintf(lineout,"*      %s %s %s:",OMF1[i].FCD[j].geom,OMF1[i].FCD[j].ECCcode, OMF1[i].FCD[j].ECCname);
                                       }
                                    sprintf(proj1,"%d",OMF1[i].FCD[j].number);
                                    sprintf(proj2,"%d",OMF2[i].FCD[k].number);
                                    }
                                 else
                                    {
                                    sprintf(lineout,"*      %s %s %s:",OMF1[i].FCD[j].geom,OMF1[i].FCD[j].ECCcode, OMF1[i].FCD[j].ECCname);
                                    sprintf(proj1,"%d",OMF1[i].FCD[j].number);
                                    strcpy(proj2,"0");
                                    }
                                 }
                              else
                                 {
                                 if(k >= 0)
                                    {
                                    if(OMF2[i].FCD[j].number == OMF1[i].FCD[k].number)
                                       {
                                       sprintf(lineout,"       %s %s %s:",OMF2[i].FCD[j].geom,OMF2[i].FCD[j].ECCcode, OMF2[i].FCD[j].ECCname);
                                       }
                                    else
                                       {
                                       sprintf(lineout,"*      %s %s %s:",OMF2[i].FCD[j].geom,OMF2[i].FCD[j].ECCcode, OMF2[i].FCD[j].ECCname);
                                       }
                                    sprintf(proj2,"%d",OMF2[i].FCD[j].number);
                                    sprintf(proj1,"%d",OMF1[i].FCD[k].number);
                                    }
                                 else
                                    {
                                    sprintf(lineout,"*      %s %s %s:",OMF2[i].FCD[j].geom,OMF2[i].FCD[j].ECCcode, OMF2[i].FCD[j].ECCname);
                                    sprintf(proj2,"%d",OMF2[i].FCD[j].number);
                                    strcpy(proj1,"0");
                                    }
                                 }
                        
                              sl2 = strlen(lineout);
                              while(sl2 > (col1start - 2))
                                 {
                                 sl1 = col1start - 1;
                                 while((sl1 > 6) && (lineout[sl1] != ' ') && (lineout[sl1] != '/'))
                                    --sl1;
                                 if(sl1 <= 6)
                                    {
                                    ii = 0;
                                    sl1 = 0;
                                    while(sl1 < (col1start - 2))
                                       {
                                       lo2[ii] = lineout[sl1];
                                       ++sl1;
                                       ++ii;
                                       }
                                    lo2[ii] = '\n';
                                    ++ii;
                                    lo2[ii] = '\0';
                                
                                    answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lo2);
                                    lineout[7] = ' ';
                                    lineout[8] = ' ';
                                    sl = 9;
                                    while(sl1 <= sl2)
                                       {
                                       lineout[sl] = lineout[sl1];
                                       ++sl;
                                       ++sl1;
                                       }
                                    }
                                 else
                                    {
                                    sl = 0;
                                    ii = 0;
                                    while(sl <= sl1)
                                       {
                                       lo2[ii] = lineout[sl];
                                       ++sl;
                                       ++ii;
                                       }

                                    lo2[ii] = '\n';
                                    ++ii;
                                    lo2[ii] = '\0';
                                    answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lo2);
                                    lineout[7] = ' ';
                                    lineout[8] = ' ';
                                    sl = 9;
                                    ++sl1;
                                    while(sl1 <= sl2)
                                       {
                                       lineout[sl] = lineout[sl1];
                                       ++sl;
                                       ++sl1;
                                       }
                                    }
                                 sl2 = strlen(lineout);
                                 }
                              for(sl = sl2; sl < col1start; sl++)
                                 lineout[sl] = ' ';
                              strcpy(&lineout[col1start],proj1);
                              sl2 = strlen(lineout);
                              for(sl = sl2; sl < col2start; sl++)
                                 lineout[sl] = ' ';
                              strcpy(&lineout[col2start],proj2);
                              strcat(lineout,"\n");
                              answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                        
                              ONc = ONc->next;
                              }
                           }

                           
                        if(ONroot != NULL)
                           {
                           ONc = ONroot;
                           while(ONc != NULL)
                              {
                              ONp = ONc;
                              ONc = ONc->next;
                              free(ONp);
                              }
                           ONroot = NULL;
                           }
                        } /*** end if(verbosity > 3) ***/
                     } /*** if(verbosity > 2) ***/
                  } /**** if((OMF1[i].e_count > 0) || (OMF2[i].e_count > 0)) ***/
               }  /*** for(i=0; i<= CONDITION_DEFINITIONS; i++) ****/
            } /*** end if(verbosity > 1) ***/
         } /*** end verbosity parameter is OK ***/
      } /*** end if(OMF2 != NULL) so are really comparing two files ***/

   else /*** just want the summary info for a single file as in OMF1 and Meta1 ***/
      {
      if(verbosity >= 0)
         {
         sprintf(lineout,"Analysis of:\n  %s\n",Meta1.project);
         /**strcat(lineout,"\n"); **/
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
         sprintf(lineout,"  inspection performed: %s\n\n",f1type);
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
         sprintf(lineout,"Condition report:\n    ");
         strcat(lineout,f1name);
         strcat(lineout,"\n");
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Point features in project: ");
         sprintf(proj1,"%s",P1PointFeatures);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Line features in project: ");
         sprintf(proj1,"%s",P1LineFeatures);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Area features in project: ");
         sprintf(proj1,"%s",P1AreaFeatures);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Elevation posts in project: ");
         sprintf(proj1,"%s",P1ElevPosts);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
            
         strcpy(lineout," Point features inspected: ");
         sscanf(P1PointFeatures,"%d",&sl1);
         TtlPts1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = ( P_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",P_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Line features inspected: ");
         sscanf(P1LineFeatures,"%d",&sl1);
         TtlL1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (L_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",L_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Area features inspected: ");
         sscanf(P1AreaFeatures,"%d",&sl1);
         TtlA1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (A_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",A_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Elevation posts inspected: ");
         sscanf(P1ElevPosts,"%d",&sl1);
         TtlEP1 = sl1;
         if(sl1 > 0)
            {
            dbltemp = (G_inspected1) /((double) sl1);
            dbltemp = dbltemp * 100.0;
            sprintf(proj1,"%.0lf (%.1lf%%)",G_inspected1,dbltemp);
            }
         else
            strcpy(proj1,"N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         sprintf(lineout," Project inspection date / time:    %s",ctime(&Meta1.inspect_time));
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Inspection area S/W start location: ");
         sprintf(proj1,"%s",Meta1.fromcoord);

         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Inspection area N/E end location: ");
         sprintf(proj1,"%s",Meta1.tocoord);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


         strcpy(lineout," Inspection instances applied: ");
         sprintf(proj1,"%d",Meta1.numinstances);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Conditions identified: ");
         sprintf(proj1,"%d",Meta1.numconditions);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Marked to IGNORE: ");
         sprintf(proj1,"%d",Meta1.numignore);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Marked to RETAIN: ");
         sprintf(proj1,"%d",Meta1.numconditions - Meta1.numignore);
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Features with conditions: ");
         tdbl = P_inspected1 + L_inspected1 + A_inspected1;
         if(tdbl > 0)
            {
            dbltemp = ((double) Meta1.numobjects) / tdbl;
            dbltemp *= 100.0;
            sprintf(proj1,"%d (%.2lf%%)",Meta1.numobjects,dbltemp);
            }
         else
            strcpy(proj1, "N/A");
         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

         strcpy(lineout," Elevation posts with conditions: ");
         if(TtlEP1 > 0)
            {
            dbltemp = ((double) Meta1.numelevp) / ((double) TtlEP1);
            dbltemp *= 100.0;
            sprintf(proj1,"%d (%.2lf%%)",Meta1.numelevp,dbltemp);
            }
         else
            strcpy(proj1, "N/A");

         sl2 = strlen(lineout);
         for(sl = sl2; sl < col1start; sl++)
            lineout[sl] = ' ';
         strcpy(&lineout[col1start],proj1);
         strcat(lineout,"\n");
         answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);


         if(verbosity > 1)
            {
            if(Meta1.numconditions > 0)
               {
               sprintf(lineout,"\n  Results by inspection type:\n");
               answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
               }
            for(i=0; i<= CONDITION_DEFINITIONS; i++)
               {
               if(OMF1[i].e_count > 0)
                  {
                  sprintf(lineout,"  ");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                  j = 0;
                  while(ErrorLookup[i].name[j] != '\0')
                     {
                     if(ErrorLookup[i].name[j] >= ' ')
                        {
                        sprintf(lineout,"%c",ErrorLookup[i].name[j]);
                        answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                        }
                     else
                        {
                        sprintf(lineout," ");
                        answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                        }
                     ++j;
                     }

                  sprintf(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                  strcpy(lineout,"     Conditions identified: ");
                  sprintf(proj1,"%d",OMF1[i].e_count);
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col1start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col1start],proj1);
                  strcat(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                  strcpy(lineout,"     Marked to IGNORE: ");
                  sprintf(proj1,"%d",OMF1[i].ign_count);
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col1start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col1start],proj1);
                  strcat(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                  strcpy(lineout,"     Marked to RETAIN: ");
                  sprintf(proj1,"%d",OMF1[i].e_count - OMF1[i].ign_count);
                  sl2 = strlen(lineout);
                  for(sl = sl2; sl < col1start; sl++)
                     lineout[sl] = ' ';
                  strcpy(&lineout[col1start],proj1);
                  strcat(lineout,"\n");
                  answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                  if(verbosity > 2)
                     {
                     strcpy(lineout,"     Active inspection instances: ");
                     sprintf(proj1,"%d",OMF1[i].numactive);
                     sl2 = strlen(lineout);
                     for(sl = sl2; sl < col1start; sl++)
                        lineout[sl] = ' ';
                     strcpy(&lineout[col1start],proj1);
                     strcat(lineout,"\n");
                     answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                     if(ErrorLookup[i].numthresholds > 0)
                        {
                        if(OMF1[i].numthreshold >= 1)
                           {
                           if(OMF1[i].t1min == OMF1[i].t1max)
                              {
                              sprintf(lineout,"     Primary tolerance (UOM: %s): ",OMF1[i].uom);
                              sprintf(proj1,"%lf",OMF1[i].t1min);
                              }
                           else
                              {
                              sprintf(lineout,"     Primary tolerance (UOM: %s): ",OMF1[i].uom);
                              sprintf(proj1,"%lf to %lf", OMF1[i].t1min, OMF1[i].t1max);
                              }
                           sl2 = strlen(lineout);
                           for(sl = sl2; sl < col1start; sl++)
                              lineout[sl] = ' ';
                           strcpy(&lineout[col1start],proj1);
                           strcat(lineout,"\n");
                           answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                           }
   
                        if(OMF1[i].numthreshold > 1)
                           {
                           if(OMF1[i].t2min == OMF1[i].t2max)
                              {
                              strcpy(lineout,"     Secondary tolerance: ");
                              sprintf(proj1,"%lf",OMF1[i].t2min);
                              }
                           else
                              {
                              strcpy(lineout,"     Secondary tolerance: ");
                              sprintf(proj1,"%lf to %lf", OMF1[i].t2min, OMF1[i].t2max);
                              }
                           sl2 = strlen(lineout);
                           for(sl = sl2; sl < col1start; sl++)
                              lineout[sl] = ' ';
                           strcpy(&lineout[col1start],proj1);
                           strcat(lineout,"\n");
                           answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
                           }
                        }


                     if(OMF1[i].usemagnitude > 0)
                        {
                        sprintf(lineout,"     Condition range: ");
                        sprintf(proj1,"%lf to %lf",OMF1[i].cond_min,OMF1[i].cond_max);
                        sl2 = strlen(lineout);
                        for(sl = sl2; sl < col1start; sl++)
                           lineout[sl] = ' ';
                        strcpy(&lineout[col1start],proj1);
                        strcat(lineout,"\n");
                        answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);

                        }
                     if(verbosity > 3)
                        {
                        if(OMF1[i].numFCODES > 0)
                           {
                           ONroot = NULL;
                           for(j=0; j< OMF1[i].numFCODES; j++)
                              {
                              ONc = (struct OrderNames *) (malloc(SzON));
                              if(ONc == NULL)
                                 {
                                 printf("all available memory has been used - terminating now!\n");
                                 exit(-1);
                                 }
                              ONc->OneOrTwo = 1;
                              ONc->index = j; 
                              ONc->pairindex = -1;
                              ONc->next = NULL;
                              if(ONroot == NULL)
                                 ONroot = ONc;
                              else
                                 {
                                 ONn = ONp = ONroot;
                                 while(ONn != NULL)
                                    {
                                    sl1 = strcmp(OMF1[i].FCD[ONn->index].ECCname, OMF1[i].FCD[ONc->index].ECCname);
                                    if(sl1 == 0)
                                       sl1 = strcmp(OMF1[i].FCD[ONn->index].geom, OMF1[i].FCD[ONc->index].geom);
                                    if(sl1 < 0)
                                       {
                                       ONp = ONn;
                                       ONn = ONn->next;
                                       }
                                    else if(sl1 == 0)
                                       {
                                       OMF1[i].FCD[ONn->index].number += OMF1[i].FCD[ONc->index].number;
                                       OMF1[i].FCD[ONc->index].number = 0;
                                       free(ONc);
                                       break;
                                       }
                                    else if(sl1 > 0)
                                       { 
                                       if(ONn == ONroot)
                                          {
                                          ONc->next = ONroot;
                                          ONroot = ONc;
                                          }
                                       else
                                          {
                                          ONp->next = ONc;
                                          ONc->next = ONn;
                                          }
                                       break;
                                       }
                                    }
                                 if(ONn == NULL)
                                    {
                                    ONp->next = ONc;
                                    ONc->next = NULL;
                                    }
                                 }
                              }

                           if(ONroot != NULL)
                              {
                              ONc = ONroot;
                              while(ONc != NULL)
                                 {
                                 j = ONc->index;
                                 sprintf(lineout,"       %s %s %s:",OMF1[i].FCD[j].geom,OMF1[i].FCD[j].ECCcode, OMF1[i].FCD[j].ECCname);
                                 sprintf(proj1,"%d",OMF1[i].FCD[j].number);

                                 sl2 = strlen(lineout);
                                 while(sl2 > (col1start - 2))
                                    {
                                    sl1 = col1start - 1;
                                    while((sl1 > 6) && (lineout[sl1] != ' ') && (lineout[sl1] != '/'))
                                       --sl1;
                                    if(sl1 <= 6)
                                       {
                                       sl1 = 0;
                                       ii = 0;
                                       while(sl1 < (col1start - 2))
                                          {
                                          lo2[ii] = lineout[sl1];
                                          ++sl1;
                                          ++ii;
                                          }
                                       lo2[ii] = '\n';
                                       ++ii;
                                       lo2[ii] = '\0';

                                       answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lo2);
                                       lineout[7] = ' ';
                                       lineout[8] = ' ';
                                       sl = 9;
                                       while(sl1 <= sl2)
                                          {
                                          lineout[sl] = lineout[sl1];
                                          ++sl;
                                          ++sl1;
                                          }
                                       }
                                    else
                                       {
                                       sl = 0;
                                       ii = 0;
                                       while(sl <= sl1)
                                          {
                                          lo2[ii] = lineout[sl];
                                          ++sl;
                                          ++ii;
                                          }

                                       lo2[ii] = '\n';
                                       ++ii;
                                       lo2[ii] = '\0';

                                       answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lo2);
                                       lineout[7] = ' ';
                                       lineout[8] = ' ';
                                       sl = 9;
                                       ++sl1;
                                       while(sl1 <= sl2)
                                          {
                                          lineout[sl] = lineout[sl1];
                                          ++sl;
                                          ++sl1;
                                          }
                                       }
                                    sl2 = strlen(lineout);
                                    }
                                 for(sl = sl2; sl < col1start; sl++)
                                    lineout[sl] = ' ';
                                 strcpy(&lineout[col1start],proj1);
                                 strcat(lineout,"\n");
                                 answerstring = AddToCompString(&answerstrcurrent, &answerstrlimit, answerstring, lineout);
   
                                 ONp = ONc;
                                 ONc = ONc->next;
                                 free(ONp);
                                 }
                              ONroot = NULL;
                              }
                           } /**** end if(OMF1[i].numFCODES > 0) ****/
                        } /*** end if(verbosity > 3) ***/
                     } /*** end if(verbosity > 2) ***/
                  } /*** end if(OMF1[i].e_count > 0) ***/
               } /*** end for(i=0; i<= CONDITION_DEFINITIONS; i++) ***/
            } /*** end if(verbosity > 1) ***/
         } /*** end verbosity parameter is OK ***/
      } /*** end just want the summary info for a single file as in OMF1 and Meta1 ***/


   for(i=0; i<= CONDITION_DEFINITIONS; i++)
      {
      if(OMF1[i].uom != NULL)
         free(OMF1[i].uom);
      if((OMF1[i].FCD != NULL) && (OMF1[i].numFCODES > 0))
         {
         for(j=0; j<OMF1[i].numFCODES; j++)
            {
            free(OMF1[i].FCD[j].geom);
            free(OMF1[i].FCD[j].ECCcode);
            free(OMF1[i].FCD[j].ECCname);
            }
         free(OMF1[i].FCD);
         }
      }
   free(Meta1.project);
   free(Meta1.fromcoord);
   free(Meta1.tocoord);
   free(OMF1);

   if(P1PointFeatures != NULL)
      free(P1PointFeatures);
   if(P1LineFeatures != NULL)
      free(P1LineFeatures);
   if(P1AreaFeatures != NULL)
      free(P1AreaFeatures);
   if(P1ElevPosts != NULL)
      free(P1ElevPosts);
   if(P2PointFeatures != NULL)
      free(P2PointFeatures);
   if(P2LineFeatures != NULL)
      free(P2LineFeatures);
   if(P2AreaFeatures != NULL)
      free(P2AreaFeatures);
   if(P2ElevPosts != NULL)
      free(P2ElevPosts);

   if(OMF2 != NULL)
      {
      for(i=0; i<= CONDITION_DEFINITIONS; i++)
         {
         if(OMF2[i].uom != NULL)
            free(OMF2[i].uom);
         if((OMF2[i].FCD != NULL) && (OMF2[i].numFCODES > 0))
            {
            for(j=0; j<OMF2[i].numFCODES; j++)
               {
               free(OMF2[i].FCD[j].geom);
               free(OMF2[i].FCD[j].ECCcode);
               free(OMF2[i].FCD[j].ECCname);
               }
            free(OMF2[i].FCD);
            }
         }
      free(Meta2.project);
      free(Meta2.fromcoord);
      free(Meta2.tocoord);

      free(OMF2);
      }

   if(f1 != NULL)
      fclose(f1);
   if(f2 != NULL)
      fclose(f2);
   file_endianness = save_endian;
   return(answerstring);
}





void FreePreviouslyIgnored(void)
{
int i;
   for(i=0; i<=CONDITION_DEFINITIONS; i++)
      {
      IGNc = IGN[i];
      while(IGNc != NULL)
         {
         IGNp = IGNc;
         IGNc = IGNc->next;
         if(IGNp->Px != NULL)
            {
            free(IGNp->Px);
            free(IGNp->Py);
            }
         if(IGNp->NV1 >= 0)
            {
            if(IGNp->X1 != NULL)
               free(IGNp->X1);
            if(IGNp->Y1 != NULL)
               free(IGNp->Y1);
            if(IGNp->IDstr1 != NULL)
               free(IGNp->IDstr1);
            }
         if(IGNp->NV2 >= 0)
            {
            if(IGNp->X2 != NULL)
               free(IGNp->X2);
            if(IGNp->Y2 != NULL)
               free(IGNp->Y2);
            if(IGNp->IDstr2 != NULL)
               free(IGNp->IDstr2);
            }
         free(IGNp);
         }
      IGN[i] = NULL;
      }
}





int EqualTo8DecPlaces(int Coord_System, double d1, double d2)
{
int answer = 0;
char s1[25];
char s2[25];

   if(Coord_System == 1)
      {
      sprintf(s1,"%.1lf",d1);
      sprintf(s2,"%.1lf",d2);
      }
   else
      {
      sprintf(s1,"%.8lf",d1);
      sprintf(s2,"%.8lf",d2);
      }
   if(strcmp(s1,s2) == 0)
      answer = 1;

   return(answer);
}



int CompGenericErrToIgn(int Template, int CloneIndex, int OrdinalIndex, int coord_syst, int UseAddedPt, int NumFeatures)
{
int i,answer, end_now;

   answer = 0;
   end_now = 0;

   IGNc = IGN[Template - 1];
   while(IGNc!= NULL)
      {
      i = GenericErr.Cnumber + 1;
      if((IGNc->Record < -99) && (IGNc->Instance == i))
         {
         if(UseAddedPt > 0)
            {
            if((IGNc->Px == NULL) || (EqualTo8DecPlaces(coord_syst,IGNc->Px[0],GenericErr.px) == 0))
               {
               end_now = 1;
               }
            else if((IGNc->Py == NULL) || (EqualTo8DecPlaces(coord_syst,IGNc->Py[0],GenericErr.py) == 0))
               {
               end_now = 1;
               }
            }
         if(end_now == 0)
            {
            if(NumFeatures == 1)
               {
               if((IGNc->GT1 == GenericErr.gform1) && (strcmp(IGNc->IDstr1,GenericErr.SID1) == 0))
                  {
                  if(IGNc->NV1 == GenericErr.numverts1)
                     {
                     for(i = 0; i<IGNc->NV1; i++)
                        {
                        if(EqualTo8DecPlaces(coord_syst,IGNc->X1[i],GenericErr.x1[i]) == 0)
                           break;
                        if(EqualTo8DecPlaces(coord_syst,IGNc->Y1[i],GenericErr.y1[i]) == 0)
                           break;
                        }
                     if(i >= IGNc->NV1)
                        {
                        IGNc->Record = CloneIndex;
                        IGNc->Cnumber = OrdinalIndex;
                        answer = 1;
                        break;
                        }
                     }
                  }
               } /*** end only one feature involved ***/
            else if(NumFeatures == 2)
               {
               if((IGNc->GT1 == GenericErr.gform1) && (strcmp(IGNc->IDstr1,GenericErr.SID1) == 0))
                  {
                  if(IGNc->NV1 == GenericErr.numverts1)
                     {
                     for(i = 0; i<IGNc->NV1; i++)
                        {
                        if(EqualTo8DecPlaces(coord_syst,IGNc->X1[i],GenericErr.x1[i]) == 0)
                           break;
                        if(EqualTo8DecPlaces(coord_syst,IGNc->Y1[i],GenericErr.y1[i]) == 0)
                           break;
                        }
                     if(i >= IGNc->NV1)
                        {
                        if((IGNc->GT2 == GenericErr.gform2) && (strcmp(IGNc->IDstr2,GenericErr.SID2) == 0))
                           {
                           if(IGNc->NV2 == GenericErr.numverts2)
                              {
                              for(i = 0; i<IGNc->NV2; i++)
                                 {
                                 if(EqualTo8DecPlaces(coord_syst,IGNc->X2[i],GenericErr.x2[i]) == 0)
                                    break;
                                 if(EqualTo8DecPlaces(coord_syst,IGNc->Y2[i],GenericErr.y2[i]) == 0)
                                    break;
                                 }
                              if(i >= IGNc->NV2)
                                 {
                                 IGNc->Record = CloneIndex;
                                 IGNc->Cnumber = OrdinalIndex;
                                 answer = 1;
                                 break;
                                 }
                              }
                           }
                        }
                     }
                  } /*** end feature 1 matches feature 1 ***/
               else if((IGNc->GT1 == GenericErr.gform2) && (strcmp(IGNc->IDstr1,GenericErr.SID2) == 0))
                  {
                  if(IGNc->NV1 == GenericErr.numverts2)
                     {
                     for(i = 0; i<IGNc->NV1; i++)
                        {
                        if(EqualTo8DecPlaces(coord_syst,IGNc->X1[i],GenericErr.x2[i]) == 0)
                           break;
                        if(EqualTo8DecPlaces(coord_syst,IGNc->Y1[i],GenericErr.y2[i]) == 0)
                           break;
                        }
                     if(i >= IGNc->NV1)
                        {
                        if((IGNc->GT2 == GenericErr.gform1) && (strcmp(IGNc->IDstr2,GenericErr.SID1) == 0))
                           {
                           if(IGNc->NV2 == GenericErr.numverts1)
                              {
                              for(i = 0; i<IGNc->NV2; i++)
                                 {
                                 if(EqualTo8DecPlaces(coord_syst,IGNc->X2[i],GenericErr.x1[i]) == 0)
                                    break;
                                 if(EqualTo8DecPlaces(coord_syst,IGNc->Y2[i],GenericErr.y1[i]) == 0)
                                    break;
                                 }
                              if(i >= IGNc->NV2)
                                 {
                                 IGNc->Record = CloneIndex;
                                 IGNc->Cnumber = OrdinalIndex;
                                 answer = 1;
                                 break;
                                 }
                              }
                           }
                        }
                     }
                  } /*** end feature 1 matches feature 2 ***/
               } /*** end two features involved ***/
            }  /*** end if(end_now == 0) ***/
         } /*** end if((IGNc->Record == 0) && (IGNc->Instance == i)) ***/
      IGNc = IGNc->next;
      }


   return(answer);
}




int CompCurrentGE_ToIgn(int Template, int CloneIndex, int OrdinalIndex, int coord_syst, int UseAddedPt, int NumFeatures)
{
int i,answer, end_now;

   answer = 0;
   end_now = 0;

   IGNc = IGN[Template - 1];
   while(IGNc!= NULL)
      {
      i = Current_GE->Cnumber + 1;

      if((IGNc->Record < -99) && (IGNc->Instance == i) && ((Template == COVERFAIL) || (EqualTo8DecPlaces(0,IGNc->Magnitude, Current_GE->magnitude) > 0)))
	{
         if((Template == ENCONNECT) || (Template == AREAKINK) ||(Template == INCLSLIVER) || (Template == FEATBRIDGE) ||
            (Template == CLAMP_DIF) || (Template == ZUNCLOSED) || (Template == NOT_FLAT) || (Template == BADENCON) ||
            (Template == CLAMP_NFLAT) || (Template == AREAUNCLOSED))
             {
             if((IGNc->Px == NULL) || (EqualTo8DecPlaces(coord_syst,IGNc->Px[0],Current_GE->x1[0]) == 0))
                  {
                  end_now = 1;
                  }
               else if((IGNc->Py == NULL) || (EqualTo8DecPlaces(coord_syst,IGNc->Py[0],Current_GE->y1[0]) == 0))
                  {
                  end_now = 1;
                  }
             if(end_now == 0)
                {
               if((IGNc->GT1 == Current_GE->gform2) && (strcmp(IGNc->IDstr1,Current_GE->SID2) == 0))
                  {
                  if(IGNc->NV1 == Current_GE->numverts2)
                     {
                     for(i = 0; i<IGNc->NV1; i++)
                        {
                        if(EqualTo8DecPlaces(coord_syst,IGNc->X1[i],Current_GE->x2[i]) == 0)
                           break;
                        if(EqualTo8DecPlaces(coord_syst,IGNc->Y1[i],Current_GE->y2[i]) == 0)
                           break;
                        }
                     if(i >= IGNc->NV1)
                        {
                        IGNc->Record = CloneIndex;
                        IGNc->Cnumber = OrdinalIndex;
                        answer = 1;
                        break;
                        }
                     }
                  }
                }
             }
         else
            {
            if(UseAddedPt > 0)
               {
               if((IGNc->Py == NULL) || (EqualTo8DecPlaces(coord_syst,IGNc->Px[0],Current_GE->px) == 0))
                  {
                  end_now = 1;
                  }
               else if((IGNc->Py == NULL) || (EqualTo8DecPlaces(coord_syst,IGNc->Py[0],Current_GE->py) == 0))
                  {
                  end_now = 1;
                  }
               }
            if(end_now == 0)
               {
               if(NumFeatures == 1)
                  {
                  if((IGNc->GT1 == Current_GE->gform1) && (strcmp(IGNc->IDstr1,Current_GE->SID1) == 0))
                     {
                     if(IGNc->NV1 == Current_GE->numverts1)
                        {
                        for(i = 0; i<IGNc->NV1; i++)
                           {
                           if(EqualTo8DecPlaces(coord_syst,IGNc->X1[i],Current_GE->x1[i]) == 0)
                              break;
                           if(EqualTo8DecPlaces(coord_syst,IGNc->Y1[i],Current_GE->y1[i]) == 0)
                              break;
                           }
                        if(i >= IGNc->NV1)
                           {
                           IGNc->Record = CloneIndex;
                           IGNc->Cnumber = OrdinalIndex;
                           answer = 1;
                           break;
                           }
                        }
                     }
                  } /*** end only one feature involved ***/
               else if(NumFeatures == 2)
                  {
                  if((IGNc->GT1 == Current_GE->gform1) && (strcmp(IGNc->IDstr1,Current_GE->SID1) == 0))
                     {
                     if(IGNc->NV1 == Current_GE->numverts1)
                        {
                        for(i = 0; i<IGNc->NV1; i++)
                           {
                           if(EqualTo8DecPlaces(coord_syst,IGNc->X1[i],Current_GE->x1[i]) == 0)
                              break;
                           if(EqualTo8DecPlaces(coord_syst,IGNc->Y1[i],Current_GE->y1[i]) == 0)
                              break;
                           }
                        if(i >= IGNc->NV1)
                           {
                           if((IGNc->GT2 == Current_GE->gform2) && (strcmp(IGNc->IDstr2,Current_GE->SID2) == 0))
                              {
                              if(IGNc->NV2 == Current_GE->numverts2)
                                 {
                                 for(i = 0; i<IGNc->NV2; i++)
                                    {
                                    if(EqualTo8DecPlaces(coord_syst,IGNc->X2[i],Current_GE->x2[i]) == 0)
                                       break;
                                    if(EqualTo8DecPlaces(coord_syst,IGNc->Y2[i],Current_GE->y2[i]) == 0)
                                       break;
                                    }
                                 if(i >= IGNc->NV2)
                                    {
                                    IGNc->Record = CloneIndex;
                                    IGNc->Cnumber = OrdinalIndex;
                                    answer = 1;
                                    break;
                                    }
                                 }
                              }
                           }
                        }
                     } /*** end feature 1 matches feature 1 ***/
                  else if((IGNc->GT1 == Current_GE->gform2) && (strcmp(IGNc->IDstr1,Current_GE->SID2) == 0))
                     {
                     if(IGNc->NV1 == Current_GE->numverts2)
                        {
                        for(i = 0; i<IGNc->NV1; i++)
                           {
                           if(EqualTo8DecPlaces(coord_syst,IGNc->X1[i],Current_GE->x2[i]) == 0)
                              break;
                           if(EqualTo8DecPlaces(coord_syst,IGNc->Y1[i],Current_GE->y2[i]) == 0)
                              break;
                           }
                        if(i >= IGNc->NV1)
                           {
                           if((IGNc->GT2 == Current_GE->gform1) && (strcmp(IGNc->IDstr2,Current_GE->SID1) == 0))
                              {
                              if(IGNc->NV2 == Current_GE->numverts1)
                                 {
                                 for(i = 0; i<IGNc->NV2; i++)
                                    {
                                    if(EqualTo8DecPlaces(coord_syst,IGNc->X2[i],Current_GE->x1[i]) == 0)
                                       break;
                                    if(EqualTo8DecPlaces(coord_syst,IGNc->Y2[i],Current_GE->y1[i]) == 0)
                                       break;
                                    }
                                 if(i >= IGNc->NV2)
                                    {
                                    IGNc->Record = CloneIndex;
                                    IGNc->Cnumber = OrdinalIndex;
                                    answer = 1;
                                    break;
                                    } 
                                 }
                              }
                           }
                        }
                     } /*** end feature 1 matches feature 2 ***/
                  } /*** end two features involved ***/
               }  /*** end if(end_now == 0) ***/
            } /**** end else is not one of the special-processing inspections ***/
         } /*** end if((IGNc->Record == 0) && (IGNc->Instance == i)) ***/
      IGNc = IGNc->next;
      }

   return(answer);
}





void DismissPreviouslyIgnored(void)
{
int i, j;

   if(ConsultPreviouslyIgnored > 0)
      {
      for(i=0; i<CONDITION_DEFINITIONS; i++)
         {
         if(IGN[i] != NULL)
            {
            IGNc = IGN[i];
            j = i + 1;
            while(IGNc != NULL)
               {
               if(IGNc->Record > -2) /*** if did not match this ignored record, will have value -101 ***/
                  {
                  if(IGNc->Record < 0) /*** set reference to ErrorLookup  ****/
                     ErrorLookup[j].keepdismiss[IGNc->Cnumber] = 0;
                  else  /*** set relative to CloneErrorLookup ***/
                     CloneErrorLookup[IGNc->Record].keepdismiss[IGNc->Cnumber] = 0;
                  }

               IGNc = IGNc->next;
               }
            }
         }

      FreePreviouslyIgnored();
      }
}





void ProcessShape(int num, TimSHPObject *psSHP, int startindex,int endindex, int type)
{
int Template, update_type;
int i, i_start, i_stop, j;
int i_geom;
int numverts;
int SzIG = sizeof(struct IG_features);
int coord_type;
extern int num_shapes_processed;

  periodic_checking_redraw(0,"Condition Report Progress","       Preparing condition report\nConsulting previously ignored conditions\n");


  num_shapes_processed = num_shapes_processed + 1;
  if(num_shapes_processed>SHAPE_IGNORE_LIMIT)
  {
	  return;
  }
  
   for(i=1; i<=CONDITION_DEFINITIONS; i++)
      {
      if(strcmp(GL_errnums[num],ErrorLookup[i].name) == 0)
         break;
      }
   if(i > CONDITION_DEFINITIONS)
      {
      printf("failed to find template id number for %s\n",GL_errnums[num]);
      exit(-1);
      }

   coord_type = Ctype();

   Template = i-1;
   numverts = endindex - startindex + 1;
   switch(GL_geoms[num][0])
      {
      case 'P': i_geom = G_POINT; break;
      case 'A': i_geom = G_AREAL;
                if(numverts > 1)
                   numverts = numverts - 1;
                break;
      case 'L': i_geom = G_LINE; break;
      case 'G': i_geom = G_GRIDPT; break;
      default: i_geom = 99; break;
      }

   if(IGN[Template] == NULL) /** adding first condition of this template type ***/
      {
      IGNn = (struct IG_features *) (malloc(SzIG));
      if(IGNn == NULL)
         {
         printf("all available memory has been consumed during previously ignored condition processing\n");
         exit(-1);
         }
      IGN[Template] = IGNn;
      IGNn->Record = -101;
      IGNn->next = NULL;
      update_type = 3; /** 3 says new record to add , but will have already allocated IGNn***/
      }
   else
      {
      IGNp = IGNc = IGN[Template];
      update_type = 2; /*** assumes will be added to the end of the list ***/
      while(IGNc != NULL)
         {
         if((IGNc->Instance == GL_instances[num]) && (IGNc->Cnumber == GL_condnums[num]))
            {
            update_type = 1; /** 1 -> just have to update (add to) an existing record  pointed to by IGNc***/
            break;
            }
         if((IGNc->Instance > GL_instances[num]) || ((IGNc->Instance == GL_instances[num]) && (IGNc->Cnumber > GL_condnums[num])))
            {
            IGNn = (struct IG_features *) (malloc(SzIG));
            if(IGNn == NULL)
               {
               printf("all available memory has been consumed during previously ignored condition processing\n");
               exit(-1);
               }
            if(IGNc == IGN[Template]) /** have to replace the root ***/
               {
               IGNn->next = IGNc;
               IGN[Template] = IGNn;
               }
            else
               {
               IGNp->next = IGNn;
               IGNn->next = IGNc;
               }
            IGNn->Record = -101;
            update_type = 3; /** 3 says new record to add , but will have already allocated IGNn***/
            break;
            }
         IGNp = IGNc;
         IGNc = IGNc->next;
         }
      if(update_type == 2) /*** have to add at end of list ***/
         {
         IGNn = (struct IG_features *) (malloc(SzIG));
         if(IGNn == NULL)
            {
            printf("all available memory has been consumed during previously ignored condition processing\n");
            exit(-1);
            }
         IGNn->Record = -101;
         IGNn->next = NULL;
         IGNp->next = IGNn;
         }
      }
   if(update_type > 1) /*** are adding a new record in ptr IGNn ***/
      {
      IGNn->Instance =  GL_instances[num];
      IGNn->Cnumber = GL_condnums[num];
      if(i_geom == 99)  /*** then the feature record here is an additional point ***/
         {
         if(type == 1)
            i_start = psSHP->panPartStart[0];
         else
            i_start = 0;
         if(numverts > 1)
            {
            printf("saved ignored condition with %d vertices for additional point\n",numverts);
            exit(-1);
            }
         IGNn->Px = (double *) (malloc(SzD));
         IGNn->Py = (double *) (malloc(SzD));
         if(IGNn->Py == NULL)
            {
            printf("available system memory has been fully consumed when processing previously ignored conditions\n");
            printf("request for memory to store %d feature vertices has failed\n",numverts);
            exit(-1);
            }
               
         if(coord_type == 1)
            {
            IGNn->Px[0] = (psSHP->padfX[i_start] - Xtranslation) * 100000.0;
            IGNn->Py[0] = (psSHP->padfY[i_start] - Ytranslation) * 100000.0;
            }
         else if(coord_type == 2)
            {
            IGNn->Px[0] = psSHP->padfX[i_start] - Xtranslation;
            IGNn->Py[0] = psSHP->padfY[i_start] - Ytranslation;
            }
         else
            {
            IGNn->Px[0] = psSHP->padfX[i_start];
            IGNn->Py[0] = psSHP->padfY[i_start];
            }

         IGNn->Magnitude = GL_magnitudes[num];
         IGNn->NV1 = -1;
         IGNn->NV2 = -1;
         IGNn->X1 = NULL;
         IGNn->Y1 = NULL;
         IGNn->X2 = NULL;
         IGNn->Y2 = NULL;
         IGNn->IDstr1 = NULL;
         IGNn->IDstr2 = NULL;
         }
      else
         {
         IGNn->Px = NULL;
         IGNn->Py = NULL;
         IGNn->X2 = NULL;
         IGNn->Y2 = NULL;
         IGNn->IDstr2 = NULL;
         IGNn->Magnitude = GL_magnitudes[num];
         IGNn->NV1 = numverts;
         IGNn->NV2 = -1;
         IGNn->GT1 = i_geom;
         IGNn->X1 = (double *) (malloc(SzD * numverts));
         IGNn->Y1 = (double *) (malloc(SzD * numverts));
         if(IGNn->Y1 == NULL)
            {
            printf("available system memory has been fully consumed when processing previously ignored conditions\n");
            printf("request for memory to store %d feature vertices has failed\n",numverts);
            exit(-1);
            }
         if(type == 1)
            {
            i_start = psSHP->panPartStart[0];
            i_stop   = psSHP->nVertices-1;
            if(i_geom == G_AREAL)
               --i_stop;
            }
         else
            { i_start = i_stop = 0; }

         j = 0;
         for(i=i_start; i<=i_stop; i++)
            {
            if(coord_type == 1)
               {
               IGNn->X1[j] = (psSHP->padfX[i] - Xtranslation) * 100000.0;
               IGNn->Y1[j] = (psSHP->padfY[i] - Ytranslation) * 100000.0;
               }
            else if(coord_type == 2)
               {
               IGNn->X1[j] = psSHP->padfX[i] - Xtranslation;
               IGNn->Y1[j] = psSHP->padfY[i] - Ytranslation;
               }
            else
               {
               IGNn->X1[j] = psSHP->padfX[i];
               IGNn->Y1[j] = psSHP->padfY[i];
               }

            ++j;
            }
         numverts = strlen(GL_IDs[num]);
         IGNn->IDstr1 = (char *) (malloc(numverts + 2));
         if(IGNn->IDstr1 == NULL)
            {
            printf("available system memory has been fully consumed when processing previously ignored conditions\n");
            printf("request for memory to store %d character feature ID string has failed\n",numverts);
            exit(-1);
            }
         strcpy(IGNn->IDstr1,GL_IDs[num]);
         }
      }
   else /*** must be update_type == 1, so just adding to (updating) an existing record pointed to by IGNc***/
      {
      if(i_geom == 99) /** adding the additional point ***/
         {
         if((IGNc->Px == NULL) && (IGNc->Py == NULL))
            {
            if(type == 1)
               i_start = psSHP->panPartStart[0];
            else
               i_start = 0;

            if(numverts > 1)
               {
               printf("saved ignored condition with %d vertices for additional point\n",numverts);
               exit(-1);
               }
            IGNc->Px = (double *) (malloc(SzD));
            IGNc->Py = (double *) (malloc(SzD));
            if(IGNc->Py == NULL)
               {
               printf("available system memory has been fully consumed when processing previously ignored conditions\n");
               printf("request for memory to store %d feature vertices has failed\n",numverts);
               exit(-1);
               }
            if(coord_type == 1)
               {
               IGNc->Px[0] = (psSHP->padfX[i_start] - Xtranslation) * 100000.0;
               IGNc->Py[0] = (psSHP->padfY[i_start] - Ytranslation) * 100000.0;
               }
            else if(coord_type == 2)
               {
               IGNc->Px[0] = psSHP->padfX[i_start] - Xtranslation;
               IGNc->Py[0] = psSHP->padfY[i_start] - Ytranslation;
               }
            else
               {
               IGNc->Px[0] = psSHP->padfX[i_start];
               IGNc->Py[0] = psSHP->padfY[i_start];
               }
            }
         else
            {
            printf("additional point overloaded in ignored condition\n"); exit(-1);
            }
         }
      else
         {
         if(IGNc->NV1 < 0) /*** add as feature # 1 ***/
            {
            IGNc->NV1 = numverts;
            IGNc->GT1 = i_geom;
            IGNc->X1 = (double *) (malloc(SzD * numverts));
            IGNc->Y1 = (double *) (malloc(SzD * numverts));
            if(IGNc->Y1 == NULL)
               {
               printf("available system memory has been fully consumed when processing previously ignored conditions\n");
               printf("request for memory to store %d feature vertices has failed\n",numverts);
               exit(-1);
               }
            if(type == 1)
               {
               i_start = psSHP->panPartStart[0];
               i_stop   = psSHP->nVertices-1;
               if(i_geom == G_AREAL)
                  --i_stop;
               }
            else
               { i_start = i_stop = 0; }
            j = 0;   
            for(i=i_start; i<=i_stop; i++)
               {
               if(coord_type == 1)
                  {
                  IGNc->X1[j] = (psSHP->padfX[i] - Xtranslation) * 100000.0;
                  IGNc->Y1[j] = (psSHP->padfY[i] - Ytranslation) * 100000.0;
                  }
               else if(coord_type == 2)
                  {
                  IGNc->X1[j] = psSHP->padfX[i] - Xtranslation;
                  IGNc->Y1[j] = psSHP->padfY[i] - Ytranslation;
                  }
               else
                  {
                  IGNc->X1[j] = psSHP->padfX[i];
                  IGNc->Y1[j] = psSHP->padfY[i];
                  }
               ++j;
               }
            numverts = strlen(GL_IDs[num]);
            IGNc->IDstr1 = (char *) (malloc(numverts + 2));
            if(IGNc->IDstr1 == NULL)
               {
               printf("available system memory has been fully consumed when processing previously ignored conditions\n");
               printf("request for memory to store %d character feature ID string has failed\n",numverts);
               exit(-1);
               }
            strcpy(IGNc->IDstr1,GL_IDs[num]);
            }
         else if(IGNc->NV2 < 0) /*** add as feature # 2 ***/
            {
            IGNc->NV2 = numverts;
            IGNc->GT2 = i_geom;
            IGNc->X2 = (double *) (malloc(SzD * numverts));
            IGNc->Y2 = (double *) (malloc(SzD * numverts));
            if(IGNc->Y2 == NULL)
               {
               printf("available system memory has been fully consumed when processing previously ignored conditions\n");
               printf("request for memory to store %d feature vertices has failed\n",numverts);
               exit(-1);
               }
            if(type == 1)
               {
               i_start = psSHP->panPartStart[0];
               i_stop   = psSHP->nVertices-1;
               if(i_geom == G_AREAL)
                  --i_stop;
               }
            else
               { i_start = i_stop = 0; }
            j = 0;
            for(i=i_start; i<=i_stop; i++)
               {
               if(coord_type == 1)
                  {
                  IGNc->X2[j] = (psSHP->padfX[i] - Xtranslation) * 100000.0;
                  IGNc->Y2[j] = (psSHP->padfY[i] - Ytranslation) * 100000.0;
                  }
               else if(coord_type == 2)
                  {
                  IGNc->X2[j] = psSHP->padfX[i] - Xtranslation;
                  IGNc->Y2[j] = psSHP->padfY[i] - Ytranslation;
                  }
               else
                  {
                  IGNc->X2[j] = psSHP->padfX[i];
                  IGNc->Y2[j] = psSHP->padfY[i];
                  }
               ++j;
               }
            numverts = strlen(GL_IDs[num]);
            IGNc->IDstr2 = (char *) (malloc(numverts + 2));
            if(IGNc->IDstr2 == NULL)
               {
               printf("available system memory has been fully consumed when processing previously ignored conditions\n");
               printf("request for memory to store %d character feature ID string has failed\n",numverts);
               exit(-1);
               }
            strcpy(IGNc->IDstr2,GL_IDs[num]);
            }
         else
            {
            printf("number of available features overloaded in ignored condition\n"); exit(-1);
            }
         }
      }
   return;
}


int FindShapeFile(int geom, int num)
{
  FILE *infile;
  char testloc[1000];
  
  if(geom==1)
    {
      sprintf(testloc,"%s%dPT.shp",IgnoredPointFile,num);
      infile = fopen(testloc,"r");
      if(infile!=NULL)
	{
	  fclose(infile);
	  
	  sprintf(testloc,"%s%dPT.shx",IgnoredPointFile,num);
	  infile = fopen(testloc,"r");
	  if(infile!=NULL)
	    {
	      fclose(infile);
	      
	      sprintf(testloc,"%s%dPT.dbf",IgnoredPointFile,num);
	      infile = fopen(testloc,"r");
	      if(infile!=NULL)
		{
		  fclose(infile);
		  
		  sprintf(IgnoredPointFile2,"%s%dPT",IgnoredPointFile,num);
		  
		  return 1;
		}
	    } 
	} 
      return 0;
    }
  else if(geom==2)
    { 
      /* line */
      sprintf(testloc,"%s%dLN.shp",IgnoredLineFile,num);
      infile = fopen(testloc,"r");
      if(infile!=NULL)
	{ 
	  fclose(infile);
	  
	  sprintf(testloc,"%s%dLN.shx",IgnoredLineFile,num);
	  infile = fopen(testloc,"r");
	  if(infile!=NULL)
	    {
	      fclose(infile);
	      
	      sprintf(testloc,"%s%dLN.dbf",IgnoredLineFile,num);
	      infile = fopen(testloc,"r");
	      if(infile!=NULL)
		{ 
		  fclose(infile);
		  
		  sprintf(IgnoredLineFile2,"%s%dLN",IgnoredLineFile,num);
		  
		  return 1;
		}  
	    }   
	}   
      return 0;
    }
  
  return 0;
}

      
      


void ReRackAllConditions(int UpdateStatus, int ZeroIngOut, int ErrorToZero, int InstanceToZero)
{
  struct IndexEntry *cIE, *pIE, *nIE;
  struct filepointers
  {
    struct IndexEntry * entry;
  } Index[CONDITION_ARRAY_SIZE];
  struct dupremoval
  {
    int IDN;
    double X;
    double Y;
    int Cindex;
    struct dupremoval * next;
  } *DupRoot, *cDP, *pDP;
  
  FILE *ObjFP;
  int si,ii,i,j,jj,k,kk,keyval,kv2,lastkeyval, LastCloneNumber, MCFG;
  int TtlCF_errors = 0;
  int coord_type,foundpointfile,foundlinefile,thisfile;
  int SzIE;
  int SzDR;
  int DupValue, duplicatefound;
  int OKtoSort;  /***, WritePG1; ***/
  int e_count, ign_count, numT, Ttle_count, Ttlign_count, I_Applied;
  int MaxCOVERFAILgroup = -100;
  int * CFGcounts = NULL;
  int NumberOfObjects;
  double e_max, e_min;
  double t1min, t1max, t2min, t2max;
  double dbljunk;
  double LastLocalID1, LastLocalID2;
  double LastPX, LastPY, LastPZ;
  long int fileposn;
  extern char unsortlog[];
  extern char errtypelog[];
  extern char CDFlog[];
  extern int num_t_dupes;
  char command2[500];
  char objfile[500];
  int *Obj_Mdl_Flags;
  long int OMFposn;
  extern char outdirectory[500]; 
  extern int file_endianness;
  int cloneindex;
  int cIEcloneIndex;
  extern int CLONE_DEFINITIONS;
  int SpecificConditionNumber;
  int LastCnumber, LastKV;
  int ObjsWithErrs, TtlObjErrs, ObjsWritten, ObjsPerPage, FPtoWrite;
  int ElevPwithErrs, ElevOwithErrs, newobjectwitherrs;
  int template_entries;
  int zeroval = 0;
  struct clonestocount *ctc, *ctcp;
  extern FILE *binsmryout;
  

  RegularConditions = (struct ConditionSort *) (malloc(sizeof(struct ConditionSort) * CONDITION_ARRAY_SIZE + 2));
  CloneConditions = (struct ConditionSort *) (malloc(sizeof(struct ConditionSort) * CLONE_DEFINITIONS + 2));
  if(CloneConditions == NULL)
    {
      printf("available memory has been exhausted during condition magnitude sort operation\n");
      exit(-1);
    }
  
  for(i=0; i<=CONDITION_DEFINITIONS; i++)
    {
      Index[i].entry = NULL;
      ErrorLookup[i].number = 0;
      ErrorLookup[i].fileposn = -1;
      RegularConditions[i].kv = -1;
      RegularConditions[i].RB_Entry = NULL;
      CCBY[i].c = NULL;
      CCBY[i].count = 0;
    }
  if(CLONE_DEFINITIONS > 0)
    {
      for(i=0; i<CLONE_DEFINITIONS; i++)
        {
	  CloneErrorLookup[i].number = 0;
	  CloneErrorLookup[i].fileposn = -1;
	  CloneConditions[i].kv = -1;
	  CloneConditions[i].RB_Entry = NULL;
        }
    }
  
  GenericErr.Cnumber = -1;
  GenericErr.msglen = 0;
  GenericErr.errmsg = NULL;
  GenericErr.SIDlen1 = 0;
  GenericErr.SID1 = NULL;
  GenericErr.SIDlen2 = 0;
  GenericErr.SID2 = NULL;
  GenericErr.x1 = NULL;
  GenericErr.y1 = NULL;
  GenericErr.z1 = NULL;
  GenericErr.numverts1 = 0;
  GenericErr.x2 = NULL;
  GenericErr.y2 = NULL;
  GenericErr.z2 = NULL;
  GenericErr.numverts2 = 0;

  coord_type = Ctype();
  
  RB_ObjTree = NULL;
  RB_ObjSortTree = NULL;
  
  SzIE = sizeof(struct IndexEntry);
  SzDR = sizeof(struct dupremoval);
  fclose(unsortout);
  
  unsortout = fopen(unsortlog,"rb");
  
  
  read_endian(unsortout);
  
  cIE = NULL;
  fileposn = ftell(unsortout);
  LastCloneNumber = lastkeyval = -1;
  
  SEEIT_fread_int(&keyval,unsortout);
  
  while(feof(unsortout) == 0)
    {

    if(UpdateStatus == 1)
       periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 1\n");
      if(keyval > CONDITION_DEFINITIONS)
        {
          printf("illegal keyvalue entry (%d) in file %s\n",keyval,unsortlog);
          exit(-1);
        }
      OKtoSort = 0;

      switch(keyval)
        {
          /** first groups below have non-sorted conditions not involving a condition magnitude **/
        case G_DUPS: /** duplicate poly (by x,y,z of vertices) **/
        case C_DUPS: /** complete duplicate, attribution and all **/
        case SAMEID: /** same GFID or FLDBID, diff geom **/
        case SAMEID_GDUP:  /** same GFID or FLDBID, same geom, diff attr ***/
        case SAMEID_CDUP: /** same GFID or FLDBID, geom & attr ***/
        /**case AGEOM_UNM_LAT:
        case AGEOM_UNM_LON:**/
        case ANY_SAMEID:  /** same unique identifier, except those that are complete duplicates ***/
        case POLYINTPOLY: /** two polygons, of selected types, intersect **/
        case POLYINAREA: /** polygon lies wholly inside an areal **/
        case PTINREGION:  /** point feature inside a typed polygon or areal **/
        case PTINPROPER:  /** point feature inside an area feature - not within tolerance of edge (or edge or hole) **/
        case ACOVERA: /* area covers area */
        case FAILMERGEA:  /** area feature that should be merged with area that shares edge ***/
        case FAILMERGEA2:  /** area feature that should be merged with area that shares edge - no accounting for metadata  ***/
        case AINSIDEHOLE: /** area inside another areal's cutout ('illegal holes') ***/
        case FEATNOTCUT:  /*** feature not cut at end node of second feature ***/
        case ISOLINE:  /** line feature completely inside an area feature ***/
        case LINSIDEA: /** line partly or entirely inside area feature ***/
        case LSEGCOVERA: /** line segment overlaps an area feature perimeter ***/
          FreadFwriteTwoObjects(keyval,0,unsortout,NULL,-1, -1, -1.0, -1.0);
          OKtoSort = 2;
          /***++kv3; ***/
          break;

        case LGEOM_UNM_LAT:
        case LGEOM_UNM_LON:
        case AGEOM_UNM_LAT:
        case AGEOM_UNM_LON:
        case AUNM_ATTR_A:
        case LUNM_ATTR_A:
          FreadFwriteMsgMagPointObjects(keyval,0,unsortout,NULL,-1.0,-1.0,-1.0,-1,-1,-1.0,-1.0);
          OKtoSort = 2;
          break;

        case LLINT: /** line - line intersection **/
        case BADFEATCUT: /** feature cut when no need ***/
        case LLNONODEINT: /* features intersect, but not at a shared node **/
        case NONODEOVLP: /** line, area have overlapping edge without common node ***/
        case LLNOENDINT: /** lines intersect, but not at end point **/
        case LLINTAWAY: /** two lines intersect, and cross over each other ***/
        case LLINTNOEND: /** two lines intersect, pt of intersection is away from either primary particpant end node ***/
        case P_O_LOOP: /*** self-intersecting line that includes P & O formations using end nodes - lines only ****/
        case LLIEX: /** line - line except intersection **/
        case LAIEX: /** line - area intersection with 3rd feature exception ***/
        case LMINT: /** line - model intersection **/
        case LAINT:  /* line - areal intersection **/
        case LACUTFAIL:  /** line not cut at intersection with area perimeter **/
        case LAINTNOEND: /** line - area intersection not at line end node ***/
        case LEAON_NOTIN: /** line end node on area edge, line not inside area ***/
        case POLYINTAREA: /* polygon - areal intersection of edges **/
        case AREAINTAREA: /* areal - areal intersection of edges **/
        case PART_ISF: /** two area features have intersecting edges and share part of their faces **/
        case CUT_INT: /** cut-out intersects parent feature outer ring ***/
        case AOVERLAPA: /** overlapping area features (second can also be inside first) **/

          FreadFwritePointAndTwoObjects(keyval,0,unsortout,NULL,-1.0,-1.0,-1.0,-1,-1,-1.0,-1.0);
          OKtoSort = 2;
          break;
	  
        case VERTSLOPE: /** vertical poly **/
        case PLPFAIL: /** point - line coincidence failure **/
        case PNOCOVERLE: /* point not covered by linear end **/
        case PNOCOV2LEA: /** point not covered by 2 line terminal nodes or area edges***/
        case PNOCOVERLV: /** point not covered by any line vertex **/
        case POLYOSIDEAREA: /** Poly completely outside all areals of given type **/
        case PTOSIDEREGION: /** point feature not inside any typed areal or poly **/
        case OBJECTWITHOUT: /** poly or areal without a point or linear inside **/
        case OBJ_WO_TWO: /** area contains secondary P,A,L but not tertiary P,A,L ***/
        case FSFAIL: /*** face sharing failure ***/
        case PSHAREFAIL:  /*** an area feature fails to share any of its perimeter with a 2d area feature ***/
        case NOCOINCIDE: /** area without line end node or segment on its perimeter ***/
        case V_DUPS: /** duplicate vertices inside object **/
        case LNOCOVERLA: /** line not covered by line or areal ***/
        case LSPANFAIL: /** line not covered by face of doesnt spand between edges ***/
        case LNOCOV2A:  /** line not covered by edges of 2 area features ***/
        case ANOCOVERLA: /** areal not covered by line or areal ***/
        case QUALANOCOVLA: /** area permin not covered by line or area AND is inside a third area ***/
        case LEINSIDEA: /** line end node properly inside an area ***/
        case COINCIDEFAIL: /** line or area feature segment fails to coincide with 2 other line or area features **/
        case ISOLATEDA:  /*** area feature does not intersect another area or a line feature ***/
        case NETISOA: /** like ISOLATEDA except allowed a transitive connection through other like features ***/
        case ANETISOA: /** area not trans connected to another area by shared edges ***/
        case NETISOFEAT: /** form a network - check for nets with one feature, but not another ***/
        case MULTIDFEAT: /** single line or area with both 2 and 3 D coordinates ***/
        case MULTISENTINEL: /** single line or area has more than one sentinel z value ***/
        case CONNECTFAIL: /** point, line, or area feature without 'connection' to specified 2nd feature **/
        /***case FEATOUTSIDE:  *** a feature lies at least partly outside the MGCP cell ***/
        case HIGHLIGHTED: /** feature is on the highlight list from view by attribution ***/
        case LLNOINT:  /** line failure to intersect a second line ***/
        case LFNOINT: /** line fails to intersect another line, area, or point and no end node on 1/4 degree line ***/
        case PLLPROXFAIL:  /** point not within specified dist from int of 2 lines ***/
        case ANOCOVERA: /** area not covered by second area ***/
        case OVERUNDER: /** any feature outside a perimeter-defining area or a line end node undershooting it **/
        case AMCOVAFAIL: /** area not coverer by adjoining areas **/
        case CUTOUT:   /** simply identifies a cut-out of an area feature ***/
        case PORTRAYF: /** write feature that fails all MGCP4 portrayal rules ***/
        case TPORTRAYF: /** write feature that fails all TDS6 portrayal rules ***/
        case MASKMONO: /** DEM not monotonic at point defined by specified mask value ***/
        case AHANG_LON: /** hanging area feature at a specified longitude meridian ***/
        case AHANG_LAT: /** hanging area feature at a specified latitude parallel ***/
        case AUNM_ACRS_A: /** area feature edge incorrectly matched across a bounding area feature ***/
          FreadFwriteObject(keyval,0,unsortout,NULL,-1, -1, -1.0);
          OKtoSort = 2;
          break;
	  
        case LOUTSIDEA: /** linear vertex falls outtside areal **/
        case LLAINT: /** line - line endpt connect at area perimeter **/
        case L_NOTL_AINT: /** line end point connects to 'not type line' at area perimeter **/
        case LENOCOVERP: /** line end node not covered by point ***/
        case ENCONFAIL: /** end node connectivity failure **/
        case NOENDCON: /** both end nodes of a line fail to connect or be covered **/
        case BOTHENDCON: /** both end nodes of a line feature are covered by specified-type point features **/
        case LENOCOVERL:  /*** line end node not within tolerance distance to another line ***/
        case NOLCOVLE:  /*** line end node not within tolerance distance to another line, including itself on a diff segment ***/
        case LOOPS: /** self-intersecting linear or areal ***/
        case COLINEAR: /** 3 consecutive vertices on line or area perim are collinear - middle one is not connecting node ***/
        case KICKBACK: /** 180 degree kink ***/
        case ENDPTINT: /** line endpoints are the same ***/
        case L_UNM_A:  /*** line endpt unmatched at area feature boundary ***/
        case LSAME_UNM_A: /*** line endpt unmatched with line of same FCODE at Area boundary ***/
        case LUNM_ACRS_A: /*** line mismatch across poly edge ***/
        case LUNMA_ACRS_A: /** line end not matched to area node across area perimeter ***/
        case LATTRCHNG:  /** line end point connects to same fdcode line, but attributes differ between the 2 features **/
        case LHANG_LON: /** hanging line feature at a specified longitude meridian ***/
        case LHANG_LAT: /** hanging line feature at a specified latitude parallel ***/
        case LE_A_UNM_LAT: /** line end node not coincident with area node at latitude parallel **/
        case LE_A_UNM_LON: /** line end node not coincident with area node at longitude meridian **/
          FreadFwritePointAndObject(keyval,0,unsortout,NULL, -1.0,-1.0,-1.0,-1,-1,-1.0);
          OKtoSort = 2;
          break;
	  
        case ATTR_PAIR: /*** NGA unexpected fcode - geom pair ***/
        case ATTR_UNEXP: /** NGA unexpected attribute assigned ***/
        case ATTR_MISSING: /** missing a required attribute ***/
        case ATTR_DT:  /** NGA - datatype encountered not as presecribed **/
        case ATTR_RNG: /** NGA attribute value range violation ***/
        case ATTR_PICK: /** NGA - pick list allowed domain violation **/
        case ATTR_VVT: /*** attribute dependency violation  **/
        case ATTR_RNULL: /*** MGCP Required attribute assigned NULL value ***/
        case ATTR_META: /** NGA - GIFD D4 metadata violation ***/
        case VVTERR1WAY: /** feature with designated attribute & value ***/
        case VVTERR2WAY:  /*** valid value type error ***/
        case VVTERR3WAY:  /*** valid values conflict between 3 attribute of a single feature ***/
        case ATTRERR: /*** attribution error **/
        case RPTD_ATTR: /** attribute error as reported  ****/
        case CONFLATE: /*** line is unique among conflation sets of data ***/
          FreadFwriteObjectAndMessage(keyval,0,unsortout,NULL);
          OKtoSort = 2;
          break;
	  
          /** next groups are for magnitude-related conditions, plus T-vertices **/
	  
        case TVERT: /** 'T' vertex **/
          FreadFwritePointAndObject(keyval,0,unsortout,NULL,-1.0,-1.0,-1.0,-1,-1,-1.0);
          /** no magnitude for Tverts, so order by unique id number *****/
          OKtoSort = 2;
          break;
	  
        case AREAKINK: /** high angle on perimeter of area feature **/
        case INCLSLIVER: /** areal with included sliver **/
        case ZUNCLOSED: /** area feat not closed in Z **/
        case AREAUNCLOSED: /** area feature unclosed in x,y, or z **/
        case NOT_FLAT:  /*** area feature with surface that is not uiform elevation ***/
        case CLAMP_NFLAT: /** area feature does not have constant elevation when clamped to underlying DEM ***/
        case CLAMP_DIF: /** difference between feature vertex z value and interpolated DEM value ***/
          FreadFwritePointObjectAndMagnitude(keyval, 0, unsortout, NULL);
          OKtoSort = 1;
          break;
	  
        case ISOTURN: /** high turn angle w/o 3d feature present ***/
        case KINK:  /** high angle between adjacent linear segments **/
        case Z_KINK: /** consecutive kinks form a 'Z' ***/
        case L_A_KINK: /** kink at intersection of line end node  and area feature perim **/
        case INTERNALKINK: /** kink internal to single line feature **/
        case SLOPEDIRCH: /*** slope direction change along linear ***/
        case CLAMP_SDC: /*slope direction change along a line that has been elevation-value clamped to underlying DEM ***/
        case CLAMP_JOINSDC: /** slope direction change at line feature connection when both are clamped to DEM ***/
        case SLIVER: /** sliver triangle **/
        case FACESIZE: /*** small area on face of area feature **/
        case ARNGE_UNM_LAT:
        case ARNGE_UNM_LON:
        case NARROW:  /** narrow triangle **/
        case SMALLOBJ:  /** small 3d area poly **/
        case HSLOPE: /** high slope poly **/
        case HTEAR: /** horizontal tear **/
        case OVERC: /** over-covered edge **/
        case GSPIKE:  /** grid data spike point ***/
        /***case AVGSPIKE: ** spike / well as compared to average elevation of neighbor posts ***/
        /***case FLOWSTEP:  ** step size in river flow above threshold ***/
        /***case WATERMMU: ** minimum mapping unit for water body below threshold ***/
        /**case RAISEDPC: ** number of raised shoreline points exceeds tolerance **/
        case GRID_STD_DEV: /** grid elev value, inside feature polygon, over range offset from std deviation **/
        case MULTIPARTL: /** multi-part line ***/
        case MULTIPARTA: /** multi-part area **/
        case MULTIPARTP: /** multi-part point **/
        case ELEVADJCHANGE:  /** change in adjacent node elevations > threshold ***/
        case FEATSPIKE: /** elevation spike along 3D feature ***/
        case SEGLEN:  /** linear or areal perimeter segment with length below threshold ***/
        case LONGSEG: /** linear or areal perimeter segment with length at or above threshold ***/
        case BIGAREA: /** area feature with large square area **/
        case SMALLAREA: /** area feaure with small square area **/
        case SMLCUTOUT: /** small included area inner ring of area feature ***/
        case OSIDE_LAT:   /**** feature coordinate above or below latitude range    **/
        case OSIDE_LON:   /**** feature coordinate above or below longitude range    **/
        case BNDRYUNDERSHT: /** feature undershoots whole degree project outside boundary ***/
        case LBNDUSHT:  /** unconnected line end node undershoots whole-degree boundary ***/
        case PERIMLEN: /*** linear or areal perimeter with short total length ***/
        case SHORTFEAT:  /** short length line feature not on quarter degree 'boundary' ***/
        case PC_SLOPE: /*** line feature segment with percent slope above tolerance ****/
        case LONGFEAT:   /** line or area feature with total length above threshold ***/
        case SHARE3SEG: /** line feature segment overlaps 2 other line feature segments and / or area perimeter edges ***/
        case CALC_AREA:  /*** point feature with LEN and WID attr values product < tolerance ***/
          FreadFwriteObjectAndMagnitude(keyval, 0, unsortout, NULL);
          OKtoSort = 1;
          break;

        case COVERFAIL: /** to detect holes in surface; MGCP landcover requirement ***/
          dbljunk = FreadFwriteObjectAndMagnitude(keyval, 0, unsortout, NULL);
          MCFG = (int) dbljunk;
          if(MCFG > MaxCOVERFAILgroup)
             MaxCOVERFAILgroup = MCFG;

          OKtoSort = 1;
          break;
	  
        case LVPROX: /** line vertex near another line **/
        case PLPROX: /** point feature within x of a line feature **/
        case PSHOOTL: /*** point feature over or undershoots a line feature ***/
        case PLPROXEX:  /** pt to line prox with exception for line end node ***/
        case ENCONNECT: /** end node connectivity **/
        case BADENCON: /** bad sequence on line feature connections ***/
        case FEATBRIDGE: /** one linear feature serves as only connection between 2 other features of same type ***/
        case LENOCOVERA: /** line end node not covered by area perimeter ***/
          FreadFwritePointEdgeAndMagnitude(keyval, 0, unsortout, NULL);
          OKtoSort = 1;
          break;
	  
        case VTEAR: /** vertical tear **/
        case LELINEPROX:  /** line end - line proximity ***/
        case LUNDERSHTL:  /** line end - line undershoot **/
        case LUSHTL_CLEAN: /* like line - line undershoot, but no condition if feature mid-undershoot **/
        case LUSHTL_DF: /** line - line undershoot, different line feature types ***/
        case LOSHTL_DF: /** line - line overshoot, different line feature types ***/
        case LVUSHTL: /** interior line vertex undershoots a different line feature **/
        case VUSHTL_CLEAN: /* like vertex - line undershoot, but no condition if feature mid-undershoot **/
        case LVOSHTL: /** interior line vertex overshoots a different line feature ***/
        case EN_EN_PROX:  /** undershoot end nodes connected by another feature **/
        case LOVERSHTL:   /** line end - line overshoot **/
        case LUNDERSHTA:  /** line end area perimeter undershoot **/
        case LOVERSHTA:  /** line end - area perimeter overshoot **/
        case LAPROX:  /** line to area proximity - smallest dist between the two features ***/
        case LASLIVER: /** sliver formed between line and area features **/
        case LSLICEA: /** line 'slices' area so as create a small piece ***/
        case LLSLIVER:  /** sliver formed between two line features **/
        case AUNDERSHTA: /** area edge undershoots neighbor area edge ***/
        case AOVERSHTA: /** area edge overshoots neighbor area edge ***/
        case LLMULTINT: /** lines intersect each other multiple times **/
        case LOC_MULTINT: /** lines with no or compatible LOC values intersect each other multiple times **/
        case L2D_L3D_MATCH:   /**** Linear End - Linear End Z Mismatch ***/
        case LEZ_PROX_3D: /** apply check L2D_L3D_MATCH to 3d line features only **/
        case CNODE_ZBUST:  /*** Z mismatch between any two connecting nodes (in x,y) ***/
        case LSPINT: /** line intersects poly with slope > x **/
        case SHARESEG: /** line feature segment overlaps 1 other line feature segment ***/
        case LLI_ANGLE: /*** 2 lines intersect at severe angle ***/
        case LSPIEXP: /** line - poly (slope > x) except when intersection **/
        case PTPTPROX: /** point to point proximity **/
        case PUNDERSHTA: /** point not on area perimeter and is outside that area feature **/
        case POVERSHTA: /** point not on area perimeter and is inside that area feature **/
        case LODELEVDIF: /**  interpolated elev difference between grids or polys in different LOD **/
        case GRIDEXACTDIF: /** Grids have post value difference at same X,Y ***/
        case MASKSHOREL: /** water body not contained by shoreline ***/
        case CLAMP_SEG: /*** catenary segment below associated DEM ****/
        case DUPLICATESEG:    /*** Linear Features That include duplicate Segments ***/
        case EXTRA_NET:   /*** vertex is a near miss in connecting to another vertex to join networks ***/
        case INTRA_NET:   /*** vertex is close to but not identical with another vertex in the same network ***/
        case LJOINSLOPEDC: /** slope direction change along linear **/
        case CONTEXT_KINK:  /*** kink based on one high angle next to one lower (moderate) angle ***/
        case SHAREPERIM:  /** Area features that share portion of perimeter **/
        case FAILMERGEL:  /** line object that should be merged with connecting line ***/
        case FAILMERGEL2:  /** line object that should be merged with connecting line no accounting for metadata  ***/
        case LRNGE_UNM_LAT:
        case LRNGE_UNM_LON:
          FreadFwriteMagnitudeAndTwoObjects(keyval, 0, unsortout, NULL);
	  
          OKtoSort = 1;
          break;

        case MASKCONSTANT: /*** DEM not constant elev at pointdefined by specified mask value ***/
        case MASKEDIT_0:
        case MASKEDIT_1: /** EDM has primary tolerance value, diff between TDR and TDF is > secondary tolerance **/
        case MASKZERO: /** DEM not zero elev at point defined by specified mask value ***/
        case MASKCONF2: /** variation of Grids with conflicting values **/
        case MASKCONFLICT: /** Grid DEM Masks have conflicting values ***/
        case PT_GRID_DIF: /** point and grid z value mismatch at exact coord, no interpolation **/

        case RAISEDPC: /** number of raised shoreline points exceeds tolerance **/
        case FLOWSTEP:  /** step size in river flow above threshold ***/
        case BREAKLINE: /** river elevation change at bad angle with shorelines ***/
        case WATERMMU: /** minimum mapping unit for water body below threshold ***/
        case AVGSPIKE: /** spike / well as compared to average elevation of neighbor posts ***/
        case GSHELF:  /** looking for shelf formations like PUE in DEM ***/
        case AWITHOUTA:  /** area that does not fully contain a second area ***/
        case LOSMINHGT:
        case ELEVGT:
        case ELEVLT: 
        case ELEVEQ:  
        case ELEVEQOPEN:
        case FEATOUTSIDE:  /*** a feature lies at least partly outside the MGCP cell ***/
           FreadFwriteDynamicInfo(keyval, 0, unsortout, NULL,-1.0,-1.0,-1.0,-1,-1,-1.0,-1.0);
           OKtoSort = 1;
           break;


        default:
          printf("bad keyval 1 (%d) in %s file\n",keyval,unsortlog);
          exit(-1);
        }  /** end switch on keyval, second time, for read & insertion sort flag set ***/
      
      if(OKtoSort > 0)
        {
          if(UpdateStatus == 1)
             periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 1\n");
          cIE = (struct IndexEntry *) (malloc(SzIE));
          if(cIE == NULL)
            {
              printf("ReRackAllConditions: out of allocation memory\n");
              exit(-1);
            }
          cIE->fileposn = fileposn;
          cIE->endfileposn = fileposn;
          cIE->Cindex = GenericErr.Cnumber;
          if(keyval == TVERT)
            cIE->magnitude = GenericErr.idn1;
          else if((keyval == MULTIPARTA) || (keyval == MULTIPARTP) || (keyval == MULTIPARTL))
            {
	      dbljunk = (double) GenericErr.idn1;
	      while(dbljunk > 0.1)
		{
		  dbljunk = dbljunk / 10.0;
		}
	      cIE->magnitude = GenericErr.magnitude + dbljunk;
            }
          else if(OKtoSort > 1) /*** these conditions don't really have a magnitude, so use the LocalID1 field ***/
            {
            cIE->magnitude = GenericErr.LocalID1;
            }
          else
            cIE->magnitude = GenericErr.magnitude;
          cIE->next = NULL;
          lastkeyval = keyval;
          LastCloneNumber = GenericErr.Cnumber;
        }
      
      else if((keyval != lastkeyval) || (LastCloneNumber != GenericErr.Cnumber))
        {
          cIE = (struct IndexEntry *) (malloc(SzIE));
          if(cIE == NULL)
            {
              printf("ReRackAllConditions: out of allocation memory\n");
              exit(-1);
            }
          cIE->fileposn = fileposn;
          cIE->endfileposn = fileposn;
          cIE->Cindex = GenericErr.Cnumber;
          if(lastkeyval != keyval)
            {
              if((Index[keyval].entry == NULL) || (cIE->Cindex <= Index[keyval].entry->Cindex))
                {
                  cIE->next = Index[keyval].entry;
                  Index[keyval].entry = cIE;
                  lastkeyval = keyval;
                }
              else
                {
                  pIE = Index[keyval].entry;
                  nIE = pIE;
                  while((pIE != NULL) && (pIE->Cindex < cIE->Cindex))
                    {
                      nIE = pIE;
                      pIE = pIE->next;
                    }
                  cIE->next = pIE;
                  nIE->next = cIE;
                }
            }
          else if((Index[keyval].entry != NULL) && (Index[keyval].entry->Cindex > cIE->Cindex))
            {
              cIE->next = Index[keyval].entry;
              Index[keyval].entry = cIE;
            }
          else if(LastCloneNumber != GenericErr.Cnumber) /***&& (GenericErr.Cnumber >= 0)) ***/
            {
              if(Index[keyval].entry == NULL)
                {
                  cIE->next = NULL;
                  Index[keyval].entry = cIE;
                }
              else
                {
                  pIE = nIE = Index[keyval].entry;
                  while(cIE->Cindex >= nIE->Cindex)
                    {
                      pIE = nIE;
                      nIE = nIE->next;
                      if(nIE == NULL)
                        {
                          break;
                        }
                    }
                  pIE->next = cIE;
                  cIE->next = nIE;
                }
            }
          lastkeyval = keyval;
          LastCloneNumber = GenericErr.Cnumber;
	  
        } /** end non-sorted loop process **/

      if(UpdateStatus == 1)
          periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 1\n");
      
      
      if(cIE != NULL)
        cIE->endfileposn = ftell(unsortout);
      
      
      /** now do the red-black tree sort ***/
      
      if((OKtoSort > 0) && (cIE != NULL))
        {
        if(UpdateStatus == 1)
             periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 1\n");
	  if(cIE->Cindex > 0) /*** must be a clone condition ****/
	    {
	      cIEcloneIndex = GetCloneIndex(cIE->Cindex,keyval);
	      if(CloneConditions[cIEcloneIndex].RB_Entry == NULL) /*** need to initialize tree ***/
		{
		  CloneConditions[cIEcloneIndex].kv = keyval;
		  
		  if(ErrorLookup[keyval].bigworse == 1)
		    {
		      CloneConditions[cIEcloneIndex].RB_Entry = ETF_RBTreeCreate(ETF_DblCompLE,ETF_DblDest,ETF_InfoDest,ETF_IntPrint,ETF_InfoPrint);
		      ETF_RBTreeInsert(CloneConditions[cIEcloneIndex].RB_Entry,&cIE->magnitude,cIE);
		    }
		  else
		    {
		      CloneConditions[cIEcloneIndex].RB_Entry = ETF_RBTreeCreate(ETF_DblCompGE,ETF_DblDest,ETF_InfoDest,ETF_IntPrint,ETF_InfoPrint);
		      ETF_RBTreeInsert(CloneConditions[cIEcloneIndex].RB_Entry,&cIE->magnitude,cIE);
		    }
		}
	      else
		{
		   ETF_RBTreeInsert(CloneConditions[cIEcloneIndex].RB_Entry,&cIE->magnitude,cIE);
		}
	    }
	  else  /*** must be a regular template instantiation ***/
	    if(RegularConditions[keyval].RB_Entry == NULL) /*** need to initialize tree ***/
              {
		RegularConditions[keyval].kv = keyval;
		if(ErrorLookup[keyval].bigworse == 1)
		  {
		    RegularConditions[keyval].RB_Entry = ETF_RBTreeCreate(ETF_DblCompLE,ETF_DblDest,ETF_InfoDest,ETF_IntPrint,ETF_InfoPrint);
		    ETF_RBTreeInsert(RegularConditions[keyval].RB_Entry,&cIE->magnitude,cIE);
		  }
		else
		  {
		    RegularConditions[keyval].RB_Entry = ETF_RBTreeCreate(ETF_DblCompGE,ETF_DblDest,ETF_InfoDest,ETF_IntPrint,ETF_InfoPrint);
		    ETF_RBTreeInsert(RegularConditions[keyval].RB_Entry,&cIE->magnitude,cIE);
		  }
              }
	    else
              {
                   ETF_RBTreeInsert(RegularConditions[keyval].RB_Entry,&cIE->magnitude,cIE);
              }
        }

     if(UpdateStatus == 1)
          periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 1\n");
      
      FreeGenericErrorAllocations();
      
      fileposn = ftell(unsortout);
      SEEIT_fread_int(&keyval,unsortout);
    }  /**** end  while(feof(unsortout) == 0) ***/


   if(MaxCOVERFAILgroup > 0)
      {
      ++MaxCOVERFAILgroup;
      CFGcounts = (int *) (malloc(SzI * MaxCOVERFAILgroup));
      for(i=0; i<MaxCOVERFAILgroup; i++)
         CFGcounts[i] = 0;
      }
  
  
  for(i=0; i<=CONDITION_DEFINITIONS; i++)
    {
    if(UpdateStatus == 1)
       periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 2\n");
      if(RegularConditions[i].RB_Entry != NULL)
	{
	  CListRoot = NULL;
	  CListLast = NULL;
	  RetrieveConditionTreeInorder(RegularConditions[i].RB_Entry,RegularConditions[i].RB_Entry->root->left);
	  Index[i].entry = CListRoot;
	  
	  nIE = CListLast;
	  nIE->next = NULL;
	  ETF_RBTreeDestroy(RegularConditions[i].RB_Entry);
	  RegularConditions[i].RB_Entry = NULL;
	  RegularConditions[i].kv = -1;
	}
    }

  
  nIE = NULL;
  for(j = 0; j<CLONE_DEFINITIONS; j++)
    {
    if(UpdateStatus == 1)
       periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 2\n");
      if(CloneConditions[j].RB_Entry != NULL)
	{
	  i = CloneConditions[j].kv;
	  CListRoot = NULL;
	  CListLast = NULL;
	  RetrieveConditionTreeInorder(CloneConditions[j].RB_Entry,CloneConditions[j].RB_Entry->root->left);
CListLast->next = Index[i].entry;
Index[i].entry = CListRoot;
	  ETF_RBTreeDestroy(CloneConditions[j].RB_Entry);
	  CloneConditions[j].RB_Entry = NULL;
	  CloneConditions[j].kv = -1;
	}
    }

/***printf("phase 2b condition report prep complete\n"); **/
  
  DupRoot = NULL;
  
  if(NGA_TYPE==0)
    {
      if(Index[TVERT].entry != NULL)
	{
	  pIE = cIE = Index[TVERT].entry;
	  while(cIE != NULL)
	    {
    if(UpdateStatus == 1)
       periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 2\n");
	      duplicatefound = 0;
	      fseek(unsortout,cIE->fileposn,SEEK_SET);
	      SEEIT_fread_int(&keyval,unsortout);
	      if(keyval == TVERT)
		{
		  FreadFwritePointAndObject(keyval,0,unsortout,NULL,-1.0,-1.0,-1.0,-1,-1,-1.0);
		  if(DupRoot == NULL)
		    {
		      cDP = (struct dupremoval *) (malloc(SzDR));
		      if(cDP == NULL)
			{
			  printf("allcoation memory exhausted during condition sort\n");
			  exit(-1);
			}
		      cDP->next = NULL;
		      cDP->IDN = GenericErr.idn1;
		      cDP->X = GenericErr.px;
		      cDP->Y = GenericErr.py;
		      cDP->Cindex = GenericErr.Cnumber;
		      DupRoot = cDP;
		    }
		  else
		    {
		      cDP = DupRoot;
		      while(cDP != NULL)
			{
			  if((cDP->IDN == GenericErr.idn1) && (cDP->Cindex == GenericErr.Cnumber) &&
			     (cDP->X == GenericErr.px) && (cDP->Y == GenericErr.py))
			    {
			      duplicatefound = 1;
			      if(Index[TVERT].entry == cIE)
				{
				  nIE = Index[TVERT].entry;
				  Index[TVERT].entry = cIE->next;
				  free(nIE);
				  pIE = cIE = Index[TVERT].entry;
				}
			      else
				{
				  pIE->next = cIE->next;
				  nIE = cIE;
				  cIE = cIE->next;
				  free(nIE);
				}
			      break;
			    }
			  else if(cDP->IDN > GenericErr.idn1)
			    {
			      cDP = NULL;
			      break;
			    }
			  cDP = cDP->next;
			}
		      if(cDP == NULL)
			{
			  cDP = (struct dupremoval *) (malloc(SzDR));
			  if(cDP == NULL)
			    {
			      printf("allcoation memory exhausted during condition sort\n");
			      exit(-1);
			    }
			  
			  cDP->next = DupRoot;
			  cDP->IDN = GenericErr.idn1;
			  cDP->X = GenericErr.px;
			  cDP->Y = GenericErr.py;
			  cDP->Cindex = GenericErr.Cnumber;
			  DupRoot = cDP;
			}
		    }
		  FreeGenericErrorAllocations();
		}
	      if(duplicatefound == 0)
		{
		  pIE = cIE;
		  cIE = cIE->next;
		}
	      else
		{
		  num_t_dupes++;
		}
	    }
	}
    }

  
  cDP = DupRoot;
  while(cDP != NULL)
    {
      pDP = cDP;
      cDP = cDP->next;
      free(pDP);
    }
  DupRoot = NULL;
  
  /** not really necessary, but reposition fileptr at top of file **/
  fseek(unsortout,0,SEEK_SET);
  /** open the new file of errors to write contiguous blocks by error type **/
  errtypeinout = fopen(errtypelog,"wb");
  if(errtypeinout == NULL)
    {
      printf("fatal error: could not open output file in function ReRackUnsortdErrors\n");
      exit(-1);
    }
  
  write_endian(errtypeinout);
  
  if(CDFREPORT)
    {
      CDFout = fopen(CDFlog,"wt");
      if(CDFout == NULL)
        {
          printf("error during attempt to open CDF report file; no CDF report will be generated\n");
          CDFREPORT = 0;
        }
    }
  
  
  if(ZeroIngOut == 1) /** are we supposed to reset the counter for some specific error condition ?   ****/
    {
      for(i=1; i<=CONDITION_DEFINITIONS; i++)
        {
          if(ErrorToZero == i)
            {
              if(InstanceToZero == 0) /*** zero out the main condition ***/
                {
                  ErrorLookup[i].number = 0;
                  pIE = cIE = Index[i].entry;
                  while(cIE != NULL)
                    {
                      pIE = cIE;
                      cIE = cIE->next;
                      if(pIE->Cindex==0) /* this error pointer is a root error, not a clone */
                        {
                          Index[i].entry = cIE;
                          free(pIE);
                        }
                      else
                        {
                          break;
                        }
                    }
                }
              else
                {
                  kv2 = GetCloneIndex(InstanceToZero,ErrorToZero);
                  CloneErrorLookup[kv2].number = 0;
		  
		  
                  pIE = NULL;
                  nIE = cIE = Index[i].entry;
                  while(cIE != NULL)
                    {
                      nIE = cIE;
                      cIE = cIE->next;
                      if(nIE->Cindex == InstanceToZero)
                        {
                          if(pIE == NULL)
                            Index[i].entry = cIE;
                          else
                            pIE->next = cIE;
                          free(nIE);
                        }
                      else if(nIE->Cindex < InstanceToZero)
                        {
                          pIE = nIE;
                        }
                      else /** must be that we have passed the clone of interest ***/
                        {
                          break;
                        }
                    }
                }
            }
        }
    }
  
  GE3 = (struct ConditionObjectForRW_DC * ) (malloc(sizeof(struct ConditionObjectForRW_DC)));
  GE4 = (struct ConditionObjectForRW_DC * ) (malloc(sizeof(struct ConditionObjectForRW_DC)));
  if(GE4 == NULL)
    {
      printf("allocation memory has been exhausted during condition sorting (malloc of ConditionObjectForRW_DC (1))\n");
      exit(-1);
    }
  GE3->MaxVerts1 = 1000;
  GE4->MaxVerts1 = 1000;
  GE3->x1 = (double *) (malloc(SzD * GE3->MaxVerts1 + 2));
  GE3->y1 = (double *) (malloc(SzD * GE3->MaxVerts1 + 2));
  GE3->z1 = (double *) (malloc(SzD * GE3->MaxVerts1 + 2));
  GE4->x1 = (double *) (malloc(SzD * GE4->MaxVerts1 + 2));
  GE4->y1 = (double *) (malloc(SzD * GE4->MaxVerts1 + 2));
  GE4->z1 = (double *) (malloc(SzD * GE4->MaxVerts1 + 2));
  
  GE3->MaxVerts2 = 1000;
  GE4->MaxVerts2 = 1000;
  GE3->x2 = (double *) (malloc(SzD * GE3->MaxVerts2 + 2));
  GE3->y2 = (double *) (malloc(SzD * GE3->MaxVerts2 + 2));
  GE3->z2 = (double *) (malloc(SzD * GE3->MaxVerts2 + 2));
  GE4->x2 = (double *) (malloc(SzD * GE4->MaxVerts2 + 2));
  GE4->y2 = (double *) (malloc(SzD * GE4->MaxVerts2 + 2));
  GE4->z2 = (double *) (malloc(SzD * GE4->MaxVerts2 + 2));
  
  if(GE4->z2 == NULL)
    {
      printf("allocation memory has been exhausted during condition sorting (malloc of ConditionObjectForRW_DC (2))\n");
      exit(-1);
    }
  
  GE3->SIDlen1Max = 200;
  GE4->SIDlen1Max = 200;
  GE3->SIDlen2Max = 200;
  GE4->SIDlen2Max = 200;
  GE3->SID1 = (char *) (malloc(GE3->SIDlen1Max + 1));
  GE3->SID2 = (char *) (malloc(GE3->SIDlen2Max + 1));
  
  GE4->SID1 = (char *) (malloc(GE4->SIDlen1Max + 1));
  GE4->SID2 = (char *) (malloc(GE4->SIDlen2Max + 1));
  
  if(GE4->SID2 == NULL)
    {
      printf("allocation memory has been exhausted during condition sorting (malloc of ConditionObjectForRW_DC) (3)\n");
      exit(-1);
    }
  
  GE3->errmsgMax = 0;
  GE3->errmsg = NULL;
  GE4->errmsgMax = 0;
  GE4->errmsg = NULL;
  
  Current_GE = GE3;
  Last_GE = NULL;
  
  GE_DUP_Root = NULL;
  
  SpecificConditionNumber = 0;
  
  LastCnumber = -1;
  LastKV = -1;
  LastLocalID1 = -1.0;
  LastLocalID2 = -1.0;
  LastPX = LastPY = LastPZ = -123455.98765;

  NMDRroot = NULL;
  
  Obj_Mdl_Flags = (int * ) (malloc(SzI * (NumberOfModels + 2)));
  if(Obj_Mdl_Flags == NULL)
     {
     printf("all available memory has been consumed, execution must terminate now\n");
     exit(-1);
     }
     
  ObjsWithErrs = 0;
  ElevPwithErrs = 0;
  ElevOwithErrs = 0;
  TtlObjErrs = 0;
  Ttle_count = 0;
  Ttlign_count = 0;
  I_Applied = 0;
  e_count = 0;
  ign_count = 0;


  for(i=0; i<= CONDITION_DEFINITIONS; i++)
     {
     IGN[i] = NULL;
     }



  if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
     {
      periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\nConsulting Previously Ignored Conditions\n");
     
	  
 	  thisfile = 1;

	  while(1)
	  {

	   foundpointfile = FindShapeFile(1,thisfile);
       foundlinefile  = FindShapeFile(2,thisfile);

 	   if(
		   ((foundpointfile + foundlinefile)==0) ||
		   (thisfile>5000)
		 )
	   {
		 break;
	   }
	   thisfile = thisfile + 1;

       if(foundpointfile==1)
	   {
	    printf("processing PT %s\n",IgnoredPointFile2);
	    ProcessSF(IgnoredPointFile2);
		printf("done\n");
	   }
      
       if(foundlinefile==1)
	   {
	    printf("processing LN %s\n",IgnoredLineFile2);
	    ProcessSF(IgnoredLineFile2);
		printf("done\n");
	   }

	  }
     }




  for(i=CONDITION_DEFINITIONS; i>= 1; i--)
    {
      for(k=0; k<= NumberOfModels; k++)
         Obj_Mdl_Flags[k] = 0;
      e_count = 0;
      ign_count = 0;
      if(Index[i].entry != NULL) /** then have recorded some errors above, so write them **/
        {
              cIE = Index[i].entry;
              lastkeyval = -1;
              LastCloneNumber = -1;
              kv2 = 0;
              while(cIE != NULL)
                {
                  if(UpdateStatus == 1) 
                     periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 3\n");
                  fseek(unsortout,cIE->fileposn,SEEK_SET);
                  SEEIT_fread_int(&keyval,unsortout);
                  fileposn = ftell(errtypeinout);
                  switch(keyval)
                    {
                    case TVERT: /** T-vertex, not really sortable, but treated that way for duplicate removal **/
                      DC_FreadFwritePointAndObject(keyval,1,&DupValue,unsortout,errtypeinout);
                      if(DupValue == 0)
			{
			  if((LastKV != keyval) || (LastCnumber != Current_GE->Cnumber))
                            {
			      LastKV = keyval;
			      LastCnumber = Current_GE->Cnumber;
			      SpecificConditionNumber = 1;
                            }
			  else
                            SpecificConditionNumber += 1;
                          LastLocalID1 = Current_GE->localID1;
			  newobjectwitherrs = AddToObjectConditionTree(Current_GE->localID1, Current_GE->idn1,keyval, Current_GE->Cnumber,
                                                   Current_GE->ECC1, -1, /** -1 signal that only 1 ECC used **/
                                                   Current_GE->magnitude, Current_GE->Lindex1, -1,
						   SpecificConditionNumber,  Current_GE->gform1, &ObjsWithErrs, &TtlObjErrs);
                          if(Current_GE->gform1 == G_GRIDPT)
                             {
                             ElevPwithErrs += 1;
                             if(newobjectwitherrs > 0)
                                ElevOwithErrs += 1;
                             }
                          Obj_Mdl_Flags[CrsWlk[Current_GE->Lindex1].crossindex] += 1;
                          e_count += 1;
			  if(Current_GE->Cnumber > 0)
			    {
			      cloneindex = GetCloneIndex(Current_GE->Cnumber,keyval);
			      
			      CloneErrorLookup[cloneindex].number += 1;
			      if(Current_GE->Cnumber != LastCloneNumber)
				{
				  LastCloneNumber = Current_GE->Cnumber;
				  CloneErrorLookup[cloneindex].fileposn = fileposn;
				}
                            if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                               {
                               ign_count = ign_count + CompGenericErrToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 1, 1);
                               }
			    }
			  else
			    {
			      ErrorLookup[i].number += 1;
                              if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                 {
                                 ign_count = ign_count + CompGenericErrToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 1, 1);
                                 }

			      if(lastkeyval != i)
				{
				  lastkeyval = i;
				  ErrorLookup[i].fileposn = fileposn;
				}
			    }
                        }
                      break;
		      
                    case AREAKINK: /** high angle on perimeter of area feature **/
                    case INCLSLIVER: /** areal with included sliver **/
                    case ZUNCLOSED: /** area feat not closed in Z **/
                    case AREAUNCLOSED: /** area feature unclosed in x,y, or z **/
                    case NOT_FLAT:  /*** area feature with surface that is not uiform elevation ***/
                    case CLAMP_NFLAT: /** area feature does not have constant elevation when clamped to underlying DEM ***/
                    case CLAMP_DIF: /** difference between feature vertex z value and interpolated DEM value ***/
                      dbljunk = DC_FreadFwritePointObjectAndMagnitude(keyval, 1, &DupValue, unsortout, errtypeinout);
                      if(DupValue == 0)
                        {
			  /*** note are only adding one of the localID objects from this - all seem to be the same ***/
			  if((LastKV != keyval) || (LastCnumber != Current_GE->Cnumber))
			    {
			      LastKV = keyval;
			      LastCnumber = Current_GE->Cnumber;
			      SpecificConditionNumber = 1;
			    }
			  else
			    SpecificConditionNumber += 1;
                          LastLocalID1 = Current_GE->localID1;
			  newobjectwitherrs = AddToObjectConditionTree(Current_GE->localID1, Current_GE->idn1,keyval, Current_GE->Cnumber,
                                                   Current_GE->ECC1, -1, /** -1 signal that only 1 ECC used **/
                                                   Current_GE->magnitude, Current_GE->Lindex1, -1,
						   SpecificConditionNumber,  Current_GE->gform1, &ObjsWithErrs, &TtlObjErrs);
                          Obj_Mdl_Flags[CrsWlk[Current_GE->Lindex1].crossindex] += 1;
                          if(Current_GE->gform1 == G_GRIDPT)
                             {
                             ElevPwithErrs += 1;
                             if(newobjectwitherrs > 0)
                                ElevOwithErrs += 1;
                             }
                          e_count += 1;
                          if(e_count == 1)
                             {
                             e_max = e_min = Current_GE->magnitude;
                             }
                          else
                             {
                             if(Current_GE->magnitude > e_max)
                                e_max = Current_GE->magnitude;
                             if(Current_GE->magnitude < e_min)
                                e_min = Current_GE->magnitude;
                             }
			  
			  if(Current_GE->Cnumber > 0)
			    {
			      cloneindex = GetCloneIndex(Current_GE->Cnumber,keyval);
			      
			      CloneErrorLookup[cloneindex].number += 1;
			      if(Current_GE->Cnumber != LastCloneNumber)
				{
				  LastCloneNumber = Current_GE->Cnumber;
				  CloneErrorLookup[cloneindex].fileposn = fileposn;
				}
                           if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0)) 
                               {
                               ign_count = ign_count + CompCurrentGE_ToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 1, 1);
                               }
			    }
			  else
			    {
			      ErrorLookup[i].number += 1;
                              if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                 {
                                 ign_count = ign_count + CompCurrentGE_ToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 1, 1);
                                 }
			      if(lastkeyval != i)
				{
				  lastkeyval = i;
				  ErrorLookup[i].fileposn = fileposn;
				}
			    }
                        }
		      break;
		      
                    case ISOTURN: /** high turn angle w/o 3d feature present ***/
                    case KINK:  /** high angle between adjacent linear segments **/
                    case Z_KINK: /** consecutive kinks form a 'Z' ***/
                    case L_A_KINK: /** kink at intersection of line end node  and area feature perim **/
                    case INTERNALKINK: /** kink internal to single line feature **/
                    /***case CONTEXT_KINK:  *** kink based on one high angle next to one lower (moderate) angle ***/
                    case SLOPEDIRCH: /*** slope direction change along linear **/
                    case CLAMP_SDC: /*slope direction change along a line that has been elevation-value clamped to underlying DEM ***/
                    case CLAMP_JOINSDC: /** slope direction change at line feature connection when both are clamped to DEM ***/
		      /*** case LOOPS: ** self-intersecting linear or areal ***/
                    case SLIVER: /** sliver triangle **/
                    case FACESIZE: /*** small area on face of area feature **/
                    case ARNGE_UNM_LAT:
                    case ARNGE_UNM_LON:
                    case NARROW:  /** narrow triangle **/
                    case SMALLOBJ:  /** small 3d area poly **/
                    case HSLOPE: /** high slope poly **/
                    case HTEAR: /** horizontal tear **/
                    case OVERC: /** over-covered edge **/
                    case GSPIKE:  /** grid data spike point ***/
                    /***case AVGSPIKE: ** spike / well as compared to average elevation of neighbor posts ***/
                    /***case FLOWSTEP:  ** step size in river flow above threshold ***/
                    /***case WATERMMU: ** minimum mapping unit for water body below threshold ***/
                    /**case RAISEDPC: ** number of raised shoreline points exceeds tolerance **/
                    case GRID_STD_DEV: /** grid elev value, inside feature polygon, over range offset from std deviation **/
                    case MULTIPARTL: /** multi-part line ***/
                    case MULTIPARTA: /** multi-part area **/
                    case MULTIPARTP: /** multi-part point **/
                    case ELEVADJCHANGE:  /** change in adjacent node elevations > threshold ***/
                    case FEATSPIKE: /** elevation spike along 3D feature ***/
                    case SEGLEN: /** linear or areal perimeter segment with length below threshold ***/
                    case LONGSEG: /** linear or areal perimeter segment with length at or above threshold ***/
                    case BIGAREA: /** area feature with large square area **/
                    case SMALLAREA: /** area feaure with small square area **/
/**case MASKMONO: ** DEM not monotonic at point defined by specified mask value ***/
                    case SMLCUTOUT: /** small included area inner ring of area feature ***/
                    case OSIDE_LAT:   /**** feature coordinate above or below latitude range    **/
                    case OSIDE_LON:   /**** feature coordinate above or below longitude range    **/
                    case BNDRYUNDERSHT: /** feature undershoots whole degree project outside boundary ***/
                    case LBNDUSHT:  /** unconnected line end node undershoots whole-degree boundary ***/
                    case PERIMLEN: /*** linear or areal perimeter with short total length ***/
                    case SHORTFEAT:  /** short length line feature not on quarter degree 'boundary' ***/
                    case PC_SLOPE: /*** line feature segment with percent slope above tolerance ****/
                    case LONGFEAT:   /** line or area feature with total length above threshold ***/
                    case SHARE3SEG: /** line feature segment overlaps 2 other line feature segments and / or area perimeter edges ***/
                    case CALC_AREA:  /*** point feature with LEN and WID attr values product < tolerance ***/
                    case COVERFAIL: /** to detect holes in surface; MGCP landcover requirement ***/
                      dbljunk =  DC_FreadFwriteObjectAndMagnitude(keyval, 1, &DupValue, unsortout, errtypeinout);
                      if(DupValue == 0)
			{
			  if((LastKV != keyval) || (LastCnumber != Current_GE->Cnumber))
                            {
			      LastKV = keyval;
			      LastCnumber = Current_GE->Cnumber;
			      SpecificConditionNumber = 1;
                            }
			  else
                            SpecificConditionNumber += 1;
                          LastLocalID1 = Current_GE->localID1;
                          if(keyval != COVERFAIL)
                             {
			     newobjectwitherrs = AddToObjectConditionTree(Current_GE->localID1, Current_GE->idn1,keyval, Current_GE->Cnumber,
                                                   Current_GE->ECC1, -1, /** -1 signal that only 1 ECC used **/
                                                   Current_GE->magnitude, Current_GE->Lindex1, -1,
						   SpecificConditionNumber,  Current_GE->gform1, &ObjsWithErrs, &TtlObjErrs);

                             Obj_Mdl_Flags[CrsWlk[Current_GE->Lindex1].crossindex] += 1;
                             }
                          else
                             {
                             MCFG = (int) Current_GE->magnitude;
                             CFGcounts[MCFG] += 1;
                             TtlCF_errors += 1;
                             }
                          if(Current_GE->gform1 == G_GRIDPT)
                             {
                             ElevPwithErrs += 1;
                             if(newobjectwitherrs > 0)
                                ElevOwithErrs += 1;
                             }

                          e_count += 1;
                          if(e_count == 1)
                             {
                             e_max = e_min = Current_GE->magnitude;
                             }
                          else
                             {
                             if(Current_GE->magnitude > e_max)
                                e_max = Current_GE->magnitude;
                             if(Current_GE->magnitude < e_min)
                                e_min = Current_GE->magnitude;
                             }
			  
			  if(Current_GE->Cnumber > 0)
			    {
			      cloneindex = GetCloneIndex(Current_GE->Cnumber,keyval);
			      
			      CloneErrorLookup[cloneindex].number += 1;
			      if(Current_GE->Cnumber != LastCloneNumber)
				{
				  LastCloneNumber = Current_GE->Cnumber;
				  CloneErrorLookup[cloneindex].fileposn = fileposn;
				}
                            if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                               {
                               ign_count = ign_count + CompCurrentGE_ToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 0, 1);
                               }
			    }
			  else
			    {
			      ErrorLookup[i].number += 1;
                              if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                 {
                                 ign_count = ign_count + CompCurrentGE_ToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 0, 1);
                                 }
			      if(lastkeyval != i)
				{
				  lastkeyval = i;
				  ErrorLookup[i].fileposn = fileposn;
				}
                            }
                        }
		      
                      break;

		      
                    case LVPROX: /** line vertex near another line **/
                    case LENOCOVERA: /** line end node not covered by area perimeter ***/
                    case PLPROX: /** point feature within x of a line feature **/
                    case PSHOOTL: /*** point feature over or undershoots a line feature ***/
                    case PLPROXEX:  /** pt to line prox with exception for line end node ***/
                    case ENCONNECT: /** end node connectivity **/
                    case BADENCON: /** bad sequence on line feature connections ***/
                    case FEATBRIDGE: /** one linear feature serves as only connection between 2 other features of same type ***/
                      dbljunk = DC_FreadFwritePointEdgeAndMagnitude(keyval, 1, &DupValue, unsortout, errtypeinout);
		      
                      if(DupValue == 0)
			{
			  if((LastKV != keyval) || (LastCnumber != Current_GE->Cnumber))
                            {
			      LastKV = keyval;
			      LastCnumber = Current_GE->Cnumber;
			      SpecificConditionNumber = 1;
                            }
			  else
                            SpecificConditionNumber += 1;
                          LastLocalID1 = Current_GE->localID1;
                          LastLocalID2 = Current_GE->localID2;
			  newobjectwitherrs = AddToObjectConditionTree(Current_GE->localID1, Current_GE->idn1,keyval, 
                                                   Current_GE->Cnumber,  Current_GE->ECC1, Current_GE->ECC2,
                                                   Current_GE->magnitude,Current_GE->Lindex1,Current_GE->Lindex2,
						   SpecificConditionNumber, Current_GE->gform1, &ObjsWithErrs, &TtlObjErrs);
                          if(Current_GE->gform1 == G_GRIDPT)
                             {
                             ElevPwithErrs += 1;
                             if(newobjectwitherrs > 0)
                                ElevOwithErrs += 1;
                             }
			  newobjectwitherrs = AddToObjectConditionTree(Current_GE->localID2, Current_GE->idn2,keyval,
                                                   Current_GE->Cnumber, Current_GE->ECC2, Current_GE->ECC1,
                                                   Current_GE->magnitude,Current_GE->Lindex2,Current_GE->Lindex1,
						   SpecificConditionNumber, Current_GE->gform2, &ObjsWithErrs, &TtlObjErrs);
                          Obj_Mdl_Flags[CrsWlk[Current_GE->Lindex1].crossindex] += 1;
                          Obj_Mdl_Flags[CrsWlk[Current_GE->Lindex2].crossindex] += 1;
                             
                          if(Current_GE->gform2 == G_GRIDPT)
                             {
                             ElevPwithErrs += 1;
                             if(newobjectwitherrs > 0)
                                ElevOwithErrs += 1;
                             }
                          e_count += 1;
                          if(e_count == 1)
                             {
                             e_max = e_min = Current_GE->magnitude;
                             }
                          else
                             {
                             if(Current_GE->magnitude > e_max)
                                e_max = Current_GE->magnitude;
                             if(Current_GE->magnitude < e_min)
                                e_min = Current_GE->magnitude;
                             }
			  
			  if(Current_GE->Cnumber > 0)
                            {
			      cloneindex = GetCloneIndex(Current_GE->Cnumber,keyval);
			      
			      CloneErrorLookup[cloneindex].number += 1;
			      if(Current_GE->Cnumber != LastCloneNumber)
				{
				  LastCloneNumber = Current_GE->Cnumber;
				  CloneErrorLookup[cloneindex].fileposn = fileposn;
				}
                            if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                               {
                               ign_count = ign_count + CompCurrentGE_ToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 1, 1);
                               }
                            }
			  else
			    {
			      ErrorLookup[i].number += 1;
                              if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                 {
                                 ign_count = ign_count + CompCurrentGE_ToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 1, 1);
                                 }
			      if(lastkeyval != i)
				{
				  lastkeyval = i;
				  ErrorLookup[i].fileposn = fileposn;
				}
			    }
                        }
                      break;
		      
                    case VTEAR: /** vertical tear **/
                    case LELINEPROX:  /** line end - line proximity ***/
                    case EN_EN_PROX:  /** undershoot end nodes connected by another feature **/
                    case LUNDERSHTL:  /** line end - line undershoot **/
                    case LUSHTL_CLEAN: /* like line - line undershoot, but no condition if feature mid-undershoot **/
                    case LVUSHTL: /** interior line vertex undershoots a different line feature **/
                    case VUSHTL_CLEAN: /* like vertex - line undershoot, but no condition if feature mid-undershoot **/
                    case LVOSHTL: /** interior line vertex overshoots a different line feature ***/
                    case LUSHTL_DF: /** line - line undershoot, different line feature types ***/
                    case LOSHTL_DF: /** line - line overshoot, different line feature types ***/
                    case LOVERSHTL:   /** line end - line overshoot **/
                    case LUNDERSHTA:  /** line end area perimeter undershoot **/
                    case LOVERSHTA:  /** line end - area perimeter overshoot **/
                    case LAPROX:  /** line to area proximity - smallest dist between the two features ***/
                    case LASLIVER: /** sliver formed between line and area features **/
                    case LSLICEA: /** line 'slices' area so as create a small piece ***/
                    case LLSLIVER:  /** sliver formed between two line features **/
                    case AUNDERSHTA: /** area edge undershoots neighbor area edge ***/
                    case AOVERSHTA: /** area edge overshoots neighbor area edge ***/
                    case LLMULTINT: /** lines intersect each other multiple times **/
                    case LOC_MULTINT: /** lines with no or compatible LOC values intersect each other multiple times **/
                    case L2D_L3D_MATCH:   /**** Linear End - Linear End Z Mismatch ***/
                    case LEZ_PROX_3D: /** apply check L2D_L3D_MATCH to 3d line features only **/
                    case CNODE_ZBUST:  /*** Z mismatch between any two connecting nodes (in x,y) ***/
                    case DUPLICATESEG:    /*** Linear Features That include duplicate Segments ***/
                    case EXTRA_NET:   /*** vertex is a near miss in connecting to another vertex to join networks ***/
                    case INTRA_NET:   /*** vertex is close to but not identical with another vertex in the same network ***/
                    case LJOINSLOPEDC: /** slope direction change along linear **/
                    case CONTEXT_KINK:  /*** kink based on one high angle next to one lower (moderate) angle ***/
                    case FAILMERGEL:  /** line object that should be merged with connecting line ***/
                    case FAILMERGEL2:  /** line object that should be merged with connecting line no accounting for metadata  ***/
                    case LRNGE_UNM_LAT:
                    case LRNGE_UNM_LON:
                    case SHAREPERIM:  /** Area features that share portion of perimeter **/
                    case LSPINT: /** line intersects poly with slope > x **/
                    case SHARESEG: /** line feature segment overlaps 1 other line feature segment ***/
                    case LLI_ANGLE: /*** 2 lines intersect at severe angle ***/
                    case LSPIEXP: /** line - poly (slope > x) except when intersection **/
                    case PTPTPROX: /** point to point proximity **/
                    case PUNDERSHTA: /** point not on area perimeter and is outside that area feature **/
                    case POVERSHTA: /** point not on area perimeter and is inside that area feature **/
                    case LODELEVDIF: /** elev difference between grids or polys in different LOD **/
                    case GRIDEXACTDIF: /** Grids have post value difference at same X,Y ***/
                    case MASKSHOREL: /** water body not contained by shoreline ***/
                    case CLAMP_SEG: /*** catenary segment below associated DEM ****/
                    case MASKCONSTANT:
                    case MASKZERO: /** DEM not zero elev at point defined by specified mask value ***/
                    case MASKEDIT_0: /** Raw DEM and Edited DEM different & Edit Mask has value zero**/
                    case MASKEDIT_1: /** EDM has primary tolerance value, diff between TDR and TDF is > secondary tolerance **/
                    case MASKCONF2: /** variation of Grids with conflicting values **/
                    case MASKCONFLICT: /** Grid DEM Masks have conflicting values ***/
                    case PT_GRID_DIF: /** point and grid z value mismatch at exact coord, no interpolation **/
                  
                    case RAISEDPC: /** number of raised shoreline points exceeds tolerance **/
                    case FLOWSTEP:  /** step size in river flow above threshold ***/
                    case BREAKLINE: /** river elevation change at bad angle with shorelines ***/
                    case WATERMMU: /** minimum mapping unit for water body below threshold ***/
                    case AVGSPIKE: /** spike / well as compared to average elevation of neighbor posts ***/
                    case GSHELF:  /** looking for shelf formations like PUE in DEM ***/
                    case AWITHOUTA:  /** area that does not fully contain a second area ***/
                    case LOSMINHGT:
                    case ELEVGT:
                    case ELEVLT:            
                    case ELEVEQ:            
                    case ELEVEQOPEN:
                    case FEATOUTSIDE:  /*** a feature lies at least partly outside the MGCP cell ***/
                      if((keyval == MASKCONSTANT) || (keyval == MASKEDIT_0) || (keyval == MASKZERO) || (keyval == AWITHOUTA) ||
                                   (keyval == MASKEDIT_1) || (keyval == MASKCONF2) || (keyval == MASKCONFLICT) ||
                                   (keyval == RAISEDPC) || (keyval == FLOWSTEP) || (keyval == WATERMMU) ||
                                   (keyval == AVGSPIKE) ||(keyval == ELEVGT) || (keyval == ELEVLT) || (keyval == GSHELF) ||
                                   (keyval == ELEVEQ) || (keyval == ELEVEQOPEN) || (keyval == LOSMINHGT) ||
                                   (keyval==BREAKLINE) || (keyval == PT_GRID_DIF) || (keyval == FEATOUTSIDE)  )
                          {
                          dbljunk = DC_FreadFwriteDynamicInfo(keyval, 1, &NumberOfObjects, &DupValue, unsortout, errtypeinout);
                          }
                      else
                         {
                         dbljunk = DC_FreadFwriteMagnitudeAndTwoObjects(keyval, 1, &DupValue, unsortout, errtypeinout); 
                         NumberOfObjects = 2;
                         }
                      if(DupValue == 0)
			{
			  if((LastKV != keyval) || (LastCnumber != Current_GE->Cnumber))
                            {
			      LastKV = keyval;
			      LastCnumber = Current_GE->Cnumber;
			      SpecificConditionNumber = 1;
                            }
			  else
                            SpecificConditionNumber += 1;

                          if(NumberOfObjects > 0)
                             {
                             LastLocalID1 = Current_GE->localID1;
			     newobjectwitherrs = AddToObjectConditionTree(Current_GE->localID1, Current_GE->idn1,
                                                   keyval, Current_GE->Cnumber, Current_GE->ECC1, Current_GE->ECC2,
                                                   Current_GE->magnitude,Current_GE->Lindex1,Current_GE->Lindex2,
						   SpecificConditionNumber, Current_GE->gform1, &ObjsWithErrs, &TtlObjErrs);
                             if(Current_GE->gform1 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                            Obj_Mdl_Flags[CrsWlk[Current_GE->Lindex1].crossindex] += 1;
                             }

                          if(NumberOfObjects > 1)
                             {
			     newobjectwitherrs = AddToObjectConditionTree(Current_GE->localID2, Current_GE->idn2,
                                                   keyval, Current_GE->Cnumber,  Current_GE->ECC2, Current_GE->ECC1,
                                                   Current_GE->magnitude,Current_GE->Lindex2,Current_GE->Lindex1,
						   SpecificConditionNumber, Current_GE->gform2, &ObjsWithErrs, &TtlObjErrs);
                             Obj_Mdl_Flags[CrsWlk[Current_GE->Lindex2].crossindex] += 1;
                             
                             if(Current_GE->gform2 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                             }
                          e_count += 1;
                          if(e_count == 1)
                             {
                             e_max = e_min = Current_GE->magnitude;
                             }
                          else
                             {
                             if(Current_GE->magnitude > e_max)
                                e_max = Current_GE->magnitude;
                             if(Current_GE->magnitude < e_min)
                                e_min = Current_GE->magnitude;
                             }

			  if(Current_GE->Cnumber > 0)
			    {
			      cloneindex = GetCloneIndex(Current_GE->Cnumber,keyval);
			      
			      CloneErrorLookup[cloneindex].number += 1;
			      if(Current_GE->Cnumber != LastCloneNumber)
				{
				  LastCloneNumber = Current_GE->Cnumber;
				  CloneErrorLookup[cloneindex].fileposn = fileposn;
				}
                            if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                               {
                               ign_count = ign_count + CompCurrentGE_ToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 0, 2);
                               }
			    }
			  else 
			    {
			      ErrorLookup[i].number += 1;
                              if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                 {
                                 ign_count = ign_count + CompCurrentGE_ToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 0, 2);
                                 }
			      if(lastkeyval != i)
				{
				  lastkeyval = i;
				  ErrorLookup[i].fileposn = fileposn;
				}
			    }
                        }
                      break;


                        case G_DUPS: /** duplicate poly (by x,y,z of vertices) **/
                        case SAMEID: /** same GFID or FLDBID, diff geom **/
                        case SAMEID_GDUP:  /** same GFID or FLDBID, same geom, diff attr ***/
                        case SAMEID_CDUP: /** same GFID or FLDBID, geom & attr ***/
                        /**case AGEOM_UNM_LAT:
                        case AGEOM_UNM_LON:**/
                        case ANY_SAMEID:  /** same unique identifier, except those that are complete duplicates ***/
                        case C_DUPS: /** complete duplicate, attribution and all **/
                        case POLYINTPOLY: /** two polygons, of selected types, intersect **/
                        case POLYINAREA: /** polygon lies wholly inside an areal **/
                        case PTINREGION:  /** point feature inside a typed polygon or areal **/
                        case PTINPROPER:  /** point feature inside an area feature - not within tolerance of edge (or edge or hole) **/
                        case ACOVERA: /* area covers area */
                        case FAILMERGEA:  /** area feature that should be merged with area that shares edge ***/
                        case FAILMERGEA2:  /** area feature that should be merged with area that shares edge - no accounting for metadata  ***/
                        case AINSIDEHOLE: /** area inside another areal's cutout ('illegal holes') ***/
                        case FEATNOTCUT:  /*** feature not cut at end node of second feature ***/
                        case ISOLINE:  /** line feature completely inside an area feature ***/
                        case LINSIDEA: /** line partly or entirely inside area feature ***/
                        case LSEGCOVERA: /** line segment overlaps an area feature perimeter ***/
                          if(FreadFwriteTwoObjects(keyval,1,unsortout,errtypeinout,LastKV,LastCnumber,LastLocalID1,LastLocalID2) > 0)
                             {
                             if((LastKV != keyval) || (LastCnumber != GenericErr.Cnumber))
                               {
                                 LastKV = keyval;
                                 LastCnumber = GenericErr.Cnumber;
                                 SpecificConditionNumber = 1;
                               }
                             else
                               SpecificConditionNumber += 1;
                             LastLocalID1 = GenericErr.LocalID1;
                             LastLocalID2 = GenericErr.LocalID2;
                             newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID1, GenericErr.idn1,
                                                   keyval, GenericErr.Cnumber, GenericErr.ECC1, GenericErr.ECC2,
                                                   GenericErr.magnitude, GenericErr.Lindex1, GenericErr.Lindex2,
                                                   SpecificConditionNumber, GenericErr.gform1, &ObjsWithErrs, &TtlObjErrs);
                             if(GenericErr.gform1 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                             newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID2, GenericErr.idn2,
                                                   keyval, GenericErr.Cnumber, GenericErr.ECC2, GenericErr.ECC1,
                                                   GenericErr.magnitude, GenericErr.Lindex2, GenericErr.Lindex1,
                                                   SpecificConditionNumber, GenericErr.gform2, &ObjsWithErrs, &TtlObjErrs);
                             Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex1].crossindex] += 1;
                             Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex2].crossindex] += 1;
                             
                             if(GenericErr.gform2 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                             e_count += 1;

                             if(GenericErr.Cnumber > 0)
                               {
                                 cloneindex = GetCloneIndex(GenericErr.Cnumber,keyval);
   
                                 CloneErrorLookup[cloneindex].number += 1;
                            
                                 if(cloneindex != LastCloneNumber)
                                   {
                                     LastCloneNumber = cloneindex;  
                                     CloneErrorLookup[cloneindex].fileposn = fileposn;
                                   }
                               if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                  {
                                  ign_count = ign_count + CompGenericErrToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 0, 2);
                                  }
                               }
                             else
                               {
                                 ErrorLookup[keyval].number += 1;
                                 if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                    {
                                    ign_count = ign_count + CompGenericErrToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 0, 2);
                                    }
                                 if(lastkeyval != i)
                                   {
                                     lastkeyval = i;
                                     ErrorLookup[i].fileposn = fileposn;
                                   }
                               }
                             }

                          FreeGenericErrorAllocations();
                          break;

                        case LGEOM_UNM_LAT:
                        case LGEOM_UNM_LON:
                        case AGEOM_UNM_LAT:
                        case AGEOM_UNM_LON:
                        case AUNM_ATTR_A:
                        case LUNM_ATTR_A:
                          jj = FreadFwriteMsgMagPointObjects(keyval,1,unsortout,errtypeinout,
                                   LastPX, LastPY, LastPZ, LastKV,LastCnumber,LastLocalID1,LastLocalID2);
                          if(jj > 0)
                             {
                             if((LastKV != keyval) || (LastCnumber != GenericErr.Cnumber))
                               {
                                 LastKV = keyval;
                                 LastCnumber = GenericErr.Cnumber;
                                 SpecificConditionNumber = 1;
                               }
                             else
                               SpecificConditionNumber += 1;
                             LastLocalID1 = GenericErr.LocalID1;
                             LastLocalID2 = GenericErr.LocalID2;
                             LastPX = GenericErr.px;
                             LastPY = GenericErr.py;
                             LastPZ = GenericErr.pz;
                             newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID1, GenericErr.idn1,
                                                   keyval, GenericErr.Cnumber, GenericErr.ECC1, GenericErr.ECC2,
                                                   GenericErr.magnitude, GenericErr.Lindex1, GenericErr.Lindex2,
                                                   SpecificConditionNumber, GenericErr.gform1, &ObjsWithErrs, &TtlObjErrs);
                             if(GenericErr.gform1 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                             newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID2, GenericErr.idn2,
                                                   keyval, GenericErr.Cnumber, GenericErr.ECC2, GenericErr.ECC1,
                                                   GenericErr.magnitude, GenericErr.Lindex2, GenericErr.Lindex1,
                                                   SpecificConditionNumber, GenericErr.gform2, &ObjsWithErrs, &TtlObjErrs);
                             Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex1].crossindex] += 1;
                             Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex2].crossindex] += 1;
                             if(GenericErr.gform2 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                             e_count += 1;

                             if(GenericErr.Cnumber > 0)
                               {
                                 cloneindex = GetCloneIndex(GenericErr.Cnumber,keyval);

                                 CloneErrorLookup[cloneindex].number += 1;

                                 if(cloneindex != LastCloneNumber)
                                   {
                                     LastCloneNumber = cloneindex;
                                     CloneErrorLookup[cloneindex].fileposn = fileposn;
                                   }

                               if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                  {
                                  ign_count = ign_count + CompGenericErrToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 1, 2);
                                  }
                               }
                             else
                               {
                                 ErrorLookup[keyval].number += 1;

                                 if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                    {
                                    ign_count = ign_count + CompGenericErrToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 1, 2);
                                    }
                                 if(lastkeyval != i)
                                   {
                                     lastkeyval = i;
                                     ErrorLookup[i].fileposn = fileposn;
                                   }
                               }
                             }

                          FreeGenericErrorAllocations();
                          break;



                        case LLINT: /** line - line intersection **/
                        case BADFEATCUT: /** feature cut when no need ***/
                        /**case LGEOM_UNM_LAT:
                        case LGEOM_UNM_LON:**/
                        case LLNONODEINT: /* features intersect, but not at a shared node **/
                        case NONODEOVLP: /** line, area have overlapping edge without common node ***/
                        case LLNOENDINT: /** lines intersect, but not at end point **/
                        case LLINTAWAY: /** two lines intersect, and cross over each other ***/
                        case LLINTNOEND: /** two lines intersect, pt of intersection is away from either primary particpant end node ***/
                        case P_O_LOOP: /*** self-intersecting line that includes P & O formations using end nodes - lines only ****/
                        case LLIEX: /** line - line except intersection **/
                        case LAIEX: /** line - area intersection with 3rd feature exception ***/
                        case LMINT: /** line - model intersection **/
                        case LAINT:  /* line - areal intersection **/
                        case LACUTFAIL:  /** line not cut at intersection with area perimeter **/
                        case LAINTNOEND: /** line - area intersection not at line end node ***/
                        case LEAON_NOTIN: /** line end node on area edge, line not inside area ***/
                        case POLYINTAREA: /* polygon - areal intersection of edges **/
                        case AREAINTAREA: /* areal - areal intersection of edges **/
                        case PART_ISF: /** two area features have intersecting edges and share part of their faces **/
                        case CUT_INT: /** cut-out intersects parent feature outer ring ***/
                        case AOVERLAPA: /** overlapping area features (second can also be inside first) **/

                          if(FreadFwritePointAndTwoObjects(keyval,1,unsortout,errtypeinout,
                                   LastPX, LastPY, LastPZ, LastKV,LastCnumber,LastLocalID1,LastLocalID2) > 0)
                             {
                             if((LastKV != keyval) || (LastCnumber != GenericErr.Cnumber))
                               {
                                 LastKV = keyval;
                                 LastCnumber = GenericErr.Cnumber;
                                 SpecificConditionNumber = 1;
                               }
                             else
                               SpecificConditionNumber += 1;
                             LastLocalID1 = GenericErr.LocalID1;
                             LastLocalID2 = GenericErr.LocalID2;
                             LastPX = GenericErr.px;
                             LastPY = GenericErr.py;
                             LastPZ = GenericErr.pz;
                             newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID1, GenericErr.idn1,
                                                   keyval, GenericErr.Cnumber, GenericErr.ECC1, GenericErr.ECC2,
                                                   GenericErr.magnitude, GenericErr.Lindex1, GenericErr.Lindex2,
                                                   SpecificConditionNumber, GenericErr.gform1, &ObjsWithErrs, &TtlObjErrs);
                             if(GenericErr.gform1 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                             newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID2, GenericErr.idn2,
                                                   keyval, GenericErr.Cnumber, GenericErr.ECC2, GenericErr.ECC1,
                                                   GenericErr.magnitude, GenericErr.Lindex2, GenericErr.Lindex1,
                                                   SpecificConditionNumber, GenericErr.gform2, &ObjsWithErrs, &TtlObjErrs);
                             Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex1].crossindex] += 1;
                             Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex2].crossindex] += 1;
                             if(GenericErr.gform2 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1;
                                if(newobjectwitherrs > 0)
                                   ElevOwithErrs += 1;
                                }
                             e_count += 1;

                             if(GenericErr.Cnumber > 0)
                               {
                                 cloneindex = GetCloneIndex(GenericErr.Cnumber,keyval);

                                 CloneErrorLookup[cloneindex].number += 1;
                                 
                                 if(cloneindex != LastCloneNumber)
                                   {
                                     LastCloneNumber = cloneindex;  
                                     CloneErrorLookup[cloneindex].fileposn = fileposn;
                                   }

                               if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                  {
                                  ign_count = ign_count + CompGenericErrToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 1, 2);
                                  }
                               }
                             else
                               {
                                 ErrorLookup[keyval].number += 1;
 
                                 if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                    {
                                    ign_count = ign_count + CompGenericErrToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 1, 2);
                                    }
                                 if(lastkeyval != i)
                                   {
                                     lastkeyval = i;
                                     ErrorLookup[i].fileposn = fileposn;
                                   }
                               }
                             }

                          FreeGenericErrorAllocations();
                          break;

                        case VERTSLOPE: /** vertical poly **/
                        case PLPFAIL: /** point - line coincidence failure **/
                        case PNOCOVERLE: /* point not covered by linear end **/
                        case PNOCOV2LEA: /** point not covered by 2 line terminal nodes or area edges***/
                        case PNOCOVERLV: /** point not covered by any line vertex **/
                        case POLYOSIDEAREA: /** Poly completely outside all areals of given type **/
                        case PTOSIDEREGION: /** point feature not inside any typed areal or poly **/
                        case OBJECTWITHOUT: /** poly or areal without a point or linear inside **/
                        case OBJ_WO_TWO: /** area contains secondary P,A,L but not tertiary P,A,L ***/
                        /** case AWITHOUTA:  area that does not fully contain a second area ***/
                        case FSFAIL: /*** face sharing failure ***/
                        case PSHAREFAIL:  /*** an area feature fails to share any of its perimeter with a 2d area feature ***/
                        case NOCOINCIDE: /** area without line end node or segment on its perimeter ***/
                        case V_DUPS: /** duplicate vertices inside object **/
                        case LNOCOVERLA: /** line not covered by line or areal ***/
                        case LSPANFAIL: /** line not covered by face of doesnt spand between edges ***/
                        case LNOCOV2A:  /** line not covered by edges of 2 area features ***/
                        /**case ISOLINE:  ** line feature completely inside an area feature ***/
                        /**case LINSIDEA: ** line partly or entirely inside area feature ***/
                        case LEINSIDEA: /** line end node properly inside an area ***/
                        case COINCIDEFAIL: /** line or area feature segment fails to coincide with 2 other line or area features **/
                        case ISOLATEDA:  /*** area feature does not intersect another area or a line feature ***/
                        case NETISOA: /** like ISOLATEDA except allowed a transitive connection through other like features ***/
                        case ANETISOA: /** area not trans connected to another area by shared edges ***/
                        case NETISOFEAT: /** form a network - check for nets with one feature, but not another ***/
                        case MULTIDFEAT: /** single line or area with both 2 and 3 D coordinates ***/
                        case MULTISENTINEL: /** single line or area has more than one sentinel z value ***/
                        case CONNECTFAIL: /** point, line, or area feature without 'connection' to specified 2nd feature **/
                        /**case FEATOUTSIDE:  *** a feature lies at least partly outside the MGCP cell ***/
                        case HIGHLIGHTED: /** feature is on the highlight list from view by attribution ***/
                        case ANOCOVERLA: /** areal not covered by line or areal ***/
                        case QUALANOCOVLA: /** area permin not covered by line or area AND is inside a third area ***/
                        case ANOCOVERA: /** area not covered by second area ***/
                        case OVERUNDER: /** any feature outside a perimeter-defining area or a line end node undershooting it **/
                        case AMCOVAFAIL: /** area not coverer by adjoining areas **/
                        case CUTOUT:   /** simply identifies a cut-out of an area feature ***/
                        case PORTRAYF: /** write feature that fails all MGCP4 portrayal rules ***/
                        case TPORTRAYF: /** write feature that fails all TDS6 portrayal rules ***/
                        case MASKMONO: /** DEM not monotonic at point defined by specified mask value ***/
                        case AHANG_LON: /** hanging area feature at a specified longitude meridian ***/
                        case AHANG_LAT: /** hanging area feature at a specified latitude parallel ***/
                        case AUNM_ACRS_A: /** area feature edge incorrectly matched across a bounding area feature ***/
                        case LLNOINT:  /** line failure to intersect a second line ***/
                        case LFNOINT: /** line fails to intersect another line, area, or point and no end node on 1/4 degree line ***/
                        case PLLPROXFAIL:  /** point not within specified dist from int of 2 lines ***/
                          if(FreadFwriteObject(keyval,1,unsortout,errtypeinout,LastKV,LastCnumber,LastLocalID1) > 0)
                             {
                          if((LastKV != keyval) || (LastCnumber != GenericErr.Cnumber))
                            {
                              LastKV = keyval;
                              LastCnumber = GenericErr.Cnumber;
                              SpecificConditionNumber = 1;
                            }
                          else
                            SpecificConditionNumber += 1;
                          LastLocalID1 = GenericErr.LocalID1;
                          newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID1, GenericErr.idn1,keyval, GenericErr.Cnumber,
                                                   GenericErr.ECC1, -1, /** -1 signal that only 1 ECC used **/
                                                   GenericErr.magnitude, GenericErr.Lindex1, -1,
                                                   SpecificConditionNumber, GenericErr.gform1, &ObjsWithErrs, &TtlObjErrs);
                          Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex1].crossindex] += 1;
                          if(GenericErr.gform1 == G_GRIDPT)
                             {
                             ElevPwithErrs += 1;
                             if(newobjectwitherrs > 0)
                                ElevOwithErrs += 1;
                             }
                                
                          e_count += 1;

                          if(GenericErr.Cnumber > 0)
                            {
                              cloneindex = GetCloneIndex(GenericErr.Cnumber,keyval);

                              CloneErrorLookup[cloneindex].number += 1;
                              /**if(GenericErr.Cnumber != LastCloneNumber) ***/
                              if(cloneindex != LastCloneNumber)
                                {
                                  LastCloneNumber = cloneindex;  /***GenericErr.Cnumber;***/
                                  CloneErrorLookup[cloneindex].fileposn = fileposn;
                                }

                            if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                               {
                               ign_count = ign_count + CompGenericErrToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 0, 1);
                               }
                            }
                          else
                            {
                              ErrorLookup[keyval].number += 1;
                              if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                 {
                                 ign_count = ign_count + CompGenericErrToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 0, 1);
                                 }
                              if(lastkeyval != i)
                                {
                                  lastkeyval = i;
                                  ErrorLookup[i].fileposn = fileposn;
                                }
                            }
                             }

                          FreeGenericErrorAllocations();

                          break;

                        case LOUTSIDEA: /** linear veretx falls outtside areal **/
                        case LLAINT: /** line - line endpt connect at area perimeter **/
                        case L_NOTL_AINT: /** line end point connects to 'not type line' at area perimeter **/
                        case LENOCOVERP: /** line end node not covered by point ***/
                        case ENCONFAIL: /** end node connectivity failure **/
                        case NOENDCON: /** both end nodes of a line fail to connect or be covered **/
                        case BOTHENDCON: /** both end nodes of a line feature are covered by specified-type point features **/
                        case LENOCOVERL:  /*** line end node not within tolerance distance to another line ***/
                        case NOLCOVLE:  /*** line end node not within tolerance distance to another line, including itself on a diff segment ***/
                        case LOOPS: /** self-intersecting linear or areal ***/
                        case COLINEAR: /** 3 consecutive vertices on line or area perim are collinear - middle one is not connecting node ***/
                        case KICKBACK: /** 180 degree kink ***/
                        case ENDPTINT: /** line endpoints are the same ***/
                        case L_UNM_A:  /*** line endpt unmatched at area feature boundary ***/
                        case LSAME_UNM_A: /*** line endpt unmatched with line of same FCODE at Area boundary ***/
                        case LUNM_ACRS_A: /*** line mismatch across poly edge ***/
                        case LUNMA_ACRS_A: /** line end not matched to area node across area perimeter ***/
                        case LATTRCHNG:  /** line end point connects to same fdcode line, but attributes differ between the 2 features **/
                        case LHANG_LON: /** hanging line feature at a specified longitude meridian ***/
                        case LHANG_LAT: /** hanging line feature at a specified latitude parallel ***/
                        case LE_A_UNM_LAT: /** line end node not coincident with area node at latitude parallel **/
                        case LE_A_UNM_LON: /** line end node not coincident with area node at longitude meridian **/
                          if(FreadFwritePointAndObject(keyval,1,unsortout,errtypeinout,
                                   LastPX, LastPY, LastPZ, LastKV,LastCnumber,LastLocalID1) > 0)
                             {
                             if((LastKV != keyval) || (LastCnumber != GenericErr.Cnumber))
                               {
                                 LastKV = keyval;
                                 LastCnumber = GenericErr.Cnumber;
                                 SpecificConditionNumber = 1;
                               }
                             else
                               SpecificConditionNumber += 1;
                             LastLocalID1 = GenericErr.LocalID1;
                             LastPX = GenericErr.px;
                             LastPY = GenericErr.py;
                             LastPZ = GenericErr.pz;
                             newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID1, GenericErr.idn1,keyval, GenericErr.Cnumber,
                                                   GenericErr.ECC1, -1, /** -1 signal that only 1 ECC used **/
                                                   GenericErr.magnitude, GenericErr.Lindex1, -1,
                                                   SpecificConditionNumber, GenericErr.gform1, &ObjsWithErrs, &TtlObjErrs);
                             Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex1].crossindex] += 1;
                             if(GenericErr.gform1 == G_GRIDPT)
                                {
                                ElevPwithErrs += 1; 
                                if(newobjectwitherrs > 0) 
                                   ElevOwithErrs += 1; 
                                }
                               
                             e_count += 1;

                             if(GenericErr.Cnumber > 0)
                               {
                                 cloneindex = GetCloneIndex(GenericErr.Cnumber,keyval);

                                 CloneErrorLookup[cloneindex].number += 1;
                                 
                                 if(cloneindex != LastCloneNumber)
                                   {
                                     LastCloneNumber = cloneindex;  
                                     CloneErrorLookup[cloneindex].fileposn = fileposn;
                                   }

                               if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                  {
                                  ign_count = ign_count + CompGenericErrToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 1, 1);
                                  }
                               }
                             else
                               {
                                 ErrorLookup[keyval].number += 1;
                                 if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                    {
                                    ign_count = ign_count + CompGenericErrToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 1, 1);
                                    }
                                 if(lastkeyval != i)
                                   {
                                     lastkeyval = i;
                                     ErrorLookup[i].fileposn = fileposn;
                                   }
                               }
                             }

                          FreeGenericErrorAllocations();
                          break;

                        case ATTR_PAIR: /*** NGA unexpected fcode - geom pair ***/
                        case ATTR_UNEXP: /** NGA unexpected attribute assigned ***/
                        case ATTR_VVT: /*** attribute dependency violation  **/
                        case ATTR_RNULL: /*** MGCP Required attribute assigned NULL value ***/
                        case ATTR_MISSING: /** missing a required attribute ***/
                        case ATTR_DT:  /** NGA - datatype encountered not as presecribed **/
                        case ATTR_RNG: /** NGA attribute value range violation ***/
                        case ATTR_PICK: /** NGA - pick list allowed domain violation **/
                        case ATTR_META: /** NGA - GIFD D4 metadata violation ***/
                        case VVTERR1WAY: /** feature with designated attribute & value ***/
                        case VVTERR2WAY:  /*** valid value type error ***/
                        case VVTERR3WAY:  /*** valid values conflict between 3 attribute of a single feature ***/
                        case ATTRERR: /*** attribution error **/
                        case RPTD_ATTR: /** attribute error as reported  ****/
                        case CONFLATE: /*** line is unique among conflation sets of data ***/

                          FreadFwriteObjectAndMessage(keyval,1,unsortout,errtypeinout);

                          if((LastKV != keyval) || (LastCnumber != GenericErr.Cnumber))
                            {
                              LastKV = keyval;
                              LastCnumber = GenericErr.Cnumber;
                              SpecificConditionNumber = 1;
                            }
                          else
                            SpecificConditionNumber += 1;
                          LastLocalID1 = GenericErr.LocalID1;
                          newobjectwitherrs = AddToObjectConditionTree(GenericErr.LocalID1, GenericErr.idn1,keyval, GenericErr.Cnumber,
                                                   GenericErr.ECC1, -1, /** -1 signal that only 1 ECC used **/
                                                   GenericErr.magnitude, GenericErr.Lindex1, -1,
                                                   SpecificConditionNumber, GenericErr.gform1, &ObjsWithErrs, &TtlObjErrs);
                          Obj_Mdl_Flags[CrsWlk[GenericErr.Lindex1].crossindex] += 1;
                          if(GenericErr.gform1 == G_GRIDPT)
                             {
                             ElevPwithErrs += 1; 
                             if(newobjectwitherrs > 0) 
                                ElevOwithErrs += 1; 
                             }
                             
                          e_count += 1;

                          if(GenericErr.Cnumber > 0)
                            {
                              cloneindex = GetCloneIndex(GenericErr.Cnumber,keyval);

                              CloneErrorLookup[cloneindex].number += 1;
                              
                              if(cloneindex != LastCloneNumber)
                                {
                                  LastCloneNumber = cloneindex;  
                                  CloneErrorLookup[cloneindex].fileposn = fileposn;
                                }
                            if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                               {
                               ign_count = ign_count + CompGenericErrToIgn(keyval, cloneindex, CloneErrorLookup[cloneindex].number, coord_type, 0, 1);
                               }
                            }
                          else
                            {
                              ErrorLookup[keyval].number += 1;
                              if((UpdateStatus == 1) && (ConsultPreviouslyIgnored > 0))
                                 {
                                 ign_count = ign_count + CompGenericErrToIgn(keyval, -1, ErrorLookup[keyval].number, coord_type, 0, 1);
                                 }
                              if(lastkeyval != i)
                                {
                                  lastkeyval = i;
                                  ErrorLookup[i].fileposn = fileposn;
                                }
                            }

                          FreeGenericErrorAllocations();
                          break;

		      
                    default:
                      printf("bad keyval 2 (%d) in %s file\n",keyval,unsortlog);
                      exit(-1);
                    }
                  pIE = cIE;
                  cIE = cIE->next;
                  ++kv2;
                  free(pIE);
                }

	  if(GE_DUP_Root != NULL)
	    {
	      Free_GE_DUP_List();
	    }
         if(NGA_TYPE >= 0) //&& (smryout != NULL))
            {
            k = 0;
            while(ErrorLookup[i].name[k] != '\0')
               {
               ++k;
               }
            I_Applied += ErrorLookup[i].anyactive;
/**************************
               if((ErrorLookup[i].numthresholds > 0) &&
                     (FindMinMaxSensitivities(i, &t1min, &t1max, &t2min, &t2max, &numT) > 0))
                  {
;
                  }
*********************/
            }

         if((NGA_TYPE >= 0) && (binsmryout != NULL))
            {
            if((ErrorLookup[i].numthresholds > 0) &&
                     (FindMinMaxSensitivities(i, &t1min, &t1max, &t2min, &t2max, &numT) > 0))
               {
               SEEIT_fwrite_int(&i,binsmryout);
               SEEIT_fwrite_int(&e_count,binsmryout);
               /***SEEIT_fwrite_int(&ign_count,binsmryout);***/
               SEEIT_fwrite_int(&ErrorLookup[i].anyactive,binsmryout);
               SEEIT_fwrite_int(&ErrorLookup[i].usemagnitude,binsmryout);
               SEEIT_fwrite_int(&numT,binsmryout);
               k = strlen(ErrorLookup[i].units);
               SEEIT_fwrite_int(&k,binsmryout);
               fwrite(&ErrorLookup[i].units[0],k,1,binsmryout);
               SEEIT_fwrite_double(&t1min,binsmryout);
               SEEIT_fwrite_double(&t1max,binsmryout);
               SEEIT_fwrite_double(&t2min,binsmryout);
               SEEIT_fwrite_double(&t2max,binsmryout);
               SEEIT_fwrite_int(&ErrorLookup[i].bigworse,binsmryout);
               SEEIT_fwrite_double(&e_min,binsmryout);
               SEEIT_fwrite_double(&e_max,binsmryout);
               }
            else
               {
               if(i == COVERFAIL)
                  {
                  e_count = 0;
                  if(CFGcounts != NULL)
                     {
                     for(ii=0; ii<MaxCOVERFAILgroup; ii++)
                        {
                        if(CFGcounts[ii] > 0)
                           ++e_count;
                        }
                     free(CFGcounts);
                     }
                  }

               numT = 0;
               t1min = t1max = t2min = t2max = 0.0;

               SEEIT_fwrite_int(&i,binsmryout);
               SEEIT_fwrite_int(&e_count,binsmryout);
               SEEIT_fwrite_int(&ErrorLookup[i].anyactive,binsmryout);
               if(i == COVERFAIL) /*** want to write a zero for use magnitude ***/
                  SEEIT_fwrite_int(&numT,binsmryout);
               else
                  SEEIT_fwrite_int(&ErrorLookup[i].usemagnitude,binsmryout);
               SEEIT_fwrite_int(&numT,binsmryout);
               k = strlen(ErrorLookup[i].units);
               SEEIT_fwrite_int(&k,binsmryout);
               fwrite(&ErrorLookup[i].units[0],k,1,binsmryout);
               SEEIT_fwrite_double(&t1min,binsmryout);
               SEEIT_fwrite_double(&t1max,binsmryout);
               SEEIT_fwrite_double(&t2min,binsmryout);
               SEEIT_fwrite_double(&t2max,binsmryout);
               SEEIT_fwrite_int(&ErrorLookup[i].bigworse,binsmryout);
               SEEIT_fwrite_double(&e_min,binsmryout);
               SEEIT_fwrite_double(&e_max,binsmryout);
               }
            kk = 0;
            for(k=0; k<NumberOfModels; k++)
               {
               if(Obj_Mdl_Flags[k] > 0)
                  ++kk;
               }
            SEEIT_fwrite_int(&kk,binsmryout);
            for(k=0; k<NumberOfModels; k++)
               {
               if(Obj_Mdl_Flags[k] > 0)
                  {
                  strcpy(command2,MdlNames[k].name);
                  si = strlen(command2);
                  SEEIT_fwrite_int(&si,binsmryout);
                  fwrite(&command2[0],si,1,binsmryout);

                  if(NGA_TYPE == 1)
                     {
                     strcpy(command2,GetECCCode(MdlNames[k].code));
                     si = strlen(command2);
                     SEEIT_fwrite_int(&si,binsmryout);
                     fwrite(&command2[0],si,1,binsmryout);
                     }

                  strcpy(command2,GetECCLabel(MdlNames[k].code));
                  si = strlen(command2);
                  SEEIT_fwrite_int(&si,binsmryout);
                  fwrite(&command2[0],si,1,binsmryout);

                  SEEIT_fwrite_int(&Obj_Mdl_Flags[k],binsmryout);
                  }
               }
            }
        }  /*** end Index[i].entry != NULL ****/
    else if(NGA_TYPE >= 0)
        {
        I_Applied += ErrorLookup[i].anyactive;

        if(binsmryout != NULL)
            {
            SEEIT_fwrite_int(&i,binsmryout);
            SEEIT_fwrite_int(&e_count,binsmryout);
            /***SEEIT_fwrite_int(&ign_count,binsmryout); ***/
            SEEIT_fwrite_int(&ErrorLookup[i].anyactive,binsmryout);
            SEEIT_fwrite_int(&ErrorLookup[i].usemagnitude,binsmryout);
            SEEIT_fwrite_int(&numT,binsmryout);
            k = strlen(ErrorLookup[i].units);
            SEEIT_fwrite_int(&k,binsmryout);
            fwrite(&ErrorLookup[i].units[0],k,1,binsmryout);
            SEEIT_fwrite_double(&t1min,binsmryout);
            SEEIT_fwrite_double(&t1max,binsmryout);
            SEEIT_fwrite_double(&t2min,binsmryout);
            SEEIT_fwrite_double(&t2max,binsmryout);
            }
        }
    Ttle_count += e_count;
    Ttlign_count += ign_count;
    }
  

  for(i=0; i<= NumberOfModels; i++)
     Obj_Mdl_Flags[i] = 0;

  sprintf(objfile,"%sobjsorted.bin",outdirectory);
  ObjFP = fopen(objfile,"wb+");
  SEEIT_fwrite_int(&ObjsWithErrs,ObjFP);
  SEEIT_fwrite_int(&TtlObjErrs,ObjFP);
  printf("writing %d objects with %d ttl errs to file %s\n",ObjsWithErrs,TtlObjErrs,objfile);

  if(NGA_TYPE >= 0) 
     {
     if(binsmryout != NULL)
        {
        SEEIT_fwrite_int(&Ttle_count,binsmryout);
        /***SEEIT_fwrite_int(&Ttlign_count,binsmryout); ***/
        SEEIT_fwrite_int(&ObjsWithErrs,binsmryout);
        SEEIT_fwrite_int(&ElevOwithErrs,binsmryout);
        SEEIT_fwrite_int(&ElevPwithErrs,binsmryout);
        SEEIT_fwrite_int(&I_Applied,binsmryout);
        }
     }
  
  ObjsWritten = 0;
  ObjsPerPage = 25;

  FPtoWrite = ObjsWithErrs / ObjsPerPage;
  i = ObjsWithErrs % ObjsPerPage;
  if(i > 0)
     FPtoWrite += 1;

  SEEIT_fwrite_int(&FPtoWrite,ObjFP);
  fileposn = 0;
  for(i=0; i<FPtoWrite; i++)
     {
     SEEIT_fwrite_long(&fileposn,ObjFP);
     }

  OMFposn = ftell(ObjFP);

  for(i=0; i<NumberOfModels; i++)
     SEEIT_fwrite_int(&Obj_Mdl_Flags[i], ObjFP);

  template_entries = 0;
  for(i=0; i<=CONDITION_DEFINITIONS; i++)
     {
     if(CCBY[i].count > 0)
        {
        template_entries += 1;
        }
     if(CCBY[i].c != NULL)
        {
        ctc = CCBY[i].c;
        while(ctc != NULL)
           {
           if(ctc->count > 0)
              {
              template_entries += 1;
              }
           ctc = ctc->next;
           }
        }
     }
  SEEIT_fwrite_int(&template_entries,ObjFP);
  for(i=0; i<=CONDITION_DEFINITIONS; i++)
     {
    if(UpdateStatus == 1)
       periodic_checking_redraw(0,"Condition Report Progress","Preparing condition report\n     Phase 4\n");
     if(CCBY[i].count > 0)
        {
        SEEIT_fwrite_int(&i,ObjFP);
        SEEIT_fwrite_int(&zeroval,ObjFP);
        SEEIT_fwrite_int(&CCBY[i].count,ObjFP);
        }
     if(CCBY[i].c != NULL)
        {
        ctc = CCBY[i].c;
        while(ctc != NULL)
           {
           SEEIT_fwrite_int(&i,ObjFP);
           SEEIT_fwrite_int(&ctc->cindex,ObjFP);
           SEEIT_fwrite_int(&ctc->count,ObjFP);
           ctcp = ctc;
           ctc = ctc->next;
           free(ctcp);
           }
        CCBY[i].c = NULL;
        CCBY[i].count = 0;
        }
     }


  if(RB_ObjTree != NULL)
    {
      PrintObjConditionTreeInorder(RB_ObjTree,RB_ObjTree->root->left);
      PrintSortedObjConditionTreeInorder(RB_ObjSortTree, RB_ObjSortTree->root->left, &ObjsWritten, ObjsPerPage, ObjFP, Obj_Mdl_Flags);
      
      ETF_RB_Obj_TreeDestroy(RB_ObjTree);
      
      ETF_RB_Obj_TreeDestroy(RB_ObjSortTree);
    }

  fseek(ObjFP, OMFposn, SEEK_SET);
  for(i=0; i<NumberOfModels; i++)
     {
     SEEIT_fwrite_int(&Obj_Mdl_Flags[i], ObjFP);
     }

  fclose(ObjFP);

  if(Obj_Mdl_Flags != NULL)
     free(Obj_Mdl_Flags);
  else
     printf("Obj_MdlFlags is unexpectedly NULL\n");


  NMDRc = NMDRroot;
  while(NMDRc != NULL)
     {
     NMDRn = NMDRc;
     NMDRc = NMDRc->next;
     free(NMDRn);
     }
  NMDRroot = NULL;

  for(i=0; i<=CONDITION_DEFINITIONS; i++)
    {
      if(CCBY[i].c != NULL)
         free(CCBY[i].c);
    }

  
  
  free(GE3->x1);
  free(GE3->y1);
  free(GE3->z1);
  free(GE3->x2);
  free(GE3->y2);
  free(GE3->z2);
  if(GE3->errmsg != NULL)
     free(GE3->errmsg);
  free(GE3->SID1);
  free(GE3->SID2);
  
  free(GE3);
  
  free(GE4->x1);
  free(GE4->y1);
  free(GE4->z1);
  free(GE4->x2);
  free(GE4->y2);
  free(GE4->z2);
  if(GE4->errmsg != NULL)
     free(GE4->errmsg);
  free(GE4->SID1);
  free(GE4->SID2);
  
  free(GE4);
  
  free(RegularConditions);
  free(CloneConditions);
  
  
  
  if(CDFREPORT)
    {
      fclose(CDFout);
      printf("comma-delimited format report sent to file: %s\n",CDFlog);
    }
  
  fclose(errtypeinout);


  fclose(unsortout);
  
  
  if((USE_DOS==0)||(SLASHTYPE==NONDOS_TYPE))
    {
      sprintf(command2,"cp \"%s\" \"%s\"",errtypelog,unsortlog);
    }
  else
    {
      sprintf(command2,"copy \"%s\" \"%s\"",errtypelog,unsortlog);
    }
  
  system(command2);
  
  
  unsortout = fopen(unsortlog,"ab+");

  errtypeinout = fopen(errtypelog,"wb");
  if(errtypeinout != NULL)
     fclose(errtypeinout);

  
}



void SetDefaultParticipants(int Check, int Clone, int Cloneindex)
{
  int i,j,k,l;
  
  
  
  if(Clone==1)
    {
      CloneErrorLookup[Cloneindex].number   = 0;
      CloneErrorLookup[Cloneindex].active   = 0;
      CloneErrorLookup[Cloneindex].selected = 0;
      
      CloneErrorLookup[Cloneindex].primaryEDCSstuff   = NULL;
      CloneErrorLookup[Cloneindex].secondaryEDCSstuff = NULL;
      CloneErrorLookup[Cloneindex].tertiaryEDCSstuff  = NULL;

      CloneErrorLookup[Cloneindex].primaryFIDstuff   = NULL;
      CloneErrorLookup[Cloneindex].secondaryFIDstuff = NULL;
      CloneErrorLookup[Cloneindex].tertiaryFIDstuff  = NULL;
      
      for(j=0;j<4;j++)
	{
	  for(k=0;k<2;k++)
	    {
	      for(l=0;l<NUM_C;l++)
		{
		  CloneErrorLookup[Cloneindex].IMarkRoot    [j][k][l] = NULL;
		  CloneErrorLookup[Cloneindex].IMarkSACRoot [j][k][l] = NULL;
		  CloneErrorLookup[Cloneindex].DO_EDCS_COMBO[j][k][l] = 0;
		  CloneErrorLookup[Cloneindex].model_index  [j][k][l] = 0;
		  CloneErrorLookup[Cloneindex].sac_index    [j][k][l] = 0;
		}
	    }
	}
      
      
      
      CloneErrorLookup[Cloneindex].Config1[0] = 2;
      CloneErrorLookup[Cloneindex].Config2[0] = 2;
      CloneErrorLookup[Cloneindex].Config3[0] = 2;
      
      for(j=1; j<NUM_C; j++)
	{
	  CloneErrorLookup[Cloneindex].Config1[j] = 0;
	  CloneErrorLookup[Cloneindex].Config2[j] = 0;
	  CloneErrorLookup[Cloneindex].Config3[j] = 0;
	}
      for(j=1; j<NUM_S; j++)
	{
	  CloneErrorLookup[Cloneindex].Stratum1[j] = 1;
	  CloneErrorLookup[Cloneindex].Stratum2[j] = 1;
	  CloneErrorLookup[Cloneindex].Stratum3[j] = 1;
	}
      CloneErrorLookup[Cloneindex].Stratum1[0] = 0;
      CloneErrorLookup[Cloneindex].Stratum1[2] = 0;
      CloneErrorLookup[Cloneindex].Stratum1[3] = 0;
      CloneErrorLookup[Cloneindex].Stratum2[0] = 0;
      CloneErrorLookup[Cloneindex].Stratum2[2] = 0;
      CloneErrorLookup[Cloneindex].Stratum2[3] = 0;
      CloneErrorLookup[Cloneindex].Stratum3[0] = 0;
      CloneErrorLookup[Cloneindex].Stratum3[2] = 0;
      CloneErrorLookup[Cloneindex].Stratum3[3] = 0;
      
      CloneErrorLookup[Cloneindex].Domain1[0] = 0;
      CloneErrorLookup[Cloneindex].Domain2[0] = 0;
      CloneErrorLookup[Cloneindex].Domain3[0] = 0;
      for(j=1;j<NUM_D;j++)
	{
	  CloneErrorLookup[Cloneindex].Domain1[j] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[j] = 1;
	  CloneErrorLookup[Cloneindex].Domain3[j] = 1;
	}
    }
  else
    {
      ErrorLookup[Check].Config1[0] = 2;
      ErrorLookup[Check].Config2[0] = 2;
      ErrorLookup[Check].Config3[0] = 2;
      
      for(j=1; j<NUM_C; j++)
	{
	  ErrorLookup[Check].Config1[j] = 0;
	  ErrorLookup[Check].Config2[j] = 0;
	  ErrorLookup[Check].Config3[j] = 0;
	}
      for(j=1; j<NUM_S; j++)
	{
	  ErrorLookup[Check].Stratum1[j] = 1;
	  ErrorLookup[Check].Stratum2[j] = 1;
	  ErrorLookup[Check].Stratum3[j] = 1;
	}
      ErrorLookup[Check].Stratum1[0] = 0;
      ErrorLookup[Check].Stratum1[2] = 0;
      ErrorLookup[Check].Stratum1[3] = 0;
      ErrorLookup[Check].Stratum2[0] = 0;
      ErrorLookup[Check].Stratum2[2] = 0;
      ErrorLookup[Check].Stratum2[3] = 0;
      ErrorLookup[Check].Stratum3[0] = 0;
      ErrorLookup[Check].Stratum3[2] = 0;
      ErrorLookup[Check].Stratum3[3] = 0;
      
      ErrorLookup[Check].Domain1[0] = 0;
      ErrorLookup[Check].Domain2[0] = 0;
      ErrorLookup[Check].Domain3[0] = 0;
      for(j=1;j<NUM_D;j++)
	{
	  ErrorLookup[Check].Domain1[j] = 1;
	  ErrorLookup[Check].Domain2[j] = 1;
	  ErrorLookup[Check].Domain3[j] = 1;
	}
    }
  
  
  
  switch(Check)
    {      
      
    case G_DUPS:      
    case SAMEID: /** same GFID or FLDBID, diff geom **/
    case SAMEID_GDUP:  /** same GFID or FLDBID, same geom, diff attr ***/
    case ANY_SAMEID:  /** same unique identifier, except those that are complete duplicates ***/
      if(Clone==1)
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[j] = 1;
              CloneErrorLookup[Cloneindex].Config2[j] = 1;
	    }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 2;
	  CloneErrorLookup[Cloneindex].Stratum1[2] = 1;
	  
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 2;
          CloneErrorLookup[Cloneindex].Stratum2[2] = 1;
	}
      else
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      ErrorLookup[Check].Config1[j] = 1;
              ErrorLookup[Check].Config2[j] = 1;
	    }
          ErrorLookup[Check].Config1[C_GRID] = 2;
	  ErrorLookup[Check].Stratum1[2] = 1;
	  
          ErrorLookup[Check].Config2[C_GRID] = 2;
          ErrorLookup[Check].Stratum2[2] = 1;
	}
      break;
      
      
    case C_DUPS:
    case SAMEID_CDUP: /** same GFID or FLDBID, geom & attr ***/
      
      if(Clone==1)
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[j] = 1;
	    }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 2;
	  CloneErrorLookup[Cloneindex].Stratum1[2] = 1;
	}
      else
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      ErrorLookup[Check].Config1[j] = 1;
	    }
          ErrorLookup[Check].Config1[C_GRID] = 2;
	  ErrorLookup[Check].Stratum1[2] = 1;
	}
      break;
      
      
    case V_DUPS:
      if(Clone==1)
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[j] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POLY]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
	}
      else
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      ErrorLookup[V_DUPS].Config1[j] = 2;
	    }
	  ErrorLookup[V_DUPS].Config1[C_POLY]  = 1;
	  ErrorLookup[V_DUPS].Config1[C_MOLI] = 1;
          ErrorLookup[V_DUPS].Config1[C_AREA]  = 1;
          ErrorLookup[V_DUPS].Config1[C_LINE] = 1;
          ErrorLookup[V_DUPS].Config1[C_FMLF] = 1;
          ErrorLookup[V_DUPS].Config1[C_FMAF]  = 1;
          ErrorLookup[V_DUPS].Config1[C_DILI] = 1;
	}
      break;
      
      
    case SLIVER:      
      if(NGA_TYPE == 1)
	{
	  if(Clone==1)
	    {
	      for(i=0; i<NUM_C; i++)
		{
		  CloneErrorLookup[Cloneindex].Config1[i] = 2;
		}
	      CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
	      CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	    }
	  else
	    {
	      for(i=0; i<NUM_C; i++)
		{
		  ErrorLookup[SLIVER].Config1[i] = 2;
		}
	      ErrorLookup[SLIVER].Config1[C_AREA]  = 1;
	      ErrorLookup[SLIVER].Config1[C_FMAF] = 0;
	    }
	}
      else
	{
	  if(Clone==1)
	    {
	      for(i=0; i<NUM_C; i++)
		{
		  CloneErrorLookup[Cloneindex].Config1[i] = 2;
		}
	      CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
	      CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	      CloneErrorLookup[Cloneindex].Config1[C_POLY]  = 1;
	      CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	    }
	  else
	    {
	      for(i=0; i<NUM_C; i++)
		{
		  ErrorLookup[SLIVER].Config1[i] = 2;
		}
	      ErrorLookup[SLIVER].Config1[C_AREA]  = 0;
	      ErrorLookup[SLIVER].Config1[C_FMAF] = 0;
	      ErrorLookup[SLIVER].Config1[C_POLY]  = 1;
	      ErrorLookup[SLIVER].Config1[C_MOLI] = 0;
	    }
	}
      break;

    case FACESIZE: /*** small area on face of area feature **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FACESIZE].Config1[i] = 2;
              ErrorLookup[FACESIZE].Config2[i] = 2;
            }
          ErrorLookup[FACESIZE].Config1[C_AREA]  = 1;
          ErrorLookup[FACESIZE].Config2[C_LINE]  = 1;
        }
      break;
      
      
    case NARROW:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[NARROW].Config1[i] = 2;
	    }
	  ErrorLookup[NARROW].Config1[C_AREA]  = 0;
	  ErrorLookup[NARROW].Config1[C_FMAF] = 0;
	  ErrorLookup[NARROW].Config1[C_POLY]  = 1;
	  ErrorLookup[NARROW].Config1[C_MOLI] = 0;
	}
      break;
      
      
    case SMALLOBJ:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_AREA] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[SMALLOBJ].Config1[i] = 2;
	    }
	  ErrorLookup[SMALLOBJ].Config1[C_AREA] = 0;
	  ErrorLookup[SMALLOBJ].Config1[C_FMAF] = 0;
	  ErrorLookup[SMALLOBJ].Config1[C_POLY] = 1;
	  ErrorLookup[SMALLOBJ].Config1[C_MOLI] = 0;
	}
      break;
      
    case ATTRERR:
      if(Clone==1)
	{
	  for(j=1; j<NUM_C; j++)
	    {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
	      CloneErrorLookup[Cloneindex].Config1[j] = 1;
	    }
	}
      else
	{
	  for(j=1; j<NUM_C; j++)
	    {
            if(j == C_GRID)
              ErrorLookup[Check].Config1[j] = 2;
            else
	      ErrorLookup[Check].Config1[j] = 1;
	    }
	}
      break;
      
    case ATTR_PAIR: /*** NGA unexpected fcode - geom pair ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_PAIR].Config1[j] = 2;
            else
              ErrorLookup[ATTR_PAIR].Config1[j] = 1;
            }
        }
      break;

    case PORTRAYF: /** write feature that fails all MGCP4 portrayal rules ***/
    case TPORTRAYF: /** write feature that fails all TDS6 portrayal rules ***/
    case RPTD_ATTR: /** attribute error as reported  ****/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[Check].Config1[j] = 2;
            else
              ErrorLookup[Check].Config1[j] = 1;
            }
        }
      break;


    case ATTR_UNEXP: /** NGA unexpected attribute assigned ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_UNEXP].Config1[j] = 2;
            else
              ErrorLookup[ATTR_UNEXP].Config1[j] = 1;
            }
        }
      break;


    case ATTR_VVT: /*** attribute dependency violation  **/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_VVT].Config1[j] = 2;
            else
              ErrorLookup[ATTR_VVT].Config1[j] = 1;
            }
        }
      break;

    case ATTR_RNULL: /*** MGCP Required attribute assigned NULL value ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_RNULL].Config1[j] = 2;
            else
              ErrorLookup[ATTR_RNULL].Config1[j] = 1;
            }
        }
      break;


    case ATTR_MISSING: /** missing a required attribute ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_MISSING].Config1[j] = 2;
            else
              ErrorLookup[ATTR_MISSING].Config1[j] = 1;
            }
        }
      break;
      
    case ATTR_DT:  /** NGA - datatype encountered not as presecribed **/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_DT].Config1[j] = 2;
            else
              ErrorLookup[ATTR_DT].Config1[j] = 1;
            }
        }
      break;

      
    case ATTR_RNG: /** NGA attribute value range violation ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_RNG].Config1[j] = 2;
            else
              ErrorLookup[ATTR_RNG].Config1[j] = 1;
            }
        }
      break;

      
    case ATTR_PICK: /** NGA - pick list allowed domain violation **/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_PICK].Config1[j] = 2;
            else
              ErrorLookup[ATTR_PICK].Config1[j] = 1;
            }
        }
      break;

      
    case ATTR_META: /** NGA - GIFD D4 metadata violation ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
            else
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
            }
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
            if(j == C_GRID)
              ErrorLookup[ATTR_META].Config1[j] = 2;
            else
              ErrorLookup[ATTR_META].Config1[j] = 1;
            }
        }
      break;

      
    case VVTERR1WAY: /** feature with designated attribute & value ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
              CloneErrorLookup[Cloneindex].Config3[j] = 2;
            }
           CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[VVTERR1WAY].Config1[j] = 2;
              ErrorLookup[VVTERR1WAY].Config2[j] = 2;
              ErrorLookup[VVTERR1WAY].Config3[j] = 2;
            }
           ErrorLookup[VVTERR1WAY].Config1[C_AREA] = 1;
           ErrorLookup[VVTERR1WAY].Config2[C_AREA] = 1;
           ErrorLookup[VVTERR1WAY].Config3[C_AREA] = 1;
           ErrorLookup[VVTERR1WAY].Config1[C_DILI] = 1;
           ErrorLookup[VVTERR1WAY].Config2[C_DILI] = 1;
           ErrorLookup[VVTERR1WAY].Config3[C_DILI] = 1;
           ErrorLookup[VVTERR1WAY].Config1[C_LINE] = 1;
           ErrorLookup[VVTERR1WAY].Config2[C_LINE] = 1;
           ErrorLookup[VVTERR1WAY].Config3[C_LINE] = 1;
           ErrorLookup[VVTERR1WAY].Config1[C_POFE] = 1;
           ErrorLookup[VVTERR1WAY].Config2[C_POFE] = 1;
           ErrorLookup[VVTERR1WAY].Config3[C_POFE] = 1;
           ErrorLookup[VVTERR1WAY].Config1[C_FMPF] = 1;
           ErrorLookup[VVTERR1WAY].Config2[C_FMPF] = 1;
           ErrorLookup[VVTERR1WAY].Config3[C_FMPF] = 1;
           ErrorLookup[VVTERR1WAY].Config1[C_FMLF] = 1;
           ErrorLookup[VVTERR1WAY].Config2[C_FMLF] = 1;
           ErrorLookup[VVTERR1WAY].Config3[C_FMLF] = 1;
           ErrorLookup[VVTERR1WAY].Config1[C_FMAF] = 1;
           ErrorLookup[VVTERR1WAY].Config2[C_FMAF] = 1;
           ErrorLookup[VVTERR1WAY].Config3[C_FMAF] = 1;
        }
      break;




      
    case VVTERR2WAY:  /*** valid value type error ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
              CloneErrorLookup[Cloneindex].Config3[j] = 2;
            }
           CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[VVTERR2WAY].Config1[j] = 2;
              ErrorLookup[VVTERR2WAY].Config2[j] = 2;
              ErrorLookup[VVTERR2WAY].Config3[j] = 2;
            }

           ErrorLookup[VVTERR2WAY].Config1[C_AREA] = 1;
           ErrorLookup[VVTERR2WAY].Config2[C_AREA] = 1;
           ErrorLookup[VVTERR2WAY].Config3[C_AREA] = 1;
           ErrorLookup[VVTERR2WAY].Config1[C_DILI] = 1;
           ErrorLookup[VVTERR2WAY].Config2[C_DILI] = 1;
           ErrorLookup[VVTERR2WAY].Config3[C_DILI] = 1;
           ErrorLookup[VVTERR2WAY].Config1[C_LINE] = 1;
           ErrorLookup[VVTERR2WAY].Config2[C_LINE] = 1;
           ErrorLookup[VVTERR2WAY].Config3[C_LINE] = 1;
           ErrorLookup[VVTERR2WAY].Config1[C_POFE] = 1;
           ErrorLookup[VVTERR2WAY].Config2[C_POFE] = 1;
           ErrorLookup[VVTERR2WAY].Config3[C_POFE] = 1;
           ErrorLookup[VVTERR2WAY].Config1[C_FMPF] = 1;
           ErrorLookup[VVTERR2WAY].Config2[C_FMPF] = 1;
           ErrorLookup[VVTERR2WAY].Config3[C_FMPF] = 1;
           ErrorLookup[VVTERR2WAY].Config1[C_FMLF] = 1;
           ErrorLookup[VVTERR2WAY].Config2[C_FMLF] = 1;
           ErrorLookup[VVTERR2WAY].Config3[C_FMLF] = 1;
           ErrorLookup[VVTERR2WAY].Config1[C_FMAF] = 1;
           ErrorLookup[VVTERR2WAY].Config2[C_FMAF] = 1;
           ErrorLookup[VVTERR2WAY].Config3[C_FMAF] = 1;
        }
      break;


    case VVTERR3WAY:  /*** valid values conflict between 3 attribute of a single feature ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
              CloneErrorLookup[Cloneindex].Config3[j] = 2;
            }
           CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[VVTERR3WAY].Config1[j] = 2;
              ErrorLookup[VVTERR3WAY].Config2[j] = 2;
              ErrorLookup[VVTERR3WAY].Config3[j] = 2;
            }

           ErrorLookup[VVTERR3WAY].Config1[C_AREA] = 1;
           ErrorLookup[VVTERR3WAY].Config2[C_AREA] = 1;
           ErrorLookup[VVTERR3WAY].Config3[C_AREA] = 1;
           ErrorLookup[VVTERR3WAY].Config1[C_DILI] = 1;
           ErrorLookup[VVTERR3WAY].Config2[C_DILI] = 1;
           ErrorLookup[VVTERR3WAY].Config3[C_DILI] = 1;
           ErrorLookup[VVTERR3WAY].Config1[C_LINE] = 1;
           ErrorLookup[VVTERR3WAY].Config2[C_LINE] = 1;
           ErrorLookup[VVTERR3WAY].Config3[C_LINE] = 1;
           ErrorLookup[VVTERR3WAY].Config1[C_POFE] = 1;
           ErrorLookup[VVTERR3WAY].Config2[C_POFE] = 1;
           ErrorLookup[VVTERR3WAY].Config3[C_POFE] = 1;
           ErrorLookup[VVTERR3WAY].Config1[C_FMPF] = 1;
           ErrorLookup[VVTERR3WAY].Config2[C_FMPF] = 1;
           ErrorLookup[VVTERR3WAY].Config3[C_FMPF] = 1;
           ErrorLookup[VVTERR3WAY].Config1[C_FMLF] = 1;
           ErrorLookup[VVTERR3WAY].Config2[C_FMLF] = 1;
           ErrorLookup[VVTERR3WAY].Config3[C_FMLF] = 1;
           ErrorLookup[VVTERR3WAY].Config1[C_FMAF] = 1;
           ErrorLookup[VVTERR3WAY].Config2[C_FMAF] = 1;
           ErrorLookup[VVTERR3WAY].Config3[C_FMAF] = 1;
        }
      break;



    case HIGHLIGHTED: /** feature is on the highlight list from view by attribution ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
              CloneErrorLookup[Cloneindex].Config3[j] = 2;
            }
           CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
           CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[Check].Config1[j] = 2;
              ErrorLookup[Check].Config2[j] = 2;
              ErrorLookup[Check].Config3[j] = 2;
            }

           ErrorLookup[Check].Config1[C_AREA] = 1;
           ErrorLookup[Check].Config2[C_AREA] = 1;
           ErrorLookup[Check].Config3[C_AREA] = 1;
           ErrorLookup[Check].Config1[C_DILI] = 1;
           ErrorLookup[Check].Config2[C_DILI] = 1;
           ErrorLookup[Check].Config3[C_DILI] = 1;
           ErrorLookup[Check].Config1[C_LINE] = 1;
           ErrorLookup[Check].Config2[C_LINE] = 1;
           ErrorLookup[Check].Config3[C_LINE] = 1;
           ErrorLookup[Check].Config1[C_POFE] = 1;
           ErrorLookup[Check].Config2[C_POFE] = 1;
           ErrorLookup[Check].Config3[C_POFE] = 1;
           ErrorLookup[Check].Config1[C_FMPF] = 1;
           ErrorLookup[Check].Config2[C_FMPF] = 1;
           ErrorLookup[Check].Config3[C_FMPF] = 1;
           ErrorLookup[Check].Config1[C_FMLF] = 1;
           ErrorLookup[Check].Config2[C_FMLF] = 1;
           ErrorLookup[Check].Config3[C_FMLF] = 1;
           ErrorLookup[Check].Config1[C_FMAF] = 1;
           ErrorLookup[Check].Config2[C_FMAF] = 1;
           ErrorLookup[Check].Config3[C_FMAF] = 1;
        }
      break;


      
    case HSLOPE:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  ErrorLookup[Cloneindex].Config1[C_POLY] = 1;
	  ErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[HSLOPE].Config1[i] = 2;
	    }
	  ErrorLookup[HSLOPE].Config1[C_POLY] = 1;
	  ErrorLookup[HSLOPE].Config1[C_MOLI] = 0;
	}
      break;
      
      
    case VERTSLOPE:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POLY]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[VERTSLOPE].Config1[i] = 2;
	    }
	  ErrorLookup[VERTSLOPE].Config1[C_POLY]  = 1;
	  ErrorLookup[VERTSLOPE].Config1[C_MOLI] = 0;
	}
      break;
      
      
    case VTEAR:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_AREA] =  0;
	  CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      CloneErrorLookup[Cloneindex].Stratum1[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[VTEAR].Config1[i] = 2;
	    }
	  ErrorLookup[VTEAR].Config1[C_AREA] =  0;
	  ErrorLookup[VTEAR].Config1[C_FMAF] = 0;
	  ErrorLookup[VTEAR].Config1[C_POLY] = 1;
	  ErrorLookup[VTEAR].Config1[C_MOLI] = 0;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      ErrorLookup[VTEAR].Stratum1[i] = 0;
	    }
	  ErrorLookup[VTEAR].Stratum1[5] = 1;
	}
      break;
      
      
    case HTEAR:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      CloneErrorLookup[Cloneindex].Stratum1[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[HTEAR].Config1[i] = 2;
	    }
	  ErrorLookup[HTEAR].Config1[C_POLY] = 1;
	  ErrorLookup[HTEAR].Config1[C_MOLI] = 0;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      ErrorLookup[HTEAR].Stratum1[i] = 0;
	    }
	  ErrorLookup[HTEAR].Stratum1[5] = 1;
	}
      break;
      
      
    case OVERC:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_AREA] =  2;
	  CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 2;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 2;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      CloneErrorLookup[Cloneindex].Stratum1[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[OVERC].Config1[i] = 2;
	    }
	  ErrorLookup[OVERC].Config1[C_AREA] =  2;
	  ErrorLookup[OVERC].Config1[C_FMAF] = 2;
	  ErrorLookup[OVERC].Config1[C_POLY] = 1;
	  ErrorLookup[OVERC].Config1[C_MOLI] = 2;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      ErrorLookup[OVERC].Stratum1[i] = 0;
	    }
	  ErrorLookup[OVERC].Stratum1[5] = 1;
	}
      break;
      
      
    case TVERT:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_AREA] =  0;
	  CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      CloneErrorLookup[Cloneindex].Stratum1[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[TVERT].Config1[i] = 2;
	    }
	  ErrorLookup[TVERT].Config1[C_AREA]  =  0;
	  ErrorLookup[TVERT].Config1[C_FMAF] = 0;
	  ErrorLookup[TVERT].Config1[C_POLY]  = 1;
	  ErrorLookup[TVERT].Config1[C_MOLI] = 0;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      ErrorLookup[TVERT].Stratum1[i] = 0;
	    }
	  ErrorLookup[TVERT].Stratum1[5] = 1;
	}
      break;
      

    case LLINT:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[7]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[LLINT].Config1[i] = 2;
	      ErrorLookup[LLINT].Config2[i] = 2;
	    }
	  ErrorLookup[LLINT].Config1[C_DILI]  = 1;
	  ErrorLookup[LLINT].Config2[C_DILI]  = 1;
	  ErrorLookup[LLINT].Config1[C_LINE]  = 1;
	  ErrorLookup[LLINT].Config2[C_LINE]  = 1;
	  ErrorLookup[LLINT].Config1[C_FMLF] = 1;
	  ErrorLookup[LLINT].Config2[C_FMLF] = 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      ErrorLookup[LLINT].Domain1[i] = 0;
	      ErrorLookup[LLINT].Domain2[i] = 0;
	    }
	  ErrorLookup[LLINT].Domain1[7]  = 1;
	  ErrorLookup[LLINT].Domain2[6]  = 1;
	  ErrorLookup[LLINT].Domain2[10] = 1;
	}
      break;

    case BADFEATCUT: /** feature cut when no need ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF]  = 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
              CloneErrorLookup[Cloneindex].Domain3[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[7]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[BADFEATCUT].Config1[i] = 2;
              ErrorLookup[BADFEATCUT].Config2[i] = 2;
              ErrorLookup[BADFEATCUT].Config3[i] = 2;
            }
          ErrorLookup[BADFEATCUT].Config1[C_DILI]  = 1;
          ErrorLookup[BADFEATCUT].Config2[C_DILI]  = 1;
          ErrorLookup[BADFEATCUT].Config3[C_DILI]  = 1;
          ErrorLookup[BADFEATCUT].Config1[C_LINE]  = 1;
          ErrorLookup[BADFEATCUT].Config2[C_LINE]  = 1;
          ErrorLookup[BADFEATCUT].Config3[C_LINE]  = 1;
          ErrorLookup[BADFEATCUT].Config1[C_FMLF] = 1;
          ErrorLookup[BADFEATCUT].Config2[C_FMLF] = 1;
          ErrorLookup[BADFEATCUT].Config3[C_FMLF]  = 1;

          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[BADFEATCUT].Domain1[i] = 0;
              ErrorLookup[BADFEATCUT].Domain2[i] = 0;
              ErrorLookup[BADFEATCUT].Domain3[i] = 0;
            }
          ErrorLookup[BADFEATCUT].Domain1[7]  = 1;
          ErrorLookup[BADFEATCUT].Domain2[6]  = 1;
          ErrorLookup[BADFEATCUT].Domain2[10] = 1;
        }
      break;

    case NONODEOVLP: /** line, area have overlapping edge without common node ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;

          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[7]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[NONODEOVLP].Config1[i] = 2;
              ErrorLookup[NONODEOVLP].Config2[i] = 2;
            }
          ErrorLookup[NONODEOVLP].Config1[C_DILI]  = 1;
          ErrorLookup[NONODEOVLP].Config2[C_DILI]  = 1;
          ErrorLookup[NONODEOVLP].Config1[C_LINE]  = 1;
          ErrorLookup[NONODEOVLP].Config2[C_LINE]  = 1;
          ErrorLookup[NONODEOVLP].Config1[C_FMLF] = 1;
          ErrorLookup[NONODEOVLP].Config2[C_FMLF] = 1;

          ErrorLookup[NONODEOVLP].Config1[C_AREA] = 1;
          ErrorLookup[NONODEOVLP].Config1[C_FMAF]= 1;
          ErrorLookup[NONODEOVLP].Config2[C_AREA] = 1;
          ErrorLookup[NONODEOVLP].Config2[C_FMAF]= 1;


          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[NONODEOVLP].Domain1[i] = 0;
              ErrorLookup[NONODEOVLP].Domain2[i] = 0;
            }
          ErrorLookup[NONODEOVLP].Domain1[4]  = 1;
          ErrorLookup[NONODEOVLP].Domain2[6]  = 1;
          ErrorLookup[NONODEOVLP].Domain2[10] = 1;
        }
      break;



      
    case LLNONODEINT: /* features intersect, but not at a shared node **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
            CloneErrorLookup[Cloneindex].Config1[i] = 1;
            CloneErrorLookup[Cloneindex].Config2[i] = 1;
            CloneErrorLookup[Cloneindex].Config3[i] = 1;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID]  = 2;
          CloneErrorLookup[Cloneindex].Config2[C_GRID]  = 2;
          CloneErrorLookup[Cloneindex].Config3[C_GRID]  = 2;
	  
          for(i=0; i<NUM_D; i++)
            {
            CloneErrorLookup[Cloneindex].Domain1[i] = 1;
            CloneErrorLookup[Cloneindex].Domain2[i] = 1;
            CloneErrorLookup[Cloneindex].Domain3[i] = 1;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
            ErrorLookup[LLNONODEINT].Config1[i] = 1;
            ErrorLookup[LLNONODEINT].Config2[i] = 1;
            ErrorLookup[LLNONODEINT].Config3[i] = 1;
            }
          ErrorLookup[LLNONODEINT].Config1[C_GRID]  = 2;
          ErrorLookup[LLNONODEINT].Config2[C_GRID]  = 2;
          ErrorLookup[LLNONODEINT].Config3[C_GRID]  = 2;
	  
          for(i=0; i<NUM_D; i++)
            {
            ErrorLookup[LLNONODEINT].Domain1[i] = 1;
            ErrorLookup[LLNONODEINT].Domain2[i] = 1;
            ErrorLookup[LLNONODEINT].Domain3[i] = 1;
            }
        }
      break;
      
      
    case LLINTAWAY: /** two lines intersect, and cross over each other ***/
    case LLINTNOEND: /** two lines intersect, pt of intersection is away from either primary particpant end node ***/
    case LLNOENDINT: /** lines intersect, but not at end point **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
	  
          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 1;
              CloneErrorLookup[Cloneindex].Domain2[i] = 1;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config2[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config2[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
	  
          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[Check].Domain1[i] = 1;
              ErrorLookup[Check].Domain2[i] = 1;
            }
        }
      break;
      
      
    case LFNOINT: /** line fails to intersect another line, area, or point and no end node on 1/4 degree line ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_POMO] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[7]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LFNOINT].Config1[i] = 2;
              ErrorLookup[LFNOINT].Config2[i] = 2;
            }
          ErrorLookup[LFNOINT].Config1[C_DILI]  = 1;
          ErrorLookup[LFNOINT].Config2[C_DILI]  = 1;
          ErrorLookup[LFNOINT].Config1[C_LINE]  = 1;
          ErrorLookup[LFNOINT].Config2[C_LINE]  = 1;
          ErrorLookup[LFNOINT].Config1[C_FMLF] = 1;
          ErrorLookup[LFNOINT].Config2[C_FMLF] = 1;

          ErrorLookup[LFNOINT].Config2[C_AREA] = 1;
          ErrorLookup[LFNOINT].Config2[C_FMAF]= 1;
          ErrorLookup[LFNOINT].Config2[C_POMO] = 1;
          ErrorLookup[LFNOINT].Config2[C_POFE] = 1;
          ErrorLookup[LFNOINT].Config2[C_FMPF] = 1;


          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[LFNOINT].Domain1[i] = 0;
              ErrorLookup[LFNOINT].Domain2[i] = 0;
            }
          ErrorLookup[LFNOINT].Domain1[4]  = 1;
          ErrorLookup[LFNOINT].Domain2[6]  = 1;
          ErrorLookup[LFNOINT].Domain2[10] = 1;
        }
      break;


      
    case LLNOINT:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
	  
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
	  
          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[7]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LLNOINT].Config1[i] = 2;
              ErrorLookup[LLNOINT].Config2[i] = 2;
            }
          ErrorLookup[LLNOINT].Config1[C_DILI]  = 1;
          ErrorLookup[LLNOINT].Config2[C_DILI]  = 1;
          ErrorLookup[LLNOINT].Config1[C_LINE]  = 1;
          ErrorLookup[LLNOINT].Config2[C_LINE]  = 1;
          ErrorLookup[LLNOINT].Config1[C_FMLF] = 1;
          ErrorLookup[LLNOINT].Config2[C_FMLF] = 1;
	  
          ErrorLookup[LLNOINT].Config2[C_AREA] = 1;
          ErrorLookup[LLNOINT].Config2[C_FMAF]= 1;
	  
	  
          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[LLNOINT].Domain1[i] = 0;
              ErrorLookup[LLNOINT].Domain2[i] = 0;
            }
          ErrorLookup[LLNOINT].Domain1[4]  = 1;
          ErrorLookup[LLNOINT].Domain2[6]  = 1;
          ErrorLookup[LLNOINT].Domain2[10] = 1;
        }
      break;
      
      
    case LAIEX: /** line - area intersection with 3rd feature exception ***/
    case LLI_ANGLE: /*** 2 lines intersect at severe angle ***/
    case LLIEX:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	      CloneErrorLookup[Cloneindex].Config3[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          if(Check == LAIEX)
             {
             CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
             CloneErrorLookup[Cloneindex].Config2[C_FMAF]  = 1;
             }
          else
             {
	     CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
	     CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
	     CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
             }
	  CloneErrorLookup[Cloneindex].Config3[C_FOMO]  = 1;
	  CloneErrorLookup[Cloneindex].Config3[C_POLY]  = 1;
	  CloneErrorLookup[Cloneindex].Config3[C_MOLI] = 1;
	  CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
	  CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMPF] = 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain3[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[7] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10]= 1;
	  CloneErrorLookup[Cloneindex].Domain3[4] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[Check].Config1[i] = 2;
	      ErrorLookup[Check].Config2[i] = 2;
	      ErrorLookup[Check].Config3[i] = 2;
	    }
	  ErrorLookup[Check].Config1[C_DILI]  = 1;
	  ErrorLookup[Check].Config1[C_LINE]  = 1;
	  ErrorLookup[Check].Config1[C_FMLF] = 1;
          if(Check == LAIEX)
             {
             ErrorLookup[Check].Config2[C_AREA]  = 1;
             ErrorLookup[Check].Config2[C_FMAF]  = 1;
             }
          else
             {
	     ErrorLookup[Check].Config2[C_DILI]  = 1;
	     ErrorLookup[Check].Config2[C_LINE]  = 1;
	     ErrorLookup[Check].Config2[C_FMLF] = 1;
             }
	  ErrorLookup[Check].Config3[C_FOMO]  = 1;
	  ErrorLookup[Check].Config3[C_POLY]  = 1;
	  ErrorLookup[Check].Config3[C_MOLI] = 1;
	  ErrorLookup[Check].Config3[C_AREA]  = 1;
	  ErrorLookup[Check].Config3[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_LINE]  = 1;
          ErrorLookup[Check].Config3[C_DILI]  = 1;
          ErrorLookup[Check].Config3[C_FMLF] = 1;
          ErrorLookup[Check].Config3[C_POFE]  = 1;
          ErrorLookup[Check].Config3[C_FMPF] = 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      ErrorLookup[Check].Domain1[i] = 0;
	      ErrorLookup[Check].Domain2[i] = 0;
	      ErrorLookup[Check].Domain3[i] = 0;
	    }
	  ErrorLookup[Check].Domain1[7] = 1;
	  ErrorLookup[Check].Domain2[6] = 1;
	  ErrorLookup[Check].Domain2[10]= 1;
	  ErrorLookup[Check].Domain3[4] = 1;
	}
      break;
      
      
    case LVPROX:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[LVPROX].Config1[i] = 2;
	      ErrorLookup[LVPROX].Config2[i] = 2;
	    }
	  ErrorLookup[LVPROX].Config1[C_DILI]  = 1;
	  ErrorLookup[LVPROX].Config2[C_DILI]  = 1;
	  ErrorLookup[LVPROX].Config1[C_LINE]  = 1;
	  ErrorLookup[LVPROX].Config2[C_LINE]  = 1;
	  ErrorLookup[LVPROX].Config1[C_FMLF] = 1;
	  ErrorLookup[LVPROX].Config2[C_FMLF] = 1;
	}
      break;
      
      
    case LELINEPROX:  /** line end - line proximity ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LELINEPROX].Config1[i] = 2;
              ErrorLookup[LELINEPROX].Config2[i] = 2;
            }
          ErrorLookup[LELINEPROX].Config1[C_DILI]  = 1;
          ErrorLookup[LELINEPROX].Config2[C_DILI]  = 1;
          ErrorLookup[LELINEPROX].Config1[C_LINE]  = 1;
          ErrorLookup[LELINEPROX].Config2[C_LINE]  = 1;
          ErrorLookup[LELINEPROX].Config1[C_FMLF] = 1;
          ErrorLookup[LELINEPROX].Config2[C_FMLF] = 1;
          ErrorLookup[LELINEPROX].Config2[C_POFE] = 1;
          ErrorLookup[LELINEPROX].Config2[C_AREA]  = 1;
          ErrorLookup[LELINEPROX].Config2[C_FMAF] = 1;
        }
      break;

    case EN_EN_PROX:  /** undershoot end nodes connected by another feature **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[EN_EN_PROX].Config1[i] = 2;
              ErrorLookup[EN_EN_PROX].Config2[i] = 2;
              ErrorLookup[EN_EN_PROX].Config3[i] = 2;
            }
          ErrorLookup[EN_EN_PROX].Config1[C_DILI]  = 1;
          ErrorLookup[EN_EN_PROX].Config2[C_DILI]  = 1;
          ErrorLookup[EN_EN_PROX].Config1[C_LINE]  = 1;
          ErrorLookup[EN_EN_PROX].Config2[C_LINE]  = 1;
          ErrorLookup[EN_EN_PROX].Config1[C_FMLF] = 1;
          ErrorLookup[EN_EN_PROX].Config2[C_FMLF] = 1;
          ErrorLookup[EN_EN_PROX].Config3[C_AREA] = 1;
          ErrorLookup[EN_EN_PROX].Config3[C_FMAF] = 1;
          ErrorLookup[EN_EN_PROX].Config3[C_DILI] = 1;
          ErrorLookup[EN_EN_PROX].Config3[C_LINE] = 1;
          ErrorLookup[EN_EN_PROX].Config3[C_FMLF] = 1;
        }
      break;

    case VUSHTL_CLEAN: /* like vertex - line undershoot, but no condition if feature mid-undershoot **/
    case LUSHTL_CLEAN: /* like line - line undershoot, but no condition if feature mid-undershoot **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config2[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config2[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
          ErrorLookup[Check].Config3[C_AREA] = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_DILI]  = 1;
          ErrorLookup[Check].Config3[C_LINE]  = 1;
          ErrorLookup[Check].Config3[C_FMLF] = 1;
        }
      break;

      
    case LVUSHTL: /** interior line vertex undershoots a different line feature **/
    case LUNDERSHTL:  /** line end - line undershoot **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config2[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config2[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
          ErrorLookup[Check].Config3[C_AREA] = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_DILI]  = 1;
          ErrorLookup[Check].Config3[C_LINE]  = 1;
          ErrorLookup[Check].Config3[C_FMLF] = 1;
        }
      break;

    case LOSHTL_DF:  
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LOSHTL_DF].Config1[i] = 2;
              ErrorLookup[LOSHTL_DF].Config2[i] = 2;
              ErrorLookup[LOSHTL_DF].Config3[i] = 2;
            }
          ErrorLookup[LOSHTL_DF].Config1[C_DILI]  = 1;
          ErrorLookup[LOSHTL_DF].Config2[C_DILI]  = 1;
          ErrorLookup[LOSHTL_DF].Config1[C_LINE]  = 1;
          ErrorLookup[LOSHTL_DF].Config2[C_LINE]  = 1;
          ErrorLookup[LOSHTL_DF].Config1[C_FMLF] = 1;
          ErrorLookup[LOSHTL_DF].Config2[C_FMLF] = 1;
          ErrorLookup[LOSHTL_DF].Config3[C_AREA] = 1;
          ErrorLookup[LOSHTL_DF].Config3[C_FMAF] = 1;
          ErrorLookup[LOSHTL_DF].Config3[C_DILI]  = 1;
          ErrorLookup[LOSHTL_DF].Config3[C_LINE]  = 1;
          ErrorLookup[LOSHTL_DF].Config3[C_FMLF] = 1;
        }
      break;

   case LUSHTL_DF: 
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LUSHTL_DF].Config1[i] = 2;
              ErrorLookup[LUSHTL_DF].Config2[i] = 2;
              ErrorLookup[LUSHTL_DF].Config3[i] = 2;
            }
          ErrorLookup[LUSHTL_DF].Config1[C_DILI]  = 1;
          ErrorLookup[LUSHTL_DF].Config2[C_DILI]  = 1;
          ErrorLookup[LUSHTL_DF].Config1[C_LINE]  = 1;
          ErrorLookup[LUSHTL_DF].Config2[C_LINE]  = 1;
          ErrorLookup[LUSHTL_DF].Config1[C_FMLF] = 1;
          ErrorLookup[LUSHTL_DF].Config2[C_FMLF] = 1;
          ErrorLookup[LUSHTL_DF].Config3[C_AREA] = 1;
          ErrorLookup[LUSHTL_DF].Config3[C_FMAF] = 1;
          ErrorLookup[LUSHTL_DF].Config3[C_DILI]  = 1;
          ErrorLookup[LUSHTL_DF].Config3[C_LINE]  = 1;
          ErrorLookup[LUSHTL_DF].Config3[C_FMLF] = 1;
        }
      break;



    case LVOSHTL: /** interior line vertex overshoots a different line feature ***/
    case LOVERSHTL:   /** line end - line overshoot **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config2[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config2[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
          ErrorLookup[Check].Config3[C_AREA] = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_DILI]  = 1;
          ErrorLookup[Check].Config3[C_LINE]  = 1;
          ErrorLookup[Check].Config3[C_FMLF] = 1;
        }
      break;
      
    case LUNDERSHTA:  /** line end area perimeter undershoot **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LUNDERSHTA].Config1[i] = 2;
              ErrorLookup[LUNDERSHTA].Config2[i] = 2;
              ErrorLookup[LUNDERSHTA].Config3[i] = 2;
            }
          ErrorLookup[LUNDERSHTA].Config1[C_DILI]  = 1;
          ErrorLookup[LUNDERSHTA].Config1[C_LINE]  = 1;
          ErrorLookup[LUNDERSHTA].Config1[C_FMLF] = 1;
          ErrorLookup[LUNDERSHTA].Config2[C_DILI]  = 1;
          ErrorLookup[LUNDERSHTA].Config2[C_LINE]  = 1;
          ErrorLookup[LUNDERSHTA].Config2[C_FMLF] = 1;
          ErrorLookup[LUNDERSHTA].Config2[C_FMAF] = 1;
          ErrorLookup[LUNDERSHTA].Config2[C_AREA]  = 1;
          ErrorLookup[LUNDERSHTA].Config3[C_FMAF] = 1;
          ErrorLookup[LUNDERSHTA].Config3[C_AREA]  = 1;
        }
      break;
      
    case LOVERSHTA:  /** line end - area perimeter overshoot **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LOVERSHTA].Config1[i] = 2;
              ErrorLookup[LOVERSHTA].Config2[i] = 2;
              ErrorLookup[LOVERSHTA].Config3[i] = 2;
            }
          ErrorLookup[LOVERSHTA].Config1[C_DILI]  = 1;
          ErrorLookup[LOVERSHTA].Config1[C_LINE]  = 1;
          ErrorLookup[LOVERSHTA].Config1[C_FMLF] = 1;
          ErrorLookup[LOVERSHTA].Config2[C_DILI]  = 1;
          ErrorLookup[LOVERSHTA].Config2[C_LINE]  = 1;
          ErrorLookup[LOVERSHTA].Config2[C_FMLF] = 1;
          ErrorLookup[LOVERSHTA].Config3[C_AREA]  = 1;
          ErrorLookup[LOVERSHTA].Config3[C_FMAF] = 1;
        }
      break;

    case LAPROX:  /** line to area proximity - smallest dist between the two features ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LAPROX].Config1[i] = 2;
              ErrorLookup[LAPROX].Config2[i] = 2;
              ErrorLookup[LAPROX].Config3[i] = 2;
            }
          ErrorLookup[LAPROX].Config1[C_DILI]  = 1;
          ErrorLookup[LAPROX].Config1[C_LINE]  = 1;
          ErrorLookup[LAPROX].Config1[C_FMLF] = 1;
          ErrorLookup[LAPROX].Config2[C_AREA]  = 1;
          ErrorLookup[LAPROX].Config2[C_FMAF] = 1;
        }
      break;

    case LASLIVER: /** sliver formed between line and area features **/
      if(Clone==1)
          {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_DILI]  = 1;
          ErrorLookup[Check].Config3[C_LINE]  = 1;
          ErrorLookup[Check].Config3[C_FMLF] = 1;
          ErrorLookup[Check].Config3[C_AREA]  = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
        }
      break;


    case LSLICEA: /** line 'slices' area so as create a small piece ***/
      if(Clone==1)
          {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1; 
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1; 
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2; 
              ErrorLookup[Check].Config3[i] = 2; 
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1; 
          ErrorLookup[Check].Config1[C_LINE]  = 1; 
          ErrorLookup[Check].Config1[C_FMLF] = 1; 
          ErrorLookup[Check].Config2[C_AREA]  = 1; 
          ErrorLookup[Check].Config2[C_FMAF] = 1; 
          ErrorLookup[Check].Config3[C_DILI]  = 1;
          ErrorLookup[Check].Config3[C_LINE]  = 1;
          ErrorLookup[Check].Config3[C_FMLF] = 1;
        }
      break;

    case LLSLIVER:  /** sliver formed between two line features **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LLSLIVER].Config1[i] = 2;
              ErrorLookup[LLSLIVER].Config2[i] = 2;
              ErrorLookup[LLSLIVER].Config3[i] = 2;
            }
          ErrorLookup[LLSLIVER].Config1[C_DILI]  = 1;
          ErrorLookup[LLSLIVER].Config1[C_LINE]  = 1;
          ErrorLookup[LLSLIVER].Config1[C_FMLF] = 1;
          ErrorLookup[LLSLIVER].Config2[C_DILI]  = 1;
          ErrorLookup[LLSLIVER].Config2[C_LINE]  = 1;
          ErrorLookup[LLSLIVER].Config2[C_FMLF] = 1;
        }
      break;


    case AUNDERSHTA: /** area edge undershoots neighbor area edge ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[AUNDERSHTA].Config1[i] = 2;
              ErrorLookup[AUNDERSHTA].Config2[i] = 2;
              ErrorLookup[AUNDERSHTA].Config3[i] = 2;
            }
          ErrorLookup[AUNDERSHTA].Config1[C_AREA]  = 1;
          ErrorLookup[AUNDERSHTA].Config1[C_FMAF] = 1;
          ErrorLookup[AUNDERSHTA].Config2[C_AREA]  = 1;
          ErrorLookup[AUNDERSHTA].Config2[C_FMAF] = 1;
          ErrorLookup[AUNDERSHTA].Config3[C_AREA]  = 1;
          ErrorLookup[AUNDERSHTA].Config3[C_FMAF] = 1;
        }
      break;

    case AOVERSHTA: /** area edge overshoots neighbor area edge ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[AOVERSHTA].Config1[i] = 2;
              ErrorLookup[AOVERSHTA].Config2[i] = 2;
              ErrorLookup[AOVERSHTA].Config3[i] = 2;
            }
          ErrorLookup[AOVERSHTA].Config1[C_AREA]  = 1;
          ErrorLookup[AOVERSHTA].Config1[C_FMAF] = 1;
          ErrorLookup[AOVERSHTA].Config2[C_AREA]  = 1;
          ErrorLookup[AOVERSHTA].Config2[C_FMAF] = 1;
        }
      break;

    case LSAME_UNM_A: /*** line endpt unmatched with line of same FCODE at Area boundary ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LSAME_UNM_A].Config1[i] = 2;
              ErrorLookup[LSAME_UNM_A].Config2[i] = 2;
              ErrorLookup[LSAME_UNM_A].Config3[i] = 2;
            }
          ErrorLookup[LSAME_UNM_A].Config1[C_DILI]  = 1;
          ErrorLookup[LSAME_UNM_A].Config1[C_LINE]  = 1;
          ErrorLookup[LSAME_UNM_A].Config1[C_FMLF] = 1;
          ErrorLookup[LSAME_UNM_A].Config2[C_AREA]  = 1;
          ErrorLookup[LSAME_UNM_A].Config2[C_FMAF] = 1;
        }
      break;

    case LUNMA_ACRS_A: /** line end not matched to area node across area perimeter ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_AREA]  = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
        }
      break;


    case LUNM_ATTR_A:
    case LUNM_ACRS_A: /*** line mismatch across poly edge ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_DILI]  = 1;
          ErrorLookup[Check].Config2[C_LINE]  = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
          ErrorLookup[Check].Config3[C_AREA]  = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
        }
      break;


    case LE_A_UNM_LAT: /** line end node not coincident with area node at latitude parallel **/
    case LE_A_UNM_LON: /** line end node not coincident with area node at longitude meridian **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;

        }
      break;


    case LRNGE_UNM_LAT:
    case LRNGE_UNM_LON:
    case LHANG_LON: /** hanging line feature at a specified longitude meridian ***/
    case LHANG_LAT: /** hanging line feature at a specified latitude parallel ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_DILI]  = 1;
          ErrorLookup[Check].Config2[C_LINE]  = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
          ErrorLookup[Check].Config3[C_DILI]  = 1;
          ErrorLookup[Check].Config3[C_LINE]  = 1;
          ErrorLookup[Check].Config3[C_FMLF] = 1;

        }
      break;


    case LGEOM_UNM_LAT:
    case LGEOM_UNM_LON:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;
        }
      break;


    case ARNGE_UNM_LAT:
    case ARNGE_UNM_LON:
    case AHANG_LON: /** hanging area feature at a specified longitude meridian ***/
    case AHANG_LAT: /** hanging area feature at a specified latitude parallel ***/
    case AUNM_ACRS_A: /** area feature edge incorrectly matched across a bounding area feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_AREA]  = 1;
          ErrorLookup[Check].Config1[C_FMAF] = 1;
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_AREA]  = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
        }
      break;


    case AGEOM_UNM_LAT:
    case AGEOM_UNM_LON:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_AREA]  = 1;
          ErrorLookup[Check].Config1[C_FMAF] = 1;
        }
      break;

    case AUNM_ATTR_A:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_AREA]  = 1;
          ErrorLookup[Check].Config1[C_FMAF] = 1;
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;
          ErrorLookup[Check].Config3[C_AREA]  = 1;
          ErrorLookup[Check].Config3[C_FMAF] = 1;
        }
      break;



    case L_UNM_A:  /*** line endpt unmatched at area feature boundary ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[L_UNM_A].Config1[i] = 2;
              ErrorLookup[L_UNM_A].Config2[i] = 2;
              ErrorLookup[L_UNM_A].Config3[i] = 2;
            }
          ErrorLookup[L_UNM_A].Config1[C_DILI]  = 1;
          ErrorLookup[L_UNM_A].Config1[C_LINE]  = 1;
          ErrorLookup[L_UNM_A].Config1[C_FMLF] = 1;
          ErrorLookup[L_UNM_A].Config2[C_AREA]  = 1;
          ErrorLookup[L_UNM_A].Config2[C_FMAF] = 1;
        }
      break;

    case LOC_MULTINT: /** lines with no or compatible LOC values intersect each other multiple times **/
    case LLMULTINT: /** lines intersect each other multiple times **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config2[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config2[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
        }
      break;
      
    case L2D_L3D_MATCH:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[L2D_L3D_MATCH].Config1[i] = 2;
              ErrorLookup[L2D_L3D_MATCH].Config2[i] = 2;
            }
          ErrorLookup[L2D_L3D_MATCH].Config1[C_DILI]  = 1;
          ErrorLookup[L2D_L3D_MATCH].Config2[C_DILI]  = 1;
          ErrorLookup[L2D_L3D_MATCH].Config1[C_LINE]  = 1;
          ErrorLookup[L2D_L3D_MATCH].Config2[C_LINE]  = 1;
          ErrorLookup[L2D_L3D_MATCH].Config1[C_FMLF] = 1;
          ErrorLookup[L2D_L3D_MATCH].Config2[C_FMLF] = 1;
        }
      break;
      
    case LEZ_PROX_3D: /** apply check L2D_L3D_MATCH to 3d line features only **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LEZ_PROX_3D].Config1[i] = 2;
              ErrorLookup[LEZ_PROX_3D].Config2[i] = 2;
            }
          ErrorLookup[LEZ_PROX_3D].Config1[C_DILI]  = 1;
          ErrorLookup[LEZ_PROX_3D].Config2[C_DILI]  = 1;
          ErrorLookup[LEZ_PROX_3D].Config1[C_LINE]  = 1;
          ErrorLookup[LEZ_PROX_3D].Config2[C_LINE]  = 1;
          ErrorLookup[LEZ_PROX_3D].Config1[C_FMLF] = 1;
          ErrorLookup[LEZ_PROX_3D].Config2[C_FMLF] = 1;
        }
      break;
      
   case CNODE_ZBUST:  /*** Z mismatch between any two connecting nodes (in x,y) ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 1;
              CloneErrorLookup[Cloneindex].Config2[j] = 1;
              CloneErrorLookup[Cloneindex].Config3[j] = 1;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 2;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 2;
          CloneErrorLookup[Cloneindex].Config3[C_GRID] = 2;
          CloneErrorLookup[Cloneindex].Stratum1[2] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[Check].Config1[j] = 1;
              ErrorLookup[Check].Config2[j] = 1;
              ErrorLookup[Check].Config3[j] = 1;
            }
          ErrorLookup[Check].Config1[C_GRID] = 2;
          ErrorLookup[Check].Config2[C_GRID] = 2;
          ErrorLookup[Check].Config3[C_GRID] = 2;
          ErrorLookup[Check].Stratum1[2] = 1;
        }
      break;

      
    case SHAREPERIM:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[SHAREPERIM].Config1[i] = 2;
              ErrorLookup[SHAREPERIM].Config2[i] = 2;
            }
          ErrorLookup[SHAREPERIM].Config1[C_AREA]  = 1;
          ErrorLookup[SHAREPERIM].Config2[C_AREA]  = 1;
          ErrorLookup[SHAREPERIM].Config1[C_FMAF] = 1;
          ErrorLookup[SHAREPERIM].Config2[C_FMAF] = 1;
        }
      break;
      
      
      
      
    case LMINT:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FOMO]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_POMO]  = 0;
	  CloneErrorLookup[Cloneindex].Config2[C_COVS] = 0;
	  CloneErrorLookup[Cloneindex].Config2[C_COVP] = 0;
	  CloneErrorLookup[Cloneindex].Config2[C_MOLI] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[LMINT].Config1[i] = 2;
	      ErrorLookup[LMINT].Config2[i] = 2;
	    }
	  ErrorLookup[LMINT].Config1[C_DILI]  = 1;
	  ErrorLookup[LMINT].Config1[C_LINE]  = 1;
	  ErrorLookup[LMINT].Config1[C_FMLF] = 1;
	  ErrorLookup[LMINT].Config2[C_FOMO]  = 1;
	  ErrorLookup[LMINT].Config2[C_POMO]  = 0;
	  ErrorLookup[LMINT].Config2[C_COVS] = 0;
	  ErrorLookup[LMINT].Config2[C_COVP] = 0;
	  ErrorLookup[LMINT].Config2[C_MOLI] = 1;
	}
      break;
      
      
    case LSPINT:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
	  CloneErrorLookup[Cloneindex].Config2[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_MOLI]= 0;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[LSPINT].Config1[i] = 2;
	      ErrorLookup[LSPINT].Config2[i] = 2;
	    }
	  ErrorLookup[LSPINT].Config1[C_DILI] = 1;
	  ErrorLookup[LSPINT].Config1[C_LINE] = 1;
	  ErrorLookup[LSPINT].Config1[C_FMLF]= 1;
	  ErrorLookup[LSPINT].Config2[C_POLY] = 1;
	  ErrorLookup[LSPINT].Config2[C_MOLI]= 0;
	}
      break;
      
      
    case LSPIEXP:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	      CloneErrorLookup[Cloneindex].Config3[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
	  
	  CloneErrorLookup[Cloneindex].Config2[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_MOLI]= 0;
	  
	  CloneErrorLookup[Cloneindex].Config3[C_AREA] = 0;
	  CloneErrorLookup[Cloneindex].Config3[C_FOMO] = 1;
	  CloneErrorLookup[Cloneindex].Config3[C_POMO] = 0;
	  CloneErrorLookup[Cloneindex].Config3[C_POLY] = 0;
	  CloneErrorLookup[Cloneindex].Config3[C_FMAF]= 0;
	  for(i=0; i<NUM_D; i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain3[i] = 0;
	    }
	  
	  CloneErrorLookup[Cloneindex].Domain1[D_LAMO] = 1;
	  
	  CloneErrorLookup[Cloneindex].Domain2[D_INWA] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[D_OPWA] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[D_TERR] = 1;
	  
	  CloneErrorLookup[Cloneindex].Domain3[D_BRID] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[LSPIEXP].Config1[i] = 2;
	      ErrorLookup[LSPIEXP].Config2[i] = 2;
	      ErrorLookup[LSPIEXP].Config3[i] = 2;
	    }
	  ErrorLookup[LSPIEXP].Config1[C_DILI] = 1;
	  ErrorLookup[LSPIEXP].Config1[C_LINE] = 1;
	  ErrorLookup[LSPIEXP].Config1[C_FMLF]= 1;
	  
	  ErrorLookup[LSPIEXP].Config2[C_POLY] = 1;
	  ErrorLookup[LSPIEXP].Config2[C_MOLI]= 0;
	  
	  ErrorLookup[LSPIEXP].Config3[C_AREA] = 0;
	  ErrorLookup[LSPIEXP].Config3[C_FOMO] = 1;
	  ErrorLookup[LSPIEXP].Config3[C_POMO] = 0;
	  ErrorLookup[LSPIEXP].Config3[C_POLY] = 0;
	  ErrorLookup[LSPIEXP].Config3[C_FMAF]= 0;
	  for(i=0; i<NUM_D; i++)
	    {
	      ErrorLookup[LSPIEXP].Domain1[i] = 0;
	      ErrorLookup[LSPIEXP].Domain2[i] = 0;
	      ErrorLookup[LSPIEXP].Domain3[i] = 0;
	    }
	  
	  ErrorLookup[LSPIEXP].Domain1[D_LAMO] = 1;
	  
	  ErrorLookup[LSPIEXP].Domain2[D_INWA] = 1;
	  ErrorLookup[LSPIEXP].Domain2[D_OPWA] = 1;
	  ErrorLookup[LSPIEXP].Domain2[D_TERR] = 1;
	  
	  ErrorLookup[LSPIEXP].Domain3[D_BRID] = 1;
	}
      break;
      
      
      
    case PLPROX:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMPF]= 1;
	  CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF]= 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain1[10] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[PLPROX].Config1[i] = 2;
	      ErrorLookup[PLPROX].Config2[i] = 2;
              ErrorLookup[PLPROX].Config3[i] = 2;
	    }
	  ErrorLookup[PLPROX].Config1[C_POFE] = 1;
	  ErrorLookup[PLPROX].Config1[C_FMPF]= 1;
	  ErrorLookup[PLPROX].Config2[C_DILI] = 1;
	  ErrorLookup[PLPROX].Config2[C_LINE] = 1;
	  ErrorLookup[PLPROX].Config2[C_FMLF]= 1;
          ErrorLookup[PLPROX].Config3[C_DILI] = 1;
          ErrorLookup[PLPROX].Config3[C_LINE] = 1;
          ErrorLookup[PLPROX].Config3[C_FMLF]= 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      ErrorLookup[PLPROX].Domain1[i] = 0;
	      ErrorLookup[PLPROX].Domain2[i] = 0;
	    }
	  ErrorLookup[PLPROX].Domain1[6]  = 1;
	  ErrorLookup[PLPROX].Domain2[6]  = 1;
	  ErrorLookup[PLPROX].Domain1[10] = 1;
	  ErrorLookup[PLPROX].Domain2[10] = 1;
	}
      break;


   case PSHOOTL: /*** point feature over or undershoots a line feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain1[10] = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PSHOOTL].Config1[i] = 2;
              ErrorLookup[PSHOOTL].Config2[i] = 2;
            }
          ErrorLookup[PSHOOTL].Config1[C_POFE] = 1;
          ErrorLookup[PSHOOTL].Config1[C_FMPF]= 1;
          ErrorLookup[PSHOOTL].Config2[C_DILI] = 1;
          ErrorLookup[PSHOOTL].Config2[C_LINE] = 1;
          ErrorLookup[PSHOOTL].Config2[C_FMLF]= 1;

          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[PSHOOTL].Domain1[i] = 0;
              ErrorLookup[PSHOOTL].Domain2[i] = 0;
            }
          ErrorLookup[PSHOOTL].Domain1[6]  = 1;
          ErrorLookup[PSHOOTL].Domain2[6]  = 1;
          ErrorLookup[PSHOOTL].Domain1[10] = 1;
          ErrorLookup[PSHOOTL].Domain2[10] = 1;
        }
      break;



    case PLPROXEX:  /** pt to line prox with exception for line end node ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain1[10] = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PLPROXEX].Config1[i] = 2;
              ErrorLookup[PLPROXEX].Config2[i] = 2;
            }
          ErrorLookup[PLPROXEX].Config1[C_POFE] = 1;
          ErrorLookup[PLPROXEX].Config1[C_FMPF]= 1;
          ErrorLookup[PLPROXEX].Config2[C_DILI] = 1;
          ErrorLookup[PLPROXEX].Config2[C_LINE] = 1;
          ErrorLookup[PLPROXEX].Config2[C_FMLF]= 1;

          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[PLPROXEX].Domain1[i] = 0;
              ErrorLookup[PLPROXEX].Domain2[i] = 0;
            }
          ErrorLookup[PLPROXEX].Domain1[6]  = 1;
          ErrorLookup[PLPROXEX].Domain2[6]  = 1;
          ErrorLookup[PLPROXEX].Domain1[10] = 1;
          ErrorLookup[PLPROXEX].Domain2[10] = 1;
        }
      break;

      
    case ENCONNECT:
    case BADENCON: /** bad sequence on line feature connections ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;
	  
          for(i=0; i<NUM_D; i++)
            {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 1;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 1;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
	      ErrorLookup[Check].Config1[i] = 2;
	      ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI] = 1;
          ErrorLookup[Check].Config1[C_LINE] = 1;
          ErrorLookup[Check].Config1[C_FMLF]= 1;
          ErrorLookup[Check].Config2[C_DILI] = 1;
          ErrorLookup[Check].Config2[C_LINE] = 1;
          ErrorLookup[Check].Config2[C_FMLF]= 1;
	  
          for(i=0; i<NUM_D; i++)
            {
	      ErrorLookup[Check].Domain1[i] = 1;
	      ErrorLookup[Check].Domain2[i] = 1;
            }
        }
      break;


    case DUPLICATESEG:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[DUPLICATESEG].Config1[i] = 2;
              ErrorLookup[DUPLICATESEG].Config2[i] = 2;
            }
          ErrorLookup[DUPLICATESEG].Config1[C_DILI]  = 1;
          ErrorLookup[DUPLICATESEG].Config2[C_DILI]  = 1;
          ErrorLookup[DUPLICATESEG].Config1[C_LINE]  = 1;
          ErrorLookup[DUPLICATESEG].Config2[C_LINE]  = 1;
          ErrorLookup[DUPLICATESEG].Config1[C_FMLF] = 1;
          ErrorLookup[DUPLICATESEG].Config2[C_FMLF] = 1;
        }
      break;

      

    case EXTRA_NET:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]= 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0; /** all eligible, but inactive **/
            }
          if(NGA_TYPE == 0)
             CloneErrorLookup[Cloneindex].Domain1[7] = 1; /** should set land transpo on as default for SEEIT ***/
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[EXTRA_NET].Config1[i] = 2;
            }
          ErrorLookup[EXTRA_NET].Config1[C_DILI] = 1;
          ErrorLookup[EXTRA_NET].Config1[C_LINE] = 1;
          ErrorLookup[EXTRA_NET].Config1[C_FMLF]= 1;
          ErrorLookup[EXTRA_NET].Config1[C_AREA] = 1;
          ErrorLookup[EXTRA_NET].Config1[C_FMAF]= 1;
          ErrorLookup[EXTRA_NET].Config1[C_POFE] = 1;

          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[EXTRA_NET].Domain1[i] = 0; /** all eligible, but inactive **/
            }
          if(NGA_TYPE == 0)
             ErrorLookup[EXTRA_NET].Domain1[7] = 1; /** should set land transpo on as default for SEEIT ***/
        }
      break;


   case CREATENET: /*** the internal check for creating networks - shouldn't appear in inspection menu ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]= 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0; /** all eligible, but inactive **/
            }
          if(NGA_TYPE == 0)
             CloneErrorLookup[Cloneindex].Domain1[7] = 1; /** should set land transpo on as default for SEEIT ***/
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[CREATENET].Config1[i] = 2;
            }
          ErrorLookup[CREATENET].Config1[C_DILI] = 1;
          ErrorLookup[CREATENET].Config1[C_LINE] = 1;
          ErrorLookup[CREATENET].Config1[C_FMLF]= 1;
          ErrorLookup[CREATENET].Config1[C_AREA] = 1;
          ErrorLookup[CREATENET].Config1[C_FMAF]= 1;
          ErrorLookup[CREATENET].Config1[C_POFE] = 1;

          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[CREATENET].Domain1[i] = 0; /** all eligible, but inactive **/
            }
          if(NGA_TYPE == 0)
             ErrorLookup[CREATENET].Domain1[7] = 1; /** should set land transpo on as default for SEEIT ***/
        }
      break;

    case INTRA_NET:   /*** vertex is close to but not identical with another vertex in the same network ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]= 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0; /** all eligible, but inactive **/
            }
          if(NGA_TYPE == 0)
             CloneErrorLookup[Cloneindex].Domain1[7] = 1; /** should set land transpo on as default for SEEIT ***/
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[INTRA_NET].Config1[i] = 2;
            }
          ErrorLookup[INTRA_NET].Config1[C_DILI] = 1;
          ErrorLookup[INTRA_NET].Config1[C_LINE] = 1;
          ErrorLookup[INTRA_NET].Config1[C_FMLF]= 1;
          ErrorLookup[INTRA_NET].Config1[C_AREA] = 1;
          ErrorLookup[INTRA_NET].Config1[C_FMAF]= 1;
          ErrorLookup[INTRA_NET].Config1[C_POFE] = 1;

          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[INTRA_NET].Domain1[i] = 0; /** all eligible, but inactive **/
            }
          if(NGA_TYPE == 0)
             ErrorLookup[INTRA_NET].Domain1[7] = 1; /** should set land transpo on as default for SEEIT ***/
        }
      break;

      
    case PLPFAIL:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[6] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[PLPFAIL].Config1[i] = 2;
	      ErrorLookup[PLPFAIL].Config2[i] = 2;
	    }
	  ErrorLookup[PLPFAIL].Config1[C_POFE]  = 1;
	  ErrorLookup[PLPFAIL].Config1[C_FMPF] = 1;
	  ErrorLookup[PLPFAIL].Config2[C_DILI]  = 1;
	  ErrorLookup[PLPFAIL].Config2[C_LINE]  = 1;
	  ErrorLookup[PLPFAIL].Config2[C_FMLF] = 1;
          ErrorLookup[PLPFAIL].Config2[C_AREA]  = 0;
          ErrorLookup[PLPFAIL].Config2[C_FMAF] = 0;
          ErrorLookup[PLPFAIL].Config2[C_POFE] = 1;
	  
	  for(i=0; i<NUM_D; i++)
	    {
	      ErrorLookup[PLPFAIL].Domain1[i] = 0;
	      ErrorLookup[PLPFAIL].Domain2[i] = 0;
	    }
	  ErrorLookup[PLPFAIL].Domain1[6] = 1;
	  ErrorLookup[PLPFAIL].Domain2[6] = 1;
	}
      break;
      
      
    case PNOCOVERLE: /* point not covered by linear end or area edge**/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
	  
          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[6] = 1;
          CloneErrorLookup[Cloneindex].Domain2[6] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PNOCOVERLE].Config1[i] = 2;
              ErrorLookup[PNOCOVERLE].Config2[i] = 2;
            }
          ErrorLookup[PNOCOVERLE].Config1[C_POFE]  = 1;
          ErrorLookup[PNOCOVERLE].Config1[C_FMPF] = 1;
          ErrorLookup[PNOCOVERLE].Config2[C_DILI]  = 1;
          ErrorLookup[PNOCOVERLE].Config2[C_LINE]  = 1;
          ErrorLookup[PNOCOVERLE].Config2[C_FMLF] = 1;
          ErrorLookup[PNOCOVERLE].Config2[C_AREA]  = 1;
          ErrorLookup[PNOCOVERLE].Config2[C_FMAF] = 1;
	  
          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[PNOCOVERLE].Domain1[i] = 0;
              ErrorLookup[PNOCOVERLE].Domain2[i] = 0;
            }
          ErrorLookup[PNOCOVERLE].Domain1[6] = 1;
          ErrorLookup[PNOCOVERLE].Domain2[6] = 1;
        }
      break;


    case PNOCOV2LEA: /** point not covered by 2 line terminal nodes or area edges***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[6] = 1;
          CloneErrorLookup[Cloneindex].Domain2[6] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PNOCOV2LEA].Config1[i] = 2;
              ErrorLookup[PNOCOV2LEA].Config2[i] = 2;
            }
          ErrorLookup[PNOCOV2LEA].Config1[C_POFE]  = 1;
          ErrorLookup[PNOCOV2LEA].Config1[C_FMPF] = 1;
          ErrorLookup[PNOCOV2LEA].Config2[C_DILI]  = 1;
          ErrorLookup[PNOCOV2LEA].Config2[C_LINE]  = 1;
          ErrorLookup[PNOCOV2LEA].Config2[C_FMLF] = 1;
          ErrorLookup[PNOCOV2LEA].Config2[C_AREA]  = 1;
          ErrorLookup[PNOCOV2LEA].Config2[C_FMAF] = 1;

          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[PNOCOV2LEA].Domain1[i] = 0;
              ErrorLookup[PNOCOV2LEA].Domain2[i] = 0;
            }
          ErrorLookup[PNOCOV2LEA].Domain1[6] = 1;
          ErrorLookup[PNOCOV2LEA].Domain2[6] = 1;
        }
      break;

      
    case PNOCOVERLV: /** point not covered by any line vertex **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
	  
          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[6] = 1;
          CloneErrorLookup[Cloneindex].Domain2[6] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PNOCOVERLV].Config1[i] = 2;
              ErrorLookup[PNOCOVERLV].Config2[i] = 2;
            }
          ErrorLookup[PNOCOVERLV].Config1[C_POFE]  = 1;
          ErrorLookup[PNOCOVERLV].Config1[C_FMPF] = 1;
          ErrorLookup[PNOCOVERLV].Config2[C_DILI]  = 1;
          ErrorLookup[PNOCOVERLV].Config2[C_LINE]  = 1;
          ErrorLookup[PNOCOVERLV].Config2[C_FMLF] = 1;
	  
          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[PNOCOVERLV].Domain1[i] = 0;
              ErrorLookup[PNOCOVERLV].Domain2[i] = 0;
            }
          ErrorLookup[PNOCOVERLV].Domain1[6] = 1;
          ErrorLookup[PNOCOVERLV].Domain2[6] = 1;
        }
      break;
      
      
      
    case PLLPROXFAIL:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
	  
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
	  
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 0;
	  
          for(i=0; i<NUM_D; i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
              CloneErrorLookup[Cloneindex].Domain3[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[6] = 1;
          CloneErrorLookup[Cloneindex].Domain2[6] = 1;
          CloneErrorLookup[Cloneindex].Domain3[7] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PLLPROXFAIL].Config1[i] = 2;
              ErrorLookup[PLLPROXFAIL].Config2[i] = 2;
              ErrorLookup[PLLPROXFAIL].Config3[i] = 2;
            }
          ErrorLookup[PLLPROXFAIL].Config1[C_POFE]  = 1;
          ErrorLookup[PLLPROXFAIL].Config1[C_FMPF] = 1;
	  
          ErrorLookup[PLLPROXFAIL].Config2[C_DILI]  = 1;
          ErrorLookup[PLLPROXFAIL].Config2[C_LINE]  = 1;
          ErrorLookup[PLLPROXFAIL].Config2[C_FMLF] = 1;
          ErrorLookup[PLLPROXFAIL].Config2[C_AREA]  = 0;
          ErrorLookup[PLLPROXFAIL].Config2[C_FMAF] = 0;
	  
          ErrorLookup[PLLPROXFAIL].Config3[C_DILI]  = 1;
          ErrorLookup[PLLPROXFAIL].Config3[C_LINE]  = 1;
          ErrorLookup[PLLPROXFAIL].Config3[C_FMLF] = 1;
          ErrorLookup[PLLPROXFAIL].Config3[C_AREA]  = 0;
          ErrorLookup[PLLPROXFAIL].Config3[C_FMAF] = 0;
	  
          for(i=0; i<NUM_D; i++)
            {
              ErrorLookup[PLLPROXFAIL].Domain1[i] = 0;
              ErrorLookup[PLLPROXFAIL].Domain2[i] = 0;
              ErrorLookup[PLLPROXFAIL].Domain3[i] = 0;
            }
          ErrorLookup[PLLPROXFAIL].Domain1[6] = 1;
          ErrorLookup[PLLPROXFAIL].Domain2[6] = 1;
          ErrorLookup[PLLPROXFAIL].Domain3[7] = 1;
        }
      break;
      
      
    case FAILMERGEL:  /** line object that should be merged with connecting line ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_POFE]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FAILMERGEL].Config1[i] = 2;
              ErrorLookup[FAILMERGEL].Config2[i] = 2;
            }
          ErrorLookup[FAILMERGEL].Config1[C_LINE]  = 1;
          ErrorLookup[FAILMERGEL].Config1[C_FMLF] = 1;
          ErrorLookup[FAILMERGEL].Config1[C_DILI]  = 1;

          ErrorLookup[FAILMERGEL].Config2[C_LINE]  = 1;
          ErrorLookup[FAILMERGEL].Config2[C_FMLF] = 1;
          ErrorLookup[FAILMERGEL].Config2[C_DILI]  = 1;

          ErrorLookup[FAILMERGEL].Config2[C_AREA]  = 0;
          ErrorLookup[FAILMERGEL].Config2[C_FMAF] = 0;
          ErrorLookup[FAILMERGEL].Config2[C_POFE]  = 0;
          ErrorLookup[FAILMERGEL].Config2[C_FMPF]  = 0;
        }
      break;

    case FAILMERGEL2:  /** line object that should be merged with connecting line no accounting for metadata  ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_POFE]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FAILMERGEL2].Config1[i] = 2;
              ErrorLookup[FAILMERGEL2].Config2[i] = 2;
            }
          ErrorLookup[FAILMERGEL2].Config1[C_LINE]  = 1;
          ErrorLookup[FAILMERGEL2].Config1[C_FMLF] = 1;
          ErrorLookup[FAILMERGEL2].Config1[C_DILI]  = 1;

          ErrorLookup[FAILMERGEL2].Config2[C_LINE]  = 1;
          ErrorLookup[FAILMERGEL2].Config2[C_FMLF] = 1;
          ErrorLookup[FAILMERGEL2].Config2[C_DILI]  = 1;

          ErrorLookup[FAILMERGEL2].Config2[C_AREA]  = 0;
          ErrorLookup[FAILMERGEL2].Config2[C_FMAF] = 0;
          ErrorLookup[FAILMERGEL2].Config2[C_POFE]  = 0;
          ErrorLookup[FAILMERGEL2].Config2[C_FMPF]  = 0;
        }
      break;

    case FAILMERGEA:  /** area feature that should be merged with area that shares edge ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FAILMERGEA].Config1[i] = 2;
            }
          ErrorLookup[FAILMERGEA].Config1[C_AREA]  = 1;
          ErrorLookup[FAILMERGEA].Config1[C_FMAF] = 1;
        }
      break;

    case FAILMERGEA2:  /** area feature that should be merged with area that shares edge - no accounting for metadata  ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FAILMERGEA2].Config1[i] = 2;
            }
          ErrorLookup[FAILMERGEA2].Config1[C_AREA]  = 1;
          ErrorLookup[FAILMERGEA2].Config1[C_FMAF] = 1;
        }
      break;

      
    case KICKBACK:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1; 
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1; 
          if(NGA_TYPE == 0)
	    CloneErrorLookup[Cloneindex].Config2[C_POLY]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[KICKBACK].Config1[i] = 2;
              ErrorLookup[KICKBACK].Config2[i] = 2;
            }
          ErrorLookup[KICKBACK].Config1[C_LINE]  = 1;
          ErrorLookup[KICKBACK].Config1[C_FMLF] = 1;
          ErrorLookup[KICKBACK].Config1[C_DILI]  = 1;
          if(NGA_TYPE == 0)
	    ErrorLookup[KICKBACK].Config2[C_POLY]  = 1;
        }
      break;


    case ISOTURN: /** high turn angle w/o 3d feature present ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 1;
            }
          CloneErrorLookup[Cloneindex].Config3[C_GRID]  = 2;

          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ISOTURN].Config1[i] = 2;
              ErrorLookup[ISOTURN].Config2[i] = 2;
              ErrorLookup[ISOTURN].Config3[i] = 1;
            }
          ErrorLookup[ISOTURN].Config3[C_GRID] = 2;

          ErrorLookup[ISOTURN].Config1[C_LINE]  = 1;
          ErrorLookup[ISOTURN].Config1[C_FMLF] = 1;
          ErrorLookup[ISOTURN].Config1[C_DILI]  = 1;
          ErrorLookup[ISOTURN].Config2[C_LINE]  = 1;
          ErrorLookup[ISOTURN].Config2[C_FMLF] = 1;
          ErrorLookup[ISOTURN].Config2[C_DILI]  = 1;
        }
      break;

      
    case KINK:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          if(NGA_TYPE == 0)
	    CloneErrorLookup[Cloneindex].Config2[C_POLY]  = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[KINK].Config1[i] = 2;
	      ErrorLookup[KINK].Config2[i] = 2;
	    }
	  ErrorLookup[KINK].Config1[C_LINE]  = 1;
	  ErrorLookup[KINK].Config1[C_FMLF] = 1;
	  ErrorLookup[KINK].Config1[C_DILI]  = 1;
          if(NGA_TYPE == 0)
	    ErrorLookup[KINK].Config2[C_POLY]  = 1;
	}
      break;

    case Z_KINK: /** consecutive kinks form a 'Z' ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Z_KINK].Config1[i] = 2;
              ErrorLookup[Z_KINK].Config2[i] = 2;
            }
          ErrorLookup[Z_KINK].Config1[C_LINE]  = 1;
          ErrorLookup[Z_KINK].Config1[C_FMLF] = 1;
          ErrorLookup[Z_KINK].Config1[C_DILI]  = 1;
        }
      break;

    case L_A_KINK: /** kink at intersection of line end node  and area feature perim **/ 
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[L_A_KINK].Config1[i] = 2;
              ErrorLookup[L_A_KINK].Config2[i] = 2;
            }
          ErrorLookup[L_A_KINK].Config1[C_LINE]  = 1;
          ErrorLookup[L_A_KINK].Config1[C_FMLF] = 1;
          ErrorLookup[L_A_KINK].Config1[C_DILI]  = 1;
          ErrorLookup[L_A_KINK].Config2[C_FMAF] = 1;
          ErrorLookup[L_A_KINK].Config2[C_AREA]  = 1;
        }
      break;

    case INTERNALKINK:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          if(NGA_TYPE == 0)
	    CloneErrorLookup[Cloneindex].Config2[C_POLY]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[INTERNALKINK].Config1[i] = 2;
              ErrorLookup[INTERNALKINK].Config2[i] = 2;
            }
          ErrorLookup[INTERNALKINK].Config1[C_LINE]  = 1;
          ErrorLookup[INTERNALKINK].Config1[C_FMLF] = 1;
          ErrorLookup[INTERNALKINK].Config1[C_DILI]  = 1;
          if(NGA_TYPE == 0)
	    ErrorLookup[INTERNALKINK].Config2[C_POLY]  = 1;
        }
      break;

    case CONTEXT_KINK:  /*** kink based on one high angle next to one lower (moderate) angle ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          if(NGA_TYPE == 0)
            CloneErrorLookup[Cloneindex].Config2[C_POLY]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[CONTEXT_KINK].Config1[i] = 2;
              ErrorLookup[CONTEXT_KINK].Config2[i] = 2;
            }
          ErrorLookup[CONTEXT_KINK].Config1[C_LINE]  = 1;
          ErrorLookup[CONTEXT_KINK].Config1[C_FMLF] = 1;
          ErrorLookup[CONTEXT_KINK].Config1[C_DILI]  = 1;
          if(NGA_TYPE == 0)
            ErrorLookup[CONTEXT_KINK].Config2[C_POLY]  = 1;
        }
      break;

      
    case AREAKINK: /** high angle on perimeter of area feature **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[AREAKINK].Config1[i] = 2;
              ErrorLookup[AREAKINK].Config2[i] = 2;
            }
          ErrorLookup[AREAKINK].Config1[C_AREA]  = 1;
          ErrorLookup[AREAKINK].Config1[C_FMAF] = 1;
        }
      break;
      
    case INCLSLIVER: /** areal with included sliver **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[INCLSLIVER].Config1[i] = 2;
              ErrorLookup[INCLSLIVER].Config2[i] = 2;
            }
          ErrorLookup[INCLSLIVER].Config1[C_AREA]  = 1;
          ErrorLookup[INCLSLIVER].Config1[C_FMAF] = 1;
        }
      break;
      

    case SLOPEDIRCH:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[SLOPEDIRCH].Config1[i] = 2;
            }
          ErrorLookup[SLOPEDIRCH].Config1[C_LINE] = 1;
          ErrorLookup[SLOPEDIRCH].Config1[C_FMLF]= 1;
          ErrorLookup[SLOPEDIRCH].Config1[C_DILI] = 1;
          ErrorLookup[SLOPEDIRCH].Config1[C_FMAF]= 1;
          ErrorLookup[SLOPEDIRCH].Config1[C_AREA] = 1;
        }
      break;



    case CLAMP_SDC: /*slope direction change along a line that has been elevation-value clamped to underlying DEM ***/
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
            CloneErrorLookup[Cloneindex].Config1[i] = 2;
            CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
	  CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
            ErrorLookup[CLAMP_SDC].Config1[i] = 2;
            ErrorLookup[CLAMP_SDC].Config2[i] = 2;
	    }
	  ErrorLookup[CLAMP_SDC].Config1[C_LINE] = 1;
	  ErrorLookup[CLAMP_SDC].Config1[C_FMLF]= 1;
	  ErrorLookup[CLAMP_SDC].Config1[C_DILI] = 1;
          ErrorLookup[CLAMP_SDC].Config1[C_FMAF]= 1;
          ErrorLookup[CLAMP_SDC].Config1[C_AREA] = 1;
          ErrorLookup[CLAMP_SDC].Config2[C_GRID] = 1;
	}
      break;
      
    case LJOINSLOPEDC:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LJOINSLOPEDC].Config1[i] = 2;
              ErrorLookup[LJOINSLOPEDC].Config2[i] = 2;
            }
          ErrorLookup[LJOINSLOPEDC].Config1[C_LINE] = 1;
          ErrorLookup[LJOINSLOPEDC].Config1[C_FMLF]= 1;
          ErrorLookup[LJOINSLOPEDC].Config1[C_DILI] = 1;
          ErrorLookup[LJOINSLOPEDC].Config2[C_LINE] = 1;
          ErrorLookup[LJOINSLOPEDC].Config2[C_FMLF]= 1;
          ErrorLookup[LJOINSLOPEDC].Config2[C_DILI] = 1;
        }
      break;

    case CLAMP_JOINSDC: /** slope direction change at line feature connection when both are clamped to DEM ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_GRID] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[CLAMP_JOINSDC].Config1[i] = 2;
              ErrorLookup[CLAMP_JOINSDC].Config2[i] = 2;
              ErrorLookup[CLAMP_JOINSDC].Config3[i] = 2;
            }
          ErrorLookup[CLAMP_JOINSDC].Config1[C_LINE] = 1;
          ErrorLookup[CLAMP_JOINSDC].Config1[C_FMLF]= 1;
          ErrorLookup[CLAMP_JOINSDC].Config1[C_DILI] = 1;
          ErrorLookup[CLAMP_JOINSDC].Config2[C_LINE] = 1;
          ErrorLookup[CLAMP_JOINSDC].Config2[C_FMLF]= 1;
          ErrorLookup[CLAMP_JOINSDC].Config2[C_DILI] = 1;
          ErrorLookup[CLAMP_JOINSDC].Config3[C_GRID] = 1;
        }
      break;

      
    case LOOPS:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LOOPS].Config1[i] = 2;
            }
          ErrorLookup[LOOPS].Config1[C_LINE] = 1;
          ErrorLookup[LOOPS].Config1[C_FMLF]= 1;
          ErrorLookup[LOOPS].Config1[C_DILI] = 1;
          ErrorLookup[LOOPS].Config1[C_AREA] = 1;
          ErrorLookup[LOOPS].Config1[C_FMAF]= 1;
        }
      break;


    case P_O_LOOP: /*** self-intersecting line that includes P & O formations using end nodes - lines only ****/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[P_O_LOOP].Config1[i] = 2;
            }
          ErrorLookup[P_O_LOOP].Config1[C_LINE] = 1;
          ErrorLookup[P_O_LOOP].Config1[C_FMLF]= 1;
          ErrorLookup[P_O_LOOP].Config1[C_DILI] = 1;
        }
      break;

      
      
      
    case ENDPTINT:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ENDPTINT].Config1[i] = 2;
            }
          ErrorLookup[ENDPTINT].Config1[C_LINE] = 1;
          ErrorLookup[ENDPTINT].Config1[C_FMLF]= 1;
          ErrorLookup[ENDPTINT].Config1[C_DILI] = 1;
        }
      break;

    case LATTRCHNG:  /** line end point connects to same fdcode line, but attributes differ between the 2 features **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LATTRCHNG].Config1[i] = 2;
            }
          ErrorLookup[LATTRCHNG].Config1[C_LINE] = 1;
          ErrorLookup[LATTRCHNG].Config1[C_FMLF]= 1;
          ErrorLookup[LATTRCHNG].Config1[C_DILI] = 1;
        }
      break;

    case PT_GRID_DIF: /** point and grid z value mismatch at exact coord, no interpolation **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_POFE] = 1;
          ErrorLookup[Check].Config2[C_GRID] = 1;
        }
      break;


    case RAISEDPC: /** number of raised shoreline points exceeds tolerance **/
    case FLOWSTEP:  /** step size in river flow above threshold ***/
    case BREAKLINE: /** river elevation change at bad angle with shorelines ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_GRID] = 1;
          ErrorLookup[Check].Config2[C_GRID] = 1;
        }
      break;

      
    case LOSMINHGT:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_GRID] = 1;
          ErrorLookup[Check].Config2[C_AREA] = 1;
        }
      break;


      
    case GSPIKE:
    case AVGSPIKE: /** spike / well as compared to average elevation of neighbor posts ***/
    case WATERMMU: /** minimum mapping unit for water body below threshold ***/
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
	    }
	  ErrorLookup[Check].Config1[C_GRID] = 1;
          ErrorLookup[Check].Config2[C_GRID] = 1;
	}
      break;


    case GRID_STD_DEV: /** grid elev value, inside feature polygon, over range offset from std deviation **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[GRID_STD_DEV].Config1[i] = 2;
              ErrorLookup[GRID_STD_DEV].Config2[i] = 2;
            }
          ErrorLookup[GRID_STD_DEV].Config1[C_GRID] = 1;
          ErrorLookup[GRID_STD_DEV].Config2[C_AREA] = 1;
        }
      break;

    case GSHELF:  /** looking for shelf formations like PUE in DEM ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[GSHELF].Config1[i] = 2;
              ErrorLookup[GSHELF].Config2[i] = 2;
              ErrorLookup[GSHELF].Config3[i] = 2;
            }

          ErrorLookup[GSHELF].Config1[C_GRID] = 1;
          ErrorLookup[GSHELF].Config2[C_GRID] = 1;
        }
      break;
      
      
    case ELEVGT:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
	}
      else
	{
	  ErrorLookup[ELEVGT].Config1[C_GRID] = 1;
	}
      break;
      
      
      
      
    case ELEVLT:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
	}
      else
	{
	  ErrorLookup[ELEVLT].Config1[C_GRID] = 1;
	}
      break;

      
    case ELEVEQ:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
	}
      else
	{
	  ErrorLookup[ELEVEQ].Config1[C_GRID] = 1;
	}
      break;

    case ELEVEQOPEN:  /** elevation in range, open interval**/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
        }
      else
        {
          ErrorLookup[ELEVEQOPEN].Config1[C_GRID] = 1;
        }
      break;

      
      
      
    case LAINT:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
	  CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF]= 1; 
	  for(i=0;i<NUM_D;i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[7]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[Check].Config1[i] = 2;
	      ErrorLookup[Check].Config2[i] = 2;
	    }
	  ErrorLookup[Check].Config1[C_DILI] = 1; 
	  ErrorLookup[Check].Config1[C_LINE] = 1; 
	  ErrorLookup[Check].Config1[C_FMLF]= 1;
	  ErrorLookup[Check].Config2[C_AREA] = 1; 
	  ErrorLookup[Check].Config2[C_FMAF]= 1; 
	  for(i=0;i<NUM_D;i++)
	    {
	      ErrorLookup[Check].Domain1[i] = 0;
	      ErrorLookup[Check].Domain2[i] = 0;
	    }
	  ErrorLookup[Check].Domain1[7]  = 1;
	  ErrorLookup[Check].Domain2[6]  = 1;
	  ErrorLookup[Check].Domain2[10] = 1;
	}
      break;
      
      
    case LOUTSIDEA:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
	  CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF]= 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[5] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[LOUTSIDEA].Config1[i] = 2;
	      ErrorLookup[LOUTSIDEA].Config2[i] = 2;
	    }
	  ErrorLookup[LOUTSIDEA].Config1[C_DILI] = 1; 
	  ErrorLookup[LOUTSIDEA].Config1[C_LINE] = 1; 
	  ErrorLookup[LOUTSIDEA].Config1[C_FMLF]= 1;
	  ErrorLookup[LOUTSIDEA].Config2[C_AREA] = 1; 
	  ErrorLookup[LOUTSIDEA].Config2[C_FMAF]= 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      ErrorLookup[LOUTSIDEA].Domain1[i] = 0;
	      ErrorLookup[LOUTSIDEA].Domain2[i] = 0;
	    }
	  ErrorLookup[LOUTSIDEA].Domain1[5] = 1;
	  ErrorLookup[LOUTSIDEA].Domain2[5] = 1;
	}
      break;
      
    case LLAINT:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF]= 1;

          for(i=0;i<NUM_D;i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
              CloneErrorLookup[Cloneindex].Domain3[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[5] = 1;
          CloneErrorLookup[Cloneindex].Domain2[5] = 1;
          CloneErrorLookup[Cloneindex].Domain3[5] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LLAINT].Config1[i] = 2;
              ErrorLookup[LLAINT].Config2[i] = 2;
              ErrorLookup[LLAINT].Config3[i] = 2;
            }
          ErrorLookup[LLAINT].Config1[C_DILI] = 1;
          ErrorLookup[LLAINT].Config1[C_LINE] = 1;
          ErrorLookup[LLAINT].Config1[C_FMLF]= 1;
          ErrorLookup[LLAINT].Config1[C_POFE]  = 1;
          ErrorLookup[LLAINT].Config1[C_FMPF] = 1;
          ErrorLookup[LLAINT].Config2[C_DILI] = 1; 
          ErrorLookup[LLAINT].Config2[C_LINE] = 1; 
          ErrorLookup[LLAINT].Config2[C_FMLF]= 1;
          ErrorLookup[LLAINT].Config3[C_AREA] = 1;
          ErrorLookup[LLAINT].Config3[C_FMAF]= 1;

          for(i=0;i<NUM_D;i++)
            {
              ErrorLookup[LLAINT].Domain1[i] = 0;
              ErrorLookup[LLAINT].Domain2[i] = 0;
              ErrorLookup[LLAINT].Domain3[i] = 0;
            }
          ErrorLookup[LLAINT].Domain1[5] = 1;
          ErrorLookup[LLAINT].Domain2[5] = 1;
          ErrorLookup[LLAINT].Domain3[5] = 1;
        }
      break;

    case L_NOTL_AINT:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF]= 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF]= 1;

          for(i=0;i<NUM_D;i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
              CloneErrorLookup[Cloneindex].Domain3[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[5] = 1;
          CloneErrorLookup[Cloneindex].Domain2[5] = 1;
          CloneErrorLookup[Cloneindex].Domain3[5] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[L_NOTL_AINT].Config1[i] = 2; 
              ErrorLookup[L_NOTL_AINT].Config2[i] = 2; 
              ErrorLookup[L_NOTL_AINT].Config3[i] = 2;
            }
          ErrorLookup[L_NOTL_AINT].Config1[C_DILI] = 1; 
          ErrorLookup[L_NOTL_AINT].Config1[C_LINE] = 1; 
          ErrorLookup[L_NOTL_AINT].Config1[C_FMLF]= 1; 
          ErrorLookup[L_NOTL_AINT].Config2[C_DILI] = 1;
          ErrorLookup[L_NOTL_AINT].Config2[C_LINE] = 1;
          ErrorLookup[L_NOTL_AINT].Config2[C_FMLF]= 1;
          ErrorLookup[L_NOTL_AINT].Config2[C_AREA] = 1;
          ErrorLookup[L_NOTL_AINT].Config2[C_FMAF]= 1;
          ErrorLookup[L_NOTL_AINT].Config3[C_AREA] = 1; 
          ErrorLookup[L_NOTL_AINT].Config3[C_FMAF]= 1; 

          for(i=0;i<NUM_D;i++)
            {
              ErrorLookup[L_NOTL_AINT].Domain1[i] = 0; 
              ErrorLookup[L_NOTL_AINT].Domain2[i] = 0; 
              ErrorLookup[L_NOTL_AINT].Domain3[i] = 0;
            }
          ErrorLookup[L_NOTL_AINT].Domain1[5] = 1;
          ErrorLookup[L_NOTL_AINT].Domain2[5] = 1;
          ErrorLookup[L_NOTL_AINT].Domain3[5] = 1;
        }
      break;


      
    case POLYINAREA:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI]= 0;
	  CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF]= 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[5]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[POLYINAREA].Config1[i] = 2;
	      ErrorLookup[POLYINAREA].Config2[i] = 2;
	    }
	  ErrorLookup[POLYINAREA].Config1[C_POLY] = 1; 
	  ErrorLookup[POLYINAREA].Config1[C_MOLI]= 0;
	  ErrorLookup[POLYINAREA].Config2[C_AREA] = 1; 
	  ErrorLookup[POLYINAREA].Config2[C_FMAF]= 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      ErrorLookup[POLYINAREA].Domain1[i] = 0;
	      ErrorLookup[POLYINAREA].Domain2[i] = 0;
	    }
	  ErrorLookup[POLYINAREA].Domain1[5]  = 1;
	  ErrorLookup[POLYINAREA].Domain2[6]  = 1;
	  ErrorLookup[POLYINAREA].Domain2[10] = 1;
	}
      break;
      
      
    case POLYOSIDEAREA:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POLY]  = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	  CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[5] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[POLYOSIDEAREA].Config1[i] = 2;
	      ErrorLookup[POLYOSIDEAREA].Config2[i] = 2;
	    }
	  ErrorLookup[POLYOSIDEAREA].Config1[C_POLY]  = 1; 
	  ErrorLookup[POLYOSIDEAREA].Config1[C_MOLI] = 0;
	  ErrorLookup[POLYOSIDEAREA].Config2[C_AREA]  = 1; 
	  ErrorLookup[POLYOSIDEAREA].Config2[C_FMAF] = 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      ErrorLookup[POLYOSIDEAREA].Domain1[i] = 0;
	      ErrorLookup[POLYOSIDEAREA].Domain2[i] = 0;
	    }
	  ErrorLookup[POLYOSIDEAREA].Domain1[5] = 1;
	  ErrorLookup[POLYOSIDEAREA].Domain2[5] = 1;
	}
      break;
      

    case PTINPROPER:  /** point feature inside an area feature - not within tolerance of edge (or edge or hole) **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(i=0;i<NUM_D;i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[5] = 1;
          CloneErrorLookup[Cloneindex].Domain2[6] = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
          CloneErrorLookup[Cloneindex].Domain2[12] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PTINPROPER].Config1[i] = 2;
              ErrorLookup[PTINPROPER].Config2[i] = 2;
            }
          ErrorLookup[PTINPROPER].Config1[C_POFE]  = 1;
          ErrorLookup[PTINPROPER].Config1[C_FMPF] = 1;
          ErrorLookup[PTINPROPER].Config2[C_AREA]  = 1;
          ErrorLookup[PTINPROPER].Config2[C_FMAF] = 1;

          for(i=0;i<NUM_D;i++)
            {
              ErrorLookup[PTINPROPER].Domain1[i] = 0;
              ErrorLookup[PTINPROPER].Domain2[i] = 0;
            }
          ErrorLookup[PTINPROPER].Domain1[5] = 1;
          ErrorLookup[PTINPROPER].Domain2[6] = 1;
          ErrorLookup[PTINPROPER].Domain2[10] = 1;
          ErrorLookup[PTINPROPER].Domain2[12] = 1;
        }
      break;


      
    case PTINREGION:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_POLY]  = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_MOLI] = 1;  
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[5] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[12] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[PTINREGION].Config1[i] = 2;
	      ErrorLookup[PTINREGION].Config2[i] = 2;
	    }
	  ErrorLookup[PTINREGION].Config1[C_POFE]  = 1; 
	  ErrorLookup[PTINREGION].Config1[C_FMPF] = 1; 
	  ErrorLookup[PTINREGION].Config2[C_AREA]  = 1; 
	  ErrorLookup[PTINREGION].Config2[C_FMAF] = 1; 
	  ErrorLookup[PTINREGION].Config2[C_POLY]  = 1; 
	  ErrorLookup[PTINREGION].Config2[C_MOLI] = 1;  
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      ErrorLookup[PTINREGION].Domain1[i] = 0;
	      ErrorLookup[PTINREGION].Domain2[i] = 0;
	    }
	  ErrorLookup[PTINREGION].Domain1[5] = 1;
	  ErrorLookup[PTINREGION].Domain2[6] = 1;
	  ErrorLookup[PTINREGION].Domain2[10] = 1;
	  ErrorLookup[PTINREGION].Domain2[12] = 1;
	}
      break;
      
      
      
    case PTOSIDEREGION:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_POLY]  = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_MOLI] = 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[5] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[PTOSIDEREGION].Config1[i] = 2;
	      ErrorLookup[PTOSIDEREGION].Config2[i] = 2;
	    }
	  ErrorLookup[PTOSIDEREGION].Config1[C_POFE]  = 1; 
	  ErrorLookup[PTOSIDEREGION].Config1[C_FMPF] = 1; 
	  ErrorLookup[PTOSIDEREGION].Config2[C_AREA]  = 1; 
	  ErrorLookup[PTOSIDEREGION].Config2[C_FMAF] = 1; 
	  ErrorLookup[PTOSIDEREGION].Config2[C_POLY]  = 1; 
	  ErrorLookup[PTOSIDEREGION].Config2[C_MOLI] = 1; 
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      ErrorLookup[PTOSIDEREGION].Domain1[i] = 0;
	      ErrorLookup[PTOSIDEREGION].Domain2[i] = 0;
	    }
	  ErrorLookup[PTOSIDEREGION].Domain1[5] = 1;
	  ErrorLookup[PTOSIDEREGION].Domain2[5] = 1;
	}
      break;
      
      
      
    case PTPTPROX:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  
	  CloneErrorLookup[Cloneindex].Config1[C_POMO] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FMPF]= 1;
	  CloneErrorLookup[Cloneindex].Config2[C_POMO] = 0;
	  CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FMPF]= 1;
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[12] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[12] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[PTPTPROX].Config1[i] = 2;
	      ErrorLookup[PTPTPROX].Config2[i] = 2;
	    }
	  
	  ErrorLookup[PTPTPROX].Config1[C_POMO] = 0;
	  ErrorLookup[PTPTPROX].Config1[C_POFE] = 1;
	  ErrorLookup[PTPTPROX].Config1[C_FMPF]= 1;
	  ErrorLookup[PTPTPROX].Config2[C_POMO] = 0;
	  ErrorLookup[PTPTPROX].Config2[C_POFE] = 1;
	  ErrorLookup[PTPTPROX].Config2[C_FMPF]= 1;
	  
	  for(i=0;i<NUM_D;i++)
	    {
	      ErrorLookup[PTPTPROX].Domain1[i] = 0;
	      ErrorLookup[PTPTPROX].Domain2[i] = 0;
	    }
	  ErrorLookup[PTPTPROX].Domain1[12] = 1;
	  ErrorLookup[PTPTPROX].Domain2[12] = 1;
	}
      break;

    case PUNDERSHTA: /** point not on area perimeter and is outside that area feature **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }

          CloneErrorLookup[Cloneindex].Config1[C_POMO] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF]= 1;

          for(i=0;i<NUM_D;i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[12] = 1;
          CloneErrorLookup[Cloneindex].Domain2[12] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PUNDERSHTA].Config1[i] = 2;
              ErrorLookup[PUNDERSHTA].Config2[i] = 2;
            }

          ErrorLookup[PUNDERSHTA].Config1[C_POMO] = 0;
          ErrorLookup[PUNDERSHTA].Config1[C_POFE] = 1;
          ErrorLookup[PUNDERSHTA].Config1[C_FMPF]= 1;
          ErrorLookup[PUNDERSHTA].Config2[C_AREA] = 1;
          ErrorLookup[PUNDERSHTA].Config2[C_FMAF]= 1;

          for(i=0;i<NUM_D;i++)
            {
              ErrorLookup[PUNDERSHTA].Domain1[i] = 0;
              ErrorLookup[PUNDERSHTA].Domain2[i] = 0;
            }
          ErrorLookup[PUNDERSHTA].Domain1[12] = 1;
          ErrorLookup[PUNDERSHTA].Domain2[12] = 1;
        }
      break;

    case POVERSHTA: /** point not on area perimeter and is inside that area feature **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }

          CloneErrorLookup[Cloneindex].Config1[C_POMO] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]= 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF]= 1;

          for(i=0;i<NUM_D;i++)
            {
              CloneErrorLookup[Cloneindex].Domain1[i] = 0;
              CloneErrorLookup[Cloneindex].Domain2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[12] = 1;
          CloneErrorLookup[Cloneindex].Domain2[12] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[POVERSHTA].Config1[i] = 2;
              ErrorLookup[POVERSHTA].Config2[i] = 2;
            }

          ErrorLookup[POVERSHTA].Config1[C_POMO] = 0;
          ErrorLookup[POVERSHTA].Config1[C_POFE] = 1;
          ErrorLookup[POVERSHTA].Config1[C_FMPF]= 1;
          ErrorLookup[POVERSHTA].Config2[C_AREA] = 1;
          ErrorLookup[POVERSHTA].Config2[C_FMAF]= 1;

          for(i=0;i<NUM_D;i++)
            {
              ErrorLookup[POVERSHTA].Domain1[i] = 0;
              ErrorLookup[POVERSHTA].Domain2[i] = 0;
            }
          ErrorLookup[POVERSHTA].Domain1[12] = 1;
          ErrorLookup[POVERSHTA].Domain2[12] = 1;
        }
      break;
      
      
      
    case POLYINTPOLY:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FOMO] = 1;
	  
	  CloneErrorLookup[Cloneindex].Config2[C_MOLI] = 0;
	  CloneErrorLookup[Cloneindex].Config2[C_POLY] = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FOMO] = 0;
	  for(i=0; i<NUM_D; i++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[i] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[i] = 0;
	    }
	  
	  CloneErrorLookup[Cloneindex].Domain1[5]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[12] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[POLYINTPOLY].Config1[i] = 2;
	      ErrorLookup[POLYINTPOLY].Config2[i] = 2;
	    }
	  ErrorLookup[POLYINTPOLY].Config1[C_MOLI] = 1;
	  ErrorLookup[POLYINTPOLY].Config1[C_POLY] = 1;
	  ErrorLookup[POLYINTPOLY].Config1[C_FOMO] = 1;
	  
	  ErrorLookup[POLYINTPOLY].Config2[C_MOLI] = 0;
	  ErrorLookup[POLYINTPOLY].Config2[C_POLY] = 1;
	  ErrorLookup[POLYINTPOLY].Config2[C_FOMO] = 0;
	  for(i=0; i<NUM_D; i++)
	    {
	      ErrorLookup[POLYINTPOLY].Domain1[i] = 0;
	      ErrorLookup[POLYINTPOLY].Domain2[i] = 0;
	    }
	  
	  ErrorLookup[POLYINTPOLY].Domain1[5]  = 1;
	  ErrorLookup[POLYINTPOLY].Domain2[6]  = 1;
	  ErrorLookup[POLYINTPOLY].Domain2[10] = 1;
	  ErrorLookup[POLYINTPOLY].Domain2[12] = 1;
	}
      break;
      
      
    case POLYINTAREA:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_FOMO]  = 1;
	  
	  CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
	  
	  for(j=1;j<NUM_D;j++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[j] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[j] = 0;
	    }
	  
	  CloneErrorLookup[Cloneindex].Domain1[5]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[12] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[POLYINTAREA].Config1[i] = 2;
	      ErrorLookup[POLYINTAREA].Config2[i] = 2;
	    }
	  ErrorLookup[POLYINTAREA].Config1[C_MOLI] = 1;
	  ErrorLookup[POLYINTAREA].Config1[C_POLY]  = 1;
	  ErrorLookup[POLYINTAREA].Config1[C_FOMO]  = 1;
	  
	  ErrorLookup[POLYINTAREA].Config2[C_AREA]  = 1;
	  ErrorLookup[POLYINTAREA].Config2[C_FMAF] = 1;
	  
	  for(j=1;j<NUM_D;j++)
	    {
	      ErrorLookup[POLYINTAREA].Domain1[j] = 0;
	      ErrorLookup[POLYINTAREA].Domain2[j] = 0;
	    }
	  
	  ErrorLookup[POLYINTAREA].Domain1[5]  = 1;
	  ErrorLookup[POLYINTAREA].Domain2[6]  = 1;
	  ErrorLookup[POLYINTAREA].Domain2[10] = 1;
	  ErrorLookup[POLYINTAREA].Domain2[12] = 1;
	}
      break;
      
      
    case FEATSPIKE: /** elevation spike along 3D feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FEATSPIKE].Config1[i] = 2;
            }
          ErrorLookup[FEATSPIKE].Config1[C_AREA]  = 1;
          ErrorLookup[FEATSPIKE].Config1[C_DILI]  = 0;
          ErrorLookup[FEATSPIKE].Config1[C_LINE]  = 0;
          ErrorLookup[FEATSPIKE].Config1[C_FMLF] = 0;
          ErrorLookup[FEATSPIKE].Config1[C_FMAF] = 0;
        }
      break;

      
    case ELEVADJCHANGE:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_POLY]  = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
	  CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[ELEVADJCHANGE].Config1[i] = 2;
	    }
	  ErrorLookup[ELEVADJCHANGE].Config1[C_AREA]  = 1;
	  ErrorLookup[ELEVADJCHANGE].Config1[C_DILI]  = 0;
	  ErrorLookup[ELEVADJCHANGE].Config1[C_LINE]  = 0;
	  ErrorLookup[ELEVADJCHANGE].Config1[C_POLY]  = 0;
	  ErrorLookup[ELEVADJCHANGE].Config1[C_MOLI] = 0;
	  ErrorLookup[ELEVADJCHANGE].Config1[C_FMLF] = 0;
	  ErrorLookup[ELEVADJCHANGE].Config1[C_FMAF] = 0;
	}
      break;
      
      
    case MASKEDIT_0: /** Raw DEM and Edited DEM different & Edit Mask has value zero**/
    case MASKEDIT_1: /** EDM has primary tolerance value, diff between TDR and TDF is > secondary tolerance **/
    case MASKMONO: /** DEM not monotonic at point defined by specified mask value ***/
    case KERNELSTATS: /** just write DEM comparison statistics - no conditions produced ***/
    case BILINSTATS: /** just write DEM comparison statistics - no conditions produced ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_GRID] = 1;
          for(i=0 ; i < NUM_S; i++)
            {
              CloneErrorLookup[Cloneindex].Stratum1[i] = 0;
              CloneErrorLookup[Cloneindex].Stratum2[i] = 0;
              CloneErrorLookup[Cloneindex].Stratum3[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
          CloneErrorLookup[Cloneindex].Stratum2[5] = 1;
          CloneErrorLookup[Cloneindex].Stratum3[5] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_GRID] = 1;
          ErrorLookup[Check].Config2[C_GRID] = 1;
          ErrorLookup[Check].Config3[C_GRID] = 1;

          for(i=0 ; i < NUM_S; i++)
            {
              ErrorLookup[Check].Stratum1[i] = 0;
              ErrorLookup[Check].Stratum2[i] = 0;
              ErrorLookup[Check].Stratum3[i] = 0;
            }
          ErrorLookup[Check].Stratum1[5] = 1;
          ErrorLookup[Check].Stratum2[5] = 1;
          ErrorLookup[Check].Stratum3[5] = 1;
        }
      break;


      
    case MASKZERO: /** DEM not zero elev at point defined by specified mask value ***/
    case MASKCONSTANT: /*** DEM not constant elev at pointdefined by specified mask value ***/
    case MASKSHOREL: /** water body not contained by shoreline ***/
    case MASKCONFLICT: /** Grid DEM Masks have conflicting values ***/
    case MASKCONF2: /** variation of Grids with conflicting values **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
          for(i=0 ; i < NUM_S; i++)
            {
              CloneErrorLookup[Cloneindex].Stratum1[i] = 0;
              CloneErrorLookup[Cloneindex].Stratum2[i] = 0;
            }
          CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
          CloneErrorLookup[Cloneindex].Stratum2[5] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_GRID] = 1;
          ErrorLookup[Check].Config2[C_GRID] = 1;
          for(i=0 ; i < NUM_S; i++)
            {
              ErrorLookup[Check].Stratum1[i] = 0;
              ErrorLookup[Check].Stratum2[i] = 0;
            }
          ErrorLookup[Check].Stratum1[5] = 1;
          ErrorLookup[Check].Stratum2[5] = 1;
        }
      break;

    case GRIDEXACTDIF: /** Grids have post value difference at same X,Y ***/
    case LODELEVDIF:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_GRID] = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_POLY] = 1;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      CloneErrorLookup[Cloneindex].Stratum1[i] = 0;
              CloneErrorLookup[Cloneindex].Stratum2[i] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Stratum1[5] = 1; 
          CloneErrorLookup[Cloneindex].Stratum2[5] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
	    }
	  ErrorLookup[Check].Config1[C_GRID] = 1; 
	  ErrorLookup[Check].Config1[C_POLY] = 1;
          ErrorLookup[Check].Config2[C_GRID] = 1;
          ErrorLookup[Check].Config2[C_POLY] = 1;
	  for(i=0 ; i < NUM_S; i++)
	    {
	      ErrorLookup[Check].Stratum1[i] = 0;
              ErrorLookup[Check].Stratum2[i] = 0;
	    }
	  ErrorLookup[Check].Stratum1[5] = 1; 
          ErrorLookup[Check].Stratum2[5] = 1;
	}
      break;



    case CLAMP_SEG: /*** catenary segment below associated DEM ****/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
              CloneErrorLookup[Cloneindex].Config3[j] = 2;
            }
           CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;

           CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[CLAMP_SEG].Config1[j] = 2;
              ErrorLookup[CLAMP_SEG].Config2[j] = 2;
              ErrorLookup[CLAMP_SEG].Config3[j] = 2;
            }
           ErrorLookup[CLAMP_SEG].Config1[C_AREA] = 1;
           ErrorLookup[CLAMP_SEG].Config1[C_LINE] = 1;

           ErrorLookup[CLAMP_SEG].Config2[C_GRID] = 1;
        }

       break;
      
      
    case OBJECTWITHOUT:
      if(Clone==1)
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[j] = 2;
	      CloneErrorLookup[Cloneindex].Config2[j] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1; 
	  CloneErrorLookup[Cloneindex].Config1[C_AREA] = 0; 
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;
	  
	  CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1; 
	  CloneErrorLookup[Cloneindex].Config2[C_DILI] = 0; 
	  CloneErrorLookup[Cloneindex].Config2[C_POFE] = 0; 
	  CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
	  CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
	  CloneErrorLookup[Cloneindex].Stratum2[1] = 1;
	  for(j=1; j<NUM_D; j++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[j] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[j] = 0;
	    }
	  CloneErrorLookup[Cloneindex].Domain1[4] = 1;
	  CloneErrorLookup[Cloneindex].Domain2[7] = 1;
	}
      else
	{
	  for(j=1; j<NUM_C; j++)
	    {
	      ErrorLookup[Check].Config1[j] = 2;
	      ErrorLookup[Check].Config2[j] = 2;
	    }
	  ErrorLookup[Check].Config1[C_POLY] = 1; 
	  ErrorLookup[Check].Config1[C_AREA] = 0; 
          ErrorLookup[Check].Config1[C_FMAF] = 1;
          ErrorLookup[Check].Config1[C_MOLI] = 0;
	  
	  ErrorLookup[Check].Config2[C_LINE] = 1; 
	  ErrorLookup[Check].Config2[C_DILI] = 0; 
	  ErrorLookup[Check].Config2[C_POFE] = 0; 
	  ErrorLookup[Check].Config2[C_FMPF] = 0;
          ErrorLookup[Check].Config2[C_AREA] = 0;
          ErrorLookup[Check].Config2[C_FMAF] = 0;
          ErrorLookup[Check].Config2[C_FMLF] = 0;
	  ErrorLookup[Check].Stratum1[5] = 1;
	  ErrorLookup[Check].Stratum2[1] = 1;
	  for(j=1; j<NUM_D; j++)
	    {
	      ErrorLookup[Check].Domain1[j] = 0;
	      ErrorLookup[Check].Domain2[j] = 0;
	    }
	  ErrorLookup[Check].Domain1[4] = 1;
	  ErrorLookup[Check].Domain2[7] = 1;
	}
      break;

   case OBJ_WO_TWO: /** area contains secondary P,A,L but not tertiary P,A,L ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
              CloneErrorLookup[Cloneindex].Config3[j] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POLY] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_MOLI] = 0;

          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;

          CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_POFE] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_FMPF] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 0;

          CloneErrorLookup[Cloneindex].Stratum1[5] = 1;
          CloneErrorLookup[Cloneindex].Stratum2[1] = 1;
          CloneErrorLookup[Cloneindex].Stratum3[1] = 1;

          for(j=1; j<NUM_D; j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[4] = 1;
          CloneErrorLookup[Cloneindex].Domain2[7] = 1;
          CloneErrorLookup[Cloneindex].Domain3[7] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[Check].Config1[j] = 2;
              ErrorLookup[Check].Config2[j] = 2;
              ErrorLookup[Check].Config3[j] = 2;
            }
          ErrorLookup[Check].Config1[C_POLY] = 1;
          ErrorLookup[Check].Config1[C_AREA] = 0;
          ErrorLookup[Check].Config1[C_FMAF] = 1;
          ErrorLookup[Check].Config1[C_MOLI] = 0;

          ErrorLookup[Check].Config2[C_LINE] = 1;
          ErrorLookup[Check].Config2[C_DILI] = 0;
          ErrorLookup[Check].Config2[C_POFE] = 0;
          ErrorLookup[Check].Config2[C_POFE] = 0;
          ErrorLookup[Check].Config2[C_FMPF] = 0;
          ErrorLookup[Check].Config2[C_AREA] = 0;
          ErrorLookup[Check].Config2[C_FMAF] = 0;
          ErrorLookup[Check].Config2[C_FMLF] = 0;

          ErrorLookup[Check].Config3[C_LINE] = 1;
          ErrorLookup[Check].Config3[C_DILI] = 0;
          ErrorLookup[Check].Config3[C_POFE] = 0;
          ErrorLookup[Check].Config3[C_POFE] = 0;
          ErrorLookup[Check].Config3[C_FMPF] = 0;
          ErrorLookup[Check].Config3[C_AREA] = 0;
          ErrorLookup[Check].Config3[C_FMAF] = 0;
          ErrorLookup[Check].Config3[C_FMLF] = 0;

          ErrorLookup[Check].Stratum1[5] = 1;
          ErrorLookup[Check].Stratum2[1] = 1;
          ErrorLookup[Check].Stratum3[1] = 1;

          for(j=1; j<NUM_D; j++)
            {
              ErrorLookup[Check].Domain1[j] = 0;
              ErrorLookup[Check].Domain2[j] = 0;
              ErrorLookup[Check].Domain3[j] = 0;
            }
          ErrorLookup[Check].Domain1[4] = 1;
          ErrorLookup[Check].Domain2[7] = 1;
          ErrorLookup[Check].Domain3[7] = 1;
        }
      break;


      

   case AWITHOUTA:  /** area that does not fully contain a second area ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;

          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_GRID] = 0;

          for(j=1; j<NUM_D; j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[4] = 1;
          CloneErrorLookup[Cloneindex].Domain2[7] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[AWITHOUTA].Config1[j] = 2;
              ErrorLookup[AWITHOUTA].Config2[j] = 2;
            }
          ErrorLookup[AWITHOUTA].Config1[C_AREA] = 1;
          ErrorLookup[AWITHOUTA].Config1[C_FMAF] = 0;

          ErrorLookup[AWITHOUTA].Config2[C_AREA] = 1;
          ErrorLookup[AWITHOUTA].Config2[C_FMAF] = 0;
          ErrorLookup[AWITHOUTA].Config2[C_DILI] = 0;
          ErrorLookup[AWITHOUTA].Config2[C_FMLF] = 0;
          ErrorLookup[AWITHOUTA].Config2[C_LINE] = 0;
          ErrorLookup[AWITHOUTA].Config2[C_POFE] = 0;
          ErrorLookup[AWITHOUTA].Config2[C_FMPF] = 0;
          ErrorLookup[AWITHOUTA].Config2[C_GRID] = 0;
          for(j=1; j<NUM_D; j++)
            {
              ErrorLookup[AWITHOUTA].Domain1[j] = 0;
              ErrorLookup[AWITHOUTA].Domain2[j] = 0;
            }
          ErrorLookup[AWITHOUTA].Domain1[4] = 1;
          ErrorLookup[AWITHOUTA].Domain2[7] = 1;
        }
      break;


    case FSFAIL: /*** face sharing failure ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;

          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
          for(j=1; j<NUM_D; j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
          CloneErrorLookup[Cloneindex].Domain1[4] = 1;
          CloneErrorLookup[Cloneindex].Domain2[7] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[FSFAIL].Config1[j] = 2;
              ErrorLookup[FSFAIL].Config2[j] = 2;
            }
          ErrorLookup[FSFAIL].Config1[C_AREA] = 1;
          ErrorLookup[FSFAIL].Config1[C_FMAF] = 0;

          ErrorLookup[FSFAIL].Config2[C_AREA] = 1;
          ErrorLookup[FSFAIL].Config2[C_FMAF] = 0;
          for(j=1; j<NUM_D; j++)
            {
              ErrorLookup[FSFAIL].Domain1[j] = 0;
              ErrorLookup[FSFAIL].Domain2[j] = 0;
            }
          ErrorLookup[FSFAIL].Domain1[4] = 1;
          ErrorLookup[FSFAIL].Domain2[7] = 1;
        }
      break;



    case PSHAREFAIL:  /*** an area feature fails to share any of its perimeter with a 2d area feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

          CloneErrorLookup[Cloneindex].Domain1[5]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[PSHAREFAIL].Config1[i] = 2;
              ErrorLookup[PSHAREFAIL].Config2[i] = 2;
              ErrorLookup[PSHAREFAIL].Config3[i] = 2;
            }
          ErrorLookup[PSHAREFAIL].Config1[C_FMAF] = 1;
          ErrorLookup[PSHAREFAIL].Config1[C_AREA]  = 1;

          ErrorLookup[PSHAREFAIL].Config2[C_AREA]  = 1;
          ErrorLookup[PSHAREFAIL].Config2[C_FMAF] = 1;

          ErrorLookup[PSHAREFAIL].Config3[C_AREA]  = 1;
          ErrorLookup[PSHAREFAIL].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[PSHAREFAIL].Domain1[j] = 0;
              ErrorLookup[PSHAREFAIL].Domain2[j] = 0;
            }

          ErrorLookup[PSHAREFAIL].Domain1[5]  = 1;
          ErrorLookup[PSHAREFAIL].Domain2[6]  = 1;
          ErrorLookup[PSHAREFAIL].Domain2[10] = 1;
        }
      break;

    case NOCOINCIDE: /** area without line end node or segment on its perimeter ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

          CloneErrorLookup[Cloneindex].Domain1[5]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[NOCOINCIDE].Config1[i] = 2;
              ErrorLookup[NOCOINCIDE].Config2[i] = 2;
            }
          ErrorLookup[NOCOINCIDE].Config1[C_FMAF] = 1;
          ErrorLookup[NOCOINCIDE].Config1[C_AREA]  = 1;

          ErrorLookup[NOCOINCIDE].Config2[C_DILI]  = 1;
          ErrorLookup[NOCOINCIDE].Config2[C_LINE]  = 1;
          ErrorLookup[NOCOINCIDE].Config2[C_FMLF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[NOCOINCIDE].Domain1[j] = 0;
              ErrorLookup[NOCOINCIDE].Domain2[j] = 0;
            }

          ErrorLookup[NOCOINCIDE].Domain1[5]  = 1;
          ErrorLookup[NOCOINCIDE].Domain2[6]  = 1;
          ErrorLookup[NOCOINCIDE].Domain2[10] = 1;
        }
      break;

    case PART_ISF: /** two area features have intersecting edges and share part of their faces **/
    case AREAINTAREA:
      if(Clone==1)
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      CloneErrorLookup[Cloneindex].Config1[i] = 2;
	      CloneErrorLookup[Cloneindex].Config2[i] = 2;
	    }
	  CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
	  CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
	  
	  CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
	  CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
	  
	  for(j=1;j<NUM_D;j++)
	    {
	      CloneErrorLookup[Cloneindex].Domain1[j] = 0;
	      CloneErrorLookup[Cloneindex].Domain2[j] = 0;
	    }
	  
	  CloneErrorLookup[Cloneindex].Domain1[5]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
	  CloneErrorLookup[Cloneindex].Domain2[10] = 1;
	}
      else
	{
	  for(i=0; i<NUM_C; i++)
	    {
	      ErrorLookup[Check].Config1[i] = 2;
	      ErrorLookup[Check].Config2[i] = 2;
	    }
	  ErrorLookup[Check].Config1[C_FMAF] = 1;
	  ErrorLookup[Check].Config1[C_AREA]  = 1;
	  
	  ErrorLookup[Check].Config2[C_AREA]  = 1;
	  ErrorLookup[Check].Config2[C_FMAF] = 1;
	  
	  for(j=1;j<NUM_D;j++)
	    {
	      ErrorLookup[Check].Domain1[j] = 0;
	      ErrorLookup[Check].Domain2[j] = 0;
	    }
	  
	  ErrorLookup[Check].Domain1[5]  = 1;
	  ErrorLookup[Check].Domain2[6]  = 1;
	  ErrorLookup[Check].Domain2[10] = 1;
	}
      break;
      
      
    case CUT_INT: /** cut-out intersects parent feature outer ring ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

          CloneErrorLookup[Cloneindex].Domain1[5]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[6]  = 1;
          CloneErrorLookup[Cloneindex].Domain2[10] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[CUT_INT].Config1[i] = 2;
              ErrorLookup[CUT_INT].Config2[i] = 2;
            }
          ErrorLookup[CUT_INT].Config1[C_FMAF] = 1;
          ErrorLookup[CUT_INT].Config1[C_AREA]  = 1;

          ErrorLookup[CUT_INT].Config2[C_AREA]  = 1;
          ErrorLookup[CUT_INT].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[CUT_INT].Domain1[j] = 0;
              ErrorLookup[CUT_INT].Domain2[j] = 0;
            }

          ErrorLookup[CUT_INT].Domain1[5]  = 1;
          ErrorLookup[CUT_INT].Domain2[6]  = 1;
          ErrorLookup[CUT_INT].Domain2[10] = 1;
        }
      break;


    case ACOVERA:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
	  
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1; 
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1; 
	  
          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ACOVERA].Config1[i] = 2;
              ErrorLookup[ACOVERA].Config2[i] = 2;
            }
          ErrorLookup[ACOVERA].Config1[C_FMAF] = 1;
          ErrorLookup[ACOVERA].Config1[C_AREA]  = 1;
	  
          ErrorLookup[ACOVERA].Config2[C_AREA]  = 1;
          ErrorLookup[ACOVERA].Config2[C_FMAF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[ACOVERA].Domain1[j] = 0;
              ErrorLookup[ACOVERA].Domain2[j] = 0;
            }
        }
      break;
      
    case AINSIDEHOLE: /** area inside another areal's cutout ('illegal holes') ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
	  
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[AINSIDEHOLE].Config1[i] = 2;
              ErrorLookup[AINSIDEHOLE].Config2[i] = 2;
            }
          ErrorLookup[AINSIDEHOLE].Config1[C_FMAF] = 1;
          ErrorLookup[AINSIDEHOLE].Config1[C_AREA]  = 1;
	  
          ErrorLookup[AINSIDEHOLE].Config2[C_AREA]  = 1;
          ErrorLookup[AINSIDEHOLE].Config2[C_FMAF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[AINSIDEHOLE].Domain1[j] = 0;
              ErrorLookup[AINSIDEHOLE].Domain2[j] = 0;
            }
        }
      break;
      
    case AOVERLAPA: /** overlapping area features (second can also be inside first) **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
	  
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[AOVERLAPA].Config1[i] = 2;
              ErrorLookup[AOVERLAPA].Config2[i] = 2;
              ErrorLookup[AOVERLAPA].Config3[i] = 2;
            }
          ErrorLookup[AOVERLAPA].Config1[C_FMAF] = 1;
          ErrorLookup[AOVERLAPA].Config1[C_AREA]  = 1;
	  
          ErrorLookup[AOVERLAPA].Config2[C_AREA]  = 1;
          ErrorLookup[AOVERLAPA].Config2[C_FMAF] = 1;

          ErrorLookup[AOVERLAPA].Config3[C_AREA]  = 1;
          ErrorLookup[AOVERLAPA].Config3[C_FMAF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[AOVERLAPA].Domain1[j] = 0;
              ErrorLookup[AOVERLAPA].Domain2[j] = 0;
              ErrorLookup[AOVERLAPA].Domain3[j] = 0;
            }
        }
      break;
      
      
    case CONF_STATS: /*** just generates conflation-information statistics, etc - no conditions to read/write  ****/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
         }
       break;
      
    case LSPANFAIL: /** line not covered by face of doesnt spand between edges ***/
    case LNOCOVERLA:
    case CONFLATE: /*** line is unique among conflation sets of data ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
	  
	  
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          if((Check == LNOCOVERLA) || (Check == CONFLATE))
             {
             CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
             CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
             CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;

             CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
             CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
             }
	  
          for(j=1;j<NUM_D;j++)
             {
             CloneErrorLookup[Cloneindex].Domain1[j] = 0;
             CloneErrorLookup[Cloneindex].Domain2[j] = 0;
             CloneErrorLookup[Cloneindex].Domain3[j] = 0;
             }
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
              ErrorLookup[Check].Config3[i] = 2;
            }
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
	  
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;
          ErrorLookup[Check].Config2[C_FMLF] = 1;
          if((Check == LNOCOVERLA) || (Check == CONFLATE))
             {
             ErrorLookup[Check].Config2[C_FMLF] = 1;
             ErrorLookup[Check].Config2[C_DILI]  = 1;
             ErrorLookup[Check].Config2[C_LINE]  = 1;

             ErrorLookup[Check].Config3[C_AREA]  = 1;
             ErrorLookup[Check].Config3[C_FMAF] = 1;
	     }

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[Check].Domain1[j] = 0;
              ErrorLookup[Check].Domain2[j] = 0;
              ErrorLookup[Check].Domain3[j] = 0;
            }
        }
      break;


    case LNOCOV2A:  /** line not covered by edges of 2 area features ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;


          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LNOCOV2A].Config1[i] = 2;
              ErrorLookup[LNOCOV2A].Config2[i] = 2;
            }
          ErrorLookup[LNOCOV2A].Config1[C_FMLF] = 1;
          ErrorLookup[LNOCOV2A].Config1[C_DILI]  = 1;
          ErrorLookup[LNOCOV2A].Config1[C_LINE]  = 1;

          ErrorLookup[LNOCOV2A].Config2[C_AREA]  = 1;
          ErrorLookup[LNOCOV2A].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LNOCOV2A].Domain1[j] = 0;
              ErrorLookup[LNOCOV2A].Domain2[j] = 0;
            }
        }
      break;


    case ISOLINE:  /** line feature completely inside an area feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_DILI] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_LINE] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_POFE] = 0;
          CloneErrorLookup[Cloneindex].Config3[C_FMPF] = 0;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ISOLINE].Config1[i] = 2;
              ErrorLookup[ISOLINE].Config2[i] = 2;
              ErrorLookup[ISOLINE].Config3[i] = 2;
            }
          ErrorLookup[ISOLINE].Config1[C_FMLF] = 1;
          ErrorLookup[ISOLINE].Config1[C_DILI]  = 1;
          ErrorLookup[ISOLINE].Config1[C_LINE]  = 1;

          ErrorLookup[ISOLINE].Config2[C_AREA]  = 1;
          ErrorLookup[ISOLINE].Config2[C_FMAF] = 1;

          ErrorLookup[ISOLINE].Config3[C_AREA] = 0;
          ErrorLookup[ISOLINE].Config3[C_FMAF] = 0;
          ErrorLookup[ISOLINE].Config3[C_DILI] = 0;
          ErrorLookup[ISOLINE].Config3[C_FMLF] = 0;
          ErrorLookup[ISOLINE].Config3[C_LINE] = 0;
          ErrorLookup[ISOLINE].Config3[C_POFE] = 0;
          ErrorLookup[ISOLINE].Config3[C_FMPF] = 0;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[ISOLINE].Domain1[j] = 0;
              ErrorLookup[ISOLINE].Domain2[j] = 0;
              ErrorLookup[ISOLINE].Domain3[j] = 0;
            }
        }
      break;

    case LSEGCOVERA: /** line segment overlaps an area feature perimeter ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LSEGCOVERA].Config1[i] = 2;
              ErrorLookup[LSEGCOVERA].Config2[i] = 2;
            }
          ErrorLookup[LSEGCOVERA].Config1[C_FMLF] = 1;
          ErrorLookup[LSEGCOVERA].Config1[C_DILI]  = 1;
          ErrorLookup[LSEGCOVERA].Config1[C_LINE]  = 1;

          ErrorLookup[LSEGCOVERA].Config2[C_AREA]  = 1;
          ErrorLookup[LSEGCOVERA].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LSEGCOVERA].Domain1[j] = 0;
              ErrorLookup[LSEGCOVERA].Domain2[j] = 0;
            }
        }
      break;




    case LINSIDEA: /** line partly or entirely inside area feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LINSIDEA].Config1[i] = 2;
              ErrorLookup[LINSIDEA].Config2[i] = 2;
              ErrorLookup[LINSIDEA].Config3[i] = 2;
            }
          ErrorLookup[LINSIDEA].Config1[C_FMLF] = 1;
          ErrorLookup[LINSIDEA].Config1[C_DILI]  = 1;
          ErrorLookup[LINSIDEA].Config1[C_LINE]  = 1;

          ErrorLookup[LINSIDEA].Config2[C_AREA]  = 1;
          ErrorLookup[LINSIDEA].Config2[C_FMAF] = 1;

          ErrorLookup[LINSIDEA].Config3[C_FMLF] = 1;
          ErrorLookup[LINSIDEA].Config3[C_DILI]  = 1;
          ErrorLookup[LINSIDEA].Config3[C_LINE]  = 1;
          ErrorLookup[LINSIDEA].Config3[C_AREA]  = 1;
          ErrorLookup[LINSIDEA].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LINSIDEA].Domain1[j] = 0;
              ErrorLookup[LINSIDEA].Domain2[j] = 0;
              ErrorLookup[LINSIDEA].Domain3[j] = 0;
            }
        }
      break;


    case LEINSIDEA: /** line end node properly inside an area ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LEINSIDEA].Config1[i] = 2;
              ErrorLookup[LEINSIDEA].Config2[i] = 2;
            }
          ErrorLookup[LEINSIDEA].Config1[C_FMLF] = 1;
          ErrorLookup[LEINSIDEA].Config1[C_DILI]  = 1;
          ErrorLookup[LEINSIDEA].Config1[C_LINE]  = 1;

          ErrorLookup[LEINSIDEA].Config2[C_AREA]  = 1;
          ErrorLookup[LEINSIDEA].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LEINSIDEA].Domain1[j] = 0;
              ErrorLookup[LEINSIDEA].Domain2[j] = 0;
            }
        }
      break;


    case LEAON_NOTIN: /** line end node on area edge, line not inside area ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LEAON_NOTIN].Config1[i] = 2;
              ErrorLookup[LEAON_NOTIN].Config2[i] = 2;
            }
          ErrorLookup[LEAON_NOTIN].Config1[C_FMLF] = 1;
          ErrorLookup[LEAON_NOTIN].Config1[C_DILI]  = 1;
          ErrorLookup[LEAON_NOTIN].Config1[C_LINE]  = 1;

          ErrorLookup[LEAON_NOTIN].Config2[C_AREA]  = 1;
          ErrorLookup[LEAON_NOTIN].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LEAON_NOTIN].Domain1[j] = 0;
              ErrorLookup[LEAON_NOTIN].Domain2[j] = 0;
            }
        }
      break;



    case SHARE3SEG: /** line feature segment overlaps 2 other line feature segments ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[SHARE3SEG].Config1[i] = 2;
              ErrorLookup[SHARE3SEG].Config2[i] = 2;
              ErrorLookup[SHARE3SEG].Config3[i] = 2;
            }
          ErrorLookup[SHARE3SEG].Config1[C_FMLF] = 1;
          ErrorLookup[SHARE3SEG].Config1[C_DILI]  = 1;
          ErrorLookup[SHARE3SEG].Config1[C_LINE]  = 1;

          ErrorLookup[SHARE3SEG].Config2[C_DILI]  = 1;
          ErrorLookup[SHARE3SEG].Config2[C_LINE]  = 1;
          ErrorLookup[SHARE3SEG].Config2[C_FMLF] = 1;
          ErrorLookup[SHARE3SEG].Config2[C_AREA]  = 1;
          ErrorLookup[SHARE3SEG].Config2[C_FMAF] = 1;

          ErrorLookup[SHARE3SEG].Config3[C_DILI]  = 1;
          ErrorLookup[SHARE3SEG].Config3[C_LINE]  = 1;
          ErrorLookup[SHARE3SEG].Config3[C_FMLF] = 1;
          ErrorLookup[SHARE3SEG].Config3[C_AREA]  = 1;
          ErrorLookup[SHARE3SEG].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[SHARE3SEG].Domain1[j] = 0;
              ErrorLookup[SHARE3SEG].Domain2[j] = 0;
              ErrorLookup[SHARE3SEG].Domain3[j] = 0;
            }
        }
      break;


    case COINCIDEFAIL: /** line or area feature segment fails to coincide with 2 other line or area features **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[COINCIDEFAIL].Config1[i] = 2;
              ErrorLookup[COINCIDEFAIL].Config2[i] = 2;
              ErrorLookup[COINCIDEFAIL].Config3[i] = 2;
            }
          ErrorLookup[COINCIDEFAIL].Config1[C_AREA]  = 1;
          ErrorLookup[COINCIDEFAIL].Config1[C_DILI]  = 1;
          ErrorLookup[COINCIDEFAIL].Config1[C_LINE]  = 1;
          ErrorLookup[COINCIDEFAIL].Config1[C_FMLF] = 1;
          ErrorLookup[COINCIDEFAIL].Config1[C_FMAF] = 1;

          ErrorLookup[COINCIDEFAIL].Config2[C_AREA]  = 1;
          ErrorLookup[COINCIDEFAIL].Config2[C_DILI]  = 1;
          ErrorLookup[COINCIDEFAIL].Config2[C_LINE]  = 1;
          ErrorLookup[COINCIDEFAIL].Config2[C_FMLF] = 1;
          ErrorLookup[COINCIDEFAIL].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[COINCIDEFAIL].Domain1[j] = 0;
              ErrorLookup[COINCIDEFAIL].Domain2[j] = 0;
            }
        }
      break;

    case ISOLATEDA:  /*** area feature does not intersect another area or a line feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ISOLATEDA].Config1[i] = 2;
              ErrorLookup[ISOLATEDA].Config2[i] = 2;
              ErrorLookup[ISOLATEDA].Config3[i] = 2;
            }
          ErrorLookup[ISOLATEDA].Config1[C_AREA]  = 1;
          ErrorLookup[ISOLATEDA].Config1[C_FMAF] = 1;

          ErrorLookup[ISOLATEDA].Config2[C_AREA]  = 1;
          ErrorLookup[ISOLATEDA].Config2[C_DILI]  = 1;
          ErrorLookup[ISOLATEDA].Config2[C_LINE]  = 1;
          ErrorLookup[ISOLATEDA].Config2[C_FMLF] = 1;
          ErrorLookup[ISOLATEDA].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[ISOLATEDA].Domain1[j] = 0;
              ErrorLookup[ISOLATEDA].Domain2[j] = 0;
            }
        }
      break;

    case ANETISOA: /** area not trans connected to another area by shared edges ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ANETISOA].Config1[i] = 2;
              ErrorLookup[ANETISOA].Config2[i] = 2;
              ErrorLookup[ANETISOA].Config3[i] = 2;
            }
          ErrorLookup[ANETISOA].Config1[C_AREA]  = 1;
          ErrorLookup[ANETISOA].Config1[C_FMAF] = 1;
          ErrorLookup[ANETISOA].Config2[C_AREA]  = 1;
          ErrorLookup[ANETISOA].Config2[C_FMAF] = 1;
          ErrorLookup[ANETISOA].Config3[C_AREA]  = 1;
          ErrorLookup[ANETISOA].Config3[C_FMAF] = 1;
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[ANETISOA].Domain1[j] = 0;
              ErrorLookup[ANETISOA].Domain2[j] = 0;
            }
        }
      break;




    case NETISOA: /** like ISOLATEDA except allowed a transitive connection through other like features ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[NETISOA].Config1[i] = 2;
              ErrorLookup[NETISOA].Config2[i] = 2;
              ErrorLookup[NETISOA].Config3[i] = 2;
            }
          ErrorLookup[NETISOA].Config1[C_AREA]  = 1;
          ErrorLookup[NETISOA].Config1[C_FMAF] = 1;

          ErrorLookup[NETISOA].Config2[C_DILI]  = 1;
          ErrorLookup[NETISOA].Config2[C_LINE]  = 1;
          ErrorLookup[NETISOA].Config2[C_FMLF] = 1;
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[NETISOA].Domain1[j] = 0;
              ErrorLookup[NETISOA].Domain2[j] = 0;
            }
        }
      break;



    case NETISOFEAT: /** form a network - check for nets with one feature, but not another ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;


          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[NETISOFEAT].Config1[i] = 2;
              ErrorLookup[NETISOFEAT].Config2[i] = 2;
              ErrorLookup[NETISOFEAT].Config3[i] = 2;
            }
          ErrorLookup[NETISOFEAT].Config1[C_AREA]  = 1;
          ErrorLookup[NETISOFEAT].Config1[C_FMAF] = 1;
          ErrorLookup[NETISOFEAT].Config1[C_DILI]  = 1;
          ErrorLookup[NETISOFEAT].Config1[C_LINE]  = 1;
          ErrorLookup[NETISOFEAT].Config1[C_FMLF] = 1;

          ErrorLookup[NETISOFEAT].Config2[C_DILI]  = 1;
          ErrorLookup[NETISOFEAT].Config2[C_LINE]  = 1;
          ErrorLookup[NETISOFEAT].Config2[C_FMLF] = 1;
          ErrorLookup[NETISOFEAT].Config2[C_AREA]  = 1;
          ErrorLookup[NETISOFEAT].Config2[C_FMAF] = 1;
          ErrorLookup[NETISOFEAT].Config3[C_DILI]  = 1;
          ErrorLookup[NETISOFEAT].Config3[C_LINE]  = 1;
          ErrorLookup[NETISOFEAT].Config3[C_FMLF] = 1;
          ErrorLookup[NETISOFEAT].Config3[C_AREA]  = 1;
          ErrorLookup[NETISOFEAT].Config3[C_FMAF] = 1;
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[NETISOFEAT].Domain1[j] = 0;
              ErrorLookup[NETISOFEAT].Domain2[j] = 0;
              /**/ErrorLookup[NETISOFEAT].Domain3[j] = 0;
            }
        }
      break;




    case SHARESEG: /** line feature segment overlaps 1 other line feature segment ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[SHARESEG].Config1[i] = 2;
              ErrorLookup[SHARESEG].Config2[i] = 2;
              ErrorLookup[SHARESEG].Config3[i] = 2;
            }
          ErrorLookup[SHARESEG].Config1[C_FMLF] = 1;
          ErrorLookup[SHARESEG].Config1[C_DILI]  = 1;
          ErrorLookup[SHARESEG].Config1[C_LINE]  = 1;

          ErrorLookup[SHARESEG].Config2[C_DILI]  = 1;
          ErrorLookup[SHARESEG].Config2[C_LINE]  = 1;
          ErrorLookup[SHARESEG].Config2[C_FMLF] = 1;

          ErrorLookup[SHARESEG].Config3[C_DILI]  = 1;
          ErrorLookup[SHARESEG].Config3[C_LINE]  = 1;
          ErrorLookup[SHARESEG].Config3[C_FMLF] = 1;
          ErrorLookup[SHARESEG].Config3[C_AREA]  = 1;
          ErrorLookup[SHARESEG].Config3[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[SHARESEG].Domain1[j] = 0;
              ErrorLookup[SHARESEG].Domain2[j] = 0;
              ErrorLookup[SHARESEG].Domain3[j] = 0;
            }
        }
      break;

    case MULTIDFEAT: /** single line or area with both 2 and 3 D coordinates ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[MULTIDFEAT].Config1[i] = 2;
              ErrorLookup[MULTIDFEAT].Config2[i] = 2;
            }
          ErrorLookup[MULTIDFEAT].Config1[C_FMLF] = 1;
          ErrorLookup[MULTIDFEAT].Config1[C_DILI]  = 1;
          ErrorLookup[MULTIDFEAT].Config1[C_LINE]  = 1;
          ErrorLookup[MULTIDFEAT].Config1[C_AREA]  = 1;
          ErrorLookup[MULTIDFEAT].Config1[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[MULTIDFEAT].Domain1[j] = 0;
              ErrorLookup[MULTIDFEAT].Domain2[j] = 0;
            }
        }
      break;


    case MULTISENTINEL: /** single line or area has more than one sentinel z value ***/ 
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[MULTISENTINEL].Config1[i] = 2;
              ErrorLookup[MULTISENTINEL].Config2[i] = 2;
            }
          ErrorLookup[MULTISENTINEL].Config1[C_FMLF] = 1;
          ErrorLookup[MULTISENTINEL].Config1[C_DILI]  = 1;
          ErrorLookup[MULTISENTINEL].Config1[C_LINE]  = 1;
          ErrorLookup[MULTISENTINEL].Config1[C_AREA]  = 1;
          ErrorLookup[MULTISENTINEL].Config1[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[MULTISENTINEL].Domain1[j] = 0;
              ErrorLookup[MULTISENTINEL].Domain2[j] = 0;
            }
        }
      break;


    case CONNECTFAIL: /** point, line, or area feature without 'connection' to specified 2nd feature **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[CONNECTFAIL].Config1[i] = 2;
              ErrorLookup[CONNECTFAIL].Config2[i] = 2;
            }
          ErrorLookup[CONNECTFAIL].Config1[C_POFE] = 1;
          ErrorLookup[CONNECTFAIL].Config1[C_AREA]  = 1;
          ErrorLookup[CONNECTFAIL].Config1[C_LINE]  = 1;

          ErrorLookup[CONNECTFAIL].Config2[C_AREA]  = 1;
          ErrorLookup[CONNECTFAIL].Config2[C_POFE] = 1;
          ErrorLookup[CONNECTFAIL].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[CONNECTFAIL].Domain1[j] = 0;
              ErrorLookup[CONNECTFAIL].Domain2[j] = 0;
            }
        }
      break;


    case FEATOUTSIDE:  /*** a feature lies at least partly outside the MGCP cell ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FEATOUTSIDE].Config1[i] = 2;
              ErrorLookup[FEATOUTSIDE].Config2[i] = 2;
            }
          ErrorLookup[FEATOUTSIDE].Config1[C_AREA] = 1;
          ErrorLookup[FEATOUTSIDE].Config1[C_LINE]  = 1;
          ErrorLookup[FEATOUTSIDE].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[FEATOUTSIDE].Domain1[j] = 0;
              ErrorLookup[FEATOUTSIDE].Domain2[j] = 0;
            }
        }
      break;

   case OSIDE_LAT:   /**** feature coordinate above or below latitude range    **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[OSIDE_LAT].Config1[i] = 2;
              ErrorLookup[OSIDE_LAT].Config2[i] = 2;
            }
          ErrorLookup[OSIDE_LAT].Config1[C_AREA] = 1;
          ErrorLookup[OSIDE_LAT].Config1[C_LINE]  = 1;
          ErrorLookup[OSIDE_LAT].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[OSIDE_LAT].Domain1[j] = 0;
              ErrorLookup[OSIDE_LAT].Domain2[j] = 0;
            }
        }
      break;

   case OSIDE_LON:   /**** feature coordinate above or below longitude range    **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[OSIDE_LON].Config1[i] = 2;
              ErrorLookup[OSIDE_LON].Config2[i] = 2;
            }
          ErrorLookup[OSIDE_LON].Config1[C_AREA] = 1;
          ErrorLookup[OSIDE_LON].Config1[C_LINE]  = 1;
          ErrorLookup[OSIDE_LON].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[OSIDE_LON].Domain1[j] = 0;
              ErrorLookup[OSIDE_LON].Domain2[j] = 0;
            }
        }
      break;

    case LBNDUSHT:  /** unconnected line end node undershoots whole-degree boundary ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LBNDUSHT].Config1[i] = 2;
              ErrorLookup[LBNDUSHT].Config2[i] = 2;
            }
          ErrorLookup[LBNDUSHT].Config1[C_LINE]  = 1;
          ErrorLookup[LBNDUSHT].Config2[C_AREA] = 1;
          ErrorLookup[LBNDUSHT].Config2[C_LINE]  = 1;
          ErrorLookup[LBNDUSHT].Config2[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LBNDUSHT].Domain1[j] = 0;
              ErrorLookup[LBNDUSHT].Domain2[j] = 0;
            }
        }
      break;



    case BNDRYUNDERSHT: /** feature undershoots whole degree project outside boundary ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[BNDRYUNDERSHT].Config1[i] = 2;
              ErrorLookup[BNDRYUNDERSHT].Config2[i] = 2;
            }
          ErrorLookup[BNDRYUNDERSHT].Config1[C_AREA] = 1;
          ErrorLookup[BNDRYUNDERSHT].Config1[C_LINE]  = 1;
          ErrorLookup[BNDRYUNDERSHT].Config1[C_POFE]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[BNDRYUNDERSHT].Domain1[j] = 0;
              ErrorLookup[BNDRYUNDERSHT].Domain2[j] = 0;
            }
        }
      break;



    case LENOCOVERL:  /*** line end node not within tolerance distance to another line ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LENOCOVERL].Config1[i] = 2;
              ErrorLookup[LENOCOVERL].Config2[i] = 2;
              ErrorLookup[LENOCOVERL].Config3[i] = 2;
            }
          ErrorLookup[LENOCOVERL].Config1[C_DILI] = 1;
          ErrorLookup[LENOCOVERL].Config1[C_FMLF]  = 1;
          ErrorLookup[LENOCOVERL].Config1[C_LINE]  = 1;

          ErrorLookup[LENOCOVERL].Config2[C_DILI]  = 1;
          ErrorLookup[LENOCOVERL].Config2[C_FMLF] = 1;
          ErrorLookup[LENOCOVERL].Config2[C_LINE] = 1;

          ErrorLookup[LENOCOVERL].Config3[C_DILI]  = 1;
          ErrorLookup[LENOCOVERL].Config3[C_FMLF] = 1;
          ErrorLookup[LENOCOVERL].Config3[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LENOCOVERL].Domain1[j] = 0;
              ErrorLookup[LENOCOVERL].Domain2[j] = 0;
              ErrorLookup[LENOCOVERL].Domain3[j] = 0;
            }
        }
      break;

    case NOLCOVLE:  /*** line end node not within tolerance distance to another line, including itself on a diff segment ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[NOLCOVLE].Config1[i] = 2;
              ErrorLookup[NOLCOVLE].Config2[i] = 2;
            }
          ErrorLookup[NOLCOVLE].Config1[C_DILI] = 1;
          ErrorLookup[NOLCOVLE].Config1[C_FMLF]  = 1;
          ErrorLookup[NOLCOVLE].Config1[C_LINE]  = 1;

          ErrorLookup[NOLCOVLE].Config2[C_DILI]  = 1;
          ErrorLookup[NOLCOVLE].Config2[C_FMLF] = 1;
          ErrorLookup[NOLCOVLE].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[NOLCOVLE].Domain1[j] = 0;
              ErrorLookup[NOLCOVLE].Domain2[j] = 0;
            }
        }
      break;

    case BOTHENDCON: /** both end nodes of a line feature are covered by specified-type point features **/
      if(Clone == 1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[BOTHENDCON].Config1[i] = 2;
              ErrorLookup[BOTHENDCON].Config2[i] = 2;
            }
          ErrorLookup[BOTHENDCON].Config1[C_DILI] = 1;
          ErrorLookup[BOTHENDCON].Config1[C_FMLF]  = 1;
          ErrorLookup[BOTHENDCON].Config1[C_LINE]  = 1;

          ErrorLookup[BOTHENDCON].Config2[C_POFE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[BOTHENDCON].Domain1[j] = 0;
              ErrorLookup[BOTHENDCON].Domain2[j] = 0;
            }
        }
      break;



    case NOENDCON: /** both end nodes of a line fail to connect or be covered **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA] = 1;

          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[NOENDCON].Config1[i] = 2;
              ErrorLookup[NOENDCON].Config2[i] = 2;
              ErrorLookup[NOENDCON].Config3[i] = 2;
            }
          ErrorLookup[NOENDCON].Config1[C_DILI] = 1;
          ErrorLookup[NOENDCON].Config1[C_FMLF]  = 1;
          ErrorLookup[NOENDCON].Config1[C_LINE]  = 1;

          ErrorLookup[NOENDCON].Config2[C_DILI]  = 1;
          ErrorLookup[NOENDCON].Config2[C_FMLF] = 1;
          ErrorLookup[NOENDCON].Config2[C_LINE] = 1;
          ErrorLookup[NOENDCON].Config2[C_POFE] = 1;
          ErrorLookup[NOENDCON].Config2[C_AREA] = 1;

          ErrorLookup[NOENDCON].Config3[C_DILI]  = 1;
          ErrorLookup[NOENDCON].Config3[C_FMLF] = 1;
          ErrorLookup[NOENDCON].Config3[C_LINE] = 1;
          ErrorLookup[NOENDCON].Config3[C_POFE] = 1;
          ErrorLookup[NOENDCON].Config3[C_AREA] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[NOENDCON].Domain1[j] = 0;
              ErrorLookup[NOENDCON].Domain2[j] = 0;
              ErrorLookup[NOENDCON].Domain3[j] = 0;
            }
        }
      break;




    case ENCONFAIL: /** end node connectivity failure **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ENCONFAIL].Config1[i] = 2;
              ErrorLookup[ENCONFAIL].Config2[i] = 2;
            }
          ErrorLookup[ENCONFAIL].Config1[C_DILI] = 1;
          ErrorLookup[ENCONFAIL].Config1[C_FMLF]  = 1;
          ErrorLookup[ENCONFAIL].Config1[C_LINE]  = 1;

          ErrorLookup[ENCONFAIL].Config2[C_DILI]  = 1;
          ErrorLookup[ENCONFAIL].Config2[C_FMLF] = 1;
          ErrorLookup[ENCONFAIL].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[ENCONFAIL].Domain1[j] = 0;
              ErrorLookup[ENCONFAIL].Domain2[j] = 0;
            }
        }
      break;

    case FEATNOTCUT:  /** feature not cut at end node of second feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FEATNOTCUT].Config1[i] = 2;
              ErrorLookup[FEATNOTCUT].Config2[i] = 2;
            }
          ErrorLookup[FEATNOTCUT].Config1[C_DILI] = 1;
          ErrorLookup[FEATNOTCUT].Config1[C_FMLF]  = 1;
          ErrorLookup[FEATNOTCUT].Config1[C_LINE]  = 1;

          ErrorLookup[FEATNOTCUT].Config2[C_DILI]  = 1;
          ErrorLookup[FEATNOTCUT].Config2[C_FMLF] = 1;
          ErrorLookup[FEATNOTCUT].Config2[C_LINE] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[FEATNOTCUT].Domain1[j] = 0;
              ErrorLookup[FEATNOTCUT].Domain2[j] = 0;
            }
        }
      break;
      
    case LENOCOVERP:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
	  
	  
          CloneErrorLookup[Cloneindex].Config2[C_POFE] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LENOCOVERP].Config1[i] = 2;
              ErrorLookup[LENOCOVERP].Config2[i] = 2;
            }
          ErrorLookup[LENOCOVERP].Config1[C_FMLF] = 1;
          ErrorLookup[LENOCOVERP].Config1[C_DILI]  = 1;
          ErrorLookup[LENOCOVERP].Config1[C_LINE]  = 1;
	  
          ErrorLookup[LENOCOVERP].Config2[C_POFE] = 1;
          ErrorLookup[LENOCOVERP].Config2[C_FMPF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LENOCOVERP].Domain1[j] = 0;
              ErrorLookup[LENOCOVERP].Domain2[j] = 0;
            }
        }
      break;

    case LACUTFAIL:  /** line not cut at intersection with area perimeter **/
    case LAINTNOEND: /** line - area intersection not at line end node ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;


          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_FMLF] = 1;
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;

          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[Check].Domain1[j] = 0;
              ErrorLookup[Check].Domain2[j] = 0;
            }
        }
      break;

    case LENOCOVERA: /** line end node not covered by area perimeter ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;


          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LENOCOVERA].Config1[i] = 2;
              ErrorLookup[LENOCOVERA].Config2[i] = 2;
            }
          ErrorLookup[LENOCOVERA].Config1[C_FMLF] = 1;
          ErrorLookup[LENOCOVERA].Config1[C_DILI]  = 1;
          ErrorLookup[LENOCOVERA].Config1[C_LINE]  = 1;

          ErrorLookup[LENOCOVERA].Config2[C_AREA]  = 1;
          ErrorLookup[LENOCOVERA].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[LENOCOVERA].Domain1[j] = 0;
              ErrorLookup[LENOCOVERA].Domain2[j] = 0;
            }
        }
      break;

      
    case ANOCOVERLA:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;

          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
	  
	  
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE]  = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ANOCOVERLA].Config1[i] = 2;
              ErrorLookup[ANOCOVERLA].Config2[i] = 2;
              ErrorLookup[ANOCOVERLA].Config3[i] = 2;
            }
          ErrorLookup[ANOCOVERLA].Config1[C_FMAF] = 1;
          ErrorLookup[ANOCOVERLA].Config1[C_AREA]  = 1;

          ErrorLookup[ANOCOVERLA].Config2[C_FMAF] = 1;
          ErrorLookup[ANOCOVERLA].Config2[C_AREA]  = 1;
	  
          ErrorLookup[ANOCOVERLA].Config3[C_AREA]  = 1;
          ErrorLookup[ANOCOVERLA].Config3[C_FMAF] = 1;
          ErrorLookup[ANOCOVERLA].Config3[C_FMLF] = 1;
          ErrorLookup[ANOCOVERLA].Config3[C_DILI]  = 1;
          ErrorLookup[ANOCOVERLA].Config3[C_LINE]  = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[ANOCOVERLA].Domain1[j] = 0;
              ErrorLookup[ANOCOVERLA].Domain2[j] = 0;
              ErrorLookup[ANOCOVERLA].Domain3[j] = 0;
            }
        }
      break;

   case QUALANOCOVLA: /** area permin not covered by line or area AND is inside a third area ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;


          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 1;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;

          CloneErrorLookup[Cloneindex].Config3[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_AREA]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
              CloneErrorLookup[Cloneindex].Domain3[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[QUALANOCOVLA].Config1[i] = 2;
              ErrorLookup[QUALANOCOVLA].Config2[i] = 2;
              ErrorLookup[QUALANOCOVLA].Config3[i] = 2;
            }
          ErrorLookup[QUALANOCOVLA].Config1[C_FMAF] = 1;
          ErrorLookup[QUALANOCOVLA].Config1[C_AREA]  = 1;

          ErrorLookup[QUALANOCOVLA].Config2[C_AREA]  = 1;
          ErrorLookup[QUALANOCOVLA].Config2[C_FMAF] = 1;
          ErrorLookup[QUALANOCOVLA].Config2[C_FMLF] = 1;
          ErrorLookup[QUALANOCOVLA].Config2[C_DILI]  = 1;
          ErrorLookup[QUALANOCOVLA].Config2[C_LINE]  = 1;
 
          ErrorLookup[QUALANOCOVLA].Config3[C_FMAF] = 1;
          ErrorLookup[QUALANOCOVLA].Config3[C_AREA]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[QUALANOCOVLA].Domain1[j] = 0;
              ErrorLookup[QUALANOCOVLA].Domain2[j] = 0;
              ErrorLookup[QUALANOCOVLA].Domain3[j] = 0;
            }
        }
      break;

    case OVERUNDER: /** any feature outside a perimeter-defining area or a line end node undershooting it **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]  = 1;


          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_FMAF] = 1;
          ErrorLookup[Check].Config1[C_AREA]  = 1;
          ErrorLookup[Check].Config1[C_FMLF]  = 1;
          ErrorLookup[Check].Config1[C_DILI]  = 1;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_POFE]  = 1;
          ErrorLookup[Check].Config1[C_FMPF]  = 1;

          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[Check].Domain1[j] = 0;
              ErrorLookup[Check].Domain2[j] = 0;
            }
        }
      break;


      
    case AMCOVAFAIL: /** area not coverer by adjoining areas **/
    case ANOCOVERA: /** area not covered by second area ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
	  
	  
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
              CloneErrorLookup[Cloneindex].Domain2[j] = 0;
            }
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
              ErrorLookup[Check].Config2[i] = 2;
            }
          ErrorLookup[Check].Config1[C_FMAF] = 1;
          ErrorLookup[Check].Config1[C_AREA]  = 1;
	  
          ErrorLookup[Check].Config2[C_AREA]  = 1;
          ErrorLookup[Check].Config2[C_FMAF] = 1;
	  
          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[Check].Domain1[j] = 0;
              ErrorLookup[Check].Domain2[j] = 0;
            }
        }
      break;

    case COLINEAR: /** 3 consecutive vertices on line or area perim are collinear - middle one is not connecting node ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
            CloneErrorLookup[Cloneindex].Config1[i] = 2;
            CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_FMPF] = 0;

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
            ErrorLookup[COLINEAR].Config1[i] = 2;
            ErrorLookup[COLINEAR].Config2[i] = 2;
            }
          ErrorLookup[COLINEAR].Config1[C_AREA]  = 0;
          ErrorLookup[COLINEAR].Config1[C_DILI]  = 0;
          ErrorLookup[COLINEAR].Config1[C_LINE]  = 1;
          ErrorLookup[COLINEAR].Config1[C_FMLF] = 0;
          ErrorLookup[COLINEAR].Config1[C_FMAF] = 0;
          ErrorLookup[COLINEAR].Config2[C_AREA]  = 0;
          ErrorLookup[COLINEAR].Config2[C_DILI]  = 0;
          ErrorLookup[COLINEAR].Config2[C_LINE]  = 1;
          ErrorLookup[COLINEAR].Config2[C_FMLF] = 0;
          ErrorLookup[COLINEAR].Config2[C_FMAF] = 0;
          ErrorLookup[COLINEAR].Config2[C_POFE] = 1;
          ErrorLookup[COLINEAR].Config2[C_FMPF] = 0;
        }
      break;


      

    case CUTOUT:   /** simply identifies a cut-out of an area feature ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              CloneErrorLookup[Cloneindex].Domain1[j] = 0;
            }

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[CUTOUT].Config1[i] = 2;
              ErrorLookup[CUTOUT].Config2[i] = 2;
              ErrorLookup[CUTOUT].Config3[i] = 2;
            }
          ErrorLookup[CUTOUT].Config1[C_FMAF] = 1;
          ErrorLookup[CUTOUT].Config1[C_AREA]  = 1;

          for(j=1;j<NUM_D;j++)
            {
              ErrorLookup[CUTOUT].Domain1[j] = 0;
              ErrorLookup[CUTOUT].Domain2[j] = 0;
            }
        }
      break;
      
      
    case SEGLEN:      
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_POLY] = 0;
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[SEGLEN].Config1[i] = 2;
            }
          ErrorLookup[SEGLEN].Config1[C_AREA]  = 0;
          ErrorLookup[SEGLEN].Config1[C_DILI]  = 0;
          ErrorLookup[SEGLEN].Config1[C_LINE]  = 1;
          ErrorLookup[SEGLEN].Config1[C_FMLF] = 0;
          ErrorLookup[SEGLEN].Config1[C_FMPF]  = 0;
          ErrorLookup[SEGLEN].Config1[C_FMAF] = 0;
          ErrorLookup[SEGLEN].Config1[C_POLY] = 0;
        }
      break;

    case LONGSEG: /** linear or areal perimeter segment with length at or above threshold ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LONGSEG].Config1[i] = 2;
            }
          ErrorLookup[LONGSEG].Config1[C_AREA]  = 0;
          ErrorLookup[LONGSEG].Config1[C_DILI]  = 0;
          ErrorLookup[LONGSEG].Config1[C_LINE]  = 1;
          ErrorLookup[LONGSEG].Config1[C_FMLF] = 0;
          ErrorLookup[LONGSEG].Config1[C_FMAF] = 0;
        }
      break;


    case FEATBRIDGE: /** one linear feature serves as only connection between 2 other features of same type ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
              CloneErrorLookup[Cloneindex].Config2[i] = 2;
              CloneErrorLookup[Cloneindex].Config3[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config2[C_LINE] = 1;
          CloneErrorLookup[Cloneindex].Config3[C_LINE] = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[FEATBRIDGE].Config1[i] = 2;
              ErrorLookup[FEATBRIDGE].Config2[i] = 2;
              ErrorLookup[FEATBRIDGE].Config3[i] = 2;
            }
          ErrorLookup[FEATBRIDGE].Config1[C_LINE]  = 1;
          ErrorLookup[FEATBRIDGE].Config2[C_LINE] = 1;
          ErrorLookup[FEATBRIDGE].Config3[C_LINE] = 1;
        }
      break;
      
    case BIGAREA: /** area feature with large square area **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[BIGAREA].Config1[i] = 2;
            }
          ErrorLookup[BIGAREA].Config1[C_AREA]  = 1;
          ErrorLookup[BIGAREA].Config1[C_FMAF] = 0;
        }
      break;

    case CLAMP_NFLAT: /** area feature does not have constant elevation when clamped to underlying DEM ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
            CloneErrorLookup[Cloneindex].Config1[i] = 2;
            CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
        CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
        CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
        CloneErrorLookup[Cloneindex].Config2[C_GRID] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
            ErrorLookup[CLAMP_NFLAT].Config1[i] = 2;
            ErrorLookup[CLAMP_NFLAT].Config2[i] = 2;
            }
        ErrorLookup[CLAMP_NFLAT].Config1[C_AREA]  = 1;
        ErrorLookup[CLAMP_NFLAT].Config1[C_FMAF] = 0;
        ErrorLookup[CLAMP_NFLAT].Config2[C_GRID] = 0;
        }
      break;



    case NOT_FLAT:  /*** area feature with surface that is not uiform elevation ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;

        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[NOT_FLAT].Config1[i] = 2;
            }
          ErrorLookup[NOT_FLAT].Config1[C_AREA]  = 1;
          ErrorLookup[NOT_FLAT].Config1[C_FMAF] = 0;
        }
      break;

    case CLAMP_DIF: /** difference between feature vertex z value and interpolated DEM value ***/
      if(Clone==1)
        {
          for(j=1; j<NUM_C; j++)
            {
              CloneErrorLookup[Cloneindex].Config1[j] = 2;
              CloneErrorLookup[Cloneindex].Config2[j] = 2;
              CloneErrorLookup[Cloneindex].Config3[j] = 2;
            }
           CloneErrorLookup[Cloneindex].Config1[C_AREA] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_DILI] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_LINE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_POFE] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMPF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 1;
           CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 1;

           CloneErrorLookup[Cloneindex].Config2[C_GRID] = 1;
        }
      else
        {
          for(j=1; j<NUM_C; j++)
            {
              ErrorLookup[CLAMP_DIF].Config1[j] = 2;
              ErrorLookup[CLAMP_DIF].Config2[j] = 2;
              ErrorLookup[CLAMP_DIF].Config3[j] = 2;
            }
           ErrorLookup[CLAMP_DIF].Config1[C_AREA] = 1;
           ErrorLookup[CLAMP_DIF].Config1[C_DILI] = 1;
           ErrorLookup[CLAMP_DIF].Config1[C_LINE] = 1;
           ErrorLookup[CLAMP_DIF].Config1[C_POFE] = 1;
           ErrorLookup[CLAMP_DIF].Config1[C_FMPF] = 1;
           ErrorLookup[CLAMP_DIF].Config1[C_FMLF] = 1;
           ErrorLookup[CLAMP_DIF].Config1[C_FMAF] = 1;

           ErrorLookup[CLAMP_DIF].Config2[C_GRID] = 1;
        }
      break;


    case ZUNCLOSED: /** area feature not closed in Z **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[ZUNCLOSED].Config1[i] = 2;
            }
          ErrorLookup[ZUNCLOSED].Config1[C_AREA]  = 1;
          ErrorLookup[ZUNCLOSED].Config1[C_FMAF] = 0;
        }
      break;
      
    case AREAUNCLOSED: /** area feature unclosed in x,y, or z **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[AREAUNCLOSED].Config1[i] = 2;
            }
          ErrorLookup[AREAUNCLOSED].Config1[C_AREA]  = 1;
          ErrorLookup[AREAUNCLOSED].Config1[C_FMAF] = 0;
        }
      break;
      
      
    case SMLCUTOUT: /** small included area inner ring of area feature ***/
    case SMALLAREA: /** area feaure with small square area **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
	  
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
            }
          ErrorLookup[Check].Config1[C_AREA]  = 1;
          ErrorLookup[Check].Config1[C_FMAF] = 0;
        }
      break;
      
      
    case MULTIPARTL:      
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[MULTIPARTL].Config1[i] = 2; 
            }
          ErrorLookup[MULTIPARTL].Config1[C_DILI]  = 0;
          ErrorLookup[MULTIPARTL].Config1[C_LINE]  = 1;
          ErrorLookup[MULTIPARTL].Config1[C_FMLF] = 0;
        }
      break;
      
    case MULTIPARTA: /** multi-part area **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[MULTIPARTA].Config1[i] = 2;
            }
          ErrorLookup[MULTIPARTA].Config1[C_AREA]  = 0;
          ErrorLookup[MULTIPARTA].Config1[C_FMAF] = 0;
        }
      break;
      
    case MULTIPARTP: /** multi-part point **/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[MULTIPARTP].Config1[i] = 2;
            }
          ErrorLookup[MULTIPARTP].Config1[C_POFE]  = 0;
        }
      break;
      
      
      
    case PERIMLEN:
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
            CloneErrorLookup[Cloneindex].Config1[i] = 2;
            CloneErrorLookup[Cloneindex].Config2[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_POLY] = 0;
          CloneErrorLookup[Cloneindex].Config2[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_LINE]  = 0;
          CloneErrorLookup[Cloneindex].Config2[C_POFE]  = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
            ErrorLookup[PERIMLEN].Config1[i] = 2;
            ErrorLookup[PERIMLEN].Config2[i] = 2;
            }
          ErrorLookup[PERIMLEN].Config1[C_AREA]  = 0;
          ErrorLookup[PERIMLEN].Config1[C_DILI]  = 0;
          ErrorLookup[PERIMLEN].Config1[C_LINE]  = 1;
          ErrorLookup[PERIMLEN].Config1[C_FMLF] = 0;
          ErrorLookup[PERIMLEN].Config1[C_FMPF]  = 0;
          ErrorLookup[PERIMLEN].Config1[C_FMAF] = 0;
          ErrorLookup[PERIMLEN].Config1[C_POLY] = 0;
          ErrorLookup[PERIMLEN].Config2[C_AREA]  = 0;
          ErrorLookup[PERIMLEN].Config2[C_LINE]  = 0;
          ErrorLookup[PERIMLEN].Config2[C_POFE]  = 0;
        }
      break;

   case LONGFEAT:   /** line or area feature with total length above threshold ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
          CloneErrorLookup[Cloneindex].Config1[C_POLY] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[LONGFEAT].Config1[i] = 2;
            }
          ErrorLookup[LONGFEAT].Config1[C_AREA]  = 0;
          ErrorLookup[LONGFEAT].Config1[C_DILI]  = 0;
          ErrorLookup[LONGFEAT].Config1[C_LINE]  = 1;
          ErrorLookup[LONGFEAT].Config1[C_FMLF] = 0;
          ErrorLookup[LONGFEAT].Config1[C_FMPF]  = 0;
          ErrorLookup[LONGFEAT].Config1[C_FMAF] = 0;
          ErrorLookup[LONGFEAT].Config1[C_POLY] = 0;
        }
      break;



   case SHORTFEAT:  /** short length line feature not on quarter degree 'boundary' ***/
   case PC_SLOPE: /*** line feature segment with percent slope above tolerance ****/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_DILI]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_LINE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMLF] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[Check].Config1[i] = 2;
            }
          ErrorLookup[Check].Config1[C_DILI]  = 0;
          ErrorLookup[Check].Config1[C_LINE]  = 1;
          ErrorLookup[Check].Config1[C_FMLF] = 0;
        }
      break;


   case CALC_AREA:  /*** point feature with LEN and WID attr values product < tolerance ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_POMO]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_POFE]  = 1;
          CloneErrorLookup[Cloneindex].Config1[C_FMPF]  = 1;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[CALC_AREA].Config1[i] = 2;
            }
          ErrorLookup[CALC_AREA].Config1[C_POMO]  = 0;
          ErrorLookup[CALC_AREA].Config1[C_POFE]  = 1;
          ErrorLookup[CALC_AREA].Config1[C_FMPF]  = 1;
        }
      break;

   case COVERFAIL: /** to detect holes in surface; MGCP landcover requirement ***/
      if(Clone==1)
        {
          for(i=0; i<NUM_C; i++)
            {
              CloneErrorLookup[Cloneindex].Config1[i] = 2;
            }
          CloneErrorLookup[Cloneindex].Config1[C_AREA]  = 0;
          CloneErrorLookup[Cloneindex].Config1[C_FMAF] = 0;
        }
      else
        {
          for(i=0; i<NUM_C; i++)
            {
              ErrorLookup[COVERFAIL].Config1[i] = 2;
            }
          ErrorLookup[COVERFAIL].Config1[C_AREA]  = 0;
          ErrorLookup[COVERFAIL].Config1[C_FMAF] = 0;
        }
      break;
      
      
      
    default:
      printf("Bad news in InitErr:  %d %d %d\n",Check,Clone,Cloneindex);
    }
  
  
  
  
  if(ErrorLookup[Check].participants == 1)
    {
      if(Clone==1)
	{
	  for(j=0; j<NUM_C; j++)
	    {
	      CloneErrorLookup[Cloneindex].Config2[j] = 2;
	      CloneErrorLookup[Cloneindex].Config3[j] = 2;
	    }
	  for(j=0; j<NUM_S; j++)
	    {
	      CloneErrorLookup[Cloneindex].Stratum2[j] = 2;
	      CloneErrorLookup[Cloneindex].Stratum3[j] = 2;
	    }
	  for(j=0;j<NUM_D;j++)
	    {
	      CloneErrorLookup[Cloneindex].Domain2[j] = 2;
	      CloneErrorLookup[Cloneindex].Domain3[j] = 2;
	    }
	}
      else
	{
	  for(j=0; j<NUM_C; j++)
	    {
	      ErrorLookup[Check].Config2[j] = 2;
	      ErrorLookup[Check].Config3[j] = 2;
	    }
	  for(j=0; j<NUM_S; j++)
	    {
	      ErrorLookup[Check].Stratum2[j] = 2;
	      ErrorLookup[Check].Stratum3[j] = 2;
	    }
	  for(j=0;j<NUM_D;j++)
	    {
	      ErrorLookup[Check].Domain2[j] = 2;
	      ErrorLookup[Check].Domain3[j] = 2;
	    }
	}
    }
  else if(ErrorLookup[Check].participants == 2)
    {
      if(Clone==1)
	{
	  for(j=0; j<NUM_C; j++)
	    {
	      CloneErrorLookup[Cloneindex].Config3[j] = 2;
	    }
	  for(j=0; j<NUM_S; j++)
	    {
	      CloneErrorLookup[Cloneindex].Stratum3[j] = 2;
	    }
	  for(j=0;j<NUM_D;j++)
	    {
	      CloneErrorLookup[Cloneindex].Domain3[j] = 2;
	    }
	}
      else
	{
	  for(j=0; j<NUM_C; j++)
	    {
	      ErrorLookup[Check].Config3[j] = 2;
	    }
	  for(j=0; j<NUM_S; j++)
	    {
	      ErrorLookup[Check].Stratum3[j] = 2;
	    }
	  for(j=0;j<NUM_D;j++)
	    {
	      ErrorLookup[Check].Domain3[j] = 2;
	    }
	}
    }
  
  if((NGA_TYPE) && (Clone == 1))
    {
      for(j=1; j<NUM_C; j++)
        {
          if((j != C_AREA) && (j != C_LINE) && (j != C_POFE) && (j != C_GRID))
	    {
	      CloneErrorLookup[Cloneindex].Config1[j] = 2;
	      CloneErrorLookup[Cloneindex].Config2[j] = 2;
	      CloneErrorLookup[Cloneindex].Config3[j] = 2;
	    }
        }
    }
  else if(NGA_TYPE)
    {
      for(j=1; j<NUM_C; j++)
        {
          if((j != C_AREA) && (j != C_LINE) && (j != C_POFE) && (j != C_GRID))
	    {
	      ErrorLookup[Check].Config1[j] = 2;
	      ErrorLookup[Check].Config2[j] = 2;
	      ErrorLookup[Check].Config3[j] = 2;
	    }
        }
    }
}






void SetDefaultSensitivities(int Check, int Clone, int Cloneindex)
{
  
  switch(Check)
    {      
    case SLIVER:
      if(NGA_TYPE == 1)
	{
	  if(Clone==1)
	    {
	      CloneErrorLookup[Cloneindex].sensitivity = 0.09;
	    }
	  else
	    {
	      ErrorLookup[SLIVER].sensitivity = 0.09;
	    }
	}
      else
	{
	  if(Clone==1)
	    {
	      CloneErrorLookup[Cloneindex].sensitivity = 0.09;
	    }
	  else
	    {
	      ErrorLookup[SLIVER].sensitivity = 0.09;
	    }
	}
      break;

    case FACESIZE: /*** small area on face of area feature **/
       if(Clone==1)
         {
         CloneErrorLookup[Cloneindex].sensitivity = 25.0;
         CloneErrorLookup[Cloneindex].sensitivity2 = 1000.0;
         CloneErrorLookup[Cloneindex].sensitivity3 = 75.0;
         CloneErrorLookup[Cloneindex].sensitivity4 = 0.9;
         }
       else
         {
         ErrorLookup[FACESIZE].sensitivity = 25.0;
         ErrorLookup[FACESIZE].sensitivity2 = 1000.0;
         ErrorLookup[FACESIZE].sensitivity3 = 75.0;
         ErrorLookup[FACESIZE].sensitivity4 = 0.9;
         }
        
      break;

   case AWITHOUTA:
       if(Clone==1)
         {
         CloneErrorLookup[Cloneindex].sensitivity = 5.0;
         CloneErrorLookup[Cloneindex].sensitivity2 = 5.0;
         }
       else
         {
         ErrorLookup[AWITHOUTA].sensitivity = 5.0;
         ErrorLookup[AWITHOUTA].sensitivity2 = 5.0;
         }

      break;
      
      
    case NARROW:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.03;
	}
      else
	{
	  ErrorLookup[NARROW].sensitivity = 0.03;
  	}
      break;
      
      
      
    case SMALLOBJ:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.01;
	}
      else
	{
	  ErrorLookup[SMALLOBJ].sensitivity = 0.01;
  	}
      break;
      
      
    case HSLOPE:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 85.0;
	}
      else
	{
	  ErrorLookup[HSLOPE].sensitivity = 85.0;
	}
      break;
      
      
      
    case VTEAR:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.0;
 	}
      else
	{
	  ErrorLookup[VTEAR].sensitivity = 0.0;
  	}
      break;
      
      
      
      
    case HTEAR:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.1;
	}
      else
	{
	  ErrorLookup[HTEAR].sensitivity = 0.1;
  	}
      break;
      
      
      
    case OVERC:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.1;
	  CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
 	}
      else
	{
	  ErrorLookup[OVERC].sensitivity = 0.1;
	  ErrorLookup[OVERC].sensitivity2 = 1.0;
  	}
      break;


    case NONODEOVLP: /** line, area have overlapping edge without common node ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[NONODEOVLP].sensitivity = 0.1;
        }
      break;

      
      
    case LLNONODEINT: /* features intersect, but not at a shared node **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LLNONODEINT].sensitivity = 0.1;
        }
      break;
      
    case LVPROX:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity2 = 5.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
	}
      else
	{
	  ErrorLookup[LVPROX].sensitivity2 = 5.0;
          ErrorLookup[LVPROX].sensitivity = 0.1;
	}
      break;
      
      
    case LELINEPROX:  /** line end - line proximity ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LELINEPROX].sensitivity = 0.1;
        }
      break;

    case EN_EN_PROX:  /** undershoot end nodes connected by another feature **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[EN_EN_PROX].sensitivity2 = 10.0;
          ErrorLookup[EN_EN_PROX].sensitivity = 0.1;
        }
      break;
      
    case LUSHTL_CLEAN: /* like line - line undershoot, but no condition if feature mid-undershoot **/
    case LUNDERSHTL:  /** line end - line undershoot **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 3.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity2 = 10.0;
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity3 = 1.0;
          ErrorLookup[Check].sensitivity4 = 3.0;
        }
      break;

    case LOSHTL_DF:  
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 3.0;
        }
      else
        {
          ErrorLookup[LOSHTL_DF].sensitivity2 = 10.0;
          ErrorLookup[LOSHTL_DF].sensitivity = 0.1;
          ErrorLookup[LOSHTL_DF].sensitivity3 = 1.0;
          ErrorLookup[LOSHTL_DF].sensitivity4 = 3.0;
        }
      break;

    case LUSHTL_DF: 
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 3.0;
        }
      else
        {
          ErrorLookup[LUSHTL_DF].sensitivity2 = 10.0;
          ErrorLookup[LUSHTL_DF].sensitivity = 0.1;
          ErrorLookup[LUSHTL_DF].sensitivity3 = 1.0;
          ErrorLookup[LUSHTL_DF].sensitivity4 = 3.0;
        }
      break;
      
    case VUSHTL_CLEAN: /* like vertex - line undershoot, but no condition if feature mid-undershoot **/
    case LVUSHTL: /** interior line vertex undershoots a different line feature ***/
    case LVOSHTL: /** interior line vertex overshoots a different line feature ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 20.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity2 = 10.0;
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity3 = 20.0;
        }
      break;


    case LOVERSHTL:   /** line end - line overshoot **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 3.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity2 = 10.0;
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity3 = 1.0;
          ErrorLookup[Check].sensitivity4 = 3.0;
        }
      break;

      
    case LUNDERSHTA:  /** line end area perimeter undershoot **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LUNDERSHTA].sensitivity2 = 10.0;
          ErrorLookup[LUNDERSHTA].sensitivity = 0.1;
        }
      break;

    case LOVERSHTA:  /** line end - area perimeter overshoot **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LOVERSHTA].sensitivity2 = 10.0;
          ErrorLookup[LOVERSHTA].sensitivity = 0.1;
        }
      break;

    case LAPROX:  /** line to area proximity - smallest dist between the two features ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 10.0;
        }
      else
        {
          ErrorLookup[LAPROX].sensitivity = 10.0;
        }
      break;

    case LASLIVER: /** sliver formed between line and area features **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
        }
      else
        {
          ErrorLookup[LASLIVER].sensitivity = 0.1;
          ErrorLookup[LASLIVER].sensitivity2 = 1.0;
        }
      break;

    case LSLICEA: /** line 'slices' area so as create a small piece ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 100.0;
        }
      else
        {
          ErrorLookup[LSLICEA].sensitivity = 100.0;
        }
      break;

    case LLSLIVER:  /** sliver formed between two line features **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
        }
      else
        {
          ErrorLookup[LLSLIVER].sensitivity = 0.1;
          ErrorLookup[LLSLIVER].sensitivity2 = 1.0;
        }
      break;


    case AUNDERSHTA: /** area edge undershoots neighbor area edge ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 2.0;
        }
      else
        {
          ErrorLookup[AUNDERSHTA].sensitivity = 0.1;
          ErrorLookup[AUNDERSHTA].sensitivity2 = 1.0;
          ErrorLookup[AUNDERSHTA].sensitivity3 = 2.0;
        }
      break;

    case AOVERSHTA: /** area edge overshoots neighbor area edge ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 2.0;
        }
      else
        {
          ErrorLookup[AOVERSHTA].sensitivity = 0.1;
          ErrorLookup[AOVERSHTA].sensitivity2 = 1.0;
          ErrorLookup[AOVERSHTA].sensitivity3 = 2.0;
        }
      break;

    case LUNMA_ACRS_A: /** line end not matched to area node across area perimeter ***/
    case LUNM_ACRS_A: /*** line mismatch across poly edge ***/
      if(Clone==1)
        {
        CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
        ErrorLookup[Check].sensitivity2 = 10.0;
        ErrorLookup[Check].sensitivity = 0.1;
        }
      break;

    case LSAME_UNM_A: /*** line endpt unmatched with line of same FCODE at Area boundary ***/
      if(Clone==1)
        {
        CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
        ErrorLookup[LSAME_UNM_A].sensitivity2 = 10.0;
        ErrorLookup[LSAME_UNM_A].sensitivity = 0.1;
        }
      break;

    case LE_A_UNM_LAT: /** line end node not coincident with area node at latitude parallel **/
    case LE_A_UNM_LON: /** line end node not coincident with area node at longitude meridian **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 10.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity2 = 1.0;
          ErrorLookup[Check].sensitivity3 = 10.0;
        }
      break;


    case LRNGE_UNM_LAT:
    case LRNGE_UNM_LON:
    case LHANG_LON: /** hanging line feature at a specified longitude meridian ***/
    case LHANG_LAT: /** hanging line feature at a specified latitude parallel ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 50.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 10.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity2 = 50.0;
          ErrorLookup[Check].sensitivity3 = 1.0;
          ErrorLookup[Check].sensitivity4 = 10.0;
        }
      break;

    case LGEOM_UNM_LON:
    case LGEOM_UNM_LAT:
    case AGEOM_UNM_LON:
    case AGEOM_UNM_LAT:
      if(Clone==1)
        {
        CloneErrorLookup[Cloneindex].sensitivity = 1.0;
        CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        CloneErrorLookup[Cloneindex].sensitivity3 = 10.0;
        }
      else
        {
        ErrorLookup[Check].sensitivity = 1.0;
        ErrorLookup[Check].sensitivity2 = 0.1;
        ErrorLookup[Check].sensitivity3 = 10.0;
        }
      break;

    case AUNM_ATTR_A:
    case LUNM_ATTR_A:
    case AUNM_ACRS_A: /** area feature edge incorrectly matched across a bounding area feature ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 50.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity2 = 50.0;
        }
      break;

    case ARNGE_UNM_LAT:
    case ARNGE_UNM_LON:
    case AHANG_LON: /** hanging area feature at a specified longitude meridian ***/
    case AHANG_LAT: /** hanging area feature at a specified latitude parallel ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 50.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 10.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity2 = 50.0;
          ErrorLookup[Check].sensitivity3 = 1.0;
          ErrorLookup[Check].sensitivity4 = 10.0;
        }
      break;


    case L_UNM_A:  /*** line endpt unmatched at area feature boundary ***/
      if(Clone==1)
        {
        CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
        ErrorLookup[L_UNM_A].sensitivity2 = 10.0;
        ErrorLookup[L_UNM_A].sensitivity = 0.1;
        }
      break;

      
    case LOC_MULTINT: /** lines with no or compatible LOC values intersect each other multiple times **/
    case LLMULTINT: /** lines intersect each other multiple times **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 20.0;

        }
      else
        {
          ErrorLookup[Check].sensitivity = 3.0;
          ErrorLookup[Check].sensitivity2 = 10.0;
          ErrorLookup[Check].sensitivity3 = 20.0;
        }
      break;
      
      
      
    case L2D_L3D_MATCH:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[L2D_L3D_MATCH].sensitivity = 0.0;
        }
      break;
      
    case LEZ_PROX_3D: /** apply check L2D_L3D_MATCH to 3d line features only **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0002;
          CloneErrorLookup[Cloneindex].sensitivity2 = 4.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 6.0;
        }
      else
        {
          ErrorLookup[LEZ_PROX_3D].sensitivity = 0.0002;
          ErrorLookup[LEZ_PROX_3D].sensitivity2 = 4.0;
          ErrorLookup[LEZ_PROX_3D].sensitivity3 = 6.0;
        }
      break;
      
    case CNODE_ZBUST:  /*** Z mismatch between any two connecting nodes (in x,y) ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.0001;
          CloneErrorLookup[Cloneindex].sensitivity3 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 20.0;
        }
      else
        {
          ErrorLookup[CNODE_ZBUST].sensitivity = 0.1;
          ErrorLookup[CNODE_ZBUST].sensitivity2 = 0.0001;
          ErrorLookup[CNODE_ZBUST].sensitivity3 = 10.0;
          ErrorLookup[CNODE_ZBUST].sensitivity4 = 20.0;
        }
      break;


    case DUPLICATESEG:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[DUPLICATESEG].sensitivity = 0.0;
        }
      break;


      
    case SHAREPERIM:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.0;
        }
      else
        {
          ErrorLookup[SHAREPERIM].sensitivity = 0.0;
          ErrorLookup[SHAREPERIM].sensitivity2 = 0.0;
        }
      break;

    case COLINEAR:
      if(Clone==1)
        {
        CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        CloneErrorLookup[Cloneindex].sensitivity3 = 500.0;
        }
      else
        {
        ErrorLookup[COLINEAR].sensitivity = 0.1;
        ErrorLookup[COLINEAR].sensitivity2 = 0.1;
        ErrorLookup[COLINEAR].sensitivity3 = 500.0;
        }
      break;
      
    case LSPANFAIL: /** line not covered by face of doesnt span between edges ***/
    case LNOCOVERLA:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
        }
      break;

   /***case CONF_STATS:  just generates conflation-information statistics, etc - no conditions to read/write **/
   case CONFLATE: /*** line is unique among conflation sets of data ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 50.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 50.0;
        }
      break;


    case LNOCOV2A:  /** line not covered by edges of 2 area features ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LNOCOV2A].sensitivity = 0.1;
        }
      break;


    case ISOLINE:  /** line feature completely inside an area feature ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[ISOLINE].sensitivity = 0.1;
        }
      break;

    case LINSIDEA: /** line partly or entirely inside area feature ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 85.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity2 = 85.0;
        }
      break;

    case LSEGCOVERA: /** line segment overlaps an area feature perimeter ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
        }
      break;

    case LEINSIDEA: /** line end node properly inside an area ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LEINSIDEA].sensitivity = 0.1;
        }
      break;

    case LEAON_NOTIN: /** line end node on area edge, line not inside area ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LEAON_NOTIN].sensitivity = 0.1;
        }
      break;

    case SHARE3SEG: /** line feature segment overlaps 2 other line feature segments ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[SHARE3SEG].sensitivity = 0.1;
          ErrorLookup[SHARE3SEG].sensitivity2 = 0.1;
        }
      break;


    case COINCIDEFAIL: /** line or area feature segment fails to coincide with 2 other line or area features **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[COINCIDEFAIL].sensitivity = 0.1;
        }
      break;


    case SHARESEG: /** line feature segment overlaps 1 other line feature segment ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 100000.0;
        }
      else
        {
          ErrorLookup[SHARESEG].sensitivity = 0.1;
          ErrorLookup[SHARESEG].sensitivity2 = 0.1;
          ErrorLookup[SHARESEG].sensitivity3 = -1.0;
        }
      break;

    case LLI_ANGLE: /*** 2 lines intersect at severe angle ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 5.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[LLI_ANGLE].sensitivity = 10.0;
          ErrorLookup[LLI_ANGLE].sensitivity2 = 0.1;
        }
      break;

    case CONNECTFAIL: /** point, line, or area feature without 'connection' to specified 2nd feature **/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[CONNECTFAIL].sensitivity = 0.0;
        }
      break; 


    case FEATOUTSIDE:  /*** a feature lies at least partly outside the MGCP cell ***/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[FEATOUTSIDE].sensitivity = 0.1;
        }
      break;

    case OSIDE_LAT:  /*** a feature lies at least partly outside the MGCP cell ***/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = -90.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 90.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.001;

        }
      else
        {
          ErrorLookup[OSIDE_LAT].sensitivity = -90.0;
          ErrorLookup[OSIDE_LAT].sensitivity2 = 90.0;
          ErrorLookup[OSIDE_LAT].sensitivity3 = 0.001;
        }
      break;

    case OSIDE_LON:  /*** a feature lies at least partly outside the MGCP cell ***/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = -180.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 180.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.001;
        }
      else
        {
          ErrorLookup[OSIDE_LON].sensitivity = -180.0;
          ErrorLookup[OSIDE_LON].sensitivity2 = 180.0;
          ErrorLookup[OSIDE_LON].sensitivity3 = 0.001;
        }
      break;

    case LBNDUSHT:  /** unconnected line end node undershoots whole-degree boundary ***/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        }
      else
        {
          ErrorLookup[LBNDUSHT].sensitivity = 0.1;
          ErrorLookup[LBNDUSHT].sensitivity2 = 10.0;
        }
      break;


    case BNDRYUNDERSHT: /** feature undershoots whole degree project outside boundary ***/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        }
      else
        {
          ErrorLookup[BNDRYUNDERSHT].sensitivity = 0.1;
          ErrorLookup[BNDRYUNDERSHT].sensitivity2 = 10.0;
        }
      break;




    case ENCONFAIL: /** end node connectivity failure **/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[ENCONFAIL].sensitivity = 0.0;
        }
      break;


    case BOTHENDCON: /** both end nodes of a line feature are covered by specified-type point features **/
    case NOENDCON: /** both end nodes of a line fail to connect or be covered **/
    case LENOCOVERL:  /*** line end node not within tolerance distance to another line ***/
    case NOLCOVLE:  /*** line end node not within tolerance distance to another line, including itself on a diff segment ***/
    case FEATNOTCUT:  /** feature not cut at end node of second feature ***/
    case ANOCOVERLA:
    case QUALANOCOVLA: /** area permin not covered by line or area AND is inside a third area ***/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
        }
      break;


    case LENOCOVERP:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.00025;
        }
      else
        {
          ErrorLookup[LENOCOVERP].sensitivity = 0.00025;
        }
      break;

    case OVERUNDER: /** any feature outside a perimeter-defining area or a line end node undershooting it **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.001;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
        }
      else
        {
          ErrorLookup[OVERUNDER].sensitivity = 0.001;
          ErrorLookup[OVERUNDER].sensitivity2 = 1.0;
        }
      break;


    case LENOCOVERA: /** line end node not covered by area perimeter ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.001;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
        }
      else
        {
          ErrorLookup[LENOCOVERA].sensitivity = 0.001;
          ErrorLookup[LENOCOVERA].sensitivity2 = 1.0;
        }
      break;

    case LACUTFAIL:  /** line not cut at intersection with area perimeter **/
    case LAINTNOEND: /** line - area intersection not at line end node ***/ 
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.01;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.01;
        }
      break;
      
    case LSPINT:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 44.0;
	}
      else
	{
	  ErrorLookup[LSPINT].sensitivity = 44.0;
  	}
      break;
      
      
      
    case LSPIEXP:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 44.0;
	}
      else
	{
	  ErrorLookup[LSPIEXP].sensitivity = 44.0;
  	}
      break;
      
      
      
    case PLPROX:
      if(Clone==1)
	{
          CloneErrorLookup[Cloneindex].sensitivity = 10.0;
	}
      else
	{
          ErrorLookup[PLPROX].sensitivity = 10.0;
  	}
      break;


    case PSHOOTL: /*** point feature over or undershoots a line feature ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        }
      else
        {
          ErrorLookup[PSHOOTL].sensitivity = 0.1;
          ErrorLookup[PSHOOTL].sensitivity2 = 10.0;
        }
      break;


    case PLPROXEX:  /** pt to line prox with exception for line end node ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.01;
        }
      else
        {
          ErrorLookup[PLPROXEX].sensitivity = 0.01;
        }
      break;
      
      
      
    case PLPFAIL:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.01;
	}
      else
	{
	  ErrorLookup[PLPFAIL].sensitivity = 0.01;
    	}
      break;
      
    case PNOCOVERLE: /* point not covered by linear end **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.01;
        }
      else
        {
          ErrorLookup[PNOCOVERLE].sensitivity = 0.01;
        }
      break;

    case PNOCOV2LEA: /** point not covered by 2 line terminal nodes or area edges***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[PNOCOV2LEA].sensitivity = 0.1;
        }
      break;
      
    case PNOCOVERLV: /** point not covered by any line vertex **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.01;
        }
      else
        {
          ErrorLookup[PNOCOVERLV].sensitivity = 0.01;
        }
      break;
      
      
    case PLLPROXFAIL:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.01;
        }
      else
        {
          ErrorLookup[PLLPROXFAIL].sensitivity = 0.01;
        }
      break;
      
    case KICKBACK:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 179.999;
        }
      else
        {
          ErrorLookup[KICKBACK].sensitivity = 179.999;
        }
      break;

    case ISOTURN: /** high turn angle w/o 3d feature present ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 5.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[ISOTURN].sensitivity = 5.0;
          ErrorLookup[ISOTURN].sensitivity2 = 0.1;
        }
      break;

    case ISOLATEDA:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[ISOLATEDA].sensitivity = 0.0;
        }
      break;

    case ANETISOA: /** area not trans connected to another area by shared edges ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.001;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.001;
        }
      break;

    case NETISOA:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.001;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.001;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.001;
          ErrorLookup[Check].sensitivity2 = 0.001;
        }
      break;
      
    case KINK:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 178.0;
	}
      else
	{
	  ErrorLookup[KINK].sensitivity = 178.0;
    	}
      break;

    case Z_KINK: /** consecutive kinks form a 'Z' ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 135.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 160.0;
        }
      else
        {
          ErrorLookup[Z_KINK].sensitivity = 135.0;
          ErrorLookup[Z_KINK].sensitivity2 = 160.0;
        }
      break;

    case L_A_KINK: /** kink at intersection of line end node  and area feature perim **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 170.0;
        }
      else
        {
          ErrorLookup[L_A_KINK].sensitivity = 170.0;
        }
      break;
      
    case CONTEXT_KINK:  /*** kink based on one high angle next to one lower (moderate) angle ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 160.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 178.0;
        }
      else
        {
          ErrorLookup[CONTEXT_KINK].sensitivity = 160.0;
          ErrorLookup[CONTEXT_KINK].sensitivity2 = 178.0;
        }
      break;

    case INTERNALKINK:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 165.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 178.0;
        }
      else
        {
          ErrorLookup[INTERNALKINK].sensitivity = 165.0;
          ErrorLookup[INTERNALKINK].sensitivity2 = 178.0;
        }
      break;
      
    case AREAKINK: /** high angle on perimeter of area feature **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 175.0;
        }
      else
        {
          ErrorLookup[AREAKINK].sensitivity = 175.0;
        }
      break; 
      
      
    case INCLSLIVER: /** areal with included sliver **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.09;
        }
      else
        {
          ErrorLookup[INCLSLIVER].sensitivity = 0.09;
        }
      break;
      

    case CLAMP_SDC: /*slope direction change along a line that has been elevation-value clamped to underlying DEM ***/
    case CLAMP_JOINSDC: /** slope direction change at line feature connection when both are clamped to DEM ***/
      if(Clone==1)
        {
        CloneErrorLookup[Cloneindex].sensitivity = 0.001;
        CloneErrorLookup[Cloneindex].sensitivity2 = 0.5;
        CloneErrorLookup[Cloneindex].sensitivity3 = 0.1;
        }
      else
        {
        ErrorLookup[Check].sensitivity = 0.001;
        ErrorLookup[Check].sensitivity2 = 0.5;
        ErrorLookup[Check].sensitivity3 = 0.1;
        }
      break;

    case SLOPEDIRCH:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.001;
	}
      else
	{
	  ErrorLookup[SLOPEDIRCH].sensitivity = 0.001;
  	}
      break;
      
    case LJOINSLOPEDC:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.001;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.001;
        }
      break;
      
      
    case LOOPS:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[LOOPS].sensitivity = 0.0;
        }
      break;


    case P_O_LOOP: /*** self-intersecting line that includes P & O formations using end nodes - lines only ****/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[LOOPS].sensitivity = 0.0;
        }
      break;

      
    case ENDPTINT:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[ENDPTINT].sensitivity = 0.0;
        }
      break;
      
    case LATTRCHNG:  /** line end point connects to same fdcode line, but attributes differ between the 2 features **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
        }
      else
        {
          ErrorLookup[LATTRCHNG].sensitivity = 0.0;
        }
      break;


   case CUT_INT: 
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.001;
        }
      else
        {
          ErrorLookup[CUT_INT].sensitivity = 0.001;
        }
      break;

    case ACOVERA:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 625.0;
        }
      else
        {
          ErrorLookup[ACOVERA].sensitivity = 625.0;
        }
      break;

      
      
      
    case GSPIKE:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 75.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = -32767.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = -32768.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 5.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 5.0;
          CloneErrorLookup[Cloneindex].use_sen5 = 1;
	}
      else
	{
	  ErrorLookup[GSPIKE].sensitivity = 75.0;
          ErrorLookup[GSPIKE].sensitivity4 = -32767.0;
          ErrorLookup[GSPIKE].sensitivity5 = -32768.0;
          ErrorLookup[GSPIKE].sensitivity3 = 5.0;
          ErrorLookup[GSPIKE].sensitivity2 = 5.0;
          ErrorLookup[GSPIKE].use_sen5 = 1;
  	}
      break;

    case PT_GRID_DIF: /** point and grid z value mismatch at exact coord, no interpolation **/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
     else
        {
          ErrorLookup[PT_GRID_DIF].sensitivity = 0.1;
          ErrorLookup[PT_GRID_DIF].sensitivity2 = 0.1;
        }
      break;

    case RAISEDPC: /** number of raised shoreline points exceeds tolerance **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 11.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 25.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = 1.0;
        }
      else
        {
          ErrorLookup[RAISEDPC].sensitivity = 11.0;
          ErrorLookup[RAISEDPC].sensitivity2 = 1.0;
          ErrorLookup[RAISEDPC].sensitivity3 = 3.0;
          ErrorLookup[RAISEDPC].sensitivity4 = 25.0;
          ErrorLookup[RAISEDPC].sensitivity5 = 1.0;
        }
      break;

    case WATERMMU: /** minimum mapping unit for water body below threshold ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 2.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 2.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 6000.0;
          //CloneErrorLookup[Cloneindex].sensitivity4 = 1.0;
        }
      else
        {
          ErrorLookup[WATERMMU].sensitivity = 2.0;
          ErrorLookup[WATERMMU].sensitivity2 = 2.0;
          ErrorLookup[WATERMMU].sensitivity3 = 6000.0;
          //ErrorLookup[WATERMMU].sensitivity4 = 1.0;
        }
      break;

    case FLOWSTEP:  /** step size in river flow above threshold ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 2.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 2.0;
        }
      else
        {
          ErrorLookup[FLOWSTEP].sensitivity = 3.0;
          ErrorLookup[FLOWSTEP].sensitivity2 = 3.0;
          ErrorLookup[FLOWSTEP].sensitivity3 = 2.0;
          ErrorLookup[FLOWSTEP].sensitivity4 = 2.0;
        }
      break;

    case BREAKLINE: /** river elevation change at bad angle with shorelines ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 20.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = 1.0;
        }
      else
        {
          ErrorLookup[BREAKLINE].sensitivity = 10.0;
          ErrorLookup[BREAKLINE].sensitivity2 = 20.0;
          ErrorLookup[BREAKLINE].sensitivity3 = 3.0;
          ErrorLookup[BREAKLINE].sensitivity4 = 3.0;
          ErrorLookup[BREAKLINE].sensitivity5 = 1.0;
        }
      break;

    case LOSMINHGT:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 30000.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = -1.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = -1.0;
        }
      else
        {
          ErrorLookup[LOSMINHGT].sensitivity = 30000.0;
          ErrorLookup[LOSMINHGT].sensitivity2 = -1.0;
          ErrorLookup[LOSMINHGT].sensitivity3 = -1.0;
        }
      break;


    case AVGSPIKE: /** spike / well as compared to average elevation of neighbor posts ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 20.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = -32767.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = -32768.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 5.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = 6.0;
          CloneErrorLookup[Cloneindex].sensitivity6 = 0.0;
        }
      else
        {
          ErrorLookup[AVGSPIKE].sensitivity = 20.0;
          ErrorLookup[AVGSPIKE].sensitivity3 = -32767.0;
          ErrorLookup[AVGSPIKE].sensitivity4 = -32768.0;
          ErrorLookup[AVGSPIKE].sensitivity2 = 5.0;
          ErrorLookup[AVGSPIKE].sensitivity5 = 6.0;
          ErrorLookup[AVGSPIKE].sensitivity6 = 0.0;
        }
      break;

    case GSHELF:  /** looking for shelf formations like PUE in DEM ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 65.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 21.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 23.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = 15.0;
          CloneErrorLookup[Cloneindex].sensitivity6 = 0.0;
        }
      else
        {
          ErrorLookup[GSHELF].sensitivity = 65.0;
          ErrorLookup[GSHELF].sensitivity2 = 21.0;
          ErrorLookup[GSHELF].sensitivity3 = 23.0;
          ErrorLookup[GSHELF].sensitivity4 = 1.0;
          ErrorLookup[GSHELF].sensitivity5 = 15.0;
          ErrorLookup[GSHELF].sensitivity6 = 0.0;
        }
      break;


    case GRID_STD_DEV: /** grid elev value, inside feature polygon, over range offset from std deviation **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = -32760.0;
        }
      else
        {
          ErrorLookup[GRID_STD_DEV].sensitivity = 1.0;
          ErrorLookup[GRID_STD_DEV].sensitivity2 = -32760.0;
        }
      break;
      
      
      
    case ELEVGT:
    case ELEVLT:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = -999.0;
          CloneErrorLookup[Cloneindex].sensitivity = 0.0;
	}
      else
	{
	  ErrorLookup[Check].sensitivity = -999.0;
          ErrorLookup[Check].sensitivity = 0.0;
  	}
      break;

      
      
    case ELEVEQ:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 0.0;
	  CloneErrorLookup[Cloneindex].sensitivity2 = 0.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.0;
	}
      else
	{
	  ErrorLookup[Check].sensitivity = 0.0;
	  ErrorLookup[Check].sensitivity2 = 0.0;
          ErrorLookup[Check].sensitivity3 = 0.0;
  	}
      break;


    case ELEVEQOPEN:  /** elevation in range, open interval**/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = -1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = -1.0;
          ErrorLookup[Check].sensitivity2 = 1.0;
          ErrorLookup[Check].sensitivity3 = 0.0;
        }
      break;
      

    case BADENCON: /** bad sequence on line feature connections ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[BADENCON].sensitivity = 0.1;
        }
      break;

    case LLINTNOEND: /** two lines intersect, pt of intersection is away from either primary particpant end node ***/
    case LLINTAWAY: /** two lines intersect, and cross over each other ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
        }
      break;

    case ENCONNECT:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
        }
      else
        {
          ErrorLookup[ENCONNECT].sensitivity = 1.0;
          ErrorLookup[ENCONNECT].sensitivity2 = 1.0;
        }
      break;

    case EXTRA_NET:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        }
      else
        {
          ErrorLookup[EXTRA_NET].sensitivity = 0.1;
          ErrorLookup[EXTRA_NET].sensitivity2 = 10.0;
        }
      break;

    case INTRA_NET:   
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        }
      else
        {
          ErrorLookup[INTRA_NET].sensitivity = 0.1;
          ErrorLookup[INTRA_NET].sensitivity2 = 10.0;
        }
      break;

    case CREATENET: /*** the internal check for creating networks - shouldn't appear in inspection menu ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity2 = 10.0;
        }
      break;
    case NETISOFEAT: /** form a network - check for nets with one feature, but not another ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 0.1;
          ErrorLookup[Check].sensitivity2 = 0.0;
        }
      break;

      
      
    case PTPTPROX:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 1.0;
	}
      else
	{
	  ErrorLookup[PTPTPROX].sensitivity = 1.0;
  	}
      break;


    case POVERSHTA: /** point not on area perimeter and is inside that area feature **/
      if(Clone==1) 
        {   
          CloneErrorLookup[Cloneindex].sensitivity = 10.0;
        }
      else
        {
          ErrorLookup[POVERSHTA].sensitivity = 10.0;
        } 
      break;

    case PUNDERSHTA: /** point not on area perimeter and is outside that area feature **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 10.0;
        }
      else
        {
          ErrorLookup[PUNDERSHTA].sensitivity = 10.0;
        }
      break;

      

    case FEATSPIKE: /** elevation spike along 3D feature ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 75.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.5;
        }
      else
        {
          ErrorLookup[FEATSPIKE].sensitivity = 75.0;
          ErrorLookup[FEATSPIKE].sensitivity2 = 0.5;
        }
      break;
      
    case ELEVADJCHANGE:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 999.0;
 	}
      else
	{
	  ErrorLookup[ELEVADJCHANGE].sensitivity = 999.0;
  	}
      break;
      
      
      
      
    case MASKZERO: /** DEM not zero elev at point defined by specified mask value ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 1.0;
          ErrorLookup[Check].sensitivity2 = 0.1;
          ErrorLookup[Check].sensitivity3 = 0.0;
        }
      break;

    case MASKCONSTANT: /*** DEM not constant elev at pointdefined by specified mask value ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 2.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 2.0;
          ErrorLookup[Check].sensitivity2 = 0.1;
          ErrorLookup[Check].sensitivity3 = 0.0;
        }
      break;

    case MASKEDIT_0:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 1.0;
          ErrorLookup[Check].sensitivity2 = 0.0;
          ErrorLookup[Check].sensitivity3 = 0.0;
        }
      break;

    case BILINSTATS: /** just write DEM comparison statistics - no conditions produced ***/
    case KERNELSTATS: /** just write DEM comparison statistics - no conditions produced ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = -32767.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 500.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = -32767.0;
          ErrorLookup[Check].sensitivity2 = 0.0;
          ErrorLookup[Check].sensitivity3 = 0.0;
          ErrorLookup[Check].sensitivity4 = 500.0;
        }
      break;

    case MASKEDIT_1: /** EDM has primary tolerance value, diff between TDR and TDF is > secondary tolerance **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 11.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = -32767.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = -50000.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 11.0;
          ErrorLookup[Check].sensitivity2 = 10.0;
          ErrorLookup[Check].sensitivity3 = -32767.0;
          ErrorLookup[Check].sensitivity4 = -50000.0;
          ErrorLookup[Check].sensitivity5 = 0.0;
        }
      break;


    case MASKCONF2: /** variation of Grids with conflicting values **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = -32767.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = -32767.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 1.0;
          ErrorLookup[Check].sensitivity2 = 1.0;
          ErrorLookup[Check].sensitivity3 = -32767.0;
          ErrorLookup[Check].sensitivity4 = -32767.0;
          ErrorLookup[Check].sensitivity5 = 0.0;
        }
      break;

    case MASKCONFLICT: /** Grid DEM Masks have conflicting values ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 8.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 8.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity4 = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity5 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 8.0;
          ErrorLookup[Check].sensitivity2 = 8.0;
          ErrorLookup[Check].sensitivity3 = 1.0;
          ErrorLookup[Check].sensitivity4 = 1.0;
          ErrorLookup[Check].sensitivity5 = 0.0;
        }
      break;

    case MASKSHOREL: /** water body not contained by shoreline ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.5;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 1.0;
          ErrorLookup[Check].sensitivity2 = 3.0;
          ErrorLookup[Check].sensitivity3 = 0.5;
        }
      break;

    case MASKMONO: /** DEM not monotonic at point defined by specified mask value ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 3.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity3 = 0.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 3.0;
          ErrorLookup[Check].sensitivity2 = 0.1;
          ErrorLookup[Check].sensitivity3 = 0.0;
        }
      break;

    case GRIDEXACTDIF: /** Grids have post value difference at same X,Y ***/
    case LODELEVDIF:
      if(Clone==1)
	{
	  CloneErrorLookup[Cloneindex].sensitivity = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = -32767.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = -32768.0;
	}
      else
	{
	  ErrorLookup[Check].sensitivity = 10.0;
          ErrorLookup[Check].sensitivity2 = -32767.0;
          ErrorLookup[Check].sensitivity3 = -32768.0;
  	}
      break;

    case CLAMP_SEG: /*** catenary segment below associated DEM ****/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 10.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = -50000.0;
          CloneErrorLookup[Cloneindex].sensitivity3 = -32760.0;
        }
      else
        {
          ErrorLookup[CLAMP_SEG].sensitivity = 10.0;
          ErrorLookup[CLAMP_SEG].sensitivity2 = -50000.0;
          ErrorLookup[CLAMP_SEG].sensitivity3 = -32760.0;
        }
      break;


    case NOCOINCIDE:
      if(Clone==1) 
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[NOCOINCIDE].sensitivity = 0.1;
        }
      break;


   case PSHAREFAIL:
      if(Clone == 1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[PSHAREFAIL].sensitivity = 0.1;
        }
      break;

   case FAILMERGEL:
      if(Clone == 1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 2.0;
        }
      else
        {
          ErrorLookup[FAILMERGEL].sensitivity = 0.1;
          ErrorLookup[FAILMERGEL].sensitivity2 = 2.0;
        }
      break;

    case FAILMERGEL2:  /** line object that should be merged with connecting line no accounting for metadata  ***/
     if(Clone == 1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 2.0;
        }
      else
        {
          ErrorLookup[FAILMERGEL2].sensitivity = 0.1;
          ErrorLookup[FAILMERGEL2].sensitivity2 = 2.0;
        }
      break;

   case FAILMERGEA:
      if(Clone == 1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[FAILMERGEA].sensitivity = 0.1;
        }
      break;

    case FAILMERGEA2: 
     if(Clone == 1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[FAILMERGEA2].sensitivity = 0.1;
          ErrorLookup[FAILMERGEA2].sensitivity2 = 0.1;
        }
      break;


 
    case SEGLEN:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[SEGLEN].sensitivity = 0.1;
        }
      break;

    case LONGSEG: /** linear or areal perimeter segment with length at or above threshold ***/
     if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1000.0;
        }
      else
        {
          ErrorLookup[LONGSEG].sensitivity = 1000.0;
        }
      break;

    case LLIEX: 
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LLIEX].sensitivity = 0.1;
        }
      break;

   case LAIEX: /** line - area intersection with 3rd feature exception ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LAIEX].sensitivity = 0.1;
        }
      break;

   case LLNOINT:  /** line failure to intersect a second line or area ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LLNOINT].sensitivity = 0.1;
        }
      break;



    case FEATBRIDGE: /** one linear feature serves as only connection between 2 other features of same type ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 30.0;
        }
      else
        {
          ErrorLookup[FEATBRIDGE].sensitivity = 30.0;
        }
      break;
      
    case BIGAREA: /** area feature with large square area **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 10000.0;
        }
      else
        {
          ErrorLookup[BIGAREA].sensitivity = 10000.0;
        }
      break;


    case NOT_FLAT:  /*** area feature with surface that is not uiform elevation ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0002;
        }
      else
        {
          ErrorLookup[NOT_FLAT].sensitivity = 0.0002;
        }
      break;

    case CLAMP_NFLAT: /** area feature does not have constant elevation when clamped to underlying DEM ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0002;
        }
      else
        {
          ErrorLookup[CLAMP_NFLAT].sensitivity = 0.0002;
        }
      break;



    case CLAMP_DIF: /** difference between feature vertex z value and interpolated DEM value ***/
      if(Clone==1)
        {
        CloneErrorLookup[Cloneindex].sensitivity = 0.0002;
        CloneErrorLookup[Cloneindex].sensitivity2 = -50000.0;
        CloneErrorLookup[Cloneindex].sensitivity3 = -32760.0;
        CloneErrorLookup[Cloneindex].sensitivity4 = 0.0;
        }
      else
        {
        ErrorLookup[CLAMP_DIF].sensitivity = 0.0002;
        ErrorLookup[CLAMP_DIF].sensitivity2 = -50000.0;
        ErrorLookup[CLAMP_DIF].sensitivity3 = -32760.0;
        ErrorLookup[CLAMP_DIF].sensitivity4 = 0.0;
        }
      break;

    case SMLCUTOUT: /** small included area inner ring of area feature ***/
    case SMALLAREA: /** area feaure with small square area **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
        }
      else
        {
          ErrorLookup[Check].sensitivity = 1.0;
        }
      break;
      
    case ZUNCLOSED: /** area feature not closed in Z **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0002;
        }
      else
        {
          ErrorLookup[ZUNCLOSED].sensitivity = 0.0002;
        }
      break;
      
    case AREAUNCLOSED: /** area feature unclosed in x,y, or z **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.0002;
        }
      else
        {
          ErrorLookup[AREAUNCLOSED].sensitivity = 0.0002;
        }
      break;
      
      
      
    case MULTIPARTL:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
        }
      else
        {
          ErrorLookup[MULTIPARTL].sensitivity = 1.0;
        }
      break;
      
    case MULTIPARTA:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
        }
      else
        {
          ErrorLookup[MULTIPARTA].sensitivity = 1.0;
        }
      break;
      
    case MULTIPARTP:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
        }
      else
        {
          ErrorLookup[MULTIPARTP].sensitivity = 1.0;
        }
      break;


    case LONGFEAT:   /** line or area feature with total length above threshold ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 600.0;
        }
      else
        {
          ErrorLookup[LONGFEAT].sensitivity = 600.0;
        }
      break;

      
      
      
    case PERIMLEN:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 5.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[PERIMLEN].sensitivity = 5.0;
          ErrorLookup[PERIMLEN].sensitivity2 = 0.1;
        }
      break;

   case SHORTFEAT:  /** short length line feature not on quarter degree 'boundary' ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 300.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[SHORTFEAT].sensitivity = 300.0;
          ErrorLookup[SHORTFEAT].sensitivity2 = 0.1;
        }
      break;

   case PC_SLOPE: /*** line feature segment with percent slope above tolerance ****/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 20.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[PC_SLOPE].sensitivity = 20.0;
          ErrorLookup[PC_SLOPE].sensitivity2 = 0.1;
        }
      break;


   case CALC_AREA:  /*** point feature with LEN and WID attr values product < tolerance ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 625.0;
        }
      else
        {
          ErrorLookup[CALC_AREA].sensitivity = 625.0;
        }
      break;

    case PTINREGION:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[PTINREGION].sensitivity = 0.1;
        }
      break;

    case PTINPROPER:  /** point feature inside an area feature - not within tolerance of edge (or edge or hole) **/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.001;
        }
      else
        {
          ErrorLookup[PTINPROPER].sensitivity = 0.001;
        }
      break;

    case COVERFAIL: /** to detect holes in surface; MGCP landcover requirement ***/
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 1.0;
          CloneErrorLookup[Cloneindex].sensitivity2 = 0.1;
        }
      else
        {
          ErrorLookup[COVERFAIL].sensitivity = 1.0;
          ErrorLookup[COVERFAIL].sensitivity2 = 0.1;
        }
      break;

    case BADFEATCUT:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[BADFEATCUT].sensitivity = 0.1;
        }
      break;

    case LLAINT:
      if(Clone==1)
        {
          CloneErrorLookup[Cloneindex].sensitivity = 0.1;
        }
      else
        {
          ErrorLookup[LLAINT].sensitivity = 0.1;
        }
      break;


      
    default:
      /* do nothing for checks with no sensitivities */
      break;
    }
}






void DoErrorSpecificInitialization(void)
{
  int i,j,k,l;
  extern struct ConditionTable * ConditionLookup;
  extern int TotalConditions;  /** in SEEIT_API - the numer of condition instances defined where an condition analysis file set was saved **/
  extern void InitArrayNames();

  TotalConditions = CONDITION_DEFINITIONS + 1;

  ConditionLookup = (struct ConditionTable *) (malloc((TotalConditions + 1) * sizeof(struct ConditionTable)));
  if(ConditionLookup == NULL)
    {
      printf("available memory insufficient for allocation of condition lookup table structure\n");
      printf("execution must terminate at this point\n");
      exit(-1);
    }

  for(i=0; i<=CONDITION_DEFINITIONS; i++)
    {
    ConditionLookup[i].conditiontype = i + 1;
    }


  InitArrayNames();
  
  
  strcpy(CGN[SIZE_GROUP].name,"Feature Size");
  strcpy(CGN[UNDERSHOOT_GROUP].name,"Undershoot or Overshoot");
  strcpy(CGN[NEW_DEM_GROUP].name,"DEM Conditions");
  strcpy(CGN[SEPDIST_GROUP].name,"Illogical Feature Separation Distance");
  strcpy(CGN[KINK_GROUP].name,"Kinks, Kickbacks, or Loops");
  strcpy(CGN[DUP_GROUP].name,"Duplication");
  strcpy(CGN[RANGE_GROUP].name,"Feature Elevation Or Grid Post-Value Range");
  strcpy(CGN[INTERSECT_GROUP].name,"Intersections");
  strcpy(CGN[OVERLAP_GROUP].name,"Overlap or Covering Relationships");
  strcpy(CGN[MATCH_GROUP].name,"Boundary Matching");
  strcpy(CGN[ATTR_GROUP].name,"Attribute Values");
  strcpy(CGN[ZVAL_GROUP].name,"Z Values");
  strcpy(CGN[CONNECT_GROUP].name,"Connectivity");
  strcpy(CGN[CONTAIN_GROUP].name,"Containment");
  strcpy(CGN[COMPO_GROUP].name,"Feature Composition");


  ErrorLookup[ANOCOVERLA].UseBorderCondsDefault = 1;
  ErrorLookup[FACESIZE].UseBorderCondsDefault = 1;
  ErrorLookup[AREAKINK].UseBorderCondsDefault = 1;
  ErrorLookup[NOENDCON].UseBorderCondsDefault = 1;
  ErrorLookup[OBJECTWITHOUT].UseBorderCondsDefault = 1;
  ErrorLookup[OBJ_WO_TWO].UseBorderCondsDefault = 1;
  ErrorLookup[SEGLEN].UseBorderCondsDefault = 1;
  ErrorLookup[L_UNM_A].UseBorderCondsDefault = 1;
  ErrorLookup[LSAME_UNM_A].UseBorderCondsDefault = 1;
  ErrorLookup[LUNM_ACRS_A].UseBorderCondsDefault = 1;
  ErrorLookup[LUNMA_ACRS_A].UseBorderCondsDefault = 1;
  ErrorLookup[ENCONFAIL].UseBorderCondsDefault = 1;
  ErrorLookup[LENOCOVERL].UseBorderCondsDefault = 1;
  ErrorLookup[NOLCOVLE].UseBorderCondsDefault = 1;
  ErrorLookup[LENOCOVERP].UseBorderCondsDefault = 1;
  ErrorLookup[LLNONODEINT].UseBorderCondsDefault = 1;
  ErrorLookup[PERIMLEN].UseBorderCondsDefault = 1;
  ErrorLookup[ISOLATEDA].UseBorderCondsDefault = 1;
  ErrorLookup[SLIVER].UseBorderCondsDefault = 1;
  ErrorLookup[LLNOINT].UseBorderCondsDefault = 1;
  ErrorLookup[AWITHOUTA].UseBorderCondsDefault = 1;
  ErrorLookup[NETISOA].UseBorderCondsDefault = 1;
  ErrorLookup[ANETISOA].UseBorderCondsDefault = 1;
  ErrorLookup[NETISOFEAT].UseBorderCondsDefault = 1;
  ErrorLookup[SMALLAREA].UseBorderCondsDefault = 1;
  ErrorLookup[LONGSEG].UseBorderCondsDefault = 1;
  ErrorLookup[PSHAREFAIL].UseBorderCondsDefault = 1;
  ErrorLookup[SMLCUTOUT].UseBorderCondsDefault = 1;
  ErrorLookup[LRNGE_UNM_LAT].UseBorderCondsDefault = 1;
  ErrorLookup[LRNGE_UNM_LON].UseBorderCondsDefault = 1;
  ErrorLookup[LHANG_LAT].UseBorderCondsDefault = 1;
  ErrorLookup[LHANG_LON].UseBorderCondsDefault = 1;
  ErrorLookup[LE_A_UNM_LAT].UseBorderCondsDefault = 1;
  ErrorLookup[LE_A_UNM_LON].UseBorderCondsDefault = 1;
  ErrorLookup[ARNGE_UNM_LAT].UseBorderCondsDefault = 1;
  ErrorLookup[ARNGE_UNM_LON].UseBorderCondsDefault = 1;
  ErrorLookup[AHANG_LAT].UseBorderCondsDefault = 1;
  ErrorLookup[AUNM_ACRS_A].UseBorderCondsDefault = 1;
  ErrorLookup[AHANG_LON].UseBorderCondsDefault = 1;
  ErrorLookup[LUNDERSHTL].UseBorderCondsDefault = 1;
  ErrorLookup[LUSHTL_CLEAN].UseBorderCondsDefault = 1;
  ErrorLookup[LOVERSHTL].UseBorderCondsDefault = 1;
  ErrorLookup[LUSHTL_DF].UseBorderCondsDefault = 1;
  ErrorLookup[LOSHTL_DF].UseBorderCondsDefault = 1;



  /** by default, set all checks to off by default **/
  for(i=1; i<=CONDITION_DEFINITIONS; i++)
    {
      ErrorLookup[i].Annotation = NULL;
      ErrorLookup[i].num_clones = 0;
      ErrorLookup[i].numthresholds = 0;
      ErrorLookup[i].checkapplies = (char) GAITandSEEITcheck;  /** check is for both SEEIT and GAIT **/
      ErrorLookup[i].usemagnitude = 0;
      ErrorLookup[i].sensitivity = 0;
      ErrorLookup[i].sensitivity2 = 0;
      ErrorLookup[i].sensitivity3 = 0;
      ErrorLookup[i].sensitivity4 = 0;
      ErrorLookup[i].sensitivity5 = 0;
      ErrorLookup[i].sensitivity6 = 0;
      ErrorLookup[i].range = 0;
      ErrorLookup[i].active = 0;
      ErrorLookup[i].draw_wholeareal = 0;
      ErrorLookup[i].magdescribe = "Magnitude";
      ErrorLookup[i].units = ConditionLookup[i-1].units;
      ErrorLookup[i].name = ConditionLookup[i-1].name;
      ErrorLookup[i].description = ConditionLookup[i-1].description;
      ErrorLookup[i].units2 = ConditionLookup[i-1].units2;
      ErrorLookup[i].units3 = ConditionLookup[i-1].units3;
      ErrorLookup[i].units4 = ConditionLookup[i-1].units4;
      ErrorLookup[i].units5 = ConditionLookup[i-1].units5;
      ErrorLookup[i].units6 = ConditionLookup[i-1].units6;
      ErrorLookup[i].tol_desc1 = NULL;
      ErrorLookup[i].tol_desc2 = NULL;
      ErrorLookup[i].tol_desc3 = NULL;
      ErrorLookup[i].tol_desc4 = NULL;
      ErrorLookup[i].tol_desc5 = NULL;
      ErrorLookup[i].tol_desc6 = NULL;

      
      for(j=0;j<4;j++)
	{
	  for(k=0;k<2;k++)
	    {
	      for(l=0;l<NUM_C;l++)
		{
		  ErrorLookup[i].IMarkRoot    [j][k][l] = NULL;
		  ErrorLookup[i].IMarkSACRoot [j][k][l] = NULL;
		  ErrorLookup[i].DO_EDCS_COMBO[j][k][l] = 0;
		  ErrorLookup[i].model_index  [j][k][l] = 0;
		  ErrorLookup[i].sac_index    [j][k][l] = 0;
		}
	    }
	}
    }
  
  
  ErrorLookup[LELINEPROX].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LELINEPROX].numthresholds = 1;
  ErrorLookup[LELINEPROX].usemagnitude = 1;
  ErrorLookup[LELINEPROX].lowrange = 0.0;
  ErrorLookup[LELINEPROX].highrange = 50.0;
  ErrorLookup[LELINEPROX].range = 1;
  ErrorLookup[LELINEPROX].participants = 2;
  ErrorLookup[LELINEPROX].tol_desc1 = "Feature proximity limit";

  ErrorLookup[EN_EN_PROX].mygroup = SEPDIST_GROUP;
  ErrorLookup[EN_EN_PROX].numthresholds = 2;
  ErrorLookup[EN_EN_PROX].usemagnitude = 1;
  ErrorLookup[EN_EN_PROX].lowrange2 = 0.00001;
  ErrorLookup[EN_EN_PROX].highrange2 = 500.0;
  ErrorLookup[EN_EN_PROX].lowrange = 0.0;
  ErrorLookup[EN_EN_PROX].highrange = 5.0;
  ErrorLookup[EN_EN_PROX].range = 1;
  ErrorLookup[EN_EN_PROX].participants = 3;
  ErrorLookup[EN_EN_PROX].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[EN_EN_PROX].tol_desc2 = "Feature proximity limit";
  
  ErrorLookup[LUNDERSHTL].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LUNDERSHTL].numthresholds = 4;
  ErrorLookup[LUNDERSHTL].usemagnitude = 1;
  ErrorLookup[LUNDERSHTL].lowrange2 = 0.00001;
  ErrorLookup[LUNDERSHTL].highrange2 = 100.0;
  ErrorLookup[LUNDERSHTL].lowrange = 0.0;
  ErrorLookup[LUNDERSHTL].highrange = 5.0;
  ErrorLookup[LUNDERSHTL].lowrange3 = 0.00001;
  ErrorLookup[LUNDERSHTL].highrange3 = 100.0;
  ErrorLookup[LUNDERSHTL].lowrange4 = 0.0;
  ErrorLookup[LUNDERSHTL].highrange4 = 10000000.0;
  ErrorLookup[LUNDERSHTL].range = 1;
  ErrorLookup[LUNDERSHTL].participants = 3;
  ErrorLookup[LUNDERSHTL].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LUNDERSHTL].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LUNDERSHTL].tol_desc3 = "Parallel line elimination limit";
  ErrorLookup[LUNDERSHTL].tol_desc4 = "Parallel line overlap length";

  ErrorLookup[LUSHTL_CLEAN].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LUSHTL_CLEAN].numthresholds = 4;
  ErrorLookup[LUSHTL_CLEAN].usemagnitude = 1;
  ErrorLookup[LUSHTL_CLEAN].lowrange2 = 0.00001;
  ErrorLookup[LUSHTL_CLEAN].highrange2 = 100.0;
  ErrorLookup[LUSHTL_CLEAN].lowrange = 0.0;
  ErrorLookup[LUSHTL_CLEAN].highrange = 5.0;
  ErrorLookup[LUSHTL_CLEAN].lowrange3 = 0.00001;
  ErrorLookup[LUSHTL_CLEAN].highrange3 = 100.0;
  ErrorLookup[LUSHTL_CLEAN].lowrange4 = 0.0;
  ErrorLookup[LUSHTL_CLEAN].highrange4 = 10000000.0;
  ErrorLookup[LUSHTL_CLEAN].range = 1;
  ErrorLookup[LUSHTL_CLEAN].participants = 3;
  ErrorLookup[LUSHTL_CLEAN].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LUSHTL_CLEAN].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LUSHTL_CLEAN].tol_desc3 = "Parallel line elimination limit";
  ErrorLookup[LUSHTL_CLEAN].tol_desc4 = "Parallel line overlap length";


  ErrorLookup[LVUSHTL].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LVUSHTL].numthresholds = 3;
  ErrorLookup[LVUSHTL].usemagnitude = 1;
  ErrorLookup[LVUSHTL].lowrange2 = 0.00001;
  ErrorLookup[LVUSHTL].highrange2 = 100.0;
  ErrorLookup[LVUSHTL].lowrange = 0.0;
  ErrorLookup[LVUSHTL].highrange = 5.0;
  ErrorLookup[LVUSHTL].lowrange3 = 0.01;
  ErrorLookup[LVUSHTL].highrange3 = 180.0;
  ErrorLookup[LVUSHTL].range = 1;
  ErrorLookup[LVUSHTL].participants = 3;
  ErrorLookup[LVUSHTL].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LVUSHTL].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LVUSHTL].tol_desc3 = "Heading change angle";


  ErrorLookup[VUSHTL_CLEAN].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[VUSHTL_CLEAN].numthresholds = 3;
  ErrorLookup[VUSHTL_CLEAN].usemagnitude = 1;
  ErrorLookup[VUSHTL_CLEAN].lowrange2 = 0.00001;
  ErrorLookup[VUSHTL_CLEAN].highrange2 = 100.0;
  ErrorLookup[VUSHTL_CLEAN].lowrange = 0.0;
  ErrorLookup[VUSHTL_CLEAN].highrange = 5.0;
  ErrorLookup[VUSHTL_CLEAN].lowrange3 = 0.01;
  ErrorLookup[VUSHTL_CLEAN].highrange3 = 180.0;
  ErrorLookup[VUSHTL_CLEAN].range = 1;
  ErrorLookup[VUSHTL_CLEAN].participants = 3;
  ErrorLookup[VUSHTL_CLEAN].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[VUSHTL_CLEAN].tol_desc2 = "Feature proximity limit";
  ErrorLookup[VUSHTL_CLEAN].tol_desc3 = "Heading change angle";

  ErrorLookup[LOSHTL_DF].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LOSHTL_DF].numthresholds = 4;
  ErrorLookup[LOSHTL_DF].usemagnitude = 1;
  ErrorLookup[LOSHTL_DF].lowrange2 = 0.00001;
  ErrorLookup[LOSHTL_DF].highrange2 = 100.0;
  ErrorLookup[LOSHTL_DF].lowrange = 0.0;
  ErrorLookup[LOSHTL_DF].highrange = 5.0;
  ErrorLookup[LOSHTL_DF].lowrange3 = 0.00001;
  ErrorLookup[LOSHTL_DF].highrange3 = 100.0;
  ErrorLookup[LOSHTL_DF].lowrange4 = 0.0;
  ErrorLookup[LOSHTL_DF].highrange4 = 10000000.0;
  ErrorLookup[LOSHTL_DF].range = 1;
  ErrorLookup[LOSHTL_DF].participants = 3;
  ErrorLookup[LOSHTL_DF].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LOSHTL_DF].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LOSHTL_DF].tol_desc3 = "Parallel line elimination limit";
  ErrorLookup[LOSHTL_DF].tol_desc4 = "Parallel line overlap length";

  ErrorLookup[LUSHTL_DF].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LUSHTL_DF].numthresholds = 4;
  ErrorLookup[LUSHTL_DF].usemagnitude = 1;
  ErrorLookup[LUSHTL_DF].lowrange2 = 0.00001;
  ErrorLookup[LUSHTL_DF].highrange2 = 100.0;
  ErrorLookup[LUSHTL_DF].lowrange = 0.0;
  ErrorLookup[LUSHTL_DF].highrange = 5.0;
  ErrorLookup[LUSHTL_DF].lowrange3 = 0.00001;
  ErrorLookup[LUSHTL_DF].highrange3 = 100.0;
  ErrorLookup[LUSHTL_DF].lowrange4 = 0.0;
  ErrorLookup[LUSHTL_DF].highrange4 = 10000000.0;
  ErrorLookup[LUSHTL_DF].range = 1;
  ErrorLookup[LUSHTL_DF].participants = 3;
  ErrorLookup[LUSHTL_DF].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LUSHTL_DF].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LUSHTL_DF].tol_desc3 = "Parallel line elimination limit";
  ErrorLookup[LUSHTL_DF].tol_desc4 = "Parallel line overlap length";

  ErrorLookup[LOVERSHTL].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LOVERSHTL].numthresholds = 4;
  ErrorLookup[LOVERSHTL].usemagnitude = 1;
  ErrorLookup[LOVERSHTL].lowrange2 = 0.00001;
  ErrorLookup[LOVERSHTL].highrange2 = 100.0;
  ErrorLookup[LOVERSHTL].lowrange = 0.0;
  ErrorLookup[LOVERSHTL].highrange = 5.0;
  ErrorLookup[LOVERSHTL].lowrange3 = 0.00001;
  ErrorLookup[LOVERSHTL].highrange3 = 100.0;
  ErrorLookup[LOVERSHTL].lowrange4 = 0.0;
  ErrorLookup[LOVERSHTL].highrange4 = 10000000.0;
  ErrorLookup[LOVERSHTL].range = 1;
  ErrorLookup[LOVERSHTL].participants = 3;
  ErrorLookup[LOVERSHTL].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LOVERSHTL].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LOVERSHTL].tol_desc3 = "Parallel line elimination limit";
  ErrorLookup[LOVERSHTL].tol_desc4 = "Parallel line overlap length";

  ErrorLookup[LVOSHTL].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LVOSHTL].numthresholds = 3;
  ErrorLookup[LVOSHTL].usemagnitude = 1;
  ErrorLookup[LVOSHTL].lowrange2 = 0.00001;
  ErrorLookup[LVOSHTL].highrange2 = 500.0;
  ErrorLookup[LVOSHTL].lowrange = 0.0;
  ErrorLookup[LVOSHTL].highrange = 5.0;
  ErrorLookup[LVOSHTL].lowrange3 = 0.01;
  ErrorLookup[LVOSHTL].highrange3 = 180.0;
  ErrorLookup[LVOSHTL].range = 1;
  ErrorLookup[LVOSHTL].participants = 3;
  ErrorLookup[LVOSHTL].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LVOSHTL].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LVOSHTL].tol_desc3 = "Heading change angle";
  
  ErrorLookup[LUNDERSHTA].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LUNDERSHTA].numthresholds = 2;
  ErrorLookup[LUNDERSHTA].usemagnitude = 1;
  ErrorLookup[LUNDERSHTA].lowrange2 = 0.00001;
  ErrorLookup[LUNDERSHTA].highrange2 = 500.0;
  ErrorLookup[LUNDERSHTA].lowrange = 0.0;
  ErrorLookup[LUNDERSHTA].highrange = 5.0;
  ErrorLookup[LUNDERSHTA].range = 1;
  ErrorLookup[LUNDERSHTA].participants = 3;
  ErrorLookup[LUNDERSHTA].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LUNDERSHTA].tol_desc2 = "Feature proximity limit";

  ErrorLookup[LOVERSHTA].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LOVERSHTA].numthresholds = 2;
  ErrorLookup[LOVERSHTA].usemagnitude = 1;
  ErrorLookup[LOVERSHTA].lowrange2 = 0.00001;
  ErrorLookup[LOVERSHTA].highrange2 = 500.0;
  ErrorLookup[LOVERSHTA].lowrange = 0.0;
  ErrorLookup[LOVERSHTA].highrange = 5.0;
  ErrorLookup[LOVERSHTA].range = 1;
  ErrorLookup[LOVERSHTA].participants = 3;
  ErrorLookup[LOVERSHTA].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LOVERSHTA].tol_desc2 = "Feature proximity limit";

  ErrorLookup[LAPROX].mygroup = SEPDIST_GROUP;
  ErrorLookup[LAPROX].numthresholds = 1;
  ErrorLookup[LAPROX].usemagnitude = 1;
  ErrorLookup[LAPROX].lowrange = 0.000001;
  ErrorLookup[LAPROX].highrange = 1000.0;
  ErrorLookup[LAPROX].participants = 2;
  ErrorLookup[LAPROX].tol_desc1 = "Feature proximity limit";

  ErrorLookup[LASLIVER].mygroup = SEPDIST_GROUP;
  ErrorLookup[LASLIVER].numthresholds = 2;
  ErrorLookup[LASLIVER].lowrange = 0.0;
  ErrorLookup[LASLIVER].highrange = 0.1;
  ErrorLookup[LASLIVER].lowrange2 = 0.000001;
  ErrorLookup[LASLIVER].highrange2 = 30.0;
  ErrorLookup[LASLIVER].range = 1;
  ErrorLookup[LASLIVER].usemagnitude = 1;
  ErrorLookup[LASLIVER].participants = 3;
  ErrorLookup[LASLIVER].tol_desc1 = "Feature proximity lower limit";
  ErrorLookup[LASLIVER].tol_desc2 = "Feature proximity upper limit";

  ErrorLookup[LSLICEA].mygroup = INTERSECT_GROUP;
  ErrorLookup[LSLICEA].numthresholds = 1;
  ErrorLookup[LSLICEA].lowrange = 0.1;
  ErrorLookup[LSLICEA].highrange = 50000.0;
  ErrorLookup[LSLICEA].range = 0;
  ErrorLookup[LSLICEA].usemagnitude = 1;
  ErrorLookup[LSLICEA].participants = 2;
  ErrorLookup[LSLICEA].tol_desc1 = "Partitioned piece square area";

  ErrorLookup[LLSLIVER].mygroup = SEPDIST_GROUP;
  ErrorLookup[LLSLIVER].numthresholds = 2;
  ErrorLookup[LLSLIVER].usemagnitude = 1;
  ErrorLookup[LLSLIVER].lowrange = 0.0;
  ErrorLookup[LLSLIVER].highrange = 0.1;
  ErrorLookup[LLSLIVER].lowrange2 = 0.000001;
  ErrorLookup[LLSLIVER].highrange2 = 20.0;
  ErrorLookup[LLSLIVER].range = 1;
  ErrorLookup[LLSLIVER].participants = 2;
  ErrorLookup[LLSLIVER].tol_desc1 = "Feature proximity lower limit";
  ErrorLookup[LLSLIVER].tol_desc2 = "Feature proximity upper limit";


  ErrorLookup[AUNDERSHTA].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[AUNDERSHTA].numthresholds = 3;
  ErrorLookup[AUNDERSHTA].usemagnitude = 1;
  ErrorLookup[AUNDERSHTA].lowrange = 0.0;
  ErrorLookup[AUNDERSHTA].highrange = 0.1;
  ErrorLookup[AUNDERSHTA].lowrange2 = 0.000001;
  ErrorLookup[AUNDERSHTA].highrange2 = 45.0;
  ErrorLookup[AUNDERSHTA].lowrange3 = 0.0;
  ErrorLookup[AUNDERSHTA].highrange3 = 45.0;
  ErrorLookup[AUNDERSHTA].range = 1;
  ErrorLookup[AUNDERSHTA].participants = 3;
  ErrorLookup[AUNDERSHTA].tol_desc1 = "Feature proximity lower limit";
  ErrorLookup[AUNDERSHTA].tol_desc2 = "Feature proximity upper limit";
  ErrorLookup[AUNDERSHTA].tol_desc3 = "Angle formed by feature edges";

  ErrorLookup[AOVERSHTA].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[AOVERSHTA].numthresholds = 3;
  ErrorLookup[AOVERSHTA].usemagnitude = 1;
  ErrorLookup[AOVERSHTA].lowrange = 0.0;
  ErrorLookup[AOVERSHTA].highrange = 0.1;
  ErrorLookup[AOVERSHTA].lowrange2 = 0.000001;
  ErrorLookup[AOVERSHTA].highrange2 = 45.0;
  ErrorLookup[AOVERSHTA].lowrange3 = 0.0;
  ErrorLookup[AOVERSHTA].highrange3 = 45.0;
  ErrorLookup[AOVERSHTA].range = 1;
  ErrorLookup[AOVERSHTA].participants = 2;
  ErrorLookup[AOVERSHTA].tol_desc1 = "Feature proximity lower limit";
  ErrorLookup[AOVERSHTA].tol_desc2 = "Feature proximity upper limit";
  ErrorLookup[AOVERSHTA].tol_desc3 = "Angle formed by feature edges";

  
  ErrorLookup[LSAME_UNM_A].mygroup = MATCH_GROUP;
  ErrorLookup[LSAME_UNM_A].numthresholds = 2;
  ErrorLookup[LSAME_UNM_A].usemagnitude = 0;
  ErrorLookup[LSAME_UNM_A].lowrange = 0.00000;
  ErrorLookup[LSAME_UNM_A].highrange = 20.0;
  ErrorLookup[LSAME_UNM_A].lowrange2 = 0.00000;
  ErrorLookup[LSAME_UNM_A].highrange2 = 100.0;
  ErrorLookup[LSAME_UNM_A].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LSAME_UNM_A].tol_desc2 = "Proximity to secondary participant";
  ErrorLookup[LSAME_UNM_A].participants = 2;
  ErrorLookup[LSAME_UNM_A].checkapplies = (char) GAITcheck;

  ErrorLookup[LUNM_ACRS_A].mygroup = MATCH_GROUP;
  ErrorLookup[LUNM_ACRS_A].numthresholds = 2;
  ErrorLookup[LUNM_ACRS_A].usemagnitude = 0;
  ErrorLookup[LUNM_ACRS_A].lowrange = 0.00000;
  ErrorLookup[LUNM_ACRS_A].highrange = 20.0;
  ErrorLookup[LUNM_ACRS_A].lowrange2 = 0.00000;
  ErrorLookup[LUNM_ACRS_A].highrange2 = 100.0;
  ErrorLookup[LUNM_ACRS_A].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LUNM_ACRS_A].tol_desc2 = "Proximity to tertiary participant";
  ErrorLookup[LUNM_ACRS_A].participants = 3;
  ErrorLookup[LUNM_ACRS_A].checkapplies = (char) GAITcheck;

  ErrorLookup[LUNMA_ACRS_A].mygroup = MATCH_GROUP;
  ErrorLookup[LUNMA_ACRS_A].numthresholds = 2;
  ErrorLookup[LUNMA_ACRS_A].usemagnitude = 0;
  ErrorLookup[LUNMA_ACRS_A].lowrange = 0.00000;
  ErrorLookup[LUNMA_ACRS_A].highrange = 20.0;
  ErrorLookup[LUNMA_ACRS_A].lowrange2 = 0.00000;
  ErrorLookup[LUNMA_ACRS_A].highrange2 = 100.0;
  ErrorLookup[LUNMA_ACRS_A].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LUNMA_ACRS_A].tol_desc2 = "Proximity to tertiary participant";
  ErrorLookup[LUNMA_ACRS_A].participants = 3;
  ErrorLookup[LUNMA_ACRS_A].checkapplies = (char) GAITcheck;


  ErrorLookup[L_UNM_A].mygroup = MATCH_GROUP;
  ErrorLookup[L_UNM_A].numthresholds = 2;
  ErrorLookup[L_UNM_A].usemagnitude = 0;
  ErrorLookup[L_UNM_A].lowrange = 0.00000;
  ErrorLookup[L_UNM_A].highrange = 50.0;
  ErrorLookup[L_UNM_A].lowrange2 = 0.00000;
  ErrorLookup[L_UNM_A].highrange2 = 100.0;
  ErrorLookup[L_UNM_A].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[L_UNM_A].tol_desc2 = "Proximity to secondary participant";
  ErrorLookup[L_UNM_A].participants = 2;
  ErrorLookup[L_UNM_A].checkapplies = (char) GAITcheck;

  ErrorLookup[LGEOM_UNM_LAT].mygroup = MATCH_GROUP;
  ErrorLookup[LGEOM_UNM_LAT].numthresholds = 3;
  ErrorLookup[LGEOM_UNM_LAT].usemagnitude = 0;
  ErrorLookup[LGEOM_UNM_LAT].lowrange   = 0.001;
  ErrorLookup[LGEOM_UNM_LAT].highrange  =  89.0;
  ErrorLookup[LGEOM_UNM_LAT].lowrange2 = 0.0;
  ErrorLookup[LGEOM_UNM_LAT].highrange2 =  50.0;
  ErrorLookup[LGEOM_UNM_LAT].lowrange3  = 0.0;
  ErrorLookup[LGEOM_UNM_LAT].highrange3 =  50.0;
  ErrorLookup[LGEOM_UNM_LAT].tol_desc1 = "Latitude parallel increment";
  ErrorLookup[LGEOM_UNM_LAT].tol_desc2 = "Coordinate equality limit";
  ErrorLookup[LGEOM_UNM_LAT].tol_desc3 = "Latitude parallel proximity";
  ErrorLookup[LGEOM_UNM_LAT].participants = 1;
  ErrorLookup[LGEOM_UNM_LAT].checkapplies = (char) GAITcheck;


  ErrorLookup[LRNGE_UNM_LAT].mygroup = MATCH_GROUP;
  ErrorLookup[LRNGE_UNM_LAT].numthresholds = 4;
  ErrorLookup[LRNGE_UNM_LAT].usemagnitude = 1;
  ErrorLookup[LRNGE_UNM_LAT].lowrange   = 0.0;
  ErrorLookup[LRNGE_UNM_LAT].highrange  =  50.0;
  ErrorLookup[LRNGE_UNM_LAT].lowrange2 = 0.0;
  ErrorLookup[LRNGE_UNM_LAT].highrange2 =  100.0;
  ErrorLookup[LRNGE_UNM_LAT].lowrange3 = 0.001;
  ErrorLookup[LRNGE_UNM_LAT].highrange3 =  10.0;
  ErrorLookup[LRNGE_UNM_LAT].lowrange4 = 0.0;
  ErrorLookup[LRNGE_UNM_LAT].highrange4 =  50.0;
  ErrorLookup[LRNGE_UNM_LAT].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LRNGE_UNM_LAT].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LRNGE_UNM_LAT].tol_desc3 = "Latitude parallel increment";
  ErrorLookup[LRNGE_UNM_LAT].tol_desc4 = "Latitude parallel proximity";
  ErrorLookup[LRNGE_UNM_LAT].participants = 3;
  ErrorLookup[LRNGE_UNM_LAT].checkapplies = (char) GAITcheck;
  ErrorLookup[LRNGE_UNM_LAT].range = 1;

  ErrorLookup[LRNGE_UNM_LON].mygroup = MATCH_GROUP;
  ErrorLookup[LRNGE_UNM_LON].numthresholds = 4;
  ErrorLookup[LRNGE_UNM_LON].usemagnitude = 1;
  ErrorLookup[LRNGE_UNM_LON].lowrange   = 0.0;
  ErrorLookup[LRNGE_UNM_LON].highrange  =  50.0;
  ErrorLookup[LRNGE_UNM_LON].lowrange2 = 0.0;
  ErrorLookup[LRNGE_UNM_LON].highrange2 =  100.0;
  ErrorLookup[LRNGE_UNM_LON].lowrange3 = 0.001;
  ErrorLookup[LRNGE_UNM_LON].highrange3 =  10.0;
  ErrorLookup[LRNGE_UNM_LON].lowrange4 = 0.0;
  ErrorLookup[LRNGE_UNM_LON].highrange4 =  50.0;
  ErrorLookup[LRNGE_UNM_LON].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LRNGE_UNM_LON].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LRNGE_UNM_LON].tol_desc3 = "Longitude meridian increment";
  ErrorLookup[LRNGE_UNM_LON].tol_desc4 = "Longitude meridian proximity";
  ErrorLookup[LRNGE_UNM_LON].participants = 3;
  ErrorLookup[LRNGE_UNM_LON].checkapplies = (char) GAITcheck;
  ErrorLookup[LRNGE_UNM_LON].range = 1;

  ErrorLookup[LE_A_UNM_LON].mygroup = MATCH_GROUP;
  ErrorLookup[LE_A_UNM_LON].numthresholds = 3;
  ErrorLookup[LE_A_UNM_LON].usemagnitude = 0;
  ErrorLookup[LE_A_UNM_LON].lowrange   = 0.0;
  ErrorLookup[LE_A_UNM_LON].highrange  =  50.0;
  ErrorLookup[LE_A_UNM_LON].lowrange2 = 0.001;
  ErrorLookup[LE_A_UNM_LON].highrange2 =  10.0;
  ErrorLookup[LE_A_UNM_LON].lowrange3 = 0.0;
  ErrorLookup[LE_A_UNM_LON].highrange3 =  50.0;
  ErrorLookup[LE_A_UNM_LON].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LE_A_UNM_LON].tol_desc2 = "Longitude meridian increment";
  ErrorLookup[LE_A_UNM_LON].tol_desc3 = "Longitude meridian proximity";
  ErrorLookup[LE_A_UNM_LON].participants = 2;
  ErrorLookup[LE_A_UNM_LON].checkapplies = (char) GAITcheck;
  ErrorLookup[LE_A_UNM_LON].range = 1;

  ErrorLookup[LE_A_UNM_LAT].mygroup = MATCH_GROUP;
  ErrorLookup[LE_A_UNM_LAT].numthresholds = 3;
  ErrorLookup[LE_A_UNM_LAT].usemagnitude = 0;
  ErrorLookup[LE_A_UNM_LAT].lowrange   = 0.0;
  ErrorLookup[LE_A_UNM_LAT].highrange  =  50.0;
  ErrorLookup[LE_A_UNM_LAT].lowrange2 = 0.001;
  ErrorLookup[LE_A_UNM_LAT].highrange2 =  10.0;
  ErrorLookup[LE_A_UNM_LAT].lowrange3 = 0.0;
  ErrorLookup[LE_A_UNM_LAT].highrange3 =  50.0;
  ErrorLookup[LE_A_UNM_LAT].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LE_A_UNM_LAT].tol_desc2 = "Longitude meridian increment";
  ErrorLookup[LE_A_UNM_LAT].tol_desc3 = "Longitude meridian proximity";
  ErrorLookup[LE_A_UNM_LAT].participants = 2;
  ErrorLookup[LE_A_UNM_LAT].checkapplies = (char) GAITcheck;
  ErrorLookup[LE_A_UNM_LAT].range = 1;


  ErrorLookup[LHANG_LON].mygroup = MATCH_GROUP;
  ErrorLookup[LHANG_LON].numthresholds = 4;
  ErrorLookup[LHANG_LON].usemagnitude = 0;
  ErrorLookup[LHANG_LON].lowrange   = 0.0;
  ErrorLookup[LHANG_LON].highrange  =  55.0;
  ErrorLookup[LHANG_LON].lowrange2 = 0.0;
  ErrorLookup[LHANG_LON].highrange2 =  100.0;
  ErrorLookup[LHANG_LON].lowrange3 = 0.001;
  ErrorLookup[LHANG_LON].highrange3 =  10.0;
  ErrorLookup[LHANG_LON].lowrange4 = 0.0;
  ErrorLookup[LHANG_LON].highrange4 =  50.0;
  ErrorLookup[LHANG_LON].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LHANG_LON].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LHANG_LON].tol_desc3 = "Longitude meridian increment";
  ErrorLookup[LHANG_LON].tol_desc4 = "Longitude meridian proximity";
  ErrorLookup[LHANG_LON].participants = 3;
  ErrorLookup[LHANG_LON].checkapplies = (char) GAITcheck;
  ErrorLookup[LHANG_LON].range = 1;

  ErrorLookup[LHANG_LAT].mygroup = MATCH_GROUP;
  ErrorLookup[LHANG_LAT].numthresholds = 4;
  ErrorLookup[LHANG_LAT].usemagnitude = 0;
  ErrorLookup[LHANG_LAT].lowrange   = 0.0;
  ErrorLookup[LHANG_LAT].highrange  =  55.0;
  ErrorLookup[LHANG_LAT].lowrange2 = 0.0;
  ErrorLookup[LHANG_LAT].highrange2 =  100.0;
  ErrorLookup[LHANG_LAT].lowrange3 = 0.001;
  ErrorLookup[LHANG_LAT].highrange3 =  10.0;
  ErrorLookup[LHANG_LAT].lowrange4 = 0.0;
  ErrorLookup[LHANG_LAT].highrange4 =  50.0;
  ErrorLookup[LHANG_LAT].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LHANG_LAT].tol_desc2 = "Feature proximity limit";
  ErrorLookup[LHANG_LAT].tol_desc3 = "Latitude parallel increment";
  ErrorLookup[LHANG_LAT].tol_desc4 = "Latitude parallel proximity";
  ErrorLookup[LHANG_LAT].participants = 3;
  ErrorLookup[LHANG_LAT].checkapplies = (char) GAITcheck;
  ErrorLookup[LHANG_LAT].range = 1;

  ErrorLookup[ARNGE_UNM_LAT].mygroup = MATCH_GROUP;
  ErrorLookup[ARNGE_UNM_LAT].numthresholds = 4;
  ErrorLookup[ARNGE_UNM_LAT].usemagnitude = 1;
  ErrorLookup[ARNGE_UNM_LAT].lowrange   = 0.0;
  ErrorLookup[ARNGE_UNM_LAT].highrange  =  5.0;
  ErrorLookup[ARNGE_UNM_LAT].lowrange2 = 0.0;
  ErrorLookup[ARNGE_UNM_LAT].highrange2 =  100.0;
  ErrorLookup[ARNGE_UNM_LAT].lowrange3 = 0.001;
  ErrorLookup[ARNGE_UNM_LAT].highrange3 =  10.0;
  ErrorLookup[ARNGE_UNM_LAT].lowrange4 = 0.0;
  ErrorLookup[ARNGE_UNM_LAT].highrange4 =  50.0;
  ErrorLookup[ARNGE_UNM_LAT].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[ARNGE_UNM_LAT].tol_desc2 = "Feature proximity limit";
  ErrorLookup[ARNGE_UNM_LAT].tol_desc3 = "Latitude parallel increment";
  ErrorLookup[ARNGE_UNM_LAT].tol_desc4 = "Latitude parallel proximity";
  ErrorLookup[ARNGE_UNM_LAT].participants = 3;
  ErrorLookup[ARNGE_UNM_LAT].checkapplies = (char) GAITcheck;
  ErrorLookup[ARNGE_UNM_LAT].range = 1;

  ErrorLookup[ARNGE_UNM_LON].mygroup = MATCH_GROUP;
  ErrorLookup[ARNGE_UNM_LON].numthresholds = 4;
  ErrorLookup[ARNGE_UNM_LON].usemagnitude = 1;
  ErrorLookup[ARNGE_UNM_LON].lowrange   = 0.0;
  ErrorLookup[ARNGE_UNM_LON].highrange  =  5.0;
  ErrorLookup[ARNGE_UNM_LON].lowrange2 = 0.0;
  ErrorLookup[ARNGE_UNM_LON].highrange2 =  100.0;
  ErrorLookup[ARNGE_UNM_LON].lowrange3 = 0.001;
  ErrorLookup[ARNGE_UNM_LON].highrange3 =  10.0;
  ErrorLookup[ARNGE_UNM_LON].lowrange4 = 0.0;
  ErrorLookup[ARNGE_UNM_LON].highrange4 =  50.0;
  ErrorLookup[ARNGE_UNM_LON].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[ARNGE_UNM_LON].tol_desc2 = "Feature proximity limit";
  ErrorLookup[ARNGE_UNM_LON].tol_desc3 = "Longitude meridian increment";
  ErrorLookup[ARNGE_UNM_LON].tol_desc4 = "Longitude meridian proximity";
  ErrorLookup[ARNGE_UNM_LON].participants = 3;
  ErrorLookup[ARNGE_UNM_LON].checkapplies = (char) GAITcheck;
  ErrorLookup[ARNGE_UNM_LON].range = 1;

  ErrorLookup[AHANG_LON].mygroup = MATCH_GROUP;
  ErrorLookup[AHANG_LON].numthresholds = 4;
  ErrorLookup[AHANG_LON].usemagnitude = 0;
  ErrorLookup[AHANG_LON].lowrange   = 0.0;
  ErrorLookup[AHANG_LON].highrange  =  5.0;
  ErrorLookup[AHANG_LON].lowrange2 = 0.0;
  ErrorLookup[AHANG_LON].highrange2 =  100.0;
  ErrorLookup[AHANG_LON].lowrange3 = 0.001;
  ErrorLookup[AHANG_LON].highrange3 =  10.0;
  ErrorLookup[AHANG_LON].lowrange4 = 0.0;
  ErrorLookup[AHANG_LON].highrange4 =  50.0;
  ErrorLookup[AHANG_LON].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[AHANG_LON].tol_desc2 = "Feature proximity limit";
  ErrorLookup[AHANG_LON].tol_desc3 = "Longitude meridian increment";
  ErrorLookup[AHANG_LON].tol_desc4 = "Longitude meridian proximity";
  ErrorLookup[AHANG_LON].participants = 3;
  ErrorLookup[AHANG_LON].checkapplies = (char) GAITcheck;
  ErrorLookup[AHANG_LON].range = 1;

  ErrorLookup[AHANG_LAT].mygroup = MATCH_GROUP;
  ErrorLookup[AHANG_LAT].numthresholds = 4;
  ErrorLookup[AHANG_LAT].usemagnitude = 0;
  ErrorLookup[AHANG_LAT].lowrange   = 0.0;
  ErrorLookup[AHANG_LAT].highrange  =  5.0;
  ErrorLookup[AHANG_LAT].lowrange2 = 0.0;
  ErrorLookup[AHANG_LAT].highrange2 =  100.0;
  ErrorLookup[AHANG_LAT].lowrange3 = 0.001;
  ErrorLookup[AHANG_LAT].highrange3 =  10.0;
  ErrorLookup[AHANG_LAT].lowrange4 = 0.0;
  ErrorLookup[AHANG_LAT].highrange4 =  50.0;
  ErrorLookup[AHANG_LAT].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[AHANG_LAT].tol_desc2 = "Feature proximity limit";
  ErrorLookup[AHANG_LAT].tol_desc3 = "Latitude parallel increment";
  ErrorLookup[AHANG_LAT].tol_desc4 = "Latitude parallel proximity";
  ErrorLookup[AHANG_LAT].participants = 3;
  ErrorLookup[AHANG_LAT].checkapplies = (char) GAITcheck;
  ErrorLookup[AHANG_LAT].range = 1;

  ErrorLookup[AUNM_ACRS_A].mygroup = MATCH_GROUP;
  ErrorLookup[AUNM_ACRS_A].numthresholds = 2;
  ErrorLookup[AUNM_ACRS_A].usemagnitude = 0;
  ErrorLookup[AUNM_ACRS_A].lowrange   = 0.0;
  ErrorLookup[AUNM_ACRS_A].highrange  =  5.0;
  ErrorLookup[AUNM_ACRS_A].lowrange2 = 0.0;
  ErrorLookup[AUNM_ACRS_A].highrange2 =  100.0;
  ErrorLookup[AUNM_ACRS_A].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[AUNM_ACRS_A].tol_desc2 = "Proximity to tertiary participant";
  ErrorLookup[AUNM_ACRS_A].participants = 3;
  ErrorLookup[AUNM_ACRS_A].checkapplies = (char) GAITcheck;

  ErrorLookup[AUNM_ATTR_A].mygroup = MATCH_GROUP;
  ErrorLookup[AUNM_ATTR_A].numthresholds = 2;
  ErrorLookup[AUNM_ATTR_A].usemagnitude = 0;
  ErrorLookup[AUNM_ATTR_A].lowrange   = 0.0;
  ErrorLookup[AUNM_ATTR_A].highrange  =  5.0;
  ErrorLookup[AUNM_ATTR_A].lowrange2 = 0.0;
  ErrorLookup[AUNM_ATTR_A].highrange2 =  100.0;
  ErrorLookup[AUNM_ATTR_A].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[AUNM_ATTR_A].tol_desc2 = "Proximity to tertiary participant";
  ErrorLookup[AUNM_ATTR_A].participants = 3;
  ErrorLookup[AUNM_ATTR_A].checkapplies = (char) GAITcheck;


  ErrorLookup[LUNM_ATTR_A].mygroup = MATCH_GROUP;
  ErrorLookup[LUNM_ATTR_A].numthresholds = 2;
  ErrorLookup[LUNM_ATTR_A].usemagnitude = 0;
  ErrorLookup[LUNM_ATTR_A].lowrange   = 0.0;
  ErrorLookup[LUNM_ATTR_A].highrange  =  5.0;
  ErrorLookup[LUNM_ATTR_A].lowrange2 = 0.0;
  ErrorLookup[LUNM_ATTR_A].highrange2 =  100.0;
  ErrorLookup[LUNM_ATTR_A].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LUNM_ATTR_A].tol_desc2 = "Proximity to tertiary participant";
  ErrorLookup[LUNM_ATTR_A].participants = 3;
  ErrorLookup[LUNM_ATTR_A].checkapplies = (char) GAITcheck;

  ErrorLookup[LGEOM_UNM_LON].mygroup = MATCH_GROUP;
  ErrorLookup[LGEOM_UNM_LON].numthresholds = 3;
  ErrorLookup[LGEOM_UNM_LON].usemagnitude = 0;
  ErrorLookup[LGEOM_UNM_LON].lowrange   = 0.001;
  ErrorLookup[LGEOM_UNM_LON].highrange  =  179.0;
  ErrorLookup[LGEOM_UNM_LON].lowrange2  = 0.0;
  ErrorLookup[LGEOM_UNM_LON].highrange2 =  50.0;
  ErrorLookup[LGEOM_UNM_LON].lowrange3  = 0.0;
  ErrorLookup[LGEOM_UNM_LON].highrange3 =  50.0;
  ErrorLookup[LGEOM_UNM_LON].tol_desc1 = "Longitude meridian increment";
  ErrorLookup[LGEOM_UNM_LON].tol_desc2 = "Coordinate equality limit";
  ErrorLookup[LGEOM_UNM_LON].tol_desc3 = "Longitude meridian proximity";
  ErrorLookup[LGEOM_UNM_LON].participants = 1;
  ErrorLookup[LGEOM_UNM_LON].checkapplies = (char) GAITcheck;

  ErrorLookup[AGEOM_UNM_LON].mygroup = MATCH_GROUP;
  ErrorLookup[AGEOM_UNM_LON].numthresholds = 3;
  ErrorLookup[AGEOM_UNM_LON].usemagnitude = 0;
  ErrorLookup[AGEOM_UNM_LON].lowrange   = 0.001;
  ErrorLookup[AGEOM_UNM_LON].highrange  =  179.0;
  ErrorLookup[AGEOM_UNM_LON].lowrange2  = 0.0;
  ErrorLookup[AGEOM_UNM_LON].highrange2 =  50.0;
  ErrorLookup[AGEOM_UNM_LON].lowrange3  = 0.0;
  ErrorLookup[AGEOM_UNM_LON].highrange3 =  50.0;
  ErrorLookup[AGEOM_UNM_LON].tol_desc1 = "Longitude meridian increment";
  ErrorLookup[AGEOM_UNM_LON].tol_desc2 = "Coordinate equality limit";
  ErrorLookup[AGEOM_UNM_LON].tol_desc3 = "Longitude meridian proximity";
  ErrorLookup[AGEOM_UNM_LON].participants = 1;
  ErrorLookup[AGEOM_UNM_LON].checkapplies = (char) GAITcheck;

  ErrorLookup[AGEOM_UNM_LAT].mygroup = MATCH_GROUP;
  ErrorLookup[AGEOM_UNM_LAT].numthresholds = 3;
  ErrorLookup[AGEOM_UNM_LAT].usemagnitude = 0;
  ErrorLookup[AGEOM_UNM_LAT].lowrange   = 0.001;
  ErrorLookup[AGEOM_UNM_LAT].highrange  =  89.0;
  ErrorLookup[AGEOM_UNM_LAT].lowrange2  = 0.0;
  ErrorLookup[AGEOM_UNM_LAT].highrange2 =  50.0;
  ErrorLookup[AGEOM_UNM_LAT].lowrange3  = 0.0;
  ErrorLookup[AGEOM_UNM_LAT].highrange3 =  50.0;
  ErrorLookup[AGEOM_UNM_LAT].tol_desc1 = "Latitude parallel increment";
  ErrorLookup[AGEOM_UNM_LAT].tol_desc2 = "Coordinate equality limit";
  ErrorLookup[AGEOM_UNM_LAT].tol_desc3 = "Latitude parallel proximity";
  ErrorLookup[AGEOM_UNM_LAT].participants = 1;
  ErrorLookup[AGEOM_UNM_LAT].checkapplies = (char) GAITcheck;

  ErrorLookup[LLMULTINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLMULTINT].numthresholds = 3;
  ErrorLookup[LLMULTINT].usemagnitude = 2;
  ErrorLookup[LLMULTINT].lowrange = 2.0;
  ErrorLookup[LLMULTINT].highrange = 100000.0;
  ErrorLookup[LLMULTINT].lowrange2 = 0.0;
  ErrorLookup[LLMULTINT].highrange2 = 1000000.0;
  ErrorLookup[LLMULTINT].lowrange3 = 0.0;
  ErrorLookup[LLMULTINT].highrange3 = 1000000.0;
  ErrorLookup[LLMULTINT].participants = 2;
  ErrorLookup[LLMULTINT].units = "intersections";
  ErrorLookup[LLMULTINT].units2 = "meters";
  ErrorLookup[LLMULTINT].units3 = "meters";
  ErrorLookup[LLMULTINT].tol_desc1 = "Number of intersections";
  ErrorLookup[LLMULTINT].tol_desc2 = "Z exclusion lower limit";
  ErrorLookup[LLMULTINT].tol_desc3 = "Z exclusion upper limit";


  ErrorLookup[LOC_MULTINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LOC_MULTINT].numthresholds = 3;
  ErrorLookup[LOC_MULTINT].usemagnitude = 2;
  ErrorLookup[LOC_MULTINT].lowrange = 2.0;
  ErrorLookup[LOC_MULTINT].highrange = 100000.0;
  ErrorLookup[LOC_MULTINT].lowrange2 = 0.0;
  ErrorLookup[LOC_MULTINT].highrange2 = 1000000.0;
  ErrorLookup[LOC_MULTINT].lowrange3 = 0.0;
  ErrorLookup[LOC_MULTINT].highrange3 = 1000000.0;
  ErrorLookup[LOC_MULTINT].participants = 2;
  ErrorLookup[LOC_MULTINT].units = "intersections";
  ErrorLookup[LOC_MULTINT].units2 = "meters";
  ErrorLookup[LOC_MULTINT].units3 = "meters";
  ErrorLookup[LOC_MULTINT].tol_desc1 = "Number of intersections";
  ErrorLookup[LOC_MULTINT].tol_desc2 = "Z exclusion lower limit";
  ErrorLookup[LOC_MULTINT].tol_desc3 = "Z exclusion upper limit";


  
  
  
  ErrorLookup[L2D_L3D_MATCH].mygroup = ZVAL_GROUP;
  ErrorLookup[L2D_L3D_MATCH].numthresholds = 0;
  ErrorLookup[L2D_L3D_MATCH].usemagnitude = 0;
  ErrorLookup[L2D_L3D_MATCH].participants = 2;
  
  
  ErrorLookup[LEZ_PROX_3D].mygroup = ZVAL_GROUP;
  ErrorLookup[LEZ_PROX_3D].numthresholds = 3;
  ErrorLookup[LEZ_PROX_3D].usemagnitude = 1;
  ErrorLookup[LEZ_PROX_3D].lowrange = 0.0;
  ErrorLookup[LEZ_PROX_3D].highrange = 100000.0;
  ErrorLookup[LEZ_PROX_3D].lowrange2 = 0.0;
  ErrorLookup[LEZ_PROX_3D].highrange2 = 1000000.0;
  ErrorLookup[LEZ_PROX_3D].lowrange3 = 0.0;
  ErrorLookup[LEZ_PROX_3D].highrange3 = 1000000.0;
  ErrorLookup[LEZ_PROX_3D].participants = 2;
  ErrorLookup[LEZ_PROX_3D].tol_desc1 = "Z Coordinate equality limit";
  ErrorLookup[LEZ_PROX_3D].units = "meters";
  ErrorLookup[LEZ_PROX_3D].units2 = "meters";
  ErrorLookup[LEZ_PROX_3D].units3 = "meters";
  ErrorLookup[LEZ_PROX_3D].tol_desc1 = "Coordinate Z value difference";
  ErrorLookup[LEZ_PROX_3D].tol_desc2 = "Z exclusion lower limit";
  ErrorLookup[LEZ_PROX_3D].tol_desc3 = "Z exclusion upper limit";


  ErrorLookup[CNODE_ZBUST].mygroup = ZVAL_GROUP;
  ErrorLookup[CNODE_ZBUST].numthresholds = 4;
  ErrorLookup[CNODE_ZBUST].usemagnitude = 1;
  ErrorLookup[CNODE_ZBUST].lowrange = 0.0;
  ErrorLookup[CNODE_ZBUST].highrange = 1.0;
  ErrorLookup[CNODE_ZBUST].lowrange2 = 0.0;
  ErrorLookup[CNODE_ZBUST].highrange2 = 10.0;
  ErrorLookup[CNODE_ZBUST].lowrange3 = 0.0;
  ErrorLookup[CNODE_ZBUST].highrange3 = 1000000.0;
  ErrorLookup[CNODE_ZBUST].lowrange4 = 0.0;
  ErrorLookup[CNODE_ZBUST].highrange4 = 1000000.0;
  ErrorLookup[CNODE_ZBUST].participants = 3;
  ErrorLookup[CNODE_ZBUST].range = 0;
  ErrorLookup[CNODE_ZBUST].units = "meters";
  ErrorLookup[CNODE_ZBUST].units2 = "meters";
  ErrorLookup[CNODE_ZBUST].units3 = "meters";
  ErrorLookup[CNODE_ZBUST].units4 = "meters";
  ErrorLookup[CNODE_ZBUST].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[CNODE_ZBUST].tol_desc2 = "Z Coordinate equality limit";
  ErrorLookup[CNODE_ZBUST].tol_desc3 = "Z exclusion lower limit";
  ErrorLookup[CNODE_ZBUST].tol_desc4 = "Z exclusion upper limit";

  ErrorLookup[DUPLICATESEG].mygroup = OVERLAP_GROUP;
  ErrorLookup[DUPLICATESEG].numthresholds = 1;
  ErrorLookup[DUPLICATESEG].usemagnitude = 2;
  ErrorLookup[DUPLICATESEG].lowrange = 0.0;
  ErrorLookup[DUPLICATESEG].highrange = 100000.0;
  ErrorLookup[DUPLICATESEG].participants = 2;
  ErrorLookup[DUPLICATESEG].tol_desc1 = "Number of duplicated segments limit";

  
  ErrorLookup[SHAREPERIM].mygroup = OVERLAP_GROUP;
  ErrorLookup[SHAREPERIM].numthresholds = 2;
  ErrorLookup[SHAREPERIM].usemagnitude = 2;
  ErrorLookup[SHAREPERIM].lowrange = 0.0;
  ErrorLookup[SHAREPERIM].highrange = 100000.0;
  ErrorLookup[SHAREPERIM].lowrange2 = 0.0;
  ErrorLookup[SHAREPERIM].highrange2 = 50.0;
  ErrorLookup[SHAREPERIM].participants = 2;
  ErrorLookup[SHAREPERIM].tol_desc1 = "Number of shared segments limit";
  ErrorLookup[SHAREPERIM].tol_desc2 = "Coordinate equality limit";
  
  
  ErrorLookup[LVPROX].mygroup = SEPDIST_GROUP;
  ErrorLookup[LVPROX].numthresholds = 2;
  ErrorLookup[LVPROX].usemagnitude = 1;
  ErrorLookup[LVPROX].lowrange2 = 0.00001;
  ErrorLookup[LVPROX].highrange2 = 100.0;
  ErrorLookup[LVPROX].lowrange = 0.0;
  ErrorLookup[LVPROX].highrange = 5.0;
  ErrorLookup[LVPROX].range = 1;
  ErrorLookup[LVPROX].participants = 2;
  ErrorLookup[LVPROX].tol_desc1 = "Coordinate proximity lower limit";
  ErrorLookup[LVPROX].tol_desc2 = "Coordinate proximity upper limit";
  
  
  
  ErrorLookup[PLPROX].mygroup = SEPDIST_GROUP;
  ErrorLookup[PLPROX].numthresholds = 1;
  ErrorLookup[PLPROX].usemagnitude = 1;
  ErrorLookup[PLPROX].lowrange = 0.0;
  ErrorLookup[PLPROX].highrange = 2000.0;
  ErrorLookup[PLPROX].range = 1;
  ErrorLookup[PLPROX].participants = 3;
  ErrorLookup[PLPROX].tol_desc1 = "Coordinate proximity limit";

  ErrorLookup[PSHOOTL].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[PSHOOTL].numthresholds = 2;
  ErrorLookup[PSHOOTL].usemagnitude = 1;
  ErrorLookup[PSHOOTL].lowrange = 0.0;
  ErrorLookup[PSHOOTL].highrange = 20.0;
  ErrorLookup[PSHOOTL].lowrange2 = 0.1;
  ErrorLookup[PSHOOTL].highrange2 = 200.0;
  ErrorLookup[PSHOOTL].range = 1;
  ErrorLookup[PSHOOTL].participants = 2;
  ErrorLookup[PSHOOTL].tol_desc1 = "Coordinate proximity lower limit";
  ErrorLookup[PSHOOTL].tol_desc2 = "Coordinate proximity upper limit";


  ErrorLookup[PLPROXEX].mygroup = SEPDIST_GROUP;
  ErrorLookup[PLPROXEX].numthresholds = 1;
  ErrorLookup[PLPROXEX].usemagnitude = 1;
  ErrorLookup[PLPROXEX].lowrange = 0.0;
  ErrorLookup[PLPROXEX].highrange = 500.0;
  ErrorLookup[PLPROXEX].participants = 2;
  ErrorLookup[PLPROXEX].tol_desc1 = "Coordinate proximity limit";
  
  
  
  
  ErrorLookup[PTPTPROX].mygroup = SEPDIST_GROUP;
  ErrorLookup[PTPTPROX].numthresholds = 1;
  ErrorLookup[PTPTPROX].usemagnitude = 1;
  ErrorLookup[PTPTPROX].lowrange = 0.00001;
  ErrorLookup[PTPTPROX].highrange = 2000.0;
  ErrorLookup[PTPTPROX].participants = 2;
  ErrorLookup[PTPTPROX].tol_desc1 = "Coordinate proximity limit";

  ErrorLookup[PUNDERSHTA].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[PUNDERSHTA].numthresholds = 1;
  ErrorLookup[PUNDERSHTA].usemagnitude = 1;
  ErrorLookup[PUNDERSHTA].lowrange = 0.00001;
  ErrorLookup[PUNDERSHTA].highrange = 1000.0;
  ErrorLookup[PUNDERSHTA].participants = 2;
  ErrorLookup[PUNDERSHTA].tol_desc1 = "Coordinate proximity limit";

  ErrorLookup[POVERSHTA].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[POVERSHTA].numthresholds = 1;
  ErrorLookup[POVERSHTA].usemagnitude = 1;
  ErrorLookup[POVERSHTA].lowrange = 0.00001;
  ErrorLookup[POVERSHTA].highrange = 1000.0;
  ErrorLookup[POVERSHTA].participants = 2;
  ErrorLookup[POVERSHTA].tol_desc1 = "Coordinate proximity limit";
  
  
  
  ErrorLookup[PLPFAIL].mygroup = SEPDIST_GROUP;
  ErrorLookup[PLPFAIL].numthresholds = 1;
  ErrorLookup[PLPFAIL].usemagnitude = 0;
  ErrorLookup[PLPFAIL].lowrange = 0.0;
  ErrorLookup[PLPFAIL].highrange = 40000.0;
  ErrorLookup[PLPFAIL].participants = 2;
  ErrorLookup[PLPFAIL].tol_desc1 = "Coordinate equality limit";
  
  ErrorLookup[PNOCOVERLE].mygroup = OVERLAP_GROUP;
  ErrorLookup[PNOCOVERLE].numthresholds = 1;
  ErrorLookup[PNOCOVERLE].usemagnitude = 0;
  ErrorLookup[PNOCOVERLE].lowrange = 0.0;
  ErrorLookup[PNOCOVERLE].highrange = 1000.0;
  ErrorLookup[PNOCOVERLE].participants = 2;
  ErrorLookup[PNOCOVERLE].tol_desc1 = "Coordinate equality limit";


  ErrorLookup[PNOCOV2LEA].mygroup = OVERLAP_GROUP;
  ErrorLookup[PNOCOV2LEA].numthresholds = 1;
  ErrorLookup[PNOCOV2LEA].usemagnitude = 0;
  ErrorLookup[PNOCOV2LEA].lowrange = 0.0;
  ErrorLookup[PNOCOV2LEA].highrange = 50.0;
  ErrorLookup[PNOCOV2LEA].participants = 2;
  ErrorLookup[PNOCOV2LEA].tol_desc1 = "Coordinate equality limit";
  
  ErrorLookup[PNOCOVERLV].mygroup = OVERLAP_GROUP;
  ErrorLookup[PNOCOVERLV].numthresholds = 1;
  ErrorLookup[PNOCOVERLV].usemagnitude = 0;
  ErrorLookup[PNOCOVERLV].lowrange = 0.0;
  ErrorLookup[PNOCOVERLV].highrange = 1000.0;
  ErrorLookup[PNOCOVERLV].participants = 2;
  ErrorLookup[PNOCOVERLV].tol_desc1 = "Coordinate equality limit";
  
  ErrorLookup[PLLPROXFAIL].mygroup = SEPDIST_GROUP;
  ErrorLookup[PLLPROXFAIL].numthresholds = 1;
  ErrorLookup[PLLPROXFAIL].usemagnitude = 0;
  ErrorLookup[PLLPROXFAIL].lowrange = 0.0;
  ErrorLookup[PLLPROXFAIL].highrange = 2000.0;
  ErrorLookup[PLLPROXFAIL].participants = 3;
  ErrorLookup[PLLPROXFAIL].tol_desc1 = "Coordinate equality limit";
  
  ErrorLookup[CONNECTFAIL].mygroup = CONNECT_GROUP;
  ErrorLookup[CONNECTFAIL].numthresholds = 1;
  ErrorLookup[CONNECTFAIL].usemagnitude = 0;
  ErrorLookup[CONNECTFAIL].lowrange = 0.0;
  ErrorLookup[CONNECTFAIL].highrange = 30.0;
  ErrorLookup[CONNECTFAIL].participants = 2;
  ErrorLookup[CONNECTFAIL].checkapplies = (char) GAITcheck;
  ErrorLookup[CONNECTFAIL].draw_wholeareal = 1;
  ErrorLookup[CONNECTFAIL].tol_desc1 = "Coordinate equality limit";

  ErrorLookup[FEATOUTSIDE].mygroup = CONTAIN_GROUP;
  ErrorLookup[FEATOUTSIDE].numthresholds = 1;
  ErrorLookup[FEATOUTSIDE].lowrange = 0.0;
  ErrorLookup[FEATOUTSIDE].highrange = 10.0;
  ErrorLookup[FEATOUTSIDE].usemagnitude = 1;
  ErrorLookup[FEATOUTSIDE].participants = 1;
  ErrorLookup[FEATOUTSIDE].checkapplies = (char) GAITcheck;
  ErrorLookup[FEATOUTSIDE].tol_desc1 = "Coordinate proximity limit";

  ErrorLookup[OSIDE_LON].mygroup = CONTAIN_GROUP;
  ErrorLookup[OSIDE_LON].numthresholds = 3;
  ErrorLookup[OSIDE_LON].lowrange = -180.0;
  ErrorLookup[OSIDE_LON].highrange = 180.0;
  ErrorLookup[OSIDE_LON].lowrange2 = -180.0;
  ErrorLookup[OSIDE_LON].highrange2 = 180.0;
  ErrorLookup[OSIDE_LON].lowrange3 = 0.0;
  ErrorLookup[OSIDE_LON].highrange3 = 100.0;
  ErrorLookup[OSIDE_LON].usemagnitude = 1;
  ErrorLookup[OSIDE_LON].participants = 1;
  ErrorLookup[OSIDE_LON].range = 1;
  ErrorLookup[OSIDE_LON].checkapplies = (char) GAITcheck;
  ErrorLookup[OSIDE_LON].tol_desc1 = "Feature coordinate lower limit";
  ErrorLookup[OSIDE_LON].tol_desc2 = "Feature coordinate upper limit";
  ErrorLookup[OSIDE_LON].tol_desc3 =  "Minimum overshoot distance";

  ErrorLookup[LBNDUSHT].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[LBNDUSHT].numthresholds = 2;
  ErrorLookup[LBNDUSHT].lowrange = 0.0;
  ErrorLookup[LBNDUSHT].highrange = 10.0;
  ErrorLookup[LBNDUSHT].lowrange2 = 0.00001;
  ErrorLookup[LBNDUSHT].highrange2 = 100.0;
  ErrorLookup[LBNDUSHT].usemagnitude = 1;
  ErrorLookup[LBNDUSHT].participants = 2;
  ErrorLookup[LBNDUSHT].checkapplies = (char) GAITcheck;
  ErrorLookup[LBNDUSHT].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[LBNDUSHT].tol_desc2 = "Boundary proximity limit";

  ErrorLookup[BNDRYUNDERSHT].mygroup = UNDERSHOOT_GROUP;
  ErrorLookup[BNDRYUNDERSHT].numthresholds = 2;
  ErrorLookup[BNDRYUNDERSHT].lowrange = 0.0;
  ErrorLookup[BNDRYUNDERSHT].highrange = 10.0;
  ErrorLookup[BNDRYUNDERSHT].lowrange2 = 0.00001;
  ErrorLookup[BNDRYUNDERSHT].highrange2 = 100.0;
  ErrorLookup[BNDRYUNDERSHT].usemagnitude = 1;
  ErrorLookup[BNDRYUNDERSHT].participants = 1;
  ErrorLookup[BNDRYUNDERSHT].checkapplies = (char) GAITcheck;
  ErrorLookup[BNDRYUNDERSHT].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[BNDRYUNDERSHT].tol_desc2 = "Boundary proximity limit";


  ErrorLookup[OSIDE_LAT].mygroup = CONTAIN_GROUP;
  ErrorLookup[OSIDE_LAT].numthresholds = 3;
  ErrorLookup[OSIDE_LAT].lowrange = -90.0;
  ErrorLookup[OSIDE_LAT].highrange = 90.0;
  ErrorLookup[OSIDE_LAT].lowrange2 = -90.0;
  ErrorLookup[OSIDE_LAT].highrange2 = 90.0;
  ErrorLookup[OSIDE_LAT].lowrange3 = 0.0;
  ErrorLookup[OSIDE_LAT].highrange3 = 100.0;
  ErrorLookup[OSIDE_LAT].usemagnitude = 1;
  ErrorLookup[OSIDE_LAT].participants = 1;
  ErrorLookup[OSIDE_LAT].range = 1;
  ErrorLookup[OSIDE_LAT].checkapplies = (char) GAITcheck;
  ErrorLookup[OSIDE_LAT].tol_desc1 = "Feature coordinate lower limit";
  ErrorLookup[OSIDE_LAT].tol_desc2 = "Feature coordinate upper limit";
  ErrorLookup[OSIDE_LAT].tol_desc3 =  "Minimum overshoot distance";

  ErrorLookup[ENCONFAIL].mygroup = CONNECT_GROUP;
  ErrorLookup[ENCONFAIL].numthresholds = 1;
  ErrorLookup[ENCONFAIL].usemagnitude = 0;
  ErrorLookup[ENCONFAIL].lowrange = 0.0;
  ErrorLookup[ENCONFAIL].highrange = 30.0;
  ErrorLookup[ENCONFAIL].participants = 2;
  ErrorLookup[ENCONFAIL].tol_desc1 = "Coordinate equality limit";


  ErrorLookup[NOENDCON].mygroup = CONNECT_GROUP;
  ErrorLookup[NOENDCON].numthresholds = 1;
  ErrorLookup[NOENDCON].usemagnitude = 0;
  ErrorLookup[NOENDCON].lowrange = 0.0;
  ErrorLookup[NOENDCON].highrange = 30.0;
  ErrorLookup[NOENDCON].participants = 3;
  ErrorLookup[NOENDCON].tol_desc1 = "Coordinate equality limit";

  ErrorLookup[BOTHENDCON].mygroup = OVERLAP_GROUP;
  ErrorLookup[BOTHENDCON].numthresholds = 1;
  ErrorLookup[BOTHENDCON].usemagnitude = 0;
  ErrorLookup[BOTHENDCON].lowrange = 0.0;
  ErrorLookup[BOTHENDCON].highrange = 30.0;
  ErrorLookup[BOTHENDCON].participants = 2;
  ErrorLookup[BOTHENDCON].tol_desc1 = "Coordinate equality limit";



  ErrorLookup[LENOCOVERL].mygroup = OVERLAP_GROUP;
  ErrorLookup[LENOCOVERL].numthresholds = 1;
  ErrorLookup[LENOCOVERL].usemagnitude = 0;
  ErrorLookup[LENOCOVERL].lowrange = 0.0;
  ErrorLookup[LENOCOVERL].highrange = 30.0;
  ErrorLookup[LENOCOVERL].participants = 3;
  ErrorLookup[LENOCOVERL].tol_desc1 = "Coordinate equality limit";

  ErrorLookup[NOLCOVLE].mygroup = OVERLAP_GROUP;
  ErrorLookup[NOLCOVLE].numthresholds = 1;
  ErrorLookup[NOLCOVLE].usemagnitude = 0;
  ErrorLookup[NOLCOVLE].lowrange = 0.0;
  ErrorLookup[NOLCOVLE].highrange = 30.0;
  ErrorLookup[NOLCOVLE].participants = 2;
  ErrorLookup[NOLCOVLE].tol_desc1 = "Coordinate equality limit";


  ErrorLookup[FEATNOTCUT].mygroup = CONNECT_GROUP;
  ErrorLookup[FEATNOTCUT].numthresholds = 1;
  ErrorLookup[FEATNOTCUT].usemagnitude = 0;
  ErrorLookup[FEATNOTCUT].lowrange = 0.0;
  ErrorLookup[FEATNOTCUT].highrange = 10.0;
  ErrorLookup[FEATNOTCUT].participants = 2;
  ErrorLookup[FEATNOTCUT].tol_desc1 = "Coordinate equality limit";
  
  
  ErrorLookup[ATTRERR].checkapplies = (char) SEEITcheck; 
  ErrorLookup[ATTRERR].participants = 1;
  ErrorLookup[ATTRERR].mygroup = ATTR_GROUP;

  ErrorLookup[RPTD_ATTR].participants = 1;
  ErrorLookup[RPTD_ATTR].mygroup = ATTR_GROUP;
  ErrorLookup[RPTD_ATTR].checkapplies = (char) GAITcheck;
  ErrorLookup[RPTD_ATTR].draw_wholeareal = 1;

  
  ErrorLookup[ATTR_PAIR].participants = 1;
  ErrorLookup[ATTR_PAIR].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_PAIR].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_PAIR].draw_wholeareal = 1;
  
  ErrorLookup[ATTR_UNEXP].participants = 1;
  ErrorLookup[ATTR_UNEXP].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_UNEXP].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_UNEXP].draw_wholeareal = 1;

  ErrorLookup[ATTR_RNULL].participants = 1;
  ErrorLookup[ATTR_RNULL].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_RNULL].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_RNULL].draw_wholeareal = 1;

  ErrorLookup[ATTR_VVT].participants = 1;
  ErrorLookup[ATTR_VVT].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_VVT].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_VVT].draw_wholeareal = 1;

  ErrorLookup[ATTR_MISSING].participants = 1;
  ErrorLookup[ATTR_MISSING].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_MISSING].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_MISSING].draw_wholeareal = 1;
  
  ErrorLookup[ATTR_DT].participants = 1;
  ErrorLookup[ATTR_DT].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_DT].checkapplies = (char) SEEITcheck;
  ErrorLookup[ATTR_DT].draw_wholeareal = 1;
  
  
  ErrorLookup[ATTR_RNG].participants = 1;
  ErrorLookup[ATTR_RNG].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_RNG].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_RNG].draw_wholeareal = 1;
  
  
  ErrorLookup[ATTR_PICK].participants = 1;
  ErrorLookup[ATTR_PICK].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_PICK].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_PICK].draw_wholeareal = 1;
  
  ErrorLookup[ATTR_META].participants = 1;
  ErrorLookup[ATTR_META].mygroup = ATTR_GROUP;
  ErrorLookup[ATTR_META].checkapplies = (char) GAITcheck;
  ErrorLookup[ATTR_META].draw_wholeareal = 1;
  
  
  ErrorLookup[VVTERR1WAY].participants = 1;
  ErrorLookup[VVTERR1WAY].mygroup = ATTR_GROUP;
  ErrorLookup[VVTERR1WAY].draw_wholeareal = 1;

  ErrorLookup[VVTERR2WAY].participants = 2;
  ErrorLookup[VVTERR2WAY].mygroup = ATTR_GROUP;
  ErrorLookup[VVTERR2WAY].draw_wholeareal = 1;

  ErrorLookup[VVTERR3WAY].participants = 3;
  ErrorLookup[VVTERR3WAY].mygroup = ATTR_GROUP;
  ErrorLookup[VVTERR3WAY].draw_wholeareal = 1;


  ErrorLookup[HIGHLIGHTED].mygroup = ATTR_GROUP;
  ErrorLookup[HIGHLIGHTED].participants = 1;
  ErrorLookup[HIGHLIGHTED].draw_wholeareal = 1;

  
  
  ErrorLookup[V_DUPS].mygroup = DUP_GROUP;
  ErrorLookup[V_DUPS].participants = 1;
  ErrorLookup[V_DUPS].numthresholds = 0;
  ErrorLookup[V_DUPS].usemagnitude = 0;
  ErrorLookup[V_DUPS].draw_wholeareal = 0;
  
  
  ErrorLookup[ISOTURN].mygroup = KINK_GROUP;
  ErrorLookup[ISOTURN].numthresholds = 2;
  ErrorLookup[ISOTURN].usemagnitude = 1;
  ErrorLookup[ISOTURN].lowrange = 0.0;
  ErrorLookup[ISOTURN].highrange = 180.0;
  ErrorLookup[ISOTURN].lowrange2 = 0.0;
  ErrorLookup[ISOTURN].highrange2 = 10.0;
  ErrorLookup[ISOTURN].participants = 3;
  ErrorLookup[ISOTURN].range = 0;
  ErrorLookup[ISOTURN].tol_desc1 = "Angle limit";
  ErrorLookup[ISOTURN].tol_desc2 = "Feature proximity limit";
  
  
  ErrorLookup[KINK].mygroup = KINK_GROUP;
  ErrorLookup[KINK].numthresholds = 1;
  ErrorLookup[KINK].usemagnitude = 1;
  ErrorLookup[KINK].lowrange = 1.0;
  ErrorLookup[KINK].highrange = 180.0;
  if(NGA_TYPE == 1)
    {
      ErrorLookup[KINK].participants = 1;
    }
  else
    {
      ErrorLookup[KINK].participants = 2;
    }
  ErrorLookup[KINK].tol_desc1 = "Angle limit";

  

  ErrorLookup[Z_KINK].mygroup = KINK_GROUP;
  ErrorLookup[Z_KINK].numthresholds = 2;
  ErrorLookup[Z_KINK].usemagnitude = 1;
  ErrorLookup[Z_KINK].lowrange = 46.0;
  ErrorLookup[Z_KINK].highrange = 180.0;
  ErrorLookup[Z_KINK].lowrange2 = 46.0;
  ErrorLookup[Z_KINK].highrange2 = 180.0;
  ErrorLookup[Z_KINK].participants = 1;
  ErrorLookup[Z_KINK].range = 1;
  ErrorLookup[Z_KINK].tol_desc1 = "Angle lower limit";
  ErrorLookup[Z_KINK].tol_desc2 = "Angle upper limit";

  
  ErrorLookup[L_A_KINK].mygroup = KINK_GROUP;
  ErrorLookup[L_A_KINK].numthresholds = 1;
  ErrorLookup[L_A_KINK].usemagnitude = 1;
  ErrorLookup[L_A_KINK].lowrange = 1.0;
  ErrorLookup[L_A_KINK].highrange = 180.0;
  ErrorLookup[L_A_KINK].checkapplies = (char) GAITcheck;
  ErrorLookup[L_A_KINK].participants = 2;
  ErrorLookup[L_A_KINK].tol_desc1 = "Angle limit";

  
  ErrorLookup[INTERNALKINK].mygroup = KINK_GROUP;
  ErrorLookup[INTERNALKINK].numthresholds = 2;
  ErrorLookup[INTERNALKINK].usemagnitude = 1;
  ErrorLookup[INTERNALKINK].lowrange = 46.0;
  ErrorLookup[INTERNALKINK].highrange = 180.0;
  ErrorLookup[INTERNALKINK].lowrange2 = 46.0;
  ErrorLookup[INTERNALKINK].highrange2 = 180.0;
  ErrorLookup[INTERNALKINK].checkapplies = (char) GAITcheck;
  ErrorLookup[INTERNALKINK].participants = 1;
  ErrorLookup[INTERNALKINK].range = 1;
  ErrorLookup[INTERNALKINK].tol_desc1 = "Angle lower limit";
  ErrorLookup[INTERNALKINK].tol_desc2 = "Angle upper limit";

  ErrorLookup[CONTEXT_KINK].mygroup = KINK_GROUP;
  ErrorLookup[CONTEXT_KINK].numthresholds = 2;
  ErrorLookup[CONTEXT_KINK].usemagnitude = 1;
  ErrorLookup[CONTEXT_KINK].lowrange = 46.0;
  ErrorLookup[CONTEXT_KINK].highrange = 180.0;
  ErrorLookup[CONTEXT_KINK].lowrange2 = 46.0;
  ErrorLookup[CONTEXT_KINK].highrange2 = 180.0;
  ErrorLookup[CONTEXT_KINK].checkapplies = (char) GAITcheck;
  ErrorLookup[CONTEXT_KINK].participants = 1;
  ErrorLookup[CONTEXT_KINK].range = 1;
  ErrorLookup[CONTEXT_KINK].tol_desc1 = "Angle lower limit";
  ErrorLookup[CONTEXT_KINK].tol_desc2 = "Angle upper limit";

  
  
  ErrorLookup[KICKBACK].mygroup = KINK_GROUP;
  ErrorLookup[KICKBACK].usemagnitude = 0;
  if(NGA_TYPE == 1)
    {
      ErrorLookup[KICKBACK].participants = 1;
    }
  else
    {
      ErrorLookup[KICKBACK].participants = 2;
    }
  
  
  
  ErrorLookup[AREAKINK].mygroup = KINK_GROUP;
  ErrorLookup[AREAKINK].numthresholds = 1;
  ErrorLookup[AREAKINK].usemagnitude = 1;
  ErrorLookup[AREAKINK].lowrange = 1.0;
  ErrorLookup[AREAKINK].highrange = 180.0;
  ErrorLookup[AREAKINK].participants = 1;
  ErrorLookup[AREAKINK].tol_desc1 = "Angle limit";
  
  ErrorLookup[INCLSLIVER].mygroup = COMPO_GROUP;
  ErrorLookup[INCLSLIVER].numthresholds = 1;
  ErrorLookup[INCLSLIVER].usemagnitude = 1;
  ErrorLookup[INCLSLIVER].lowrange = 0.0;
  ErrorLookup[INCLSLIVER].highrange = 1.0;
  ErrorLookup[INCLSLIVER].participants = 1;
  ErrorLookup[INCLSLIVER].tol_desc1 = "Area to perimeter ratio limit";
  
  
  
  ErrorLookup[SEGLEN].mygroup = SIZE_GROUP;
  ErrorLookup[SEGLEN].numthresholds = 1;
  ErrorLookup[SEGLEN].usemagnitude = 1;
  ErrorLookup[SEGLEN].lowrange = 0.0;
  ErrorLookup[SEGLEN].highrange = 1000.0;
  ErrorLookup[SEGLEN].participants = 1;
  ErrorLookup[SEGLEN].tol_desc1 = "Segment length limit";

  ErrorLookup[LONGSEG].mygroup = SIZE_GROUP;
  ErrorLookup[LONGSEG].numthresholds = 1;
  ErrorLookup[LONGSEG].usemagnitude = 1;
  ErrorLookup[LONGSEG].lowrange = 10.0;
  ErrorLookup[LONGSEG].highrange = 100000.0;
  ErrorLookup[LONGSEG].participants = 1;
  ErrorLookup[LONGSEG].tol_desc1 = "Segment length limit";


  ErrorLookup[FEATBRIDGE].mygroup = CONNECT_GROUP;
  ErrorLookup[FEATBRIDGE].numthresholds = 1;
  ErrorLookup[FEATBRIDGE].usemagnitude = 1;
  ErrorLookup[FEATBRIDGE].lowrange = 0.0;
  ErrorLookup[FEATBRIDGE].highrange = 100.0;
  ErrorLookup[FEATBRIDGE].participants = 3;
  ErrorLookup[FEATBRIDGE].tol_desc1 = "Feature length limit";
  
  ErrorLookup[SMALLAREA].mygroup = SIZE_GROUP;
  ErrorLookup[SMALLAREA].numthresholds = 1;
  ErrorLookup[SMALLAREA].usemagnitude = 1;
  ErrorLookup[SMALLAREA].lowrange = 0.0;
  ErrorLookup[SMALLAREA].highrange = 1000000.0;
  ErrorLookup[SMALLAREA].participants = 1;
  ErrorLookup[SMALLAREA].draw_wholeareal = 1;
  ErrorLookup[SMALLAREA].checkapplies = (char) GAITcheck;
  ErrorLookup[SMALLAREA].tol_desc1 = "Feature square area limit";
  
  ErrorLookup[SMLCUTOUT].mygroup = SIZE_GROUP;
  ErrorLookup[SMLCUTOUT].numthresholds = 1;
  ErrorLookup[SMLCUTOUT].usemagnitude = 1;
  ErrorLookup[SMLCUTOUT].lowrange = 0.0;
  ErrorLookup[SMLCUTOUT].highrange = 1000000.0;
  ErrorLookup[SMLCUTOUT].participants = 1;
  ErrorLookup[SMLCUTOUT].draw_wholeareal = 1;
  ErrorLookup[SMLCUTOUT].checkapplies = (char) GAITcheck;
  ErrorLookup[SMLCUTOUT].tol_desc1 = "Feature square area limit";
  
  ErrorLookup[BIGAREA].mygroup = SIZE_GROUP;
  ErrorLookup[BIGAREA].numthresholds = 1;
  ErrorLookup[BIGAREA].usemagnitude = 1;
  ErrorLookup[BIGAREA].lowrange = 0.0;
  ErrorLookup[BIGAREA].highrange = 10000000.0;
  ErrorLookup[BIGAREA].participants = 1;
  ErrorLookup[BIGAREA].draw_wholeareal = 1;
  ErrorLookup[BIGAREA].tol_desc1 = "Feature square area limit";
  

  ErrorLookup[NOT_FLAT].mygroup = ZVAL_GROUP;
  ErrorLookup[NOT_FLAT].numthresholds = 1;
  ErrorLookup[NOT_FLAT].usemagnitude = 1;
  ErrorLookup[NOT_FLAT].lowrange = 0.0;
  ErrorLookup[NOT_FLAT].highrange = 1000.0;
  ErrorLookup[NOT_FLAT].participants = 1;
  ErrorLookup[NOT_FLAT].draw_wholeareal = 0;
  ErrorLookup[NOT_FLAT].tol_desc1 = "Elevation value difference limit";


  ErrorLookup[CLAMP_NFLAT].mygroup = NEW_DEM_GROUP;
  ErrorLookup[CLAMP_NFLAT].numthresholds = 1;
  ErrorLookup[CLAMP_NFLAT].usemagnitude = 1;
  ErrorLookup[CLAMP_NFLAT].lowrange = 0.0;
  ErrorLookup[CLAMP_NFLAT].highrange = 1000.0;
  ErrorLookup[CLAMP_NFLAT].participants = 2;
  ErrorLookup[CLAMP_NFLAT].draw_wholeareal = 0;
  ErrorLookup[CLAMP_NFLAT].tol_desc1 = "Clamped elevation value difference limit";

  ErrorLookup[CLAMP_DIF].mygroup = NEW_DEM_GROUP;
  ErrorLookup[CLAMP_DIF].numthresholds = 4;
  ErrorLookup[CLAMP_DIF].usemagnitude = 1;
  ErrorLookup[CLAMP_DIF].lowrange = 0.0;
  ErrorLookup[CLAMP_DIF].highrange = 1000000.0;
  ErrorLookup[CLAMP_DIF].lowrange2 = -9999999.0;
  ErrorLookup[CLAMP_DIF].highrange2 = 1000000.0;
  ErrorLookup[CLAMP_DIF].lowrange3 = -9999999.0;
  ErrorLookup[CLAMP_DIF].highrange3 = 1000000.0;
  ErrorLookup[CLAMP_DIF].lowrange4 = 0.0;
  ErrorLookup[CLAMP_DIF].highrange4 = 1.0;
  ErrorLookup[CLAMP_DIF].participants = 2;
  ErrorLookup[CLAMP_DIF].draw_wholeareal = 0;
  ErrorLookup[CLAMP_DIF].units = "meters";
  ErrorLookup[CLAMP_DIF].units2 = "meters";
  ErrorLookup[CLAMP_DIF].units3 = "meters";
  ErrorLookup[CLAMP_DIF].units4 = "Boolean";
  ErrorLookup[CLAMP_DIF].tol_desc1 = "DEM interpolated value - Z difference";
  ErrorLookup[CLAMP_DIF].tol_desc2 = "Ignore vertex values below";
  ErrorLookup[CLAMP_DIF].tol_desc3 = "Ignore DEM values below";
  ErrorLookup[CLAMP_DIF].tol_desc4 = "Apply along segments between vertices";
  ErrorLookup[CLAMP_DIF].checkapplies = (char) GAITcheck;



  ErrorLookup[ZUNCLOSED].mygroup = ZVAL_GROUP;
  ErrorLookup[ZUNCLOSED].numthresholds = 1;
  ErrorLookup[ZUNCLOSED].usemagnitude = 1;
  ErrorLookup[ZUNCLOSED].lowrange = 0.0;
  ErrorLookup[ZUNCLOSED].highrange = 1000.0;
  ErrorLookup[ZUNCLOSED].participants = 1;
  ErrorLookup[ZUNCLOSED].checkapplies = (char) GAITcheck;
  ErrorLookup[ZUNCLOSED].tol_desc1 = "Elevation value difference limit";
  
  ErrorLookup[AREAUNCLOSED].mygroup = COMPO_GROUP;
  ErrorLookup[AREAUNCLOSED].numthresholds = 1;
  ErrorLookup[AREAUNCLOSED].usemagnitude = 1;
  ErrorLookup[AREAUNCLOSED].lowrange = 0.0;
  ErrorLookup[AREAUNCLOSED].highrange = 1000.0;
  ErrorLookup[AREAUNCLOSED].participants = 1;
  ErrorLookup[AREAUNCLOSED].draw_wholeareal = 1;
  ErrorLookup[AREAUNCLOSED].checkapplies = (char) GAITcheck;
  ErrorLookup[AREAUNCLOSED].tol_desc1 = "3D coordinate difference limit";
  
  ErrorLookup[MULTIPARTL].mygroup = COMPO_GROUP;
  ErrorLookup[MULTIPARTL].checkapplies = (char) GAITcheck;
  ErrorLookup[MULTIPARTL].numthresholds = 1;
  ErrorLookup[MULTIPARTL].usemagnitude = 2;
  ErrorLookup[MULTIPARTL].lowrange = 0.0;
  ErrorLookup[MULTIPARTL].highrange = 1000.0;
  ErrorLookup[MULTIPARTL].participants = 1;
  ErrorLookup[MULTIPARTL].draw_wholeareal = 1;
  ErrorLookup[MULTIPARTL].tol_desc1 = "Number of parts limit";
  
  
  ErrorLookup[MULTIPARTA].mygroup = COMPO_GROUP;
  ErrorLookup[MULTIPARTA].checkapplies = (char) GAITcheck;
  ErrorLookup[MULTIPARTA].numthresholds = 1;
  ErrorLookup[MULTIPARTA].usemagnitude = 2;
  ErrorLookup[MULTIPARTA].lowrange = 0.0;
  ErrorLookup[MULTIPARTA].highrange = 1000.0;
  ErrorLookup[MULTIPARTA].participants = 1;
  ErrorLookup[MULTIPARTA].draw_wholeareal = 1;
  ErrorLookup[MULTIPARTA].tol_desc1 = "Number of parts limit";
  
  ErrorLookup[MULTIPARTP].mygroup = COMPO_GROUP;
  ErrorLookup[MULTIPARTP].checkapplies = (char) GAITcheck;
  ErrorLookup[MULTIPARTP].numthresholds = 1;
  ErrorLookup[MULTIPARTP].usemagnitude = 2;
  ErrorLookup[MULTIPARTP].lowrange = 0.0;
  ErrorLookup[MULTIPARTP].highrange = 1000.0;
  ErrorLookup[MULTIPARTP].participants = 1;
  ErrorLookup[MULTIPARTP].tol_desc1 = "Number of parts limit";
  
  
  ErrorLookup[PERIMLEN].mygroup = SIZE_GROUP;
  ErrorLookup[PERIMLEN].numthresholds = 2;
  ErrorLookup[PERIMLEN].usemagnitude = 1;
  ErrorLookup[PERIMLEN].lowrange = 0.0;
  ErrorLookup[PERIMLEN].highrange = 1000000.0;
  ErrorLookup[PERIMLEN].lowrange2 = 0.0;
  ErrorLookup[PERIMLEN].highrange2 = 10.0;
  ErrorLookup[PERIMLEN].participants = 2;
  ErrorLookup[PERIMLEN].draw_wholeareal = 1;
  ErrorLookup[PERIMLEN].tol_desc1 = "Feature perimeter/length limit";
  ErrorLookup[PERIMLEN].tol_desc2 = "Proximity to second feature";

  ErrorLookup[LONGFEAT].mygroup = SIZE_GROUP;
  ErrorLookup[LONGFEAT].numthresholds = 1;
  ErrorLookup[LONGFEAT].usemagnitude = 1;
  ErrorLookup[LONGFEAT].lowrange = 1.0;
  ErrorLookup[LONGFEAT].highrange = 1000000.0;
  ErrorLookup[LONGFEAT].participants = 1;
  ErrorLookup[LONGFEAT].draw_wholeareal = 1;
  ErrorLookup[LONGFEAT].tol_desc1 = "Feature perimeter/length limit";

  ErrorLookup[SHORTFEAT].mygroup = SIZE_GROUP;
  ErrorLookup[SHORTFEAT].numthresholds = 2;
  ErrorLookup[SHORTFEAT].usemagnitude = 1;
  ErrorLookup[SHORTFEAT].lowrange = 1.0;
  ErrorLookup[SHORTFEAT].highrange = 1000000.0;
  ErrorLookup[SHORTFEAT].lowrange2 = 0.0;
  ErrorLookup[SHORTFEAT].highrange2 = 10.0;
  ErrorLookup[SHORTFEAT].participants = 1;
  ErrorLookup[SHORTFEAT].tol_desc1 = "Feature perimeter/length limit";
  ErrorLookup[SHORTFEAT].tol_desc2 = "Degree line proximity limit";


  ErrorLookup[PC_SLOPE].mygroup = ZVAL_GROUP;
  ErrorLookup[PC_SLOPE].numthresholds = 2;
  ErrorLookup[PC_SLOPE].usemagnitude = 1;
  ErrorLookup[PC_SLOPE].lowrange = 0.0001;
  ErrorLookup[PC_SLOPE].highrange = 500.0;
  ErrorLookup[PC_SLOPE].lowrange2 = 0.00001;
  ErrorLookup[PC_SLOPE].highrange2 = 100000.0;
  ErrorLookup[PC_SLOPE].participants = 1;
  ErrorLookup[PC_SLOPE].tol_desc1 = "Percent Slope limit";
  ErrorLookup[PC_SLOPE].tol_desc2 = "Segment minimum length limit";

  ErrorLookup[CALC_AREA].mygroup = ATTR_GROUP;
  ErrorLookup[CALC_AREA].numthresholds = 1;
  ErrorLookup[CALC_AREA].usemagnitude = 1;
  ErrorLookup[CALC_AREA].lowrange = 1.0;
  ErrorLookup[CALC_AREA].highrange = 1000000.0;
  ErrorLookup[CALC_AREA].participants = 1;
  ErrorLookup[CALC_AREA].tol_desc1 = "Square area limit";


  ErrorLookup[COVERFAIL].checkapplies = (char) GAITcheck;
  ErrorLookup[COVERFAIL].numthresholds = 2;
  ErrorLookup[COVERFAIL].usemagnitude = 2;
  ErrorLookup[COVERFAIL].participants = 1;
  ErrorLookup[COVERFAIL].magdescribe = "Group ID ";
  ErrorLookup[COVERFAIL].lowrange = 0.125;
  ErrorLookup[COVERFAIL].highrange = 1.0;
  ErrorLookup[COVERFAIL].lowrange2 = 0.0;
  ErrorLookup[COVERFAIL].highrange2 = 10.0;
  ErrorLookup[COVERFAIL].tol_desc1 = "Decimal part cell size (0.125, 1.0, ...)";
  ErrorLookup[COVERFAIL].tol_desc2 = "Proximity to cell outer boundary";
  ErrorLookup[COVERFAIL].mygroup = OVERLAP_GROUP;
  
  
  
  
  ErrorLookup[LAINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LAINT].participants = 2;

  ErrorLookup[LODELEVDIF].mygroup = NEW_DEM_GROUP;
  ErrorLookup[LODELEVDIF].numthresholds = 3;
  ErrorLookup[LODELEVDIF].usemagnitude = 1;
  ErrorLookup[LODELEVDIF].lowrange = 0.0;
  ErrorLookup[LODELEVDIF].highrange = 100000.0;
  ErrorLookup[LODELEVDIF].lowrange2 = -21474836467.0;
  ErrorLookup[LODELEVDIF].highrange2 = 21474836467.0;
  ErrorLookup[LODELEVDIF].lowrange3 = -21474836467.0;
  ErrorLookup[LODELEVDIF].highrange3 = 21474836467.0;
  ErrorLookup[LODELEVDIF].participants = 2;
  ErrorLookup[LODELEVDIF].tol_desc1 = "Elevation surface difference limit";
  ErrorLookup[LODELEVDIF].tol_desc2 = "Elevation value to ignore";
  ErrorLookup[LODELEVDIF].tol_desc3 = "Elevation value to ignore";

  ErrorLookup[GRIDEXACTDIF].mygroup = NEW_DEM_GROUP;
  ErrorLookup[GRIDEXACTDIF].numthresholds = 3;
  ErrorLookup[GRIDEXACTDIF].usemagnitude = 1;
  ErrorLookup[GRIDEXACTDIF].lowrange = 0.0;
  ErrorLookup[GRIDEXACTDIF].highrange = 100000.0;
  ErrorLookup[GRIDEXACTDIF].lowrange2 = -21474836467.0;
  ErrorLookup[GRIDEXACTDIF].highrange2 = 21474836467.0;
  ErrorLookup[GRIDEXACTDIF].lowrange3 = -21474836467.0;
  ErrorLookup[GRIDEXACTDIF].highrange3 = 21474836467.0;
  ErrorLookup[GRIDEXACTDIF].participants = 2;
  ErrorLookup[GRIDEXACTDIF].tol_desc1 = "Elevation surface difference limit";
  ErrorLookup[GRIDEXACTDIF].tol_desc2 = "Elevation value to ignore";
  ErrorLookup[GRIDEXACTDIF].tol_desc3 = "Elevation value to ignore";


  ErrorLookup[MASKZERO].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKZERO].numthresholds = 3;
  ErrorLookup[MASKZERO].usemagnitude = 1;
  ErrorLookup[MASKZERO].lowrange = 0.0;
  ErrorLookup[MASKZERO].highrange = 5.0;
  ErrorLookup[MASKZERO].lowrange2 = 0.0;
  ErrorLookup[MASKZERO].highrange2 = 1.0;
  ErrorLookup[MASKZERO].lowrange3 = 0.0;
  ErrorLookup[MASKZERO].highrange3 = 2.0;
  ErrorLookup[MASKZERO].participants = 2;
  ErrorLookup[MASKZERO].tol_desc1 = "Mask post value";
  ErrorLookup[MASKZERO].tol_desc2 = "Elevation value tolerance (+/-)";
  ErrorLookup[MASKZERO].tol_desc3 = "Show points (0) billboards (1) both (2)";
  ErrorLookup[MASKZERO].checkapplies = (char) GAITcheck;

  ErrorLookup[MASKCONSTANT].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKCONSTANT].numthresholds = 3;
  ErrorLookup[MASKCONSTANT].usemagnitude = 1;
  ErrorLookup[MASKCONSTANT].lowrange = 0.0;
  ErrorLookup[MASKCONSTANT].highrange = 5.0;
  ErrorLookup[MASKCONSTANT].lowrange2 = 0.0;
  ErrorLookup[MASKCONSTANT].highrange2 = 1.0;
  ErrorLookup[MASKCONSTANT].lowrange3 = 0.0;
  ErrorLookup[MASKCONSTANT].highrange3 = 2.0;
  ErrorLookup[MASKCONSTANT].participants = 2;
  ErrorLookup[MASKCONSTANT].tol_desc1 = "Mask post value";
  ErrorLookup[MASKCONSTANT].tol_desc2 = "Elevation value tolerance (+/-)";
  ErrorLookup[MASKCONSTANT].tol_desc3 = "Show points (0) billboards (1) both (2)";
  ErrorLookup[MASKCONSTANT].checkapplies = (char) GAITcheck;

  ErrorLookup[KERNELSTATS].mygroup = NEW_DEM_GROUP;
  ErrorLookup[KERNELSTATS].numthresholds = 4;
  ErrorLookup[KERNELSTATS].lowrange = -10000000.0;
  ErrorLookup[KERNELSTATS].highrange = 1000000.0;
  ErrorLookup[KERNELSTATS].lowrange2 = 0.0;
  ErrorLookup[KERNELSTATS].highrange2 = 20.0;
  ErrorLookup[KERNELSTATS].lowrange3 = 0.0;
  ErrorLookup[KERNELSTATS].highrange3 = 20.0;
  ErrorLookup[KERNELSTATS].lowrange4 = 0.0;
  ErrorLookup[KERNELSTATS].highrange4 = 500000.0;
  ErrorLookup[KERNELSTATS].usemagnitude = 0;
  ErrorLookup[KERNELSTATS].participants = 3;
  ErrorLookup[KERNELSTATS].checkapplies = (char) GAITcheck;
  ErrorLookup[KERNELSTATS].tol_desc1 = "Ignore elevations below";
  ErrorLookup[KERNELSTATS].tol_desc2 = "Post value lower (closed) limit";
  ErrorLookup[KERNELSTATS].tol_desc3 = "Post value upper (closed) limit";
  ErrorLookup[KERNELSTATS].tol_desc4 = "Elevation Difference upper limit";

  ErrorLookup[BILINSTATS].mygroup = NEW_DEM_GROUP;
  ErrorLookup[BILINSTATS].numthresholds = 4;
  ErrorLookup[BILINSTATS].lowrange = -10000000.0;
  ErrorLookup[BILINSTATS].highrange = 1000000.0;
  ErrorLookup[BILINSTATS].lowrange2 = 0.0;
  ErrorLookup[BILINSTATS].highrange2 = 20.0;
  ErrorLookup[BILINSTATS].lowrange3 = 0.0;
  ErrorLookup[BILINSTATS].highrange3 = 20.0;
  ErrorLookup[BILINSTATS].lowrange4 = 0.0;
  ErrorLookup[BILINSTATS].highrange4 = 500000.0;
  ErrorLookup[BILINSTATS].usemagnitude = 0;
  ErrorLookup[BILINSTATS].participants = 3;
  ErrorLookup[BILINSTATS].checkapplies = (char) GAITcheck;
  ErrorLookup[BILINSTATS].tol_desc1 = "Ignore elevations below";
  ErrorLookup[BILINSTATS].tol_desc2 = "Post value lower (closed) limit";
  ErrorLookup[BILINSTATS].tol_desc3 = "Post value upper (closed) limit";
  ErrorLookup[BILINSTATS].tol_desc4 = "Elevation Difference upper limit";

  ErrorLookup[MASKEDIT_0].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKEDIT_0].numthresholds = 3;
  ErrorLookup[MASKEDIT_0].lowrange = 0.0;
  ErrorLookup[MASKEDIT_0].highrange = 12.0;
  ErrorLookup[MASKEDIT_0].lowrange2 = 0.0;
  ErrorLookup[MASKEDIT_0].highrange2 = 2.0;
  ErrorLookup[MASKEDIT_0].lowrange3 = 0.0;
  ErrorLookup[MASKEDIT_0].highrange3 = 2000.0;
  ErrorLookup[MASKEDIT_0].usemagnitude = 1;
  ErrorLookup[MASKEDIT_0].participants = 3;
  ErrorLookup[MASKEDIT_0].checkapplies = (char) GAITcheck;
  ErrorLookup[MASKEDIT_0].tol_desc1 = "Mask post value";
  ErrorLookup[MASKEDIT_0].tol_desc2 = "Show points (0) billboards (1) both (2)";

  ErrorLookup[MASKSHOREL].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKSHOREL].numthresholds = 3;
  ErrorLookup[MASKSHOREL].usemagnitude = 1;
  ErrorLookup[MASKSHOREL].lowrange = 0.0;
  ErrorLookup[MASKSHOREL].highrange = 500.0;
  ErrorLookup[MASKSHOREL].lowrange2 = 0.0;
  ErrorLookup[MASKSHOREL].highrange2 = 5011.0;
  ErrorLookup[MASKSHOREL].lowrange3 = 0.0;
  ErrorLookup[MASKSHOREL].highrange3 = 50.0;
  ErrorLookup[MASKSHOREL].participants = 2;
  ErrorLookup[MASKSHOREL].tol_desc1 = "Lower post value";
  ErrorLookup[MASKSHOREL].tol_desc2 = "Upper post value";
  ErrorLookup[MASKSHOREL].tol_desc3 = "Shoreline offset above water";
  ErrorLookup[MASKSHOREL].checkapplies = (char) GAITcheck;

  ErrorLookup[MASKCONFLICT].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKCONFLICT].numthresholds = 5;
  ErrorLookup[MASKCONFLICT].usemagnitude = 0;
  ErrorLookup[MASKCONFLICT].lowrange = -10000000.0;
  ErrorLookup[MASKCONFLICT].highrange = 50000000.0;
  ErrorLookup[MASKCONFLICT].lowrange2 = -10000000.0;
  ErrorLookup[MASKCONFLICT].highrange2 = 50000000.0;
  ErrorLookup[MASKCONFLICT].lowrange3 = -10000000.0;
  ErrorLookup[MASKCONFLICT].highrange3 = 50000000.0;
  ErrorLookup[MASKCONFLICT].lowrange4 = -10000000.0;
  ErrorLookup[MASKCONFLICT].highrange4 = 50000000.0;
  ErrorLookup[MASKCONFLICT].lowrange5 = 0.0;
  ErrorLookup[MASKCONFLICT].highrange5 = 2.0;
  ErrorLookup[MASKCONFLICT].participants = 2;
  ErrorLookup[MASKCONFLICT].tol_desc1 = "First mask lower post value";
  ErrorLookup[MASKCONFLICT].tol_desc2 = "First mask upper post value";
  ErrorLookup[MASKCONFLICT].tol_desc3 = "Second mask lower post value";
  ErrorLookup[MASKCONFLICT].tol_desc4 = "Second mask upper post value";
  ErrorLookup[MASKCONFLICT].tol_desc5 = "Show points (0) billboards (1) both (2)";
  ErrorLookup[MASKCONFLICT].checkapplies = (char) GAITcheck;

  ErrorLookup[MASKEDIT_1].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKEDIT_1].numthresholds = 5;
  ErrorLookup[MASKEDIT_1].usemagnitude = 1;
  ErrorLookup[MASKEDIT_1].lowrange = 0.0;
  ErrorLookup[MASKEDIT_1].highrange = 12.0;
  ErrorLookup[MASKEDIT_1].lowrange2 = 0.0;
  ErrorLookup[MASKEDIT_1].highrange2 = 1000.0;
  ErrorLookup[MASKEDIT_1].lowrange3 = -10000000.0;
  ErrorLookup[MASKEDIT_1].highrange3 = 50000000.0;
  ErrorLookup[MASKEDIT_1].lowrange4 = -10000000.0;
  ErrorLookup[MASKEDIT_1].highrange4 = 50000000.0;
  ErrorLookup[MASKEDIT_1].lowrange5 = 0.0;
  ErrorLookup[MASKEDIT_1].highrange5 = 2.0;
  ErrorLookup[MASKEDIT_1].participants = 3;
  ErrorLookup[MASKEDIT_1].tol_desc1 = "First mask post value";
  ErrorLookup[MASKEDIT_1].tol_desc2 = "Greatest allowed post value difference";
  ErrorLookup[MASKEDIT_1].tol_desc3 = "Post value to ignore";
  ErrorLookup[MASKEDIT_1].tol_desc4 = "Post value to ignore";
  ErrorLookup[MASKEDIT_1].tol_desc5 = "Show points (0) billboards (1) both (2)";
  ErrorLookup[MASKEDIT_1].checkapplies = (char) GAITcheck;

  ErrorLookup[MASKCONF2].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKCONF2].numthresholds = 5;
  ErrorLookup[MASKCONF2].usemagnitude = 0;
  ErrorLookup[MASKCONF2].lowrange = -10000000.0;
  ErrorLookup[MASKCONF2].highrange = 50000000.0;
  ErrorLookup[MASKCONF2].lowrange2 = -10000000.0;
  ErrorLookup[MASKCONF2].highrange2 = 50000000.0;
  ErrorLookup[MASKCONF2].lowrange3 = -10000000.0;
  ErrorLookup[MASKCONF2].highrange3 = 50000000.0;
  ErrorLookup[MASKCONF2].lowrange4 = -10000000.0;
  ErrorLookup[MASKCONF2].highrange4 = 50000000.0;
  ErrorLookup[MASKCONF2].lowrange5 = 0.0;
  ErrorLookup[MASKCONF2].highrange5 = 2.0;
  ErrorLookup[MASKCONF2].participants = 2;
  ErrorLookup[MASKCONF2].tol_desc1 = "First mask lower post value";
  ErrorLookup[MASKCONF2].tol_desc2 = "First mask upper post value";
  ErrorLookup[MASKCONF2].tol_desc3 = "Second mask lower post value";
  ErrorLookup[MASKCONF2].tol_desc4 = "Second mask upper post value";
  ErrorLookup[MASKCONF2].tol_desc5 = "Show points (0) billboards (1) both (2)";
  ErrorLookup[MASKCONF2].checkapplies = (char) GAITcheck;

  ErrorLookup[MASKMONO].mygroup = NEW_DEM_GROUP;
  ErrorLookup[MASKMONO].numthresholds = 3;
  ErrorLookup[MASKMONO].usemagnitude = 0;
  ErrorLookup[MASKMONO].lowrange = 0.0;
  ErrorLookup[MASKMONO].highrange = 5.0;
  ErrorLookup[MASKMONO].lowrange2 = 0.0;
  ErrorLookup[MASKMONO].highrange2 = 1.0;
  ErrorLookup[MASKMONO].lowrange3 = 0.0;
  ErrorLookup[MASKMONO].highrange3 = 1.0;
  ErrorLookup[MASKMONO].participants = 2;
  ErrorLookup[MASKMONO].tol_desc1 = "Mask post value";
  ErrorLookup[MASKMONO].tol_desc2 = "Post value tolerance (+/-)";
  ErrorLookup[MASKMONO].tol_desc3 = "Algorithm sensitivity";
  ErrorLookup[MASKMONO].checkapplies = (char) GAITcheck;


  ErrorLookup[CLAMP_SEG].mygroup = NEW_DEM_GROUP;
  ErrorLookup[CLAMP_SEG].numthresholds = 3;
  ErrorLookup[CLAMP_SEG].usemagnitude = 1;
  ErrorLookup[CLAMP_SEG].lowrange = 0.0;
  ErrorLookup[CLAMP_SEG].highrange = 1000000.0;
  ErrorLookup[CLAMP_SEG].lowrange2 = -9999999.0;
  ErrorLookup[CLAMP_SEG].highrange2 = 1000000.0;
  ErrorLookup[CLAMP_SEG].lowrange3 = -9999999.0;
  ErrorLookup[CLAMP_SEG].highrange3 = 1000000.0;
  ErrorLookup[CLAMP_SEG].participants = 2;
  ErrorLookup[CLAMP_SEG].draw_wholeareal = 0;
  ErrorLookup[CLAMP_SEG].units = "meters";
  ErrorLookup[CLAMP_SEG].units2 = "meters";
  ErrorLookup[CLAMP_SEG].units3 = "meters";
  ErrorLookup[CLAMP_SEG].tol_desc1 = "Elevation surface difference limit";
  ErrorLookup[CLAMP_SEG].tol_desc2 = "Ignore vertex values below";
  ErrorLookup[CLAMP_SEG].tol_desc3 = "Ignore DEM values below";

  
  
  
  ErrorLookup[AREAINTAREA].mygroup = INTERSECT_GROUP;
  ErrorLookup[AREAINTAREA].participants = 2;
  ErrorLookup[AREAINTAREA].numthresholds = 0;

  ErrorLookup[PART_ISF].mygroup = OVERLAP_GROUP;
  ErrorLookup[PART_ISF].participants = 2;
  ErrorLookup[PART_ISF].numthresholds = 0;
  ErrorLookup[PART_ISF].usemagnitude = 0;

  
  ErrorLookup[CUT_INT].mygroup = COMPO_GROUP;
  ErrorLookup[CUT_INT].numthresholds = 1;
  ErrorLookup[CUT_INT].usemagnitude = 0;
  ErrorLookup[CUT_INT].participants = 1;
  ErrorLookup[CUT_INT].lowrange = 0.0;
  ErrorLookup[CUT_INT].highrange = 20.0;
  ErrorLookup[CUT_INT].tol_desc1 = "Proximity limit";
  
  ErrorLookup[ACOVERA].mygroup = CONTAIN_GROUP;
  ErrorLookup[ACOVERA].participants = 2;
  ErrorLookup[ACOVERA].numthresholds = 0;
  ErrorLookup[ACOVERA].lowrange = 0.0;
  ErrorLookup[ACOVERA].highrange = 10000000.0;
  ErrorLookup[ACOVERA].usemagnitude = 0;
  ErrorLookup[ACOVERA].checkapplies = (char) GAITcheck;
  
  
  ErrorLookup[AINSIDEHOLE].mygroup = CONTAIN_GROUP;
  ErrorLookup[AINSIDEHOLE].participants = 2;
  ErrorLookup[AINSIDEHOLE].numthresholds = 0;
  ErrorLookup[AINSIDEHOLE].draw_wholeareal = 1;
  ErrorLookup[AINSIDEHOLE].checkapplies = (char) GAITcheck;

  
  
  ErrorLookup[AOVERLAPA].mygroup = OVERLAP_GROUP;
  ErrorLookup[AOVERLAPA].participants = 3;
  ErrorLookup[AOVERLAPA].numthresholds = 0;
  
  
  ErrorLookup[LLNONODEINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLNONODEINT].participants = 3;
  ErrorLookup[LLNONODEINT].numthresholds = 1;
  ErrorLookup[LLNONODEINT].usemagnitude = 0;
  ErrorLookup[LLNONODEINT].lowrange = 0.0;
  ErrorLookup[LLNONODEINT].highrange = 10.0;
  ErrorLookup[LLNONODEINT].draw_wholeareal = 0;
  ErrorLookup[LLNONODEINT].tol_desc1 = "Coordinate equality limit";

  ErrorLookup[NONODEOVLP].mygroup = OVERLAP_GROUP;
  ErrorLookup[NONODEOVLP].participants = 2;
  ErrorLookup[NONODEOVLP].numthresholds = 1;
  ErrorLookup[NONODEOVLP].usemagnitude = 0;
  ErrorLookup[NONODEOVLP].lowrange = 0.0;
  ErrorLookup[NONODEOVLP].highrange = 10.0;
  ErrorLookup[NONODEOVLP].draw_wholeareal = 0;
  ErrorLookup[NONODEOVLP].tol_desc1 = "Coordinate equality limit";
  
  ErrorLookup[LLNOENDINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLNOENDINT].participants = 2;

  ErrorLookup[LLINTAWAY].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLINTAWAY].participants = 2;
  ErrorLookup[LLINTAWAY].numthresholds = 1;
  ErrorLookup[LLINTAWAY].usemagnitude = 0;
  ErrorLookup[LLINTAWAY].lowrange = 0.0;
  ErrorLookup[LLINTAWAY].highrange = 50.0;
  ErrorLookup[LLINTAWAY].tol_desc1 = "Coordinate equality limit";

  ErrorLookup[LLINTNOEND].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLINTNOEND].participants = 2;
  ErrorLookup[LLINTNOEND].numthresholds = 1;
  ErrorLookup[LLINTNOEND].usemagnitude = 0;
  ErrorLookup[LLINTNOEND].lowrange = 0.0;
  ErrorLookup[LLINTNOEND].highrange = 50.0;
  ErrorLookup[LLINTNOEND].tol_desc1 = "Coordinate equality limit";
  
  
  
  ErrorLookup[LLINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLINT].participants = 2;

  ErrorLookup[BADFEATCUT].mygroup = CONNECT_GROUP;
  ErrorLookup[BADFEATCUT].participants = 3;
  ErrorLookup[BADFEATCUT].numthresholds = 1;
  ErrorLookup[BADFEATCUT].usemagnitude = 0;
  ErrorLookup[BADFEATCUT].lowrange = 0.0;
  ErrorLookup[BADFEATCUT].highrange = 10.0;
  ErrorLookup[BADFEATCUT].draw_wholeareal = 0;
  ErrorLookup[BADFEATCUT].tol_desc1 = "Coordinate equality limit";
  
  ErrorLookup[FAILMERGEL].mygroup = CONNECT_GROUP;
  ErrorLookup[FAILMERGEL].participants = 2;
  ErrorLookup[FAILMERGEL].numthresholds = 2;
  ErrorLookup[FAILMERGEL].usemagnitude = 1;
  ErrorLookup[FAILMERGEL].lowrange = 0.0;
  ErrorLookup[FAILMERGEL].highrange = 0.9;
  ErrorLookup[FAILMERGEL].lowrange2= 1.0;
  ErrorLookup[FAILMERGEL].highrange2 = 100.0;
  ErrorLookup[FAILMERGEL].draw_wholeareal = 0;
  ErrorLookup[FAILMERGEL].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[FAILMERGEL].tol_desc2 = "Connected feature limit";

  ErrorLookup[FAILMERGEL2].mygroup = CONNECT_GROUP;
  ErrorLookup[FAILMERGEL2].participants = 2;
  ErrorLookup[FAILMERGEL2].numthresholds = 2;
  ErrorLookup[FAILMERGEL2].usemagnitude = 1;
  ErrorLookup[FAILMERGEL2].lowrange = 0.0;
  ErrorLookup[FAILMERGEL2].highrange = 0.9;
  ErrorLookup[FAILMERGEL2].lowrange2= 1.0;
  ErrorLookup[FAILMERGEL2].highrange2 = 100.0;
  ErrorLookup[FAILMERGEL2].draw_wholeareal = 0;
  ErrorLookup[FAILMERGEL2].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[FAILMERGEL2].tol_desc2 = "Connected feature limit";

  ErrorLookup[FAILMERGEA].mygroup = CONNECT_GROUP;
  ErrorLookup[FAILMERGEA].participants = 1;
  ErrorLookup[FAILMERGEA].numthresholds = 1;
  ErrorLookup[FAILMERGEA].usemagnitude = 0;
  ErrorLookup[FAILMERGEA].lowrange = 0.0;
  ErrorLookup[FAILMERGEA].highrange = 0.9;
  ErrorLookup[FAILMERGEA].draw_wholeareal = 1;
  ErrorLookup[FAILMERGEA].tol_desc1 = "Coordinate equality limit";

  ErrorLookup[FAILMERGEA2].mygroup = CONNECT_GROUP;
  ErrorLookup[FAILMERGEA2].participants = 1;
  ErrorLookup[FAILMERGEA2].numthresholds = 2;
  ErrorLookup[FAILMERGEA2].usemagnitude = 0;
  ErrorLookup[FAILMERGEA2].lowrange = 0.0;
  ErrorLookup[FAILMERGEA2].highrange = 0.9;
  ErrorLookup[FAILMERGEA2].lowrange2= 0.0;
  ErrorLookup[FAILMERGEA2].highrange2 = 100.0;
  ErrorLookup[FAILMERGEA2].draw_wholeareal = 1; 
  ErrorLookup[FAILMERGEA2].tol_desc1 = "Coordinate equality limit";
  ErrorLookup[FAILMERGEA2].tol_desc2 = "Latitude / Longitude proximity";
  
  
  ErrorLookup[LLIEX].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLIEX].participants = 3;
  ErrorLookup[LLIEX].numthresholds = 1;
  ErrorLookup[LLIEX].usemagnitude = 0;
  ErrorLookup[LLIEX].lowrange = 0.0;
  ErrorLookup[LLIEX].highrange = 10.0;
  ErrorLookup[LLIEX].tol_desc1 = "Feature proximity limit";


  ErrorLookup[LAIEX].mygroup = INTERSECT_GROUP;
  ErrorLookup[LAIEX].participants = 3;
  ErrorLookup[LAIEX].numthresholds = 1;
  ErrorLookup[LAIEX].usemagnitude = 0;
  ErrorLookup[LAIEX].lowrange = 0.0;
  ErrorLookup[LAIEX].highrange = 10.0;
  ErrorLookup[LAIEX].tol_desc1 = "Feature proximity limit";

  
  ErrorLookup[LLNOINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLNOINT].participants = 2;
  ErrorLookup[LLNOINT].numthresholds = 1;
  ErrorLookup[LLNOINT].usemagnitude = 0;
  ErrorLookup[LLNOINT].lowrange = 0.0;
  ErrorLookup[LLNOINT].highrange = 75.0;
  ErrorLookup[LLNOINT].tol_desc1 = "Feature proximity limit";

  ErrorLookup[LFNOINT].mygroup = INTERSECT_GROUP;
  ErrorLookup[LFNOINT].participants = 2;

  
  
  ErrorLookup[LOUTSIDEA].mygroup = CONTAIN_GROUP;
  ErrorLookup[LOUTSIDEA].participants = 2;
  

  ErrorLookup[LLAINT].mygroup = CONNECT_GROUP;
  ErrorLookup[LLAINT].participants = 3;
  ErrorLookup[LLAINT].numthresholds = 1;
  ErrorLookup[LLAINT].usemagnitude = 0;
  ErrorLookup[LLAINT].lowrange = 0.0;
  ErrorLookup[LLAINT].highrange = 50.0;
  ErrorLookup[LLAINT].tol_desc1 = "Feature proximity limit";

  ErrorLookup[L_NOTL_AINT].mygroup = CONNECT_GROUP;
  ErrorLookup[L_NOTL_AINT].participants = 3;
  ErrorLookup[L_NOTL_AINT].numthresholds = 1;
  ErrorLookup[L_NOTL_AINT].usemagnitude = 0;
  ErrorLookup[L_NOTL_AINT].lowrange = 0.0;
  ErrorLookup[L_NOTL_AINT].highrange = 5.0;
  ErrorLookup[L_NOTL_AINT].tol_desc1 = "Feature proximity limit";

  
  ErrorLookup[PTINREGION].mygroup = CONTAIN_GROUP;
  ErrorLookup[PTINREGION].participants = 2;
  ErrorLookup[PTINREGION].draw_wholeareal = 1;
  ErrorLookup[PTINREGION].numthresholds = 1;
  ErrorLookup[PTINREGION].usemagnitude = 0;
  ErrorLookup[PTINREGION].lowrange = 0.0;
  ErrorLookup[PTINREGION].highrange = 100.0;
  ErrorLookup[PTINREGION].tol_desc1 = "Feature proximity limit";

  ErrorLookup[PTINPROPER].mygroup = CONTAIN_GROUP;
  ErrorLookup[PTINPROPER].participants = 2;
  ErrorLookup[PTINPROPER].draw_wholeareal = 1;
  ErrorLookup[PTINPROPER].numthresholds = 1;
  ErrorLookup[PTINPROPER].usemagnitude = 0;
  ErrorLookup[PTINPROPER].lowrange = 0.0;
  ErrorLookup[PTINPROPER].highrange = 50.0;
  ErrorLookup[PTINPROPER].tol_desc1 = "Feature proximity limit";
  
  
  ErrorLookup[PTOSIDEREGION].mygroup = CONTAIN_GROUP;
  ErrorLookup[PTOSIDEREGION].participants = 2;
  
  
  ErrorLookup[OBJECTWITHOUT].participants = 2;
  ErrorLookup[OBJECTWITHOUT].mygroup = CONTAIN_GROUP;
  ErrorLookup[OBJECTWITHOUT].draw_wholeareal = 1;

  ErrorLookup[OBJ_WO_TWO].participants = 3;
  ErrorLookup[OBJ_WO_TWO].mygroup = CONTAIN_GROUP;
  ErrorLookup[OBJ_WO_TWO].draw_wholeareal = 1;


  ErrorLookup[AWITHOUTA].participants = 2;
  ErrorLookup[AWITHOUTA].mygroup = CONTAIN_GROUP;
  ErrorLookup[AWITHOUTA].draw_wholeareal = 1;
  ErrorLookup[AWITHOUTA].numthresholds = 2;
  ErrorLookup[AWITHOUTA].usemagnitude = 0;
  ErrorLookup[AWITHOUTA].lowrange = -50000.0;
  ErrorLookup[AWITHOUTA].highrange = 50000.0;
  ErrorLookup[AWITHOUTA].lowrange2 = -50000.0;
  ErrorLookup[AWITHOUTA].highrange2 = 50000.0;
  ErrorLookup[AWITHOUTA].tol_desc1 = "Grid / Mask lower value limit";
  ErrorLookup[AWITHOUTA].tol_desc2 = "Grid / Mask upper value limit";

  ErrorLookup[FSFAIL].participants = 2;
  ErrorLookup[FSFAIL].mygroup = OVERLAP_GROUP;
  ErrorLookup[FSFAIL].draw_wholeareal = 1;


  ErrorLookup[PSHAREFAIL].participants = 3;
  ErrorLookup[PSHAREFAIL].mygroup = OVERLAP_GROUP;
  ErrorLookup[PSHAREFAIL].numthresholds = 1;
  ErrorLookup[PSHAREFAIL].usemagnitude = 0;
  ErrorLookup[PSHAREFAIL].lowrange = 0.0;
  ErrorLookup[PSHAREFAIL].highrange = 10.0;
  ErrorLookup[PSHAREFAIL].draw_wholeareal = 1;
  ErrorLookup[PSHAREFAIL].tol_desc1 = "Feature proximity limit";

  ErrorLookup[NOCOINCIDE].participants = 2;
  ErrorLookup[NOCOINCIDE].mygroup = CONNECT_GROUP;
  ErrorLookup[NOCOINCIDE].draw_wholeareal = 1;
  ErrorLookup[NOCOINCIDE].numthresholds = 1;
  ErrorLookup[NOCOINCIDE].usemagnitude = 0;
  ErrorLookup[NOCOINCIDE].lowrange = 0.0;
  ErrorLookup[NOCOINCIDE].highrange = 50.0;
  ErrorLookup[NOCOINCIDE].tol_desc1 = "Feature proximity limit";
  
  
  ErrorLookup[ANOCOVERLA].mygroup = OVERLAP_GROUP;
  ErrorLookup[ANOCOVERLA].numthresholds = 1;
  ErrorLookup[ANOCOVERLA].usemagnitude = 0;
  ErrorLookup[ANOCOVERLA].lowrange = 0.0;
  ErrorLookup[ANOCOVERLA].highrange = 100.0;
  ErrorLookup[ANOCOVERLA].participants = 3;
  ErrorLookup[ANOCOVERLA].draw_wholeareal = 0;
  ErrorLookup[ANOCOVERLA].tol_desc1 = "Feature proximity limit";

  ErrorLookup[QUALANOCOVLA].mygroup = OVERLAP_GROUP;
  ErrorLookup[QUALANOCOVLA].numthresholds = 1;
  ErrorLookup[QUALANOCOVLA].usemagnitude = 0;
  ErrorLookup[QUALANOCOVLA].lowrange = 0.0;
  ErrorLookup[QUALANOCOVLA].highrange = 100.0;
  ErrorLookup[QUALANOCOVLA].participants = 3;
  ErrorLookup[QUALANOCOVLA].tol_desc1 = "Feature proximity limit";
  
  ErrorLookup[AMCOVAFAIL].mygroup = OVERLAP_GROUP;
  ErrorLookup[AMCOVAFAIL].numthresholds = 0;
  ErrorLookup[AMCOVAFAIL].usemagnitude = 0;
  ErrorLookup[AMCOVAFAIL].participants = 2;
  ErrorLookup[AMCOVAFAIL].checkapplies = (char) BUSTEDcheck;
  
  ErrorLookup[ANOCOVERA].mygroup = OVERLAP_GROUP;
  ErrorLookup[ANOCOVERA].numthresholds = 0;
  ErrorLookup[ANOCOVERA].usemagnitude = 0;
  ErrorLookup[ANOCOVERA].participants = 2;

  ErrorLookup[OVERUNDER].mygroup = CONTAIN_GROUP;
  ErrorLookup[OVERUNDER].numthresholds = 2;
  ErrorLookup[OVERUNDER].usemagnitude = 0;
  ErrorLookup[OVERUNDER].lowrange = 0.0;
  ErrorLookup[OVERUNDER].highrange = 100.0;
  ErrorLookup[OVERUNDER].lowrange2  = 0.0;
  ErrorLookup[OVERUNDER].highrange2 =  100.0;
  ErrorLookup[OVERUNDER].participants = 2;
  ErrorLookup[OVERUNDER].tol_desc1 = "Coordinate proximity lower limit";
  ErrorLookup[OVERUNDER].tol_desc2 = "Coordinate proximity upper limit";
  
  ErrorLookup[CUTOUT].mygroup = COMPO_GROUP;
  ErrorLookup[CUTOUT].numthresholds = 0;
  ErrorLookup[CUTOUT].usemagnitude = 0;
  ErrorLookup[CUTOUT].participants = 1;
  ErrorLookup[CUTOUT].draw_wholeareal = 1;
  ErrorLookup[CUTOUT].checkapplies = (char) GAITcheck;

  ErrorLookup[PORTRAYF].mygroup = ATTR_GROUP;
  ErrorLookup[PORTRAYF].numthresholds = 0;
  ErrorLookup[PORTRAYF].usemagnitude = 0;
  ErrorLookup[PORTRAYF].participants = 1;
  ErrorLookup[PORTRAYF].draw_wholeareal = 1;
  ErrorLookup[PORTRAYF].checkapplies = (char) GAITcheck;

  ErrorLookup[TPORTRAYF].mygroup = ATTR_GROUP;
  ErrorLookup[TPORTRAYF].numthresholds = 0;
  ErrorLookup[TPORTRAYF].usemagnitude = 0;
  ErrorLookup[TPORTRAYF].participants = 1;
  ErrorLookup[TPORTRAYF].draw_wholeareal = 1;
  ErrorLookup[TPORTRAYF].checkapplies = (char) GAITcheck;

  ErrorLookup[COLINEAR].mygroup = COMPO_GROUP;
  ErrorLookup[COLINEAR].numthresholds = 3;
  ErrorLookup[COLINEAR].usemagnitude = 0;
  ErrorLookup[COLINEAR].lowrange = 0.0;
  ErrorLookup[COLINEAR].highrange = 10.0;
  ErrorLookup[COLINEAR].lowrange2 = 0.0;
  ErrorLookup[COLINEAR].highrange2 = 10.0;
  ErrorLookup[COLINEAR].lowrange3 = 0.0;
  ErrorLookup[COLINEAR].highrange3 = 100000.0;
  ErrorLookup[COLINEAR].participants = 2;
  ErrorLookup[COLINEAR].draw_wholeareal = 1;
  ErrorLookup[COLINEAR].checkapplies = (char) GAITcheck;
  ErrorLookup[COLINEAR].tol_desc1 = "True collinear offset limit";
  ErrorLookup[COLINEAR].tol_desc2 = "Vertex proximity limit";
  ErrorLookup[COLINEAR].tol_desc3 = "Adjacent vertex distance limit";

  ErrorLookup[LNOCOVERLA].mygroup = OVERLAP_GROUP;
  ErrorLookup[LNOCOVERLA].numthresholds = 1;
  ErrorLookup[LNOCOVERLA].usemagnitude = 0;
  ErrorLookup[LNOCOVERLA].lowrange = 0.0;
  ErrorLookup[LNOCOVERLA].highrange = 100.0;
  ErrorLookup[LNOCOVERLA].participants = 3; 
  ErrorLookup[LNOCOVERLA].tol_desc1 = "Vertex proximity limit";

  ErrorLookup[CONFLATE].mygroup = OVERLAP_GROUP;
  ErrorLookup[CONFLATE].numthresholds = 1;
  ErrorLookup[CONFLATE].usemagnitude = 0;
  ErrorLookup[CONFLATE].lowrange = 0.0;
  ErrorLookup[CONFLATE].highrange = 100.0;
  ErrorLookup[CONFLATE].participants = 3;
  ErrorLookup[CONFLATE].tol_desc1 = "Vertex proximity limit";

  ErrorLookup[CONF_STATS].mygroup = OVERLAP_GROUP;
  ErrorLookup[CONF_STATS].numthresholds = 0;
  ErrorLookup[CONF_STATS].usemagnitude = 0;
  ErrorLookup[CONF_STATS].participants = 1;

  ErrorLookup[LSPANFAIL].mygroup = OVERLAP_GROUP;
  ErrorLookup[LSPANFAIL].numthresholds = 1;
  ErrorLookup[LSPANFAIL].usemagnitude = 0;
  ErrorLookup[LSPANFAIL].lowrange = 0.0;
  ErrorLookup[LSPANFAIL].highrange = 100.0;
  ErrorLookup[LSPANFAIL].participants = 2;
  ErrorLookup[LSPANFAIL].tol_desc1 = "Vertex proximity limit";

  ErrorLookup[LNOCOV2A].mygroup = OVERLAP_GROUP;
  ErrorLookup[LNOCOV2A].numthresholds = 1;
  ErrorLookup[LNOCOV2A].usemagnitude = 0;
  ErrorLookup[LNOCOV2A].lowrange = 0.0;
  ErrorLookup[LNOCOV2A].highrange = 50.0;
  ErrorLookup[LNOCOV2A].participants = 2;
  ErrorLookup[LNOCOV2A].tol_desc1 = "Vertex proximity limit";

  ErrorLookup[ISOLINE].mygroup = CONTAIN_GROUP;
  ErrorLookup[ISOLINE].numthresholds = 1;
  ErrorLookup[ISOLINE].usemagnitude = 0;
  ErrorLookup[ISOLINE].lowrange = 0.0;
  ErrorLookup[ISOLINE].highrange = 10.0;
  ErrorLookup[ISOLINE].tol_desc1 = "Vertex proximity limit";
  ErrorLookup[ISOLINE].participants = 3;

  ErrorLookup[LINSIDEA].mygroup = CONTAIN_GROUP;
  ErrorLookup[LINSIDEA].numthresholds = 2;
  ErrorLookup[LINSIDEA].usemagnitude = 0;
  ErrorLookup[LINSIDEA].lowrange = 0.0;
  ErrorLookup[LINSIDEA].highrange = 10.0;
  ErrorLookup[LINSIDEA].lowrange2  = 0.0;
  ErrorLookup[LINSIDEA].highrange2 =  91.0;
  ErrorLookup[LINSIDEA].participants = 3;
  ErrorLookup[LINSIDEA].tol_desc1 = "Vertex proximity limit";
  ErrorLookup[LINSIDEA].tol_desc2 = "Acute angle at intersection";

  ErrorLookup[LSEGCOVERA].mygroup = OVERLAP_GROUP;
  ErrorLookup[LSEGCOVERA].numthresholds = 1;
  ErrorLookup[LSEGCOVERA].usemagnitude = 0;
  ErrorLookup[LSEGCOVERA].lowrange = 0.0;
  ErrorLookup[LSEGCOVERA].highrange = 10.0;
  ErrorLookup[LSEGCOVERA].participants = 2;
  ErrorLookup[LSEGCOVERA].tol_desc1 = "Feature segment proximity limit";

  ErrorLookup[LEINSIDEA].mygroup = CONTAIN_GROUP;
  ErrorLookup[LEINSIDEA].numthresholds = 1;
  ErrorLookup[LEINSIDEA].usemagnitude = 0;
  ErrorLookup[LEINSIDEA].lowrange = 0.0;
  ErrorLookup[LEINSIDEA].highrange = 10.0;
  ErrorLookup[LEINSIDEA].participants = 2;
  ErrorLookup[LEINSIDEA].tol_desc1 = "Feature proximity limit";

  ErrorLookup[LEAON_NOTIN].mygroup = CONTAIN_GROUP;
  ErrorLookup[LEAON_NOTIN].numthresholds = 1;
  ErrorLookup[LEAON_NOTIN].usemagnitude = 0;
  ErrorLookup[LEAON_NOTIN].lowrange = 0.0;
  ErrorLookup[LEAON_NOTIN].highrange = 10.0;
  ErrorLookup[LEAON_NOTIN].participants = 2;
  ErrorLookup[LEAON_NOTIN].tol_desc1 = "Vertex proximity limit";

  ErrorLookup[COINCIDEFAIL].mygroup = OVERLAP_GROUP;
  ErrorLookup[COINCIDEFAIL].numthresholds = 1;
  ErrorLookup[COINCIDEFAIL].usemagnitude = 0;
  ErrorLookup[COINCIDEFAIL].lowrange = 0.0;
  ErrorLookup[COINCIDEFAIL].highrange = 50.0;
  ErrorLookup[COINCIDEFAIL].participants = 2;
  ErrorLookup[COINCIDEFAIL].tol_desc1 = "Feature proximity limit";

  ErrorLookup[ISOLATEDA].mygroup = INTERSECT_GROUP;
  ErrorLookup[ISOLATEDA].numthresholds = 1;
  ErrorLookup[ISOLATEDA].lowrange = 0.0;
  ErrorLookup[ISOLATEDA].highrange = 5.0;
  ErrorLookup[ISOLATEDA].usemagnitude = 0;
  ErrorLookup[ISOLATEDA].participants = 2;
  ErrorLookup[ISOLATEDA].tol_desc1 = "Feature proximity limit";

  ErrorLookup[ANETISOA].mygroup = INTERSECT_GROUP;
  ErrorLookup[ANETISOA].numthresholds = 1;
  ErrorLookup[ANETISOA].lowrange = 0.0;
  ErrorLookup[ANETISOA].highrange = 5.0;
  ErrorLookup[ANETISOA].usemagnitude = 0;
  ErrorLookup[ANETISOA].participants = 3;
  ErrorLookup[ANETISOA].tol_desc1 = "Feature proximity limit";
  ErrorLookup[ANETISOA].tol_desc2 = "Feature proximity limit";


  ErrorLookup[NETISOA].mygroup = INTERSECT_GROUP;
  ErrorLookup[NETISOA].numthresholds = 2;
  ErrorLookup[NETISOA].lowrange = 0.0;
  ErrorLookup[NETISOA].highrange = 5.0;
  ErrorLookup[NETISOA].lowrange2  = 0.0;
  ErrorLookup[NETISOA].highrange2 =  5.0;
  ErrorLookup[NETISOA].usemagnitude = 0;
  ErrorLookup[NETISOA].participants = 2;
  ErrorLookup[NETISOA].tol_desc1 = "Feature proximity limit";
  ErrorLookup[NETISOA].tol_desc2 = "Feature proximity limit";

  ErrorLookup[SHARE3SEG].mygroup = OVERLAP_GROUP;
  ErrorLookup[SHARE3SEG].numthresholds = 2;
  ErrorLookup[SHARE3SEG].usemagnitude = 1;
  ErrorLookup[SHARE3SEG].lowrange = 0.0;
  ErrorLookup[SHARE3SEG].highrange = 50.0;
  ErrorLookup[SHARE3SEG].lowrange2 = 0.0;
  ErrorLookup[SHARE3SEG].highrange2 = 50.0;
  ErrorLookup[SHARE3SEG].participants = 3;
  ErrorLookup[SHARE3SEG].tol_desc1 = "Feature proximity limit";
  ErrorLookup[SHARE3SEG].tol_desc2 = "Overlapping segment length limit";
  

  ErrorLookup[SHARESEG].mygroup = OVERLAP_GROUP;
  ErrorLookup[SHARESEG].numthresholds = 3;
  ErrorLookup[SHARESEG].usemagnitude = 1;
  ErrorLookup[SHARESEG].lowrange = 0.0;
  ErrorLookup[SHARESEG].highrange = 50.0;
  ErrorLookup[SHARESEG].lowrange2  = 0.0;
  ErrorLookup[SHARESEG].highrange2 =  100000.0;
  ErrorLookup[SHARESEG].lowrange3  = -10.0;
  ErrorLookup[SHARESEG].highrange3 =  100000.0;
  ErrorLookup[SHARESEG].participants = 3;
  ErrorLookup[SHARESEG].tol_desc1 = "Feature proximity limit";
  ErrorLookup[SHARESEG].tol_desc2 = "Overlapping segment length limit";
  ErrorLookup[SHARESEG].tol_desc3 = "Z-value maximum difference";


  ErrorLookup[LLI_ANGLE].mygroup = INTERSECT_GROUP;
  ErrorLookup[LLI_ANGLE].numthresholds = 2;
  ErrorLookup[LLI_ANGLE].usemagnitude = 1;
  ErrorLookup[LLI_ANGLE].lowrange = 0.0;
  ErrorLookup[LLI_ANGLE].highrange = 90.0;
  ErrorLookup[LLI_ANGLE].lowrange2  = 0.0;
  ErrorLookup[LLI_ANGLE].highrange2 =  25.0;
  ErrorLookup[LLI_ANGLE].participants = 3;
  ErrorLookup[LLI_ANGLE].tol_desc1 = "Angle limit";
  ErrorLookup[LLI_ANGLE].tol_desc2 = "Feature proximity limit";

  ErrorLookup[MULTIDFEAT].mygroup = ZVAL_GROUP;
  ErrorLookup[MULTIDFEAT].numthresholds = 0;
  ErrorLookup[MULTIDFEAT].usemagnitude = 0;
  ErrorLookup[MULTIDFEAT].lowrange = 0.0;
  ErrorLookup[MULTIDFEAT].highrange = 0.0;
  ErrorLookup[MULTIDFEAT].participants = 1;
  ErrorLookup[MULTIDFEAT].checkapplies = (char) GAITcheck;

  ErrorLookup[MULTISENTINEL].mygroup = ZVAL_GROUP;
  ErrorLookup[MULTISENTINEL].numthresholds = 0;
  ErrorLookup[MULTISENTINEL].usemagnitude = 0;
  ErrorLookup[MULTISENTINEL].lowrange = 0.0;
  ErrorLookup[MULTISENTINEL].highrange = 0.0;
  ErrorLookup[MULTISENTINEL].participants = 1;
  ErrorLookup[MULTISENTINEL].checkapplies = (char) GAITcheck;

  
  ErrorLookup[LENOCOVERP].mygroup = OVERLAP_GROUP;
  ErrorLookup[LENOCOVERP].numthresholds = 1;
  ErrorLookup[LENOCOVERP].usemagnitude = 0;
  ErrorLookup[LENOCOVERP].lowrange = 0.0000;
  ErrorLookup[LENOCOVERP].highrange = 1000.0;
  ErrorLookup[LENOCOVERP].participants = 2;
  ErrorLookup[LENOCOVERP].tol_desc1 = "Coordinate equality limit";

  ErrorLookup[LENOCOVERA].mygroup = OVERLAP_GROUP;
  ErrorLookup[LENOCOVERA].numthresholds = 2;
  ErrorLookup[LENOCOVERA].usemagnitude = 1;
  ErrorLookup[LENOCOVERA].lowrange = 0.0;
  ErrorLookup[LENOCOVERA].highrange = 1000.0;
  ErrorLookup[LENOCOVERA].lowrange2  = 0.0;
  ErrorLookup[LENOCOVERA].highrange2 =  1000.0;
  ErrorLookup[LENOCOVERA].participants = 2;
  ErrorLookup[LENOCOVERA].tol_desc1 = "Coordinate proximity lower limit";
  ErrorLookup[LENOCOVERA].tol_desc2 = "Coordinate proximity upper limit";

  ErrorLookup[LAINTNOEND].mygroup = INTERSECT_GROUP;
  ErrorLookup[LAINTNOEND].numthresholds = 1;
  ErrorLookup[LAINTNOEND].usemagnitude = 0;
  ErrorLookup[LAINTNOEND].lowrange = 0.0000;
  ErrorLookup[LAINTNOEND].highrange = 1000.0;
  ErrorLookup[LAINTNOEND].participants = 2;
  ErrorLookup[LAINTNOEND].tol_desc1 = "Coordinate proximity limit";

  ErrorLookup[LACUTFAIL].mygroup = INTERSECT_GROUP;
  ErrorLookup[LACUTFAIL].numthresholds = 1;
  ErrorLookup[LACUTFAIL].usemagnitude = 0;
  ErrorLookup[LACUTFAIL].lowrange = 0.0000;
  ErrorLookup[LACUTFAIL].highrange = 1000.0;
  ErrorLookup[LACUTFAIL].participants = 2;
  ErrorLookup[LACUTFAIL].tol_desc1 = "Coordinate proximity limit";
  
  ErrorLookup[GSPIKE].mygroup = NEW_DEM_GROUP;
  ErrorLookup[GSPIKE].numthresholds = 5;
  ErrorLookup[GSPIKE].usemagnitude = 1;
  ErrorLookup[GSPIKE].lowrange = 0.0;
  ErrorLookup[GSPIKE].highrange = 90.0;
  ErrorLookup[GSPIKE].lowrange5 = -21474836467.0;
  ErrorLookup[GSPIKE].highrange5 = 21474836467.0;
  ErrorLookup[GSPIKE].lowrange4 = -21474836467.0;
  ErrorLookup[GSPIKE].highrange4 = 21474836467.0;
  ErrorLookup[GSPIKE].lowrange3 = 1.0;
  ErrorLookup[GSPIKE].highrange3 = 8.0;
  ErrorLookup[GSPIKE].lowrange2 = 0.0;
  ErrorLookup[GSPIKE].highrange2 = 1000.0;
  ErrorLookup[GSPIKE].participants = 1;
  ErrorLookup[GSPIKE].tol_desc1 = "Angle limit";
  ErrorLookup[GSPIKE].tol_desc4 = "Elevation value to ignore";
  ErrorLookup[GSPIKE].tol_desc5 = "Elevation value to ignore";
  ErrorLookup[GSPIKE].tol_desc3 = "Qualifying neighbors";
  ErrorLookup[GSPIKE].tol_desc2 = "Minimum height difference";

  ErrorLookup[FLOWSTEP].mygroup = NEW_DEM_GROUP;
  ErrorLookup[FLOWSTEP].numthresholds = 4;
  ErrorLookup[FLOWSTEP].usemagnitude = 1;
  ErrorLookup[FLOWSTEP].lowrange4 = 0.0;
  ErrorLookup[FLOWSTEP].highrange4 = 2.0;
  ErrorLookup[FLOWSTEP].lowrange3 = 0.0;
  ErrorLookup[FLOWSTEP].highrange3 = 100.0;
  ErrorLookup[FLOWSTEP].lowrange2 = 0.0;
  ErrorLookup[FLOWSTEP].highrange2 = 3.0;
  ErrorLookup[FLOWSTEP].lowrange = 0.0;
  ErrorLookup[FLOWSTEP].highrange = 3.0;
  ErrorLookup[FLOWSTEP].participants = 2;
  ErrorLookup[FLOWSTEP].tol_desc1 = "Post value lower (closed) limit";
  ErrorLookup[FLOWSTEP].tol_desc2 = "Post value upper (closed) limit";
  ErrorLookup[FLOWSTEP].tol_desc3 = "Maximum step";
  ErrorLookup[FLOWSTEP].tol_desc4 = "Show points (0) billboards (1) both (2)";

  ErrorLookup[BREAKLINE].mygroup = NEW_DEM_GROUP;
  ErrorLookup[BREAKLINE].numthresholds = 4;
  ErrorLookup[BREAKLINE].usemagnitude = 1;
  ErrorLookup[BREAKLINE].lowrange5 = 1.0;
  ErrorLookup[BREAKLINE].highrange5 = 1.0;
  ErrorLookup[BREAKLINE].lowrange4 = 0.0;
  ErrorLookup[BREAKLINE].highrange4 = 3.0;
  ErrorLookup[BREAKLINE].lowrange3 = 0.0;
  ErrorLookup[BREAKLINE].highrange3 = 3.0;
  ErrorLookup[BREAKLINE].lowrange2 = 3.0;
  ErrorLookup[BREAKLINE].highrange2 = 89.0;
  ErrorLookup[BREAKLINE].lowrange = 5.0;
  ErrorLookup[BREAKLINE].highrange = 10000.0;
  ErrorLookup[BREAKLINE].participants = 2;
  ErrorLookup[BREAKLINE].tol_desc1 = "Minimum number of posts";
  ErrorLookup[BREAKLINE].tol_desc2 = "Minimum angle offset ";
  ErrorLookup[BREAKLINE].tol_desc3 = "Post value lower (closed) limit";
  ErrorLookup[BREAKLINE].tol_desc4 = "Post value upper (closed) limit";
  ErrorLookup[BREAKLINE].tol_desc5 = "Show points (0) billboards (1) both (2)";

  ErrorLookup[LOSMINHGT].mygroup = NEW_DEM_GROUP;
  ErrorLookup[LOSMINHGT].numthresholds = 3;
  ErrorLookup[LOSMINHGT].usemagnitude = 1;
  ErrorLookup[LOSMINHGT].lowrange3 = -1.0;
  ErrorLookup[LOSMINHGT].highrange3 = 181.0;
  ErrorLookup[LOSMINHGT].lowrange2 = -1.0;
  ErrorLookup[LOSMINHGT].highrange2 = 181.0;
  ErrorLookup[LOSMINHGT].lowrange = 0.0;
  ErrorLookup[LOSMINHGT].highrange = 100000.0;
  ErrorLookup[LOSMINHGT].participants = 2;
  ErrorLookup[LOSMINHGT].tol_desc1 = "Maximum Range";
  ErrorLookup[LOSMINHGT].tol_desc2 = "Longitude Coordinate";
  ErrorLookup[LOSMINHGT].tol_desc3 = "Latitude Coordinate";
  ErrorLookup[LOSMINHGT].checkapplies = (char) BUSTEDcheck;



  ErrorLookup[AVGSPIKE].mygroup = NEW_DEM_GROUP;
  ErrorLookup[AVGSPIKE].numthresholds = 6;
  ErrorLookup[AVGSPIKE].usemagnitude = 1;
  ErrorLookup[AVGSPIKE].lowrange4 = -21474836467.0;
  ErrorLookup[AVGSPIKE].highrange4 = 21474836467.0;
  ErrorLookup[AVGSPIKE].lowrange3 = -21474836467.0;
  ErrorLookup[AVGSPIKE].highrange3 = 21474836467.0;
  ErrorLookup[AVGSPIKE].lowrange2 = 1.0;
  ErrorLookup[AVGSPIKE].highrange2 = 8.0;
  ErrorLookup[AVGSPIKE].lowrange = 0.0;
  ErrorLookup[AVGSPIKE].highrange = 1000.0;
  ErrorLookup[AVGSPIKE].lowrange5 = 1.0;
  ErrorLookup[AVGSPIKE].highrange5 = 8.0;
  ErrorLookup[AVGSPIKE].lowrange6 = 0.0;
  ErrorLookup[AVGSPIKE].highrange6 = 2.0;
  ErrorLookup[AVGSPIKE].participants = 2;
  ErrorLookup[AVGSPIKE].tol_desc3 = "Elevation value to ignore";
  ErrorLookup[AVGSPIKE].tol_desc4 = "Elevation value to ignore";
  ErrorLookup[AVGSPIKE].tol_desc2 = "Qualifying neighbors";
  ErrorLookup[AVGSPIKE].tol_desc1 = "Minimum height difference";
  ErrorLookup[AVGSPIKE].tol_desc5 = "Neighbors above / below";
  ErrorLookup[AVGSPIKE].tol_desc6 = "Show points (0) billboards (1) both (2)";

  ErrorLookup[GSHELF].mygroup = NEW_DEM_GROUP;
  ErrorLookup[GSHELF].numthresholds = 6;
  ErrorLookup[GSHELF].usemagnitude = 1;
  ErrorLookup[GSHELF].lowrange4 = 1.0;
  ErrorLookup[GSHELF].highrange4 = 8.0;
  ErrorLookup[GSHELF].lowrange3 = 2.0;
  ErrorLookup[GSHELF].highrange3 = 100001.0;
  ErrorLookup[GSHELF].lowrange2 = 1.0;
  ErrorLookup[GSHELF].highrange2 = 100000.0;
  ErrorLookup[GSHELF].lowrange = 1.0;
  ErrorLookup[GSHELF].highrange = 90.0;
  ErrorLookup[GSHELF].lowrange5 = 1.0;
  ErrorLookup[GSHELF].highrange5 = 1000.0;
  ErrorLookup[GSHELF].lowrange6 = -1.0;
  ErrorLookup[GSHELF].highrange6 = 15.0;
  ErrorLookup[GSHELF].participants = 2;
  ErrorLookup[GSHELF].tol_desc3 = "Elevation difference maximum";
  ErrorLookup[GSHELF].tol_desc2 = "Elevation difference minimum";
  ErrorLookup[GSHELF].tol_desc4 = "Qualifying neighbors";
  ErrorLookup[GSHELF].tol_desc1 = "Minimum angle";
  ErrorLookup[GSHELF].tol_desc5 = "Minimum contiguous points";
  ErrorLookup[GSHELF].tol_desc6 = "Ignore post value";
  ErrorLookup[GSHELF].checkapplies = (char) BUSTEDcheck;

  ErrorLookup[PT_GRID_DIF].mygroup = NEW_DEM_GROUP;
  ErrorLookup[PT_GRID_DIF].numthresholds = 2;
  ErrorLookup[PT_GRID_DIF].participants = 2;
  ErrorLookup[PT_GRID_DIF].usemagnitude = 1;
  ErrorLookup[PT_GRID_DIF].lowrange = 0.0;
  ErrorLookup[PT_GRID_DIF].highrange = 20.0;
  ErrorLookup[PT_GRID_DIF].lowrange2 = 0.0;
  ErrorLookup[PT_GRID_DIF].highrange2 = 999.0;
  ErrorLookup[PT_GRID_DIF].tol_desc1 = "Position difference maximum";
  ErrorLookup[PT_GRID_DIF].tol_desc3 = "Post value difference";

  ErrorLookup[RAISEDPC].mygroup = NEW_DEM_GROUP;
  ErrorLookup[RAISEDPC].numthresholds = 5;
  ErrorLookup[RAISEDPC].usemagnitude = 1;
  ErrorLookup[RAISEDPC].lowrange5 = 0.0;
  ErrorLookup[RAISEDPC].highrange5 = 2.0;
  ErrorLookup[RAISEDPC].lowrange4 = 1.0;
  ErrorLookup[RAISEDPC].highrange4 = 99.0;
  ErrorLookup[RAISEDPC].lowrange3 = 0.0;
  ErrorLookup[RAISEDPC].highrange3 = 3.0;
  ErrorLookup[RAISEDPC].lowrange2 = 0.0;
  ErrorLookup[RAISEDPC].highrange2 = 3.0;
  ErrorLookup[RAISEDPC].lowrange = 0.0;
  ErrorLookup[RAISEDPC].highrange = 12.0;
  ErrorLookup[RAISEDPC].participants = 2;
  ErrorLookup[RAISEDPC].tol_desc1 = "Post value";
  ErrorLookup[RAISEDPC].tol_desc3 = "Post value upper (closed) limit";
  ErrorLookup[RAISEDPC].tol_desc2 = "Post value lower (closed) limit";
  ErrorLookup[RAISEDPC].tol_desc4 = "Percentage";
  ErrorLookup[RAISEDPC].tol_desc5 = "Show points (0) billboards (1) both (2)";

  ErrorLookup[WATERMMU].mygroup = NEW_DEM_GROUP;
  ErrorLookup[WATERMMU].numthresholds = 3;
  ErrorLookup[WATERMMU].usemagnitude = 1;
  ErrorLookup[WATERMMU].lowrange3 = 0.0;
  ErrorLookup[WATERMMU].highrange3 = 500000.0;
  ErrorLookup[WATERMMU].lowrange2 = 0.0;
  ErrorLookup[WATERMMU].highrange2 = 30.0;
  ErrorLookup[WATERMMU].lowrange = 0.0;
  ErrorLookup[WATERMMU].highrange = 3.0;
  ErrorLookup[WATERMMU].participants = 1;
  ErrorLookup[WATERMMU].tol_desc2 = "Post value upper (closed) limit";
  ErrorLookup[WATERMMU].tol_desc1 = "Post value lower (closed) limit";
  ErrorLookup[WATERMMU].tol_desc3 = "Contiguous posts sq m lower limit";


  ErrorLookup[GRID_STD_DEV].mygroup = NEW_DEM_GROUP;
  ErrorLookup[GRID_STD_DEV].numthresholds = 2;
  ErrorLookup[GRID_STD_DEV].usemagnitude = 1;
  ErrorLookup[GRID_STD_DEV].lowrange = 0.0;
  ErrorLookup[GRID_STD_DEV].highrange = 100000.0;
  ErrorLookup[GRID_STD_DEV].lowrange2 = -9999999.0;
  ErrorLookup[GRID_STD_DEV].highrange2 = 1000000.0;
  ErrorLookup[GRID_STD_DEV].participants = 2;
  ErrorLookup[GRID_STD_DEV].tol_desc1 = "Standard Deviation (meters)";
  ErrorLookup[GRID_STD_DEV].tol_desc2 = "Ignore DEM values below";
  ErrorLookup[GRID_STD_DEV].range = 0;

  
  ErrorLookup[ELEVGT].mygroup = RANGE_GROUP;
  ErrorLookup[ELEVGT].numthresholds = 2;
  ErrorLookup[ELEVGT].usemagnitude = 1;
  ErrorLookup[ELEVGT].lowrange = -10000000.0;
  ErrorLookup[ELEVGT].highrange = 10000000.0;
  ErrorLookup[ELEVGT].lowrange2 = 0.0;
  ErrorLookup[ELEVGT].highrange2 = 2.0;
  ErrorLookup[ELEVGT].participants = 1;
  ErrorLookup[ELEVGT].tol_desc1 = "Elevation value limit";
  ErrorLookup[ELEVGT].tol_desc2 = "Show points (0) billboards (1) both (2)";
  ErrorLookup[ELEVGT].range = 0;
  
  ErrorLookup[ELEVLT].mygroup = RANGE_GROUP;
  ErrorLookup[ELEVLT].numthresholds = 2;
  ErrorLookup[ELEVLT].usemagnitude = 1;
  ErrorLookup[ELEVLT].lowrange  = -10000000.0;
  ErrorLookup[ELEVLT].highrange =  10000000.0;
  ErrorLookup[ELEVLT].lowrange2 = 0.0;
  ErrorLookup[ELEVLT].highrange2 = 2.0;
  ErrorLookup[ELEVLT].participants = 1;
  ErrorLookup[ELEVLT].tol_desc1 = "Elevation value limit";
  ErrorLookup[ELEVLT].tol_desc2 = "Show points (0) billboards (1) both (2)";
  ErrorLookup[ELEVLT].range = 0;
  
  
  ErrorLookup[ELEVEQ].mygroup = RANGE_GROUP;
  ErrorLookup[ELEVEQ].numthresholds = 3;
  ErrorLookup[ELEVEQ].usemagnitude = 1;
  ErrorLookup[ELEVEQ].lowrange   = -10000000.0;
  ErrorLookup[ELEVEQ].highrange  =  10000000.0;
  ErrorLookup[ELEVEQ].lowrange2  = -10000000.0;
  ErrorLookup[ELEVEQ].highrange2 =  10000000.0;
  ErrorLookup[ELEVEQ].lowrange3 = 0.0;
  ErrorLookup[ELEVEQ].highrange3 = 2.0;
  ErrorLookup[ELEVEQ].range = 1;
  ErrorLookup[ELEVEQ].participants = 1;
  ErrorLookup[ELEVEQ].tol_desc1 = "Elevation lower (closed) limit";
  ErrorLookup[ELEVEQ].tol_desc2 = "Elevation upper (closed) limit";
  ErrorLookup[ELEVEQ].tol_desc3 = "Show points (0) billboards (1) both (2)";

  ErrorLookup[ELEVEQOPEN].mygroup = RANGE_GROUP;
  ErrorLookup[ELEVEQOPEN].numthresholds = 3;
  ErrorLookup[ELEVEQOPEN].usemagnitude = 1;
  ErrorLookup[ELEVEQOPEN].lowrange   = -10000000.0;
  ErrorLookup[ELEVEQOPEN].highrange  =  10000000.0;
  ErrorLookup[ELEVEQOPEN].lowrange2  = -10000000.0;
  ErrorLookup[ELEVEQOPEN].highrange2 =  10000000.0;
  ErrorLookup[ELEVEQOPEN].lowrange3 = 0.0;
  ErrorLookup[ELEVEQOPEN].highrange3 = 2.0;
  ErrorLookup[ELEVEQOPEN].range = 1;
  ErrorLookup[ELEVEQOPEN].participants = 1;
  ErrorLookup[ELEVEQOPEN].tol_desc1 = "Elevation lower (open) limit";
  ErrorLookup[ELEVEQOPEN].tol_desc2 = "Elevation upper (open) limit";
  ErrorLookup[ELEVEQOPEN].tol_desc3 = "Show points (0) billboards (1) both (2)";
  
  
  ErrorLookup[ENCONNECT].mygroup = CONNECT_GROUP;
  ErrorLookup[ENCONNECT].numthresholds = 2;
  ErrorLookup[ENCONNECT].usemagnitude = 2;
  ErrorLookup[ENCONNECT].lowrange   = 0.0;
  ErrorLookup[ENCONNECT].highrange  =  100.0;
  ErrorLookup[ENCONNECT].lowrange2  = 0.0;
  ErrorLookup[ENCONNECT].highrange2 =  100.0;
  ErrorLookup[ENCONNECT].range = 1;
  ErrorLookup[ENCONNECT].participants = 2;
  ErrorLookup[ENCONNECT].tol_desc1 = "Number of connections lower limit";
  ErrorLookup[ENCONNECT].tol_desc2 = "Number of connections upper limit";


  ErrorLookup[BADENCON].mygroup = CONNECT_GROUP;
  ErrorLookup[BADENCON].numthresholds = 1;
  ErrorLookup[BADENCON].usemagnitude = 0;
  ErrorLookup[BADENCON].lowrange   = 0.0;
  ErrorLookup[BADENCON].highrange  =  25.0;
  ErrorLookup[BADENCON].range = 0;
  ErrorLookup[BADENCON].participants = 2;
  ErrorLookup[BADENCON].tol_desc1 = "Coordinate equality limit";


  ErrorLookup[EXTRA_NET].mygroup = CONNECT_GROUP;
  ErrorLookup[EXTRA_NET].numthresholds = 2;
  ErrorLookup[EXTRA_NET].usemagnitude = 1;
  ErrorLookup[EXTRA_NET].lowrange   = 0.0;
  ErrorLookup[EXTRA_NET].highrange  =  0.9;
  ErrorLookup[EXTRA_NET].lowrange2  = 5.0;
  ErrorLookup[EXTRA_NET].highrange2 =  20.0;
  ErrorLookup[EXTRA_NET].range = 1;
  ErrorLookup[EXTRA_NET].participants = 1;
  ErrorLookup[EXTRA_NET].tol_desc1 = "Coordinate proximity lower limit";
  ErrorLookup[EXTRA_NET].tol_desc2 = "Coordinate proximity upper limit";
  ErrorLookup[EXTRA_NET].checkapplies = (char) BUSTEDcheck;

  ErrorLookup[INTRA_NET].mygroup = CONNECT_GROUP;
  ErrorLookup[INTRA_NET].numthresholds = 2;
  ErrorLookup[INTRA_NET].usemagnitude = 1;
  ErrorLookup[INTRA_NET].lowrange   = 0.0;
  ErrorLookup[INTRA_NET].highrange  =  0.9;
  ErrorLookup[INTRA_NET].lowrange2  = 5.0;
  ErrorLookup[INTRA_NET].highrange2 =  20.0;
  ErrorLookup[INTRA_NET].range = 1;
  ErrorLookup[INTRA_NET].participants = 1;
  ErrorLookup[INTRA_NET].tol_desc1 = "Coordinate proximity lower limit";
  ErrorLookup[INTRA_NET].tol_desc2 = "Coordinate proximity upper limit";
  ErrorLookup[INTRA_NET].checkapplies = (char) BUSTEDcheck;


  ErrorLookup[CREATENET].mygroup = CONNECT_GROUP;
  ErrorLookup[CREATENET].numthresholds = 1;
  ErrorLookup[CREATENET].usemagnitude = 1;
  ErrorLookup[CREATENET].lowrange   = 0.0;
  ErrorLookup[CREATENET].highrange  =  0.9;
  ErrorLookup[CREATENET].lowrange2  = 5.0;
  ErrorLookup[CREATENET].highrange2 =  20.0;
  ErrorLookup[CREATENET].range = 1;
  ErrorLookup[CREATENET].participants = 1;
  ErrorLookup[CREATENET].tol_desc1 = "Coordinate proximity upper limit";
  ErrorLookup[CREATENET].tol_desc2 = "Tolerance no longer used";
  ErrorLookup[CREATENET].checkapplies = (char) BUSTEDcheck;

  ErrorLookup[NETISOFEAT].mygroup = INTERSECT_GROUP;
  ErrorLookup[NETISOFEAT].numthresholds = 2;
  ErrorLookup[NETISOFEAT].lowrange = 0.0;
  ErrorLookup[NETISOFEAT].highrange = 0.9;
  ErrorLookup[NETISOFEAT].lowrange2  = 0.0;
  ErrorLookup[NETISOFEAT].highrange2 =  1.0;
  ErrorLookup[NETISOFEAT].usemagnitude = 0;
  ErrorLookup[NETISOFEAT].participants = 3;
  ErrorLookup[NETISOFEAT].tol_desc1 = "Coordinate proximity upper limit";
  ErrorLookup[NETISOFEAT].tol_desc2 = "Intersections (1) / Connecting Nodes (0)";
  
  
  ErrorLookup[FEATSPIKE].mygroup = ZVAL_GROUP;
  ErrorLookup[FEATSPIKE].numthresholds = 2;
  ErrorLookup[FEATSPIKE].usemagnitude = 1;
  ErrorLookup[FEATSPIKE].lowrange = 0.0;
  ErrorLookup[FEATSPIKE].highrange = 180.0;
  ErrorLookup[FEATSPIKE].participants = 1;
  ErrorLookup[FEATSPIKE].tol_desc1 = "Angle limit";
  ErrorLookup[FEATSPIKE].lowrange2 = 0.0;
  ErrorLookup[FEATSPIKE].highrange2 = 100.0;
  ErrorLookup[FEATSPIKE].tol_desc2 = "Segment Length";
  
  ErrorLookup[ELEVADJCHANGE].mygroup = ZVAL_GROUP;
  ErrorLookup[ELEVADJCHANGE].numthresholds = 1;
  ErrorLookup[ELEVADJCHANGE].usemagnitude = 1;
  ErrorLookup[ELEVADJCHANGE].lowrange = 0.0;
  ErrorLookup[ELEVADJCHANGE].highrange = 1000000.0;
  ErrorLookup[ELEVADJCHANGE].participants = 1;
  ErrorLookup[ELEVADJCHANGE].tol_desc1 = "Z value difference limit";
  
  
  
  ErrorLookup[SLOPEDIRCH].mygroup = ZVAL_GROUP;
  ErrorLookup[SLOPEDIRCH].numthresholds = 1;
  ErrorLookup[SLOPEDIRCH].usemagnitude = 1;
  ErrorLookup[SLOPEDIRCH].lowrange = 0.0;
  ErrorLookup[SLOPEDIRCH].highrange = 180.0;
  ErrorLookup[SLOPEDIRCH].participants = 1;
  ErrorLookup[SLOPEDIRCH].tol_desc1 = "Angle limit";

  ErrorLookup[CLAMP_SDC].mygroup = NEW_DEM_GROUP;
  ErrorLookup[CLAMP_SDC].numthresholds = 2;
  ErrorLookup[CLAMP_SDC].usemagnitude = 1;
  ErrorLookup[CLAMP_SDC].lowrange = 0.0;
  ErrorLookup[CLAMP_SDC].highrange = 180.0;
  ErrorLookup[CLAMP_SDC].lowrange2 = 0.0;
  ErrorLookup[CLAMP_SDC].highrange2 = 1000000.0;
  ErrorLookup[CLAMP_SDC].range = 0;
  ErrorLookup[CLAMP_SDC].participants = 2;
  ErrorLookup[CLAMP_SDC].tol_desc1 = "Angle limit";
  ErrorLookup[CLAMP_SDC].tol_desc2 = "Offset from monotonic slope";

  
  ErrorLookup[LJOINSLOPEDC].mygroup = ZVAL_GROUP;
  ErrorLookup[LJOINSLOPEDC].numthresholds = 1;
  ErrorLookup[LJOINSLOPEDC].usemagnitude = 1;
  ErrorLookup[LJOINSLOPEDC].lowrange = 0.0;
  ErrorLookup[LJOINSLOPEDC].highrange = 180.0;
  ErrorLookup[LJOINSLOPEDC].participants = 2;
  ErrorLookup[LJOINSLOPEDC].tol_desc1 = "Angle limit";

  ErrorLookup[CLAMP_JOINSDC].mygroup = NEW_DEM_GROUP;
  ErrorLookup[CLAMP_JOINSDC].numthresholds = 3;
  ErrorLookup[CLAMP_JOINSDC].usemagnitude = 1;
  ErrorLookup[CLAMP_JOINSDC].lowrange = 0.0;
  ErrorLookup[CLAMP_JOINSDC].highrange = 180.0;
  ErrorLookup[CLAMP_JOINSDC].lowrange2 = 0.0;
  ErrorLookup[CLAMP_JOINSDC].highrange2 = 10000000.0;
  ErrorLookup[CLAMP_JOINSDC].lowrange3 = 0.0;
  ErrorLookup[CLAMP_JOINSDC].highrange3 = 10.0;
  ErrorLookup[CLAMP_JOINSDC].participants = 3;
  ErrorLookup[CLAMP_JOINSDC].tol_desc1 = "Angle limit";
  ErrorLookup[CLAMP_JOINSDC].tol_desc2 = "Offset from monotonic slope";
  ErrorLookup[CLAMP_JOINSDC].tol_desc3 = "Coordinate proximity limit";

  
  ErrorLookup[LOOPS].mygroup = KINK_GROUP;
  ErrorLookup[LOOPS].participants = 1;

  ErrorLookup[P_O_LOOP].mygroup = KINK_GROUP;
  ErrorLookup[P_O_LOOP].participants = 1;
  

 
  ErrorLookup[ENDPTINT].mygroup = COMPO_GROUP;
  ErrorLookup[ENDPTINT].participants = 1;


  ErrorLookup[LATTRCHNG].mygroup = CONNECT_GROUP;
  ErrorLookup[LATTRCHNG].participants = 1;
  
  
  ErrorLookup[G_DUPS].participants = 2;
  ErrorLookup[G_DUPS].mygroup = DUP_GROUP;
  ErrorLookup[G_DUPS].draw_wholeareal = 1;

  
  ErrorLookup[C_DUPS].participants = 1;
  ErrorLookup[C_DUPS].mygroup = DUP_GROUP;
  ErrorLookup[C_DUPS].draw_wholeareal = 1;

  ErrorLookup[SAMEID_GDUP].participants = 2;
  ErrorLookup[SAMEID_GDUP].mygroup = DUP_GROUP;
  ErrorLookup[SAMEID_GDUP].draw_wholeareal = 1;
  ErrorLookup[SAMEID_GDUP].checkapplies = (char) GAITcheck;

  ErrorLookup[SAMEID].participants = 2;
  ErrorLookup[SAMEID].mygroup = DUP_GROUP;
  ErrorLookup[SAMEID].draw_wholeareal = 1;
  ErrorLookup[SAMEID].checkapplies = (char) GAITcheck;
  

  ErrorLookup[SAMEID_CDUP].participants = 1;
  ErrorLookup[SAMEID_CDUP].mygroup = DUP_GROUP;
  ErrorLookup[SAMEID_CDUP].draw_wholeareal = 1;
  ErrorLookup[SAMEID_CDUP].checkapplies = (char) GAITcheck;

  ErrorLookup[ANY_SAMEID].participants = 2;
  ErrorLookup[ANY_SAMEID].mygroup = DUP_GROUP;
  ErrorLookup[ANY_SAMEID].draw_wholeareal = 1;
  ErrorLookup[ANY_SAMEID].checkapplies = (char) GAITcheck;
  
  
  
  if(NGA_TYPE == 1)
    {
      ErrorLookup[SLIVER].numthresholds = 1;
      ErrorLookup[SLIVER].usemagnitude = 1;
      ErrorLookup[SLIVER].lowrange = 0.0;
      ErrorLookup[SLIVER].highrange = 1.0;
      ErrorLookup[SLIVER].participants = 1;
    }
  else
    {
      ErrorLookup[SLIVER].numthresholds = 1;
      ErrorLookup[SLIVER].usemagnitude = 1;
      ErrorLookup[SLIVER].lowrange = 0.0;
      ErrorLookup[SLIVER].highrange = 1.0;
      ErrorLookup[SLIVER].participants = 1;
    }
  ErrorLookup[SLIVER].mygroup = COMPO_GROUP;
  ErrorLookup[SLIVER].draw_wholeareal = 1;
  ErrorLookup[SLIVER].tol_desc1 = "Area to perimeter ratio limit";

  ErrorLookup[FACESIZE].numthresholds = 4;
  ErrorLookup[FACESIZE].usemagnitude = 1;
  ErrorLookup[FACESIZE].lowrange = 0.1;
  ErrorLookup[FACESIZE].highrange = 700.0;
  ErrorLookup[FACESIZE].lowrange2 = 1.0;
  ErrorLookup[FACESIZE].highrange2 = 10000000.0;
  ErrorLookup[FACESIZE].lowrange3 = 0.0;
  ErrorLookup[FACESIZE].highrange3 = 100000.0;
  ErrorLookup[FACESIZE].lowrange4 = 0.0;
  ErrorLookup[FACESIZE].highrange4 = 1000.0;
  ErrorLookup[FACESIZE].participants = 2;
  ErrorLookup[FACESIZE].mygroup = COMPO_GROUP;
  ErrorLookup[FACESIZE].draw_wholeareal = 0;
  ErrorLookup[FACESIZE].tol_desc1 = "Face portion width limit";
  ErrorLookup[FACESIZE].tol_desc2 = "Feature minimum square area";
  ErrorLookup[FACESIZE].tol_desc3 = "Face minimum perimeter length";
  ErrorLookup[FACESIZE].tol_desc4 = "Feature proximity limit";

  ErrorLookup[NARROW].checkapplies = (char) SEEITcheck;
  ErrorLookup[NARROW].numthresholds = 1;
  ErrorLookup[NARROW].usemagnitude = 1;
  ErrorLookup[NARROW].lowrange = 0.0;
  ErrorLookup[NARROW].highrange = 0.1;
  ErrorLookup[NARROW].participants = 1;
  ErrorLookup[NARROW].mygroup = COMPO_GROUP;
  
  ErrorLookup[SMALLOBJ].checkapplies = (char) SEEITcheck;
  ErrorLookup[SMALLOBJ].numthresholds = 1;
  ErrorLookup[SMALLOBJ].usemagnitude = 1;
  ErrorLookup[SMALLOBJ].lowrange = 0.0;
  ErrorLookup[SMALLOBJ].highrange = 99.9;
  ErrorLookup[SMALLOBJ].participants = 1;
  ErrorLookup[SMALLOBJ].mygroup = SIZE_GROUP;
  
  ErrorLookup[HSLOPE].checkapplies = (char) SEEITcheck;
  ErrorLookup[HSLOPE].numthresholds = 1;
  ErrorLookup[HSLOPE].usemagnitude = 1;
  ErrorLookup[HSLOPE].lowrange = 10.0;
  ErrorLookup[HSLOPE].highrange = 90.0;
  ErrorLookup[HSLOPE].participants = 1;
  ErrorLookup[HSLOPE].mygroup = COMPO_GROUP;
  
  ErrorLookup[VERTSLOPE].checkapplies = (char) SEEITcheck;
  ErrorLookup[VERTSLOPE].participants = 1;
  ErrorLookup[VERTSLOPE].mygroup = COMPO_GROUP;
  
  ErrorLookup[VTEAR].checkapplies = (char) SEEITcheck;
  ErrorLookup[VTEAR].numthresholds = 1;
  ErrorLookup[VTEAR].usemagnitude = 1;
  ErrorLookup[VTEAR].lowrange = 0.0;
  ErrorLookup[VTEAR].highrange = 100000.0;
  ErrorLookup[VTEAR].participants = 1;
  ErrorLookup[VTEAR].mygroup = SEPDIST_GROUP;
  
  ErrorLookup[HTEAR].checkapplies = (char) SEEITcheck;
  ErrorLookup[HTEAR].numthresholds = 1;
  ErrorLookup[HTEAR].usemagnitude = 0;
  ErrorLookup[HTEAR].lowrange = 0.0;
  ErrorLookup[HTEAR].highrange = 10000.0;
  ErrorLookup[HTEAR].participants = 1;
  ErrorLookup[HTEAR].mygroup = OVERLAP_GROUP;
  
  ErrorLookup[OVERC].checkapplies = (char) SEEITcheck;
  ErrorLookup[OVERC].numthresholds = 2;
  ErrorLookup[OVERC].usemagnitude = 1;
  ErrorLookup[OVERC].lowrange = 0.0;
  ErrorLookup[OVERC].highrange = 100.0;
  ErrorLookup[OVERC].lowrange2  = 0.0001;
  ErrorLookup[OVERC].highrange2 =  100000.0;
  ErrorLookup[OVERC].participants = 1;
  ErrorLookup[OVERC].mygroup = OVERLAP_GROUP;
  
  ErrorLookup[TVERT].checkapplies = (char) SEEITcheck;
  ErrorLookup[TVERT].participants = 1;
  ErrorLookup[TVERT].usemagnitude = 0;
  ErrorLookup[TVERT].mygroup = CONNECT_GROUP;
  
  
  ErrorLookup[LMINT].checkapplies = (char) SEEITcheck;
  ErrorLookup[LMINT].participants = 2;
  ErrorLookup[LMINT].mygroup = INTERSECT_GROUP;
  
  
  ErrorLookup[LSPINT].checkapplies = (char) SEEITcheck;
  ErrorLookup[LSPINT].numthresholds = 1;
  ErrorLookup[LSPINT].usemagnitude = 1;
  ErrorLookup[LSPINT].lowrange = 0.0;
  ErrorLookup[LSPINT].highrange = 90.0;
  ErrorLookup[LSPINT].participants = 2;
  ErrorLookup[LSPINT].mygroup = INTERSECT_GROUP;
  
  ErrorLookup[LSPIEXP].numthresholds = 1;
  ErrorLookup[LSPIEXP].checkapplies = (char) SEEITcheck;
  ErrorLookup[LSPIEXP].usemagnitude = 1;
  ErrorLookup[LSPIEXP].lowrange = 0.0;
  ErrorLookup[LSPIEXP].highrange = 90.0;
  ErrorLookup[LSPIEXP].participants = 3;
  ErrorLookup[LSPIEXP].mygroup = INTERSECT_GROUP;
  
  ErrorLookup[POLYINAREA].checkapplies = (char) SEEITcheck;
  ErrorLookup[POLYINAREA].participants = 2;
  ErrorLookup[POLYINAREA].mygroup = INTERSECT_GROUP;
  
  ErrorLookup[POLYOSIDEAREA].participants = 2;
  ErrorLookup[POLYOSIDEAREA].mygroup = CONTAIN_GROUP;
  ErrorLookup[POLYOSIDEAREA].checkapplies = (char) SEEITcheck;
  
  ErrorLookup[POLYINTPOLY].checkapplies = (char) SEEITcheck;
  ErrorLookup[POLYINTPOLY].participants = 2;
  ErrorLookup[POLYINTPOLY].numthresholds = 0;
  ErrorLookup[POLYINTPOLY].mygroup = INTERSECT_GROUP;
  
  ErrorLookup[POLYINTAREA].checkapplies = (char) SEEITcheck;
  ErrorLookup[POLYINTAREA].participants = 2;
  ErrorLookup[POLYINTAREA].numthresholds = 0;
  ErrorLookup[POLYINTAREA].mygroup = INTERSECT_GROUP;
  
  
  for(i=1; i<=CONDITION_DEFINITIONS; i++)
   {
     if(ErrorLookup[i].numthresholds >= 1)
      {
	ErrorLookup[i].bigworse = 0;
      }
   }
  
  ErrorLookup[HSLOPE].bigworse              = 1;
  ErrorLookup[MASKEDIT_0].bigworse          = 1;
  ErrorLookup[L2D_L3D_MATCH].bigworse       = 1;
  ErrorLookup[LEZ_PROX_3D].bigworse         = 1;
  ErrorLookup[CNODE_ZBUST].bigworse         = 1;
  ErrorLookup[DUPLICATESEG].bigworse        = 1;
  ErrorLookup[SHAREPERIM].bigworse          = 1;
  ErrorLookup[VTEAR].bigworse               = 1;
  ErrorLookup[HTEAR].bigworse               = 1;
  ErrorLookup[OVERC].bigworse               = 1;
  ErrorLookup[LSPINT].bigworse              = 1;
  ErrorLookup[SHARESEG].bigworse            = 1;
  ErrorLookup[SHARE3SEG].bigworse           = 1;
  ErrorLookup[LLI_ANGLE].bigworse           = 1;
  ErrorLookup[LSPIEXP].bigworse             = 1;
  ErrorLookup[ISOTURN].bigworse             = 1;
  ErrorLookup[KINK].bigworse                = 1;
  ErrorLookup[Z_KINK].bigworse              = 1;
  ErrorLookup[L_A_KINK].bigworse            = 1;
  ErrorLookup[INTERNALKINK].bigworse        = 1;
  ErrorLookup[CONTEXT_KINK].bigworse        = 1;
  ErrorLookup[AREAKINK].bigworse            = 1;
  ErrorLookup[LONGFEAT].bigworse            = 1;
  ErrorLookup[SLOPEDIRCH].bigworse          = 1;
  ErrorLookup[CLAMP_SDC].bigworse           = 1;
  ErrorLookup[LJOINSLOPEDC].bigworse        = 1;
  ErrorLookup[CLAMP_JOINSDC].bigworse       = 1;
  ErrorLookup[GSPIKE].bigworse              = 1;
  ErrorLookup[AVGSPIKE].bigworse            = 1;
  ErrorLookup[GSHELF].bigworse              = 1;
  ErrorLookup[FLOWSTEP].bigworse            = 1;
  ErrorLookup[BREAKLINE].bigworse           = 1;
  ErrorLookup[FEATOUTSIDE].bigworse         = 1;
  ErrorLookup[GRID_STD_DEV].bigworse        = 1;
  ErrorLookup[ELEVGT].bigworse              = 1;
  ErrorLookup[ELEVADJCHANGE].bigworse       = 1;
  ErrorLookup[FEATSPIKE].bigworse           = 1;
  ErrorLookup[LODELEVDIF].bigworse          = 1;
  ErrorLookup[GRIDEXACTDIF].bigworse        = 1;
  ErrorLookup[MASKZERO].bigworse            = 1;
  ErrorLookup[MASKCONSTANT].bigworse        = 1;
  ErrorLookup[RAISEDPC].bigworse            = 1;
  ErrorLookup[PT_GRID_DIF].bigworse         = 1;
  ErrorLookup[MASKEDIT_1].bigworse          = 1;
  ErrorLookup[MULTIPARTL].bigworse          = 1;
  ErrorLookup[MULTIPARTA].bigworse          = 1;
  ErrorLookup[MULTIPARTP].bigworse          = 1;
  ErrorLookup[LLMULTINT].bigworse           = 1;
  ErrorLookup[LOC_MULTINT].bigworse         = 1;
  ErrorLookup[ZUNCLOSED].bigworse           = 1;
  ErrorLookup[AREAUNCLOSED].bigworse        = 1;
  ErrorLookup[BIGAREA].bigworse             = 1;
  ErrorLookup[NOT_FLAT].bigworse            = 1;
  ErrorLookup[CLAMP_NFLAT].bigworse         = 1;
  ErrorLookup[CLAMP_DIF].bigworse           = 1;
  ErrorLookup[CALC_AREA].bigworse           = 1;
  ErrorLookup[LONGSEG].bigworse             = 1;
  ErrorLookup[PC_SLOPE].bigworse            = 1;
  
  
  
  for(i=1; i<=CONDITION_DEFINITIONS; i++)
    {
      SetDefaultParticipants (i, 0, 0);
      SetDefaultSensitivities(i, 0, 0);
    }

  free(ConditionLookup);
  ConditionLookup = NULL;
}


