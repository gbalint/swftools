#include <assert.h>
#include "../q.h"
#include "active.h"

actlist_t* actlist_new()
{
    NEW(actlist_t, a);
    return a;
}
void actlist_destroy(actlist_t*a)
{
    free(a);
}

void actlist_verify_and_dump(actlist_t*a, int32_t y)
{
    segment_t*s = a->list;
    assert(!s || !s->left);
    double lastx;
    while(s) {
        if(y) {
            double x = ((double)s->delta.x*(y-s->a.y)/s->delta.y)+s->a.x;
            if(s!=a->list) {
                if(lastx>x) fprintf(stderr, "?%f<->%f? ", lastx, x);
            }
            lastx = x;
        }
        assert(!s->left || s->left->right == s);
        assert(!s->right || s->right->left == s);
        fprintf(stderr, "[%d]", s->nr);
        s = s->right;
        if(s) fprintf(stderr, " ");
        else fprintf(stderr, "\n");
    }
}

segment_t* actlist_find(actlist_t*a, point_t p1, point_t p2)
{
    /* this runs in O(n) right now, and should be optimized using a splay
       tree to run in ammortized O(log(n)) 
       (update: currently only 2.5% of the algorithm's running time is spent here,
        so maybe we don't need to bother)
     */
    segment_t*last=0, *s = a->list;
    if(!s) return last;
    while(s) {
        double d = LINE_EQ(p1, s);
        if(d==0) {
            d = LINE_EQ(p2, s);
            if(d==0) {
                /* We default to always inserting the new segment to the right of the old segment.
                   We do this so that we don't place new segments into the middle of already
                   overlapping lines which may have intersections scheduled.
                 */
                //fprintf(stderr, "Notice: actlist_find: both points (%d,%d) and (%d,%d) exactly on segment [%d]\n", p1.x, p1.y, p2.x, p2.y, s->nr);
            }
        }
        if(d<0)
            break;
        last = s;
        s = s->right;
    }
    return last;
}

static void actlist_insert_after(actlist_t*a, segment_t*left, segment_t*s)
{
    s->left = left;
    if(left) {
        s->right = left->right;
    } else {
        s->right = a->list;
        a->list = s;
    }
    if(s->left) 
        s->left->right = s;
    if(s->right) 
        s->right->left = s;
    a->size++;
}

void actlist_insert(actlist_t*a, point_t p, segment_t*s)
{
    segment_t*left = actlist_find(a, p, s->b);
    actlist_insert_after(a, left, s);
}

void actlist_delete(actlist_t*a, segment_t*s)
{
    if(s->left) {
        s->left->right = s->right;
    } else {
        a->list = s->right;
    }
    if(s->right) {
        s->right->left = s->left;
    }
    s->left = s->right = 0;
    a->size--;
}
int actlist_size(actlist_t*a)
{
    return a->size;
}

segment_t* actlist_leftmost(actlist_t*a)
{
    return a->list;
}

segment_t* actlist_left(actlist_t*a, segment_t*s)
{
    return s->left;
}

segment_t* actlist_right(actlist_t*a, segment_t*s)
{
    if(s) return s->right;
    else  return a->list;
}

void actlist_swap(actlist_t*a, segment_t*s1, segment_t*s2)
{
    actlist_delete(a, s1);
    actlist_insert_after(a, s2, s1);
}
