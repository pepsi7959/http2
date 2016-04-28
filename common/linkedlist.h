
/*
   Copyright (C) 2008-2009, TORO, MIMO Tech Co., Ltd.

   linkedlist.h

   Revision history

   1.0.0   Initialize.

*/

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_ 1

#ifdef __cplusplus
extern "C" {
#endif


/*
   item
   {
      *prev;
      *next;
   }
*/

#define LINKEDLIST_PREPEND(_first,_item)                    \
{                                                           \
   if ((_first) == NULL)                                    \
   {                                                        \
      (_first) = (_item)->prev = (_item)->next = (_item);   \
   }                                                        \
   else                                                     \
   {                                                        \
      (_item)->prev = (_first)->prev;                       \
      (_item)->next = (_first);                             \
      (_first)->prev->next = (_item);                       \
      (_first)->prev = (_item);                             \
      (_first) = (_item);                                   \
   }                                                        \
}
#define LINKEDLIST_APPEND(_first,_item)                     \
{                                                           \
   if ((_first) == NULL)                                    \
   {                                                        \
      (_first) = (_item)->prev = (_item)->next = (_item);   \
   }                                                        \
   else                                                     \
   {                                                        \
      (_item)->prev = (_first)->prev;                       \
      (_item)->next = (_first);                             \
      (_first)->prev->next = (_item);                       \
      (_first)->prev = (_item);                             \
   }                                                        \
}
#define LINKEDLIST_REMOVE(_first,_item)                     \
{                                                           \
   if ((_item)->prev == NULL || (_item)->next == NULL)      \
      ;                                                     \
   else if ((_first) == (_item))                            \
   {                                                        \
      if ((_first)->next == (_item))                        \
         (_first) = NULL;                                   \
      else                                                  \
      {                                                     \
         (_first) = (_item)->next;                          \
         (_item)->next->prev = (_item)->prev;               \
         (_item)->prev->next = (_item)->next;               \
      }                                                     \
   }                                                        \
   else                                                     \
   {                                                        \
      (_item)->next->prev = (_item)->prev;                  \
      (_item)->prev->next = (_item)->next;                  \
   }                                                        \
   (_item)->prev = (_item)->next = NULL;                    \
}
#define LINKEDLIST_SHUFFLE(_first,_item)                    \
{                                                           \
   if ((_first) == (_item))                                 \
   {                                                        \
      if ((_first)->next == (_item))                        \
         ;                                                  \
      else                                                  \
      {                                                     \
         (_first) = (_item)->next;                          \
         (_item)->next->prev = (_item)->prev;               \
         (_item)->prev->next = (_item)->next;               \
         (_item)->prev = (_first)->prev;                    \
         (_item)->next = (_first);                          \
         (_first)->prev->next = (_item);                    \
         (_first)->prev = (_item);                          \
      }                                                     \
   }                                                        \
   else                                                     \
   {                                                        \
      (_item)->next->prev = (_item)->prev;                  \
      (_item)->prev->next = (_item)->next;                  \
      (_item)->prev = (_first)->prev;                       \
      (_item)->next = (_first);                             \
      (_first)->prev->next = (_item);                       \
      (_first)->prev = (_item);                             \
   }                                                        \
}


#ifdef __cplusplus
}
#endif

#endif

