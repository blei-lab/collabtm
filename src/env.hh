#ifndef ENV_HH
#define ENV_HH

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include <list>
#include <vector>
#include "matrix.hh"
#include "log.hh"

typedef uint8_t yval_t;

typedef D2Array<yval_t> AdjMatrix;
typedef D2Array<double> Matrix;
typedef D3Array<double> D3;
typedef D2Array<KV> MatrixKV;

typedef std::map<uint32_t, uint32_t> IDMap;
typedef std::map<uint32_t, uint32_t> FreqMap;
typedef std::map<string, uint32_t> FreqStrMap;
typedef std::map<string, uint32_t> StrMap;
typedef std::map<uint32_t, string> StrMapInv;

typedef D1Array<std::vector<uint32_t> *> SparseMatrix;
typedef std::vector<Rating> RatingList;
typedef std::map<uint32_t, bool> UserMap;
typedef std::map<uint32_t, bool> BoolMap;
typedef std::map<uint32_t, double> DoubleMap;
typedef std::map<uint32_t, Array *> ArrayMap;
typedef std::map<uint32_t, uint32_t> NodeValMap;
typedef std::map<uint32_t, vector<uint32_t> > MapVec;
typedef MapVec SparseMatrix2;
typedef std::map<Rating, bool> SampleMap;
typedef std::map<Rating, int> CountMap;
typedef std::map<Rating, double> ValueMap;
typedef std::map<uint32_t, string> StrMapInv;

class Env {
public:
  Env(uint32_t N, uint32_t M, uint32_t K, string fname, 
      bool nmi, string ground_truth_fname, uint32_t rfreq,
      bool strid, string label, bool alogl, double rseed,
      uint32_t max_iterations, bool load, string loc, 
      bool gen_hout);

  ~Env() { fclose(_plogf); }

  static string prefix;
  static Logger::Level level;

  uint32_t n;  // users
  uint32_t m;  // movies
  uint32_t k;
  uint32_t t;
  uint32_t mini_batch_size;

  double a;
  double b;
  double c;
  double d;

  double alpha;
  double tau0;
  double tau1;
  double heldout_ratio;
  double validation_ratio;
  int reportfreq;
  double epsilon;
  double logepsilon;
  bool nolambda;
  bool strid;
  bool logl;
  uint32_t max_iterations;
  double seed;
  bool terminate;
  string datfname;
  string label;
  bool nmi;
  string ground_truth_fname;
  bool model_load;
  string model_location;
  bool gen_heldout;

  template<class T> static void plog(string s, const T &v);
  static string file_str(string fname);

private:
  static FILE *_plogf;
};


template<class T> inline void
Env::plog(string s, const T &v)
{
  fprintf(_plogf, "%s: %s\n", s.c_str(), v.s().c_str());
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const double &v)
{
  fprintf(_plogf, "%s: %.9f\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const string &v)
{
  fprintf(_plogf, "%s: %s\n", s.c_str(), v.c_str());
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const bool &v)
{
  fprintf(_plogf, "%s: %s\n", s.c_str(), v ? "True": "False");
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const int &v)
{
  fprintf(_plogf, "%s: %d\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const unsigned &v)
{
  fprintf(_plogf, "%s: %d\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const short unsigned int &v)
{
  fprintf(_plogf, "%s: %d\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const uint64_t &v)
{
  fprintf(_plogf, "%s: %" PRIu64 "\n", s.c_str(), v);
  fflush(_plogf);
}

#ifdef __APPLE__
template<> inline void
Env::plog(string s, const long unsigned int &v)
{
  fprintf(_plogf, "%s: %lu\n", s.c_str(), v);
  fflush(_plogf);
}
#endif

inline string
Env::file_str(string fname)
{
  string s = prefix + fname;
  return s;
}

inline
Env::Env(uint32_t N, uint32_t M, uint32_t K, string fname, 
	 bool nmival, string gfname, uint32_t rfreq,
	 bool sid, string lbl, bool alogl, double rseed,
	 uint32_t maxitr, bool load, 
	 string loc, bool gen_hout)
  : n(N),
    m(M),
    k(K),
    t(2),
    mini_batch_size(100),
    a(.0), b(.0), c(.0), d(.0),
    tau0(0),
    tau1(0),
    heldout_ratio(0.001),
    validation_ratio(0.01),
    reportfreq(rfreq),
    epsilon(0.001),
    logepsilon(log(epsilon)),
    nolambda(true),
    strid(sid),
    logl(alogl),
    max_iterations(maxitr),
    seed(rseed),
    terminate(false),
    datfname(fname),
    label(lbl),
    nmi(nmival),
    ground_truth_fname(gfname),
    model_load(load),
    model_location(loc),
    gen_heldout(gen_hout)
{
  ostringstream sa;
  sa << "n" << n << "-";
  sa << "m" << m << "-";
  sa << "k" << k;
  if (label != "")
    sa << "-" << label;
  else if (datfname.length() > 3) {
    string q = datfname.substr(0,2);
    if (isalpha(q[0]))
      sa << "-" << q;
  }

  if (!gen_heldout)
    sa << "-batch";

  prefix = sa.str();
  level = Logger::TEST;

  fprintf(stdout, "+ Creating directory %s\n", prefix.c_str());
  fflush(stdout);

  assert (Logger::initialize(prefix, "infer.log", 
			     true, level) >= 0);
  _plogf = fopen(file_str("/param.txt").c_str(), "w");
  if (!_plogf)  {
    printf("cannot open param file:%s\n",  strerror(errno));
    exit(-1);
  }
  
  plog("n", n);
  plog("k", k);
  plog("t", t);
  plog("alpha", alpha);
  plog("heldout_ratio", heldout_ratio);
  plog("validation_ratio", validation_ratio);
  plog("nolambda", nolambda);
  plog("max iterations", max_iterations);
  plog("seed", seed);
  plog("gen_heldout", gen_heldout);
  plog("model_location", model_location);
  plog("load", model_load);

  string ndatfname = file_str("/network.dat");
  unlink(ndatfname.c_str());
  assert (symlink(datfname.c_str(), ndatfname.c_str()) >= 0);
  unlink(file_str("/mutual.txt").c_str());
}

/*
   src: http://www.delorie.com/gnu/docs/glibc/libc_428.html
   Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.
*/
inline int
timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

inline void
timeval_add (struct timeval *result, const struct timeval *x)
{
  result->tv_sec  += x->tv_sec;
  result->tv_usec += x->tv_usec;

  if (result->tv_usec >= 1000000) {
    result->tv_sec++;
    result->tv_usec -= 1000000;
  }
}

#endif
