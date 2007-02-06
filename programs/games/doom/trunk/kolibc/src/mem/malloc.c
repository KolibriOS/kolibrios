
#include "mf.h"
#include "kolibri.h"

void * kmemcpy(void * dst, const void * src, size_t count);

static struct m_state ms;

void mf_init()
{ mchunkptr chp;
  int i;
  char  *p;
  
  dword psize=  0x40000;
  p = 0x2600000;      //(char*)UserAlloc(psize);
  ms.top = (mchunkptr)p;
  ms.topsize = psize;
  ms.smallmap=0;
  ms.treemap=0;
      
  chp = (mchunkptr)p;
  chp->head=psize|1;
    
  for(i=0; i<32; i++)
  {
    mchunkptr p = &ms.smallbins[i];
    p->fd = p;
    p->bk = p;
   
  };   
}

void *dlmalloc(size_t size)
{ size_t nb, psize,rsize;
  dword idx;
  dword smallbits;
  mchunkptr B,F,p,r;
  void *mem;

  nb = ((size+7)&~7)+8;   
  
  if (nb < 256)
  {
    idx = nb  >> 3;
//    smallbits= (-1<<idx) & ms.smallmap;
    _asm
    {
      mov ecx, [idx]
      or eax, -1
      shl eax, cl
      and eax, dword ptr [ms] 
      mov dword ptr [smallbits], eax    
    }
    if (smallbits)
    {
      _asm
      { bsf eax, dword ptr [smallbits]
        mov [idx], eax   
      };
    
      psize= idx<<3;
      rsize= psize-nb;

      B = &ms.smallbins[idx];
      p = B->fd;
      F = p->fd;
      if (B == F)
      {
//        ms.smallmap &=  ~(1<< idx);
        _asm
        {
          mov eax, [idx]
          btr dword ptr [ms], eax
        }
      }  
      B->fd = F;
      F->bk = B;

      if(rsize<16)
      { 
        p->head = psize|PINUSE_BIT|CINUSE_BIT;
        ((mchunkptr)((char*)p + psize))->head |= PINUSE_BIT;
        return chunk2mem(p);
      };
      p->head = nb|PINUSE_BIT|CINUSE_BIT;
      r = chunk_plus_offset(p, nb);
      r->head = rsize|PINUSE_BIT;
      ((mchunkptr)((char*)r + rsize))->prev_foot = rsize;
      {
        dword I;
        mchunkptr B;
        mchunkptr F;  
  
        I  = rsize>>3;
        ms.smallmap |=  1<< I;
        B = &ms.smallbins[I];
        F = B->fd;
        B->fd = r;
        F->bk = r;
        r->fd = F;
        r->bk = B;
       }
      return chunk2mem(p);
    }
    if (ms.treemap != 0 && (mem = malloc_small(nb)) != 0)
      return mem;  
  }
  else
  {
    if (ms.treemap != 0 && (mem = malloc_large(nb)) != 0)
      return mem;  
  };  
  
  if (nb < ms.topsize)
  { 
    size_t rsize = ms.topsize -= nb;
    mchunkptr p = ms.top;
    mchunkptr r = ms.top = chunk_plus_offset(p, nb);
    r->head = rsize | PINUSE_BIT;
    p->head = nb |PINUSE_BIT|CINUSE_BIT;
    return chunk2mem(p);
  };
 
  return 0; 
};

void dlfree(void *mem)
{ size_t psize;
  size_t prevsize;
  size_t nsize;
  mchunkptr next;
  mchunkptr prev;
  
  mchunkptr p  = mem2chunk(mem);
  if(p->head & CINUSE_BIT)
  {
    psize = p->head & (~3);
    next = chunk_plus_offset(p, psize);

    if(!(p->head & PINUSE_BIT))
    {
      prevsize = p->prev_foot;
      prev=(mchunkptr)(((char*)p) - prevsize);
      psize += prevsize;
      p = prev;
      if (prevsize < 256)
      {  
        dword I;
        mchunkptr F = p->fd;
        mchunkptr B = p->bk;
        I = prevsize>>3;
        if (F == B)
        {
          ms.smallmap &=  ~(1<< I);
        }
        F->bk = B;
        B->fd = F;
      }  
      else
        unlink_large_chunk((tchunkptr)p);
    };
           
    if(next->head & PINUSE_BIT)
    {
      if (! (next->head & CINUSE_BIT))
      {  
        if (next == ms.top)
        {  size_t tsize = ms.topsize += psize;
           ms.top = p;
           p->head = tsize | PINUSE_BIT;
           return;
         }
           
         nsize = next->head & ~INUSE_BITS;
         psize += nsize;

         if (nsize < 256)
         {
           dword I;
           mchunkptr F = next->fd;
           mchunkptr B = next->bk;
           I = nsize>>3;
           if (F == B)
             ms.smallmap &=  ~(1<< I);
           F->bk = B;
           B->fd = F;
         }
         else
           unlink_large_chunk((tchunkptr)next);
         
         p->head = psize|PINUSE_BIT;
         ((mchunkptr)((char*)p+psize))->prev_foot = psize;
      }
      else
      {
        next->head &= ~PINUSE_BIT;
        p->head = psize|PINUSE_BIT;
        ((mchunkptr)((char*)p+psize))->prev_foot = psize;
      };
      insert_chunk(p,psize);
    }; 
  };
}

