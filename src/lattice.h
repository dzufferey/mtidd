#pragma once


namespace mtidd
{

  // comparison in a partial ordering
  typedef enum { Equal, Smaller, Greater, Different} lattice_compare;

  // struct that contains the methods to
  template<typename T>
  class lattice {
  public:
    virtual T bottom() const = 0;
    virtual T top() const = 0;

    virtual T least_upper_bound(const T&, const T&) const = 0;
    virtual T greatest_lower_bound(const T&, const T&) const = 0;

    virtual lattice_compare compare(const T&, const T&) const = 0;
    virtual bool equal(const T&, const T&) const = 0;
  };



  // Simple instances of Lattices

  class boolean_lattice: lattice<bool> {
  public:
    bool bottom() const;
    bool top() const;
    bool least_upper_bound(const bool& x, const bool& y) const;
    bool greatest_lower_bound(const bool& x, const bool& y) const;
    lattice_compare compare(const bool& x, const bool& y) const;
    bool equal(const bool& x, const bool& y) const;
  };

  class integer_lattice: lattice<int> {
  public:
    int bottom() const;
    int top() const;
    int least_upper_bound(const int& x, const int& y) const;
    int greatest_lower_bound(const int& x, const int& y) const;
    lattice_compare compare(const int& x, const int& y) const;
    bool equal(const int& x, const int& y) const;
  };

  class long_lattice: lattice<long> {
  public:
    long bottom() const;
    long top() const;
    long least_upper_bound(const long& x, const long& y) const;
    long greatest_lower_bound(const long& x, const long& y) const;
    lattice_compare compare(const long& x, const long& y) const;
    bool equal(const long& x, const long& y) const;
  };

  class float_lattice: lattice<float> {
  public:
    float bottom() const;
    float top() const;
    float least_upper_bound(const float& x, const float& y) const;
    float greatest_lower_bound(const float& x, const float& y) const;
    lattice_compare compare(const float& x, const float& y) const;
    bool equal(const float& x, const float& y) const;
  };

  class double_lattice: lattice<double> {
  public:
    double bottom() const;
    double top() const;
    double least_upper_bound(const double& x, const double& y) const;
    double greatest_lower_bound(const double& x, const double& y) const;
    lattice_compare compare(const double& x, const double& y) const;
    bool equal(const double& x, const double& y) const;
  };

  // TODO an example for set<T>

} // end namespace
