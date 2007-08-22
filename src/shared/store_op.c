/* @(#) $Id$ */

/* Copyright (C) 2007 Daniel B. Cid <dcid@ossec.net>
 * All right reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

/* Common API for dealing with ordered lists.
 * Provides a fast search on average (n/2).
 */ 


#include "shared.h"


/* Create the list storage 
 * Return NULL on error
 */
OSStore *OSStore_Create()
{
    OSStore *my_list;

    my_list = calloc(1, sizeof(OSStore));
    if(!my_list)
        return(NULL);
    
    my_list->first_node = NULL;
    my_list->last_node = NULL;
    my_list->cur_node = NULL;
    my_list->currently_size = 0;
    my_list->max_size = 0;
    my_list->free_data_function = NULL;
    
    return(my_list);
}


/* Set the maximum number of elements
 * in the storage. Returns 0 on error or
 * 1 on success.
 */
int OSStore_SetMaxSize(OSStore *list, int max_size)
{
    if(!list)
    {
        return(0);
    }
    
    /* Minimum size is 1 */
    if(max_size <= 1)
    {
        return(0);
    }

    list->max_size = max_size;

    return(1);
}


/* Set the pointer to the function to free the memory
 * data.
 */
int OSStore_SetFreeDataPointer(OSStore *list, void *free_data_function)
{
    if(!list)
    {
        return(0);
    }
    
    list->free_data_function = free_data_function;
    return(1);
}



/* Get key position from storage
 * Returns 0 if not present or the key
 * if available.
 * (position may change after each PUT)
 */
int OSStore_GetPosition(OSStore *list, char *key)
{
    int chk_rc, pos = 1;
    list->cur_node = list->first_node;

    while(list->cur_node)
    {
        if((chk_rc = strcmp(list->cur_node->key, key)) >= 0)
        {
            /* Found */
            if(chk_rc == 0)
                return(pos);

            /* Not found */
            return(0);
        }

        list->cur_node = list->cur_node->next;
        pos++;
    }
    return(0);
}



/* Get data from storage.
 * Returns NULL if not present.
 */
void *OSStore_Get(OSStore *list, char *key)
{
    int chk_rc;
    list->cur_node = list->first_node;
    
    while(list->cur_node)
    {
        if((chk_rc = strcmp(list->cur_node->key, key)) >= 0)
        {
            /* Found */
            if(chk_rc == 0)
                return(list->cur_node->data);

            /* Not found */    
            return(NULL);
        }

        list->cur_node = list->cur_node->next;
    }
    return(NULL);
}



/* Check if key is present on storage.
 * Returns 0 if not present.
 */
int OSStore_Check(OSStore *list, char *key)
{
    int chk_rc;
    list->cur_node = list->first_node;
    
    while(list->cur_node)
    {
        if((chk_rc = strcmp(list->cur_node->key, key)) >= 0)
        {
            /* Found */
            if(chk_rc == 0)
                return(1);

            /* Not found */    
            return(0);
        }

        list->cur_node = list->cur_node->next;
    }
    return(0);
}



/* Check if key is present on storage (using strncmp).
 * Returns 0 if not present.
 */
int OSStore_NCheck(OSStore *list, char *key)
{
    int chk_rc;
    list->cur_node = list->first_node;

    while(list->cur_node)
    {
        if((chk_rc = strncmp(list->cur_node->key, key, 
                             list->cur_node->key_size)) >= 0)
        {
            /* Found */
            if(chk_rc == 0)
                return(1);

            /* Not found */
            return(0);
        }

        list->cur_node = list->cur_node->next;
    }
    return(0);
}



/* Check if key is present on storage (case insensitive).
 * Returns 0 if not present.
 */
int OSStore_NCaseCheck(OSStore *list, char *key)
{
    int chk_rc;
    list->cur_node = list->first_node;

    while(list->cur_node)
    {
        if((chk_rc = strncasecmp(list->cur_node->key, key,
                                 list->cur_node->key_size)) == 0)
        {
            return(1);
        }

        list->cur_node = list->cur_node->next;
    }
    return(0);
}



/* Delete this node from list
 * Pointer goes to the next node available.
 */
void OSStore_Delete(OSStore *list, char *key)
{
    return;
}



/* Add data to the list
 * Returns 1 on sucess and 0 on failure
 */
int OSStore_Put(OSStore *list, char *key, void *data)
{
    int chk_rc;
    OSStoreNode *newnode;    


    /* Allocating memory for new node */
    newnode = calloc(1, sizeof(OSStoreNode));
    if(!newnode)
    {
        merror(MEM_ERROR, __local_name);
        return(0);
    }

    newnode->prev = NULL;
    newnode->next = NULL;
    newnode->data = data;
    newnode->key = key;
    newnode->key_size = strlen(key);


    /* If we don't have first node, assign it */
    if(!list->first_node)
    {
        list->first_node = newnode;
        list->last_node = newnode;
    }
    
 
    /* Store the data in order */ 
    else
    {
        list->cur_node = list->first_node;
        while(list->cur_node)
        {
            if((chk_rc = strcmp(list->cur_node->key, key)) >= 0)
            {
                /* Duplicated entry */
                if(chk_rc == 0)
                {
                    return(1);
                }
                
                /* If there is no prev node, it is because
                 * this is the first node. 
                 */
                if(list->cur_node->prev)
                    list->cur_node->prev->next = newnode;
                else
                    list->first_node = newnode;
                                            
                
                newnode->prev = list->cur_node->prev;
                
                list->cur_node->prev = newnode;
                newnode->next = list->cur_node;
                break;
            }

            list->cur_node = list->cur_node->next;
        }

        /* New node is the higher key */
        if(!newnode->next)
        {
            list->last_node->next = newnode;
            newnode->prev = list->last_node;
            list->last_node = newnode;
        }
    }
    

    /* Increment list size */
    list->currently_size++;
    
    return(1);
}

/* EOF */