static void insert_chunk(mchunkptr P, size_t S)
{
  dword I;
  mchunkptr B;
  mchunkptr F;  
  if (S < 256)
  {
    I  = S>>3;
    B = &ms.smallbins[I];
    F = B->fd;
    ms.smallmap |=  1<< I;
   
    B->fd = P;
    F->bk = P;
    P->fd = F;
    P->bk = B;
  }
  else
    insert_large_chunk((tchunkptr)P, S);
};  

/********
static void insert_small_chunk(mchunkptr p, size_t s)
{
  DWORD I  = s>>3;
   
  mchunkptr B = &ms.smallbins[I];
  mchunkptr F = B->fd;
  if (!(ms.smallmap & 1<<I))
    ms.smallmap |=  1<< I;

  B->fd = p;
  F->bk = p;
  p->fd = F;
  p->bk = B;
}
*********/

static dword compute_tree_index(size_t s)
{  dword idx;

    _asm
    {   mov edx, [s]
        shr edx, 8  
        bsr eax, edx
        lea ecx, [eax+7]
        mov edx, [s]
        shr edx, cl
        and edx, 1
        lea eax, [edx+eax*2]
        mov [idx], eax
    };

    return idx;
};

static void insert_large_chunk(tchunkptr X, size_t S)
{
  tbinptr* H;
  dword I;
  I = compute_tree_index(S);
  H = &ms.treebins[I];
  X->index = I;
  X->child[0] = X->child[1] = 0;
  if (!(ms.treemap & 1<<I))
  {
    ms.treemap |= 1<<I;
    *H = X;
    X->parent = (tchunkptr)H;
    X->fd = X->bk = X;
  }
  else
  {
    tchunkptr T = *H;
    size_t K = S << leftshift_for_tree_index(I);
    for (;;)
    {
      if ((T->head & ~INUSE_BITS) != S)
      {
        tchunkptr* C = &(T->child[(K >> 31) & 1]);
        K <<= 1;
        if (*C != 0)
          T = *C;
        else
        {
          *C = X;
          X->parent = T;
          X->fd = X->bk = X;
          break;
        }
      }
      else
      {
        tchunkptr F = T->fd;
        T->fd = F->bk = X;
        X->fd = F;
        X->bk = T;
        X->parent = 0;
        break;
      };
    };
  };
};

static void* malloc_small(size_t nb)
{
  tchunkptr t, v;
  mchunkptr r;
  size_t rsize;
  dword i;
  
  _asm
  { bsf ecx,dword ptr [ms+4]
    mov [i], ecx
  }  
  v = t =  ms.treebins[i];
  rsize = (t->head & ~INUSE_BITS) - nb;

  while ((t = leftmost_child(t)) != 0)
  {
    size_t trem = (t->head & ~INUSE_BITS) - nb;
    if (trem < rsize)
    { rsize = trem;
      v = t;
    }
  }

  r = chunk_plus_offset((mchunkptr)v, nb);
  unlink_large_chunk(v);
  if (rsize < 16)
  {
    v->head = (rsize + nb)|PINUSE_BIT|CINUSE_BIT;
    ((mchunkptr)((char*)v+rsize + nb))->head |= PINUSE_BIT;
  }
  else
  {
    v->head = nb|PINUSE_BIT|CINUSE_BIT;
    r->head = rsize|PINUSE_BIT;
    ((mchunkptr)((char*)r+rsize))->prev_foot = rsize;
    insert_chunk(r, rsize);
//    replace_dv(r, rsize);
  }
  return chunk2mem(v);
}

/********
static void replace_dv(mchunkptr P, size_t S)
{
  size_t DVS = ms.dvsize;
  if (DVS != 0)
  {
    mchunkptr DV = ms.dv;
    insert_small_chunk(DV, DVS);
  }
  ms.dvsize = S;
  ms.dv = P;
}
***********/

void static unlink_large_chunk(tchunkptr X)
{
  tchunkptr XP = X->parent;
  tchunkptr R;
  if (X->bk != X)
  {
    tchunkptr F = X->fd;
    R = X->bk;
    F->bk = R;
    R->fd = F;
  }
  else
  {
    tchunkptr* RP;
    if (((R = *(RP = &(X->child[1]))) != 0) ||
        ((R = *(RP = &(X->child[0]))) != 0))
    {
      tchunkptr* CP;
      while ((*(CP = &(R->child[1])) != 0) ||
             (*(CP = &(R->child[0])) != 0))
      {
        R = *(RP = CP);
      }
      *RP = 0;
    }
  }
  if (XP != 0)
  {
    tbinptr* H = &ms.treebins[X->index];
    if (X == *H)
    {
      if ((*H = R) == 0)
        ms.treemap &= ~(1<<X->index);
    }
    else
    {
      if (XP->child[0] == X) 
        XP->child[0] = R;
      else 
        XP->child[1] = R;
    };    
    if (R != 0)
    {
      tchunkptr C0, C1;
      R->parent = XP;
      if ((C0 = X->child[0]) != 0)
      {
          R->child[0] = C0;
          C0->parent = R;
      }
      if ((C1 = X->child[1]) != 0)
      {
        R->child[1] = C1;
        C1->parent = R;
      }
    }
  }
}

