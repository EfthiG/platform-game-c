#include <stdio.h>

#include "ADTSet.h"

Pointer set_find_eq_or_greater(Set set, Pointer value){
    Pointer val = set_find(set, value);
    if(val == NULL){
        set_insert(set, value);
        val = set_node_value(set, set_next(set, set_find_node(set, value)));
        set_remove(set, set_node_value(set, set_previous(set, set_find_node(set,val))));
    }
    return val;
}

Pointer set_find_eq_or_smaller(Set set, Pointer value){
    Pointer val = set_find(set, value);
    if(val == NULL){
        set_insert(set, value);
        val = set_node_value(set, set_previous(set, set_find_node(set, value))); 
        set_remove(set, set_node_value(set, set_next(set, set_find_node(set,val))));    
    }
    return val;
}

