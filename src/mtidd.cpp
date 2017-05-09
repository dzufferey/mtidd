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
//- union / +
//- intersection / *
//- T lookup(std::map<V,double> point)
//- Idd<V, T> renaming(std::map<V, V> substitution)
//- subset: how to compare the terminals ?

//- ...

//////////////////
//////////////////
//////////////////

//TODO Hashing
// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
// https://github.com/scala/scala/blob/v2.12.1/src/library/scala/util/hashing/MurmurHash3.scala (list/product)

//////////////////
//////////////////
//////////////////

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
