#include "effort_metadata.h"
using namespace std;

#include "io_utils.h"
using namespace wavelet;

#include "string_utils.h"
using namespace stringutils;

namespace effort {

  effort_metadata::effort_metadata() : type(0) { }

  effort_metadata::effort_metadata(const string& m, int t, const Callpath& start, const Callpath& end) 
    : metric(m), type(t), start_path(start), end_path(end) { }


  void effort_metadata::write_out(ostream& out) {
    vl_write(out, metric.size());
    out.write(metric.c_str(), metric.size());
    write_generic(out, type);
    start_path.write_out(out);
    end_path.write_out(out);
  }


  void effort_metadata::read_in(istream& in, effort_metadata& md) {
    size_t metric_size = vl_read(in);
    char buf[metric_size+1];
    in.read(buf, metric_size);
    buf[metric_size] = '\0';
    md.metric = string(buf);

    md.type = read_generic<int>(in);
    md.start_path = Callpath::read_in(in);
    md.end_path = Callpath::read_in(in);
  }


  bool parse_filename(const string& filename, string *metric, int *out_type, int *number) {
    if (filename.find("effort") != 0) {
      return false;
    }

    vector<string> parts;
    split(filename, "-", parts);
    if (parts.size() != 4) {
      return false;
    }
  
    if (metric) {
      *metric = parts[1];
    }

    char *err;
    int t = strtol(parts[2].c_str(), &err, 10);
    if (*err) return false;
    if (out_type) *out_type = t;

    int n = strtol(parts[3].c_str(), &err, 10);
    if (*err) return false;
    if (number) *number = n;

    return true;
  }

  std::ostream& operator<<(std::ostream& out, const effort_metadata& md) {
    out << "[" 
        << md.metric << " "
        << md.type << " " 
        << md.start_path << " => " << md.end_path 
        << "]";
    
    return out;
  }


} // namespace
