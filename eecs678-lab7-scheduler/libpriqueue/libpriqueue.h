/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_


//nodeType structure to keep track of data, next, and prev
typedef struct _nodeType
{
  void * data;
  void * next;
  void * prev;

} nodeType;

/**
  Priqueue Data Structure
*/
typedef struct _priqueue_t
{
  int size;
  nodeType * top;
  nodeType * tail;
  int ( * comparer )( const void *, const void * );

} priqueue_t;




void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */
