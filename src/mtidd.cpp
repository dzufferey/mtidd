//TODO look at https://github.com/johnyf/dd and https://github.com/utwente-fmt/sylvan


//the memory for V and T is not managed. The user needs to take care of it.
//Operations on the manager
//- IddManager<V,T> newManager(std::iterator<std::input_iterator_tag, V> variableOrdering)
//- Idd<V,T> create(V variable, T default = T.bottom) 
//- void clear()
//- void updateOrdering(std::iterator<std::input_iterator_tag, V> variableOrdering)
//what about automatically finding a good ordering ?
//what about simplification: remove the too small intervals (threashold) to shrink the IDD (lossy operation, hot to control the loss ?)

//Operation on the idds
//- not / !
//- union / +
//- intersection / *
//- T lookup(std::map<V,double> point)
//- Idd<V, T> renaming(std::map<V, V> substitution)
//- subset: how to compare the terminals ?

//- ...

// Since we are looking at C++, we should make those structures be classes
//TODO what is the open-close semantics of the interval and point solution
// start from -∞ open goes to +∞ open
// if a bound is open it neighbor is close and vice-versa, except for the extremities
typedef struct {
    int name;
    IddPartitionEntry_t* partition; // an array of pairs: (number, child) where the number is the lower/upper (?) bound and the child is the right/left 
    int partition_size
} IddNode_t;

typedef struct Idd {
    enum { NODE, TERMINAL } type;
    union { IddNode_t node; IddTerminal_t terminal; } value;
    uint32_t hashvalue;
    uint32_t id;
    //TODO reference counting to free memory: use C++ shared_ptr<T>
} Idd_t;


//////////////////
//////////////////
//////////////////

//TODO replace the cache by an IDD manager

//TODO replace by
// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
// https://github.com/scala/scala/blob/v2.12.1/src/library/scala/util/hashing/MurmurHash3.scala (list/product)
uint32_t iddHashGetHashValue(char *idd_string_rep) {
    uint32_t h = 0;
    int pos; /* position in idd_string_rep */
    for (pos = 0; idd_string_rep[pos] != ’\0’; pos++) {
        h = (64*h + idd_string_rep[pos]) % MAXHASH;
    }
    return h;
}

typedef struct {
    uint32_t tag;
    Idd_t *arg1;
    Idd_t *arg2;
    Idd_t *result;
} cacheentry_t;

int iddCacheInit(int s)
{
    iddCacheDestroy();
    size = s;
    entries = calloc(sizeof(cacheentry_t), s);
    return entries ? 1 : 0;
}

void iddCacheDestroy()
{
    free(entries);
    size = 0;
    entries = NULL;
}

cacheentry_t *iddCacheLookup(unsigned int hash)
{
    return &entries[hash % size];
}

//////////////////
//////////////////
//////////////////

Idd_t *iddOperatorDeepClone(Idd_t *idd, IddHash_t *hashtable) {
    Idd_t *result;
    result = iddOperatorDeepCloneHelper(hashtable, idd);
    return result;
}

Idd_t *iddOperatorDeepCloneHelper(IddHash_t *hashtable, Idd_t *idd) {
    Idd_t *result;
    int i;
    IddPartitionEntry_t *partition;
    int partition_size;
    cacheentry_t *entry;
    if (idd->type == TERMINAL) {
        if (idd->value.terminal == TRUE) {
            result = iddMakeTerminal(hashtable, TRUE);
            return result;
        } else {
            result = iddMakeTerminal(hashtable, FALSE);
            return result;
        }
    } else if (idd->type == NODE) {
        entry = iddCacheLookup(HASH1(idd->hashvalue));
        if (entry->tag == opcounter+1 && entry->arg1 == idd) {
            //printf("iddOperatorDeepCloneHelper: match found in operator cache\n");
            return entry->result;
        }
        partition = iddPartitionAlloc(idd->value.node.partition_size);
        partition_size = idd->value.node.partition_size;
        for (i = 0; i < idd->value.node.partition_size; i++) {
            //printf("iddOperatorDeepCloneHelper: i = %d\n", i);
            partition[i].lower_bound = idd->value.node.partition[i].lower_bound;
            partition[i].upper_bound = idd->value.node.partition[i].upper_bound;
            partition[i].idd = iddOperatorDeepCloneHelper(hashtable,
                    idd->value.node.partition[i].idd);
        }
        result = iddMakeNode(hashtable, idd->value.node.name,
                partition, partition_size);
        entry->tag = opcounter+1;
        entry->arg1 = idd;
        entry->result = result;
        return result;
    }
    return result;
}

//////////////////

Idd_t *iddNot(Idd_t *idd) {
    IddHash_t hashtable;
    Idd_t *result;
    opcounter++;
    iddHashInit(&hashtable, IDDHASHTBLSIZE);
    result = iddNotHelper(&hashtable, idd);
    iddHashFree(&hashtable);
    return result;
}

Idd_t *iddNotHelper(IddHash_t *hashtable, Idd_t *idd) {
    Idd_t *result = NULL;
    int i;
    IddPartitionEntry_t *partition;
    int partition_size;
    cacheentry_t *entry;
    /* If idd is a terminal just return the opposite */
    if (idd->type == TERMINAL) {
        if (idd->value.terminal == TRUE) {
            result = iddMakeTerminal(hashtable, FALSE);
        } else {
            result = iddMakeTerminal(hashtable, TRUE);
        }
        printf("iddNotHelper: DANGER! We shouldn’t get here.\n");
    } else if (idd->type == NODE) {
        entry = iddCacheLookup(HASH1(idd->hashvalue));
        if (entry->tag == opcounter && entry->arg1 == idd) {
            return entry->result;
        }
        partition = iddPartitionAlloc(idd->value.node.partition_size);
        partition_size = idd->value.node.partition_size;
        if(partition_size < 1) {
            printf("iddNotHelper: partition_size less than 1, exiting\n");
            exit(0);
        }
        for (i = 0; i < idd->value.node.partition_size; i++) {
            //printf("iddNotHelper: i = %d\n", i);
            partition[i].lower_bound = idd->value.node.partition[i].lower_bound;
            partition[i].upper_bound = idd->value.node.partition[i].upper_bound;
            partition[i].idd = iddNotHelper(hashtable,
                    idd->value.node.partition[i].idd);
        }
        result = iddMakeNode(hashtable,
                idd->value.node.name,
                partition,
                partition_size);
        entry->tag = opcounter;
        entry->arg1 = idd;
        entry->result = result;
    }
    return result;
}

//////////////////

Idd_t *iddAnd(Idd_t *a_idd, Idd_t *b_idd) {
    Idd_t *result;
    IddHash_t hashtable;
    iddHashInit(&hashtable, IDDHASHTBLSIZE);
    opcounter++;
    result = iddAndHelper(&hashtable, a_idd, b_idd);
    opcounter++;
    iddHashFree(&hashtable);
    return result;
}

