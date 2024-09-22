/* The MIT License

   Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)
*/

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>

#define APP_LOG(level, fmt, ...) fprintf(level, "[%s:%d]\t" fmt "\n", __FUNCTION__, __LINE__,  ##__VA_ARGS__)

#define APP_DEBUG(fmt, ...) APP_LOG(stdout, "[D] " fmt, ##__VA_ARGS__)
#define APP_NOTICE(fmt, ...) APP_LOG(stdout, "[I] " fmt, ##__VA_ARGS__)
#define APP_WARNING(fmt, ...) APP_LOG(stderr, "[W] " fmt, ##__VA_ARGS__)
#define APP_FATAL(fmt, ...) APP_LOG(stderr, "[E] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <set>
#include <map>
#include <vector>

/// STL support

#define RM_REF(X) typename std::remove_reference<X>::type
#define RM_CONST(X) typename std::remove_const<X>::type
#define ELEMENT(X) RM_CONST(RM_REF(decltype(*begin(X))))

// If the second argument is of type 'T' then the class doesn't work with floating point types.

template<typename T,T VALUE> class Default {
    T t;

  public:
    template<typename INIT>
    explicit Default(INIT const& i):t(i){}

    Default& operator=(Default const& d){
        t=d.t;
        return *this;
    }

    Default& operator=(T const& v){
        t=v;
        return *this;
    }

    T& value() { return t; }

    T const& value()const { return t; }

    operator T&(){ return t; }
    operator T const&()const{ return t; }
    bool operator<(T v)const{ return t<v; }

    Default():t(VALUE) {}
    Default(Default const& d):t(d.t) {}
    Default(T const& a):t(a) {}
};

template<typename T> class Maybe {
    T *p;

  public:
    operator bool()const { return !!p; }

    T& operator*() { assert(p); return *p; }

    T const& operator*()const { assert(p); return *p; }

    const T *operator->()const { assert(p); return p; }

    T *operator->() { assert(p); return p; }

    bool operator==(Maybe const& m)const{
        if(p && m){
            return *p==*m;
        }
        return !p && !m;
    }

    bool operator==(T const& t)const { return p && *p==t; }

    bool operator!=(Maybe const& a)const{ return !(a==*this); }

    bool operator<(Maybe const& a)const{
        if(p){
            if(a){
                return *p<*a;
            }else{
                return 0;
            }
        }else{
            return !!a;
        }
    }

    Maybe() : p(NULL) {}
    Maybe(T const& t) : p(new T(t)) {}
    Maybe(Maybe const& m) : p(NULL) {
        if(m.p) {
            p = new T(*m.p);
        }
    }

    ~Maybe() { delete p; }

    Maybe& operator=(Maybe const& a) {
        T *next=NULL;
        if(a.p) next=new T(*a.p);
        delete p;
        p=next;
        return *this;
    }
};

template<typename T> std::ostream& operator<<(std::ostream& o,Maybe<T> const& m) {
    if(m) {
        o<<*m;
    } else {
        o<<"NULL";
    }
    return o;
}

template<typename T,T VALUE>
std::ostream& operator<<(std::ostream& o,Default<T,VALUE> const& d) {
  return o<<d.value();
}

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,std::pair<A,B> const& p) {
  o<<"pair("<<p.first<<","<<p.second<<")";
  return o;
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,std::map<K,V> const& m) {
  o<<"{";
  for(auto at=m.begin();at!=m.end();at++){
      o<<*at;
      auto next=at;
      next++;
      if(next!=m.end()){
          o<<",";
      }
  }
  return o<<"}";
}

template<typename T>
std::vector<T>& operator|=(std::vector<T>& v,T t) {
  v.push_back(t);
  return v;
}

template<typename T>
std::vector<T>& operator|=(std::vector<T> &v,std::vector<T> a) {
  for(auto elem:a) v|=elem;
  return v;
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::vector<T> const& v) {
  o<<"[";
  for(unsigned i=0;i<v.size();i++){
      o<<v[i];
      if(i+1<v.size()) o<<",";
  }
  return o<<"]";
}

std::string join(std::vector<std::string> const&,char);

// aka cdr
template<typename T>
std::vector<T> tail(std::vector<T> const& v) {
  std::vector<T> r;
  for(unsigned i=1;i<v.size();i++){
      r|=v[i];
  }
  return r;
}

template<typename T>
std::string as_string(T const& t){
  std::stringstream ss;
  ss<<t;
  return ss.str();
}

