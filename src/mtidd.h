#include <list>
#include <memory>
#include <functional>
#include "lattice.h"

using namespace std;

namespace mtidd
{

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  class idd
  {
  private:
    // TODO
    // the variable corresponding to that node
    // a partition
    // a reference to the manager
    // caching the hash value
    // an unique id
  public:
    // use shared_ptr as the idd_manager has ownership of the idds and caches them
    shared_ptr<idd<V, T, L>> operator!() const;
    shared_ptr<idd<V, T, L>> operator&&(idd<V, T, L> const& rhs) const;
    shared_ptr<idd<V, T, L>> operator||(idd<V, T, L> const& rhs) const;
    bool operator==(idd<V, T, L> const& rhs);
    bool operator!=(idd<V, T, L> const& rhs) { return !(this == rhs); }
    bool operator<(idd<V, T, L> const& rhs);
    T lookup(std::map<V,double> const& point) const;
    size_t hash() const;
  }

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  class idd_manager
  {
  private:
    // TODO
    // cache: hashmap
    // counter for unique ids
    // an ordering for the variables
  public:
    shared_ptr<idd<V, T, L>> create(V const & variable, T value = L.bottom);
    shared_ptr<idd<V, T, L>> create(V const & variable, T left_value, double bound, bool closed, T right_value);
  }

} // end namespace

// custom parial specialization of std::hash
namespace std
{
  template<V, T, L> struct hash<mtidd<V,T,L>>
  {
    typedef mtidd<V,T,L> argument_type;
    typedef size_t result_type;
    result_type operator()(argument_type const& s) const
    {
      //TODO
    }
  };
}