Idd_t *iddAndHelper(IddHash_t *hashtable, Idd_t *a_idd, Idd_t *b_idd) {
    int err;
    IddPartitionEntry_t *partition;
    int partition_size;
    Idd_t *result = NULL;
    IddPartitionEntryTwo_t *merge_res;
    int merge_res_size, merge_res_used;
    cacheentry_t *entry;
    if (a_idd->type == TERMINAL && b_idd->type == TERMINAL) {
        if (a_idd->value.terminal == TRUE && b_idd->value.terminal == TRUE) {
            result = iddMakeTerminal(hashtable, TRUE);
        } else {
            result = iddMakeTerminal(hashtable, FALSE);
        }
    } else {
        if (a_idd->type == TERMINAL) {
            if (a_idd->value.terminal == FALSE) {
                result = iddMakeTerminal(hashtable, FALSE);
            } else {
                /* if a is true then return a copy of b_idd */
                result = iddOperatorDeepClone(b_idd, hashtable);
            }
        } else if (b_idd->type == TERMINAL) {
            if (b_idd->value.terminal == FALSE) {
                result = iddMakeTerminal(hashtable, FALSE);
            } else {
                /* if b is true then return copy of a */
                result = iddOperatorDeepClone(a_idd, hashtable);
            }
        } else {
            entry = iddCacheLookup(HASH2(a_idd->hashvalue, b_idd->hashvalue));
            if (entry->tag == opcounter &&
                    entry->arg1 == a_idd &&
                    entry->arg2 == b_idd) {
                return entry->result;
            }
            if (a_idd->value.node.name > b_idd->value.node.name) {
                //printf("iddAndHelper: a > b\n");
                /* do the apply left thing */
                partition_size = b_idd->value.node.partition_size;
                partition = iddPartitionAlloc(partition_size);
                err = iddAndApplyLeft(hashtable, partition, partition_size,
                        b_idd->value.node.partition,
                        b_idd->value.node.partition_size,
                        a_idd);
                if (err == 0) {
                    printf("iddAndHelper received: %d from iddAndApplyLeft\n", err);
                }
                result = iddMakeNode(hashtable, b_idd->value.node.name,
                        partition, partition_size);
            } else if (a_idd->value.node.name < b_idd->value.node.name) {
                //printf("iddAndHelper: a < b\n");
                /* do the apply right b thing */
                partition_size = a_idd->value.node.partition_size;
                partition = iddPartitionAlloc(partition_size);
                err = iddAndApplyRight(hashtable, partition, partition_size,
                        a_idd->value.node.partition,
                        a_idd->value.node.partition_size,
                        b_idd);
                if (err == 0)
                    printf("iddAndHelper received: %d from iddAndApplyRight\n", err);
                result = iddMakeNode(hashtable, a_idd->value.node.name,
                        partition, partition_size);
            } else {
                //printf("iddAndHelper: a == b\n");
                /* do the merge thing */
                merge_res_size = a_idd->value.node.partition_size +
                    b_idd->value.node.partition_size;
                merge_res = iddPartitionTwoAlloc(merge_res_size);
                merge_res_used = iddMergeTwo(a_idd->value.node.partition,
                        a_idd->value.node.partition_size,
                        b_idd->value.node.partition,
                        b_idd->value.node.partition_size,
                        merge_res, merge_res_size);
                if (merge_res_used == 0)
                    printf("iddAndHelper received error %d form iddMergeTwo",
                            merge_res_used);
                partition_size = merge_res_used;
                partition = iddPartitionAlloc(partition_size);
                err = iddAndMerge(hashtable, merge_res, merge_res_used,
                        partition, partition_size);
                if (err == 0)
                    printf("iddAndHelper received error %d from iddAndMerge", err);
                iddPartitionTwoFree(merge_res); /* won’t need this anymore */
                result = iddMakeNode(hashtable,
                        a_idd->value.node.name,
                        partition,
                        partition_size);
            }
            entry->tag = opcounter;
            entry->arg1 = a_idd;
            entry->arg2 = b_idd;
            entry->result = result;
        }
    }
    return result;
}

//////////////////

/* Registers all nodes and terminals in the idd */
void iddRegister(Idd_t *idd) {
    int i;
    if (idd->marked) {
        return;
    }
    idd->marked = 1;
    iddarray[array_used] = idd;
    array_used++;
    if (array_used >= IDD_ARRAY_SIZE) {
        printf("iddRegister out of memory");
        exit(0);
    }
    if (idd->type == NODE) {
        for (i = 0; i < idd->value.node.partition_size; i++) {
            iddRegister(idd->value.node.partition[i].idd);
        }
    }
}

void iddDeepFree(Idd_t *idd) {
    int i;
    array_used = 0;
    iddRegister(idd);
    /* free list of unique idds */
    for (i = 0; i < array_used; i++) {
        if (iddarray[i]->type == NODE) {
            iddPartitionFree(iddarray[i]->value.node.partition);
        }
        iddFree(iddarray[i]);
    }
}

