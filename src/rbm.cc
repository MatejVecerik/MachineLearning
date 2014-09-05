class Rmb {
  int n_in;
  int n_out;
  
  vector<double> w;
  vector<double> dw;
  float NonLinFunc(float a) const {
//    return 1 - 2/(1 + exp(a));
    if (a <= 0)
      return 0;
    else
      return a;
  }
  float NonLinFuncD(float a) const {
//    return a * (1 - a);
    if (a <= 0)
      return 0;
    else
      return 1;
  }
  void Zero() {
    w.resize((n_in + 1) * n_out);
    dw.resize((n_in + 1) * n_out);
    fill(w.begin(), w.end(), 0);
    fill(dw.begin(), dw.end(), 0);
  }
 public:
  void Init(int n_in_, int n_out_) {
    n_in = n_in_;
    n_out = n_out_;
    Zero();
  }
  bool Load(string file);
  bool Save(string file);
};