static void* malloc_large(size_t nb)
{
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  dword idx;
  idx = compute_tree_index(nb);

  if ((t = ms.treebins[idx]) != 0)
  {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_tree_index(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    for (;;)
    {
      tchunkptr rt;
      size_t trem = (t->head & ~INUSE_BITS) - nb;

      if (trem < rsize)
      {
        v = t;
        if ((rsize = trem) == 0)
          break;
      }
      rt = t->child[1];
      t = t->child[(sizebits >> 31) & 1];
      if (rt != 0 && rt != t)
        rst = rt;
      if (t == 0)
      {
        t = rst; /* set t to least subtree holding sizes > nb */
        break;
      }
      sizebits <<= 1;
    }
  }

  if (t == 0 && v == 0)
  { /* set t to root of next non-empty treebin */
    dword leftbits = (-1<<idx) & ms.treemap;
    if (leftbits != 0)
    { dword i;
      _asm
      { bsf eax, [leftbits]
        mov [i], eax
      }
      t = ms.treebins[i];
    }
  }

  while (t != 0)
  { /* find smallest of tree or subtree */
    size_t trem = (t->head & ~INUSE_BITS) - nb;
    if (trem < rsize)
    {
      rsize = trem;
      v = t;
    }
    t = leftmost_child(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  if (v != 0)
  {
    mchunkptr r = chunk_plus_offset((mchunkptr)v, nb);
    unlink_large_chunk(v);
    if (rsize < 16)
    {      
      v->head = (rsize + nb)|PINUSE_BIT|CINUSE_BIT;
      ((mchunkptr)((char*)v+rsize + nb))->head |= PINUSE_BIT;
    }  
    else
    {
      v->head = nb|PINUSE_BIT|CINUSE_BIT;
      r->head = rsize|PINUSE_BIT;
      ((mchunkptr)((char*)r+rsize))->prev_foot = rsize;
      insert_chunk((mchunkptr)r, rsize);
    }
    return chunk2mem(v);
  }
  return 0;
}

static void* internal_realloc(struct m_state *m, void* oldmem,
                               size_t bytes);

void* dlrealloc(void* oldmem, size_t bytes)
{
  if (oldmem == 0)
    return dlmalloc(bytes);
  else
  {
    struct m_state *m = &ms;
    return internal_realloc(m, oldmem, bytes);
  }
};

#define check_inuse_chunk(M,P)

#define MAX_REQUEST 256*1024*1024
#define set_inuse(M,p,s)\
  ((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
  ((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

static void* internal_realloc(struct m_state *m, void* oldmem, size_t bytes)
{ mchunkptr oldp;
  size_t oldsize;
  mchunkptr next;
  mchunkptr newp;
  void* extra;
  
  if (bytes >= MAX_REQUEST)
    return 0;
  
  oldp = mem2chunk(oldmem);
  oldsize = oldp->head & ~INUSE_BITS;
  next = chunk_plus_offset(oldp, oldsize);
  newp = 0;
  extra = 0;

    /* Try to either shrink or extend into top. Else malloc-copy-free */

  if( (oldp->head & CINUSE_BIT) &&
       ((char*)oldp < (char*)next)&&
       (next->head & PINUSE_BIT))
  {
    size_t nb = ((bytes+7)&~7)+8;
    if (oldsize >= nb)
    { /* already big enough */
      size_t rsize = oldsize - nb;
      newp = oldp;
      if (rsize >= 16)
      {
        mchunkptr remainder = chunk_plus_offset(newp, nb);
        set_inuse(m, newp, nb);
        set_inuse(m, remainder, rsize);
        extra = chunk2mem(remainder);
      }
    }
    else
    if (next == m->top && oldsize + m->topsize > nb)
    {
        /* Expand into top */
      size_t newsize = oldsize + m->topsize;
      size_t newtopsize = newsize - nb;
      mchunkptr newtop = chunk_plus_offset(oldp, nb);
      set_inuse(m, oldp, nb);
      newtop->head = newtopsize |PINUSE_BIT;
      m->top = newtop;
      m->topsize = newtopsize;
      newp = oldp;
    }
  }
  else
  {
    return 0;
  }

 
  if (newp != 0)
  {
    if (extra != 0) 
       dlfree(extra);
      
    check_inuse_chunk(m, newp);
    return chunk2mem(newp);
  }
  else
  {
    void* newmem = dlmalloc(bytes);
    if (newmem != 0)
    {
       size_t oc = oldsize - 4;
       memcpy(newmem, oldmem, (oc < bytes)? oc : bytes);
       dlfree(oldmem);
    }
    return newmem;
  }
  return 0;
}
  
