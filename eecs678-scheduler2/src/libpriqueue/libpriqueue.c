/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"

/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init( priqueue_t* q, int ( *comparer )( const void*, const void* ) ) {
    //set all the base attributes to default settings
    q->size = 0;
    q->top = NULL;
    q->comparer = comparer;
}

/**
  Insert the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer( priqueue_t* q, void* ptr ) {
    
  if( q->size != 0 )
  {
    nodeType *adder = malloc( sizeof( nodeType ) );
    nodeType *temp = q->top;

    adder->data = ptr;
    adder->next = NULL;

    int i = 0;
    void *temp_ptr;
    q->size++;

    while( temp != NULL )
    {
      if( q->comparer( ptr, temp->data ) < 0 )
      {
        while( temp != NULL )
        {
          temp_ptr = temp->data;
          temp->data = adder->data;
          adder->data = temp_ptr;

          if( temp->next == NULL )
          {
            temp->next = adder;
            break;
          }

          temp = temp->next;
        }

        break;
      }

      i++;

      if( temp->next == NULL )
      {
        temp->next = adder;
        break;
      }

      temp = temp->next;
    }

    return i;
  }

  else
  {
   // create a temporary node and fill ti with the paramater ptr
    nodeType *temp = malloc( sizeof( nodeType ) );
    temp->data = ptr;
    temp->next = NULL;

    q->top = temp; // that's crazy

    return  q->size++;
  }

}

/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void* priqueue_peek( priqueue_t* q ) {
    if ( q->size > 0 ) {
        return q->top;
    }
    return NULL;
}

/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void* priqueue_poll( priqueue_t* q ) {

    //get the top of the queue
    if( q->size != 0 ){
        nodeType * tmp = q->top;
        if( q->top->next != NULL )
        {
            q->top = q->top->next;
        }
        else
        {
            q->top = NULL;
        }
        q->size--;

        void* tmp_ptr = tmp->data;
        free(tmp);
        return tmp_ptr;
    }
    else
    {
        return NULL;
    }

}

/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void* priqueue_at( priqueue_t* q, int index ) {
    nodeType* item = q->top;
    int top_index = 0;
     if(index == top_index){

        return item->data;  

     }
        
     while(top_index < q->size) {
    
            top_index++;
            item = item->next;
    
        if(top_index == index){

            return item->data;

        }
            
    }
	return NULL;
}

/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove( priqueue_t* q, void* ptr ) {
  if( q->size == 0 ){
    return 0;
  }	
  else
  {
    int count = 0;
    nodeType *temp = q->top,*prev = q->top;
    while( temp != NULL )
    {
      if( temp->data == ptr )
      {
        if( temp == q->top ) 
        {
          q->top = prev->next;
          prev = q->top;
        }

        else prev->next = temp->next;
        count++;
      }

      temp = temp->next;
    }
    q->size -= count;
    free( temp );
    return count;
  }
}

/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void* priqueue_remove_at( priqueue_t* q, int index ) {
    nodeType *to_remove;
    nodeType *trail;
    nodeType *lead;

  to_remove = q->top;

  if( to_remove == NULL || q->size - 1 < index ){
    return NULL;
  }
  else
  {
    if ( index == 0 )
    {
      q->top = q->top->next;
      q->size--;
      void *temp_ptr = to_remove->data;
      free( to_remove );

      return temp_ptr;
    }

    trail = to_remove;
    to_remove = to_remove->next;

    if( to_remove->next != NULL ) lead = to_remove->next;

    for( int i = 1; i < index; i++ )
    {
      if ( lead->next != NULL ) lead = lead->next;
      else lead = NULL;
      to_remove = to_remove->next;
      trail = trail->next;
    }

    trail->next = lead;
    q->size--;
    void *tempPtr = to_remove->data;
    free( to_remove );

    return tempPtr;
  }
}

/**
  Return the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size( priqueue_t* q ) {

    return q->size;

}

/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy( priqueue_t* q ) {
    nodeType *temp = q->top;
    nodeType *temp_hold; 

  q->top = NULL;

  while( temp != NULL )
  {
    temp_hold = temp->next;
    free( temp );
    temp = temp_hold;
    q->size--;
  }

  return;
}