template<typename Func,typename Collection>
auto mapf(Func f,Collection const& c)->std::vector<decltype(f(*begin(c)))> {
  std::vector<decltype(f(*begin(c)))> r;
  for(auto elem:c) r|=f(elem);
  return r;
}

template<typename Func,typename Collection>
Collection filter(Func f,Collection const& in) {
    Collection r;
    for(auto elem:in){
        if(f(elem)) r|=elem;
    }
    return r;
}

// like the 'group' function in SQL
template<typename Func,typename T>
auto segregate(Func f,std::vector<T> in)->std::map<decltype(f(in[0])),std::vector<T>> {
    std::map<decltype(f(in[0])),std::vector<T>> r;
    for(auto a:in) {
        r[f(a)]|=a;
    }
    return r;
}

template<typename Func,typename K,typename V>
auto map_map(Func f,std::map<K,V> m)->std::map<K,decltype(f(begin(m)->second))> {
    std::map<K,decltype(f(begin(m)->second))> r;
    for(auto p:m) r[p.first]=f(p.second);
    return r;
}

template<typename T>
std::vector<T> take(unsigned lim,std::vector<T> const& in) {
    std::vector<T> r;
    for(unsigned i=0;i<lim && i<in.size();i++) {
        r|=in[i];
    }
    return r;
}

template<typename T>
T take(std::vector<T>& in) {
    T e = in.back();
    in.pop_back();
    return e;
}

template<typename T>
std::vector<T> skip(unsigned i,std::vector<T> const& v) {
    std::vector<T> r;
    for(;i<v.size();i++) r|=v[i];
    return r;
}

template<typename T>
std::vector<T> flatten(std::vector<std::vector<T>> const& v) {
    std::vector<T> r;
    for(auto a:v) for(auto elem:a) r|=elem;
    return r;
}

template<typename T>
bool contains(std::vector<T> const& v,T t) {
    for(auto a:v) {
        if(a==t) return 1;
    }
    return 0;
}

template<typename T>
T max(std::vector<T> const& v) {
    T r=*begin(v);
    for(auto a:v) r=std::max(r,a);
    return r;
}

template<typename T>
T min(std::vector<T> const& v){
    T r=*begin(v);
    for(auto a:v) r=std::min(r,a);
    return r;
}

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,std::array<T,N> a) {
    o<<"[ ";
    for(auto elem:a) o<<elem<<" ";
    return o<<"]";
}

//could also do a multiset...
template<typename T>
std::map<T,Default<unsigned,0>> count(std::vector<T> v) {
    std::map<T,Default<unsigned,0>> r;
    for(auto a:v) r[a]++;
    return r;
}

template<typename T>
T sum(std::vector<T> v) {
    T total{};
    for(auto a:v) total+=a;
    return total;
}

template<typename T>
double mean(std::vector<T> v) {
    return sum(v)/(v.size()+0.0);
}

template<typename T>
std::vector<T> sorted(std::vector<T> v) {
    sort(begin(v),end(v));
    return v;
}

std::map<std::string,std::string> env_vars(char **envp);
std::vector<std::string> args(int argc,char **argv);

template<typename T>
std::vector<T> reverse(std::set<T> s) {
    std::vector<T> r1,r;
    for(auto a:s) r1|=a;
    for(int i=r1.size()-1;i>=0;i--){
        r|=r1[i];
    }
    return r;
}

template<typename T>
std::vector<T> reversed(std::vector<T> v) {
    std::vector<T> r;
    for(int i=v.size()-1;i>=0;i--){
        r|=v[i];
    }
    return r;
}

template<typename A,typename B>
std::pair<B,A> reverse(std::pair<A,B> p) {
    return make_pair(p.second,p.first);
}

template<typename A,typename B>
std::vector<std::pair<B,A>> reverse_pairs(std::vector<std::pair<A,B>> v) {
    std::vector<std::pair<B,A>> r;
    for(auto a:v) r|=reverse(a);
    return r;
}

template<typename Collection>
auto enumerate(Collection const& v)->std::vector<std::pair<unsigned,ELEMENT(v)>> {
    std::vector<std::pair<unsigned,ELEMENT(v)>> r;
    unsigned i=0;
    for(auto e:v){
        r.push_back(std::make_pair(i,e));
        i++;
    }
    return r;
}

int atoi(std::string const&);

template<typename T>
std::map<unsigned,std::vector<T>> count2(std::vector<T> v) {
    std::map<unsigned,std::vector<T>> r;
    for(auto p:count(v)){
        r[p.second]|=p.first;
    }
    return r;
}

void typeset_table_inner(std::vector<std::vector<std::string>> const&);

template<typename Collection>
void typeset_table(Collection const& c) {
    using std::vector;
    using std::string;
    vector<vector<string>> v;
    for(auto a:c){
        vector<string> v1;
        for(auto b:a){
            v1|=as_string(b);
        }
        v|=v1;
    }
    typeset_table_inner(v);
}

template<typename Collection>
void print_table(Collection const& c) {
    for(auto const& a:c){
        print(std::cout,"\t",a);
        std::cout<<"\n";
        //cout<<a<<"\n";
    }
}

template<typename T>
std::vector<T>& operator/=(std::vector<T>& v,double d) {
    //return mapf([d](T t){ return t/d; },v);
    for(auto &a:v){
        a/=d;
    }
    return v;
}

template<typename Collection>
bool all(Collection const& c) {
    for(auto elem:c){
        if(!elem) return 0;
    }
    return 1;
}


/// TablePrinter

// Print a pretty table into your output of choice.
//
// Usage:
//   TablePrinter tp(&std::cout);
//   tp.AddColumn("Name", 25);
//   tp.AddColumn("Age", 3);
//   tp.AddColumn("Position", 30);
//
//   tp.PrintHeader();
//   tp << "Dat Chu" << 25 << "Research Assistant";
//   tp << "John Doe" << 26 << "Professional Anonymity";
//   tp << "Jane Doe" << tp.SkipToNextLine();
//   tp << "Tom Doe" << 7 << "Student";
//   tp.PrintFooter();

namespace tblfmt {

class endl{};

class TablePrinter {
   void PrintHorizontalLine();

  template<typename T> void OutputDecimalNumber(T input);

  std::ostream *out_stream_;
  std::vector<std::string> column_headers_;
  std::vector<int> column_widths_;
  std::string separator_;

  int i_; // index of current row
  int j_; // index of current column

  int table_width_;

public:
  int get_num_columns() const;
  int get_table_width() const;
  void set_separator(const std::string & separator);

  void AddColumn(const std::string & header_name, int column_width);
  void PrintHeader();
  void PrintFooter();

  TablePrinter& operator<<(endl input) {
    while (j_ != 0) {
      *this << "";
    }
    return *this;
  }

  // Can we merge these?
  TablePrinter& operator<<(float input);
  TablePrinter& operator<<(double input);

  template<typename T> TablePrinter& operator<<(T input) {
    if (j_ == 0) {
      *out_stream_ << "|";
    }

    // Leave 3 extra space: One for negative sign, one for zero, one for decimal
    *out_stream_ << std::setw(column_widths_.at(j_)) << input;

    if (j_ == get_num_columns()-1) {
      *out_stream_ << "|\n";
      i_ = i_ + 1;
      j_ = 0;
    } else {
      *out_stream_ << separator_;
      j_ = j_ + 1;
    }

    return *this;
  }

  TablePrinter(std::ostream * output, const std::string &separator = "|");
  ~TablePrinter();
};

template<typename T> void TablePrinter::OutputDecimalNumber(T input) {
  // If we cannot handle this number, indicate so
  if (input < 10*(column_widths_.at(j_)-1) || input > 10*column_widths_.at(j_)) {
    std::stringstream string_out;
    string_out << std::setiosflags(std::ios::fixed)
               << std::setprecision(column_widths_.at(j_))
               << std::setw(column_widths_.at(j_))
               << input;

    std::string string_rep_of_number = string_out.str();

    string_rep_of_number[column_widths_.at(j_)-1] = '*';
    std::string string_to_print = string_rep_of_number.substr(0, column_widths_.at(j_));
    *out_stream_ << string_to_print;

  } else {

    // determine what precision we need
    int precision = column_widths_.at(j_) - 1; // leave room for the decimal point
    if (input < 0) {
      --precision; // leave room for the minus sign
    }

    // leave room for digits before the decimal?
    if (input < -1 || input > 1){
      int num_digits_before_decimal = 1 + (int)log10(std::abs(input));
      precision -= num_digits_before_decimal;
    } else {
      precision --; // e.g. 0.12345 or -0.1234
    }

    if (precision < 0) {
      precision = 0; // don't go negative with precision
    }

    *out_stream_ << std::setiosflags(std::ios::fixed)
                 << std::setprecision(precision)
                 << std::setw(column_widths_.at(j_))
                 << input;
  }

  if (j_ == get_num_columns()-1) {
    *out_stream_ << "|\n";
    i_ = i_ + 1;
    j_ = 0;
  } else {
    *out_stream_ << separator_;
    j_ = j_ + 1;
  }
}

} // namspace tblfmt

#endif // __cplusplus

#endif // OUTPUT_H
