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

//TODO Hashing
// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
// https://github.com/scala/scala/blob/v2.12.1/src/library/scala/util/hashing/MurmurHash3.scala (list/product)