////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

Idd_t *iddDeepClone(Idd_t *idd_in_root) {
    IddIteratorPreorder_t iddite;
    Idd_t *res_root = NULL; /* the root node of the result */
    Idd_t *new = NULL; /* a new idd node waiting to be assinged */
    IddPartitionEntry_t *cur_p = NULL; /* the current partition */
    int cur_name;
    int cur_p_size;
    int cur_p_pos; /* p for partition */
    Idd_t *idd = idd_in_root;
    IddPartitionStack_t pstack;
    iddPartitionStackInit(&pstack);
    iddIteratorPreorderInit(&iddite, idd_in_root);
    idd = iddIteratorPreorderNext(&iddite); /* return the root */
    while (idd != NULL) {
        switch (idd->type) {
            case TERMINAL:
                new = iddAlloc();
                iddTerminalInit(new, idd->value.terminal);
                /* assign res to some partition entry */
                if (cur_p != NULL) {
                    cur_p[cur_p_pos].idd = new;
                    cur_p_pos++;
                } else {
                    res_root = new; /* idd_in_root is a terminal */
                }
                break;
            case NODE:
                if (cur_p != NULL)
                    iddPartitionStackPush(&pstack, cur_name, cur_p, cur_p_size, cur_p_pos);
                cur_name = idd->value.node.name;
                cur_p = iddPartitionCopy(idd->value.node.partition,
                        idd->value.node.partition_size);
                cur_p_pos = 0;
                cur_p_size = idd->value.node.partition_size;
                break;
            default:
                printf("iddDeepClone: unitialized idd detected!\n");
        }
        /* if partition is full then instantiate node and backtrack */
        /* note: by ensuring that res_root==NULL we know that cur_p has been set */
        while ((cur_p_pos == cur_p_size) && (res_root == NULL)) {
            new = iddAlloc();
            iddNodeInit(new, cur_name, cur_p, cur_p_size);
            /* pop from the stack and set pos and size again */
            if (!iddPartitionStackIsEmpty(&pstack)) {
                iddPartitionStackPop(&pstack, &cur_name, &cur_p,
                        &cur_p_size, &cur_p_pos);
                cur_p[cur_p_pos].idd = new;
                cur_p_pos++;
            } else { /* if stack is empty then we are done */
                res_root = new;
            }
        }
        idd = iddIteratorPreorderNext(&iddite);
    }
    iddIteratorPreorderFree(&iddite);
    if (res_root == NULL) {
        printf("iddDeepClone Failed\n");
    }
    return res_root;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Idd_t *iddNot(Idd_t *idd) {
    IddHash_t iddhash;
    Idd_t *tmp_idd = NULL;
    Idd_t *res_root = NULL;
    IddIteratorPreorder_t iddite;
    Idd_t *cur_idd = NULL; /* idd returned from iterator */
    IddPartitionStack_t pstack; /* store partitions that we have not finished */
    Idd_t *new_idd; /* the idd we are building */
    IddPartitionEntry_t *cur_p = NULL; /* partition we work on currently*/
    int cur_p_size = 0; /* size of the current partition */
    int cur_p_pos = 0; /* current pos in the new idd */
    int cur_name;
    int i;
    iddPartitionStackInit(&pstack);
    iddHashInit(&iddhash, IDDHASHTBLSIZE);
    iddIteratorPreorderInit(&iddite, idd);
    while ((cur_idd = iddIteratorPreorderNext(&iddite)) != NULL) {
        switch (cur_idd->type) {
            case TERMINAL:
                new_idd = iddAlloc();
                if (cur_idd->value.terminal == FALSE)
                    iddTerminalInit(new_idd, TRUE);
                else
                    iddTerminalInit(new_idd, FALSE);
                tmp_idd = iddHashFind(&iddhash, new_idd);
                if (tmp_idd == NULL) {
                    iddHashInsert(&iddhash, new_idd);
                } else {
                    iddFree(new_idd);
                    new_idd = tmp_idd;
                }
                if (cur_p != NULL) {
                    cur_p[cur_p_pos].idd = new_idd;
                    cur_p_pos++; /* later we check if new_p is full */
                } else {
                    res_root = new_idd;
                }
                break;
            case NODE:
                if (cur_p != NULL) {
                    iddPartitionStackPush(&pstack, cur_name, cur_p,
                            cur_p_size, cur_p_pos);
                }
                cur_name = cur_idd->value.node.name;
                cur_p_size = cur_idd->value.node.partition_size;
                cur_p_pos = 0;
                cur_p = iddPartitionAlloc(cur_p_size);
                for (i = 0; i < cur_p_size; i++) { /* copy bounds */
                    cur_p[i].lower_bound =
                        cur_idd->value.node.partition[i].lower_bound;
                    cur_p[i].upper_bound =
                        cur_idd->value.node.partition[i].upper_bound;
                    cur_p[i].idd = NULL;
                }
                break;
            default:
                printf("iddOptimize FATEL error: uinitialized idd detected!\n");
        }
        while ((cur_p_pos == cur_p_size) && (res_root == NULL)) {
            new_idd = iddAlloc();
            iddNodeInit(new_idd, cur_name, cur_p, cur_p_size);
            /* pop from stack to set cur_* again */
            if (!iddPartitionStackIsEmpty(&pstack)) {
                iddPartitionStackPop(&pstack, &cur_name, &cur_p,
                        &cur_p_size, &cur_p_pos);
                /* optimize a little */
                if (new_idd->value.node.partition_size == 1) {
                    cur_p[cur_p_pos].idd = new_idd->value.node.partition[0].idd;
                    iddPartitionFree(new_idd->value.node.partition);
                    iddFree(new_idd);
                } else {
                    /* check if similar idd exists */
                    tmp_idd = iddHashFind(&iddhash, new_idd);
                    if (tmp_idd == NULL) {
                        iddHashInsert(&iddhash, new_idd);
                    } else {
                        /* free partition and idd */
                        iddPartitionFree(new_idd->value.node.partition);
                        iddFree(new_idd); /* well we dont need this no more */
                        new_idd = tmp_idd;
                    }
                    cur_p[cur_p_pos].idd = new_idd;
                }
                cur_p_pos++;
            } else { /* if stack is empty then we are done */
                if (new_idd->value.node.partition_size > 1) {
                    res_root = new_idd;
                } else {
                    res_root = new_idd->value.node.partition[0].idd;
                }
            }
        } /* while cur_p_pos */
    } /* while preordernext*/
    iddIteratorPreorderFree(&iddite);
    if (res_root == NULL) {
        printf("iddNot failed\n");
    }
    return res_root;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

Idd_t *iddAnd(Idd_t *a_idd, Idd_t *b_idd) {
    int err;
    IddPartitionEntry_t *partition;
    int partition_size;
    Idd_t *result;
    IddPartitionEntryTwo_t *merge_res;
    int merge_res_size, merge_res_used;
    if (a_idd->type == TERMINAL) {
        if (b_idd->type == TERMINAL) {
            /* and the two terminals and return the result */
            result = iddAlloc();
            err = iddTerminalInit(result, iddTermAnd(a_idd->value.terminal,
                        b_idd->value.terminal));
            if (err == 0)
                printf("iddAnd received: %d from iddTerminalInit\n", err);
        } else {
            /* if a is false then return false term */
            if (a_idd->value.terminal == FALSE) {
                result = iddAlloc();
                err = iddTerminalInit(result, FALSE);
                if (err == 0)
                    printf("iddAnd received: %d from iddTerminalInit\n", err);
            } else { /* if a is true then return a copy of b_idd */
                result = iddDeepClone(b_idd);
            }
        }
    } else {
        if (b_idd->type == TERMINAL) {
            /* if b is false then return false */
            if (b_idd->value.terminal == FALSE) {
                printf("iddAnd: b_idd->value.terminal == FALSE\n");
                result = iddAlloc();
                err = iddTerminalInit(result, FALSE);
                if (err == 0)
                    printf("iddAnd received: %d from iddTerminalInit\n", err);
            } else {
                /* if b is true then return copy of a */
                result = iddDeepClone(a_idd);
            }
        } else {
            if (a_idd->value.node.name > b_idd->value.node.name) {
                /* do the apply left thing */
                partition_size = b_idd->value.node.partition_size;
                partition = iddPartitionAlloc(partition_size);
                err = iddAndApplyLeft(partition, partition_size,
                        b_idd->value.node.partition,
                        b_idd->value.node.partition_size,
                        a_idd);
                if (err == 0) {
                    printf("iddAnd received: %d from iddAndApplyLeft\n", err);
                }
                result = iddAlloc();
                err = iddNodeInit(result, b_idd->value.node.name,
                        partition, partition_size);
                if (err == 0)
                    printf("iddAnd received: %d from iddNodeInit\n", err);
            } else if (a_idd->value.node.name < b_idd->value.node.name) {
                /* do the apply right b thing */
                partition_size = a_idd->value.node.partition_size;
                partition = iddPartitionAlloc(partition_size);
                err = iddAndApplyRight(partition, partition_size,
                        a_idd->value.node.partition,
                        a_idd->value.node.partition_size,
                        b_idd);
                if (err == 0)
                    printf("iddAnd received: %d from iddAndApplyRight\n", err);
                result = iddAlloc();
                err = iddNodeInit(result, a_idd->value.node.name,
                        partition, partition_size);
                if (err == 0)
                    printf("iddAnd received: %d from iddNodeInit\n", err);
            } else {
                /* do the merge thing */
                merge_res_size = a_idd->value.node.partition_size +
                    b_idd->value.node.partition_size;
                merge_res = iddPartitionTwoAlloc(merge_res_size);
                merge_res_used = iddMergeTwo(a_idd->value.node.partition,
                        a_idd->value.node.partition_size,
                        b_idd->value.node.partition,
                        b_idd->value.node.partition_size,
                        merge_res, merge_res_size);
                if (merge_res_used == 0)
                    printf("iddAnd received error %d form iddMergeTwo", merge_res_used);
                partition_size = merge_res_used;
                partition = iddPartitionAlloc(partition_size);
                err = iddAndMerge(merge_res, merge_res_used,
                        partition, partition_size);
                if (err == 0)
                    printf("iddAnd received error %d from iddAndMerge", err);
                iddPartitionTwoFree(merge_res); /* won’t need this anymore */
                result = iddAlloc();
                err = iddNodeInit(result, a_idd->value.node.name,
                        partition, partition_size);
                if (err == 0)
                    printf("iddAnd received: %d from iddNodeInit\n", err);
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void iddDeepFree(Idd_t *idd) {
    IddIteratorPostorder_t iddite;
    Idd_t *cur_idd = NULL;
    int i;
    int parray_used = 0;
    int match = 0;
    if (idd == NULL) {
        return;
    }
    iddIteratorPostorderInit(&iddite, idd);
    while ((cur_idd = iddIteratorPostorderNext(&iddite)) != NULL) {
        match = 0;
        for (i = 0; i < parray_used; i++) {
            if (parray[i] == cur_idd) {
                match = 1;
                break;
            }
        }
        if (!match) {
            parray[parray_used] = cur_idd;
            parray_used++;
        }
        if (parray_used >= IDD_PARRAY_SIZE) {
            printf("iddDeepFree out of memory");
            exit(0);
        }
    }
    iddIteratorPostorderFree(&iddite);
    /* free list of unique idd’s */
    for (i = 0; i < parray_used; i++) {
        if (parray[i]->type == NODE)
            iddPartitionFree(parray[i]->value.node.partition);
        iddFree(parray[i]);
    }
}
