#include "img.cc"

#if false
#define PRINT(a) {printf("%3d ", __LINE__); print(a, #a);}
#else
#define PRINT(a) {}
#endif

double SQ(double a) {return a * a;} 

// Best: 10   130.390656
// Best: 150   38.940434  --16380 epochs, training speed = 1e-4, dw_decay_time = 1000

/*
 Comparing non-lin functions: 50 hidden neurons
 Geom: 5.208333
 Act: 4.575000
 Sigm:

*/

class NeuralNetwork {
  float training_speed;
  int n1;
  int n2;
  int n3;

  int batch_size;
  int step;

  vector<float> w1;
  vector<float> w2;
  vector<float> dw1;
  vector<float> dw2;
  vector<float> val;
  vector<float> err;
  vector<float> errf;
  float NonLinFunc(float a) const {
    return 1 - 2/(1 + exp(a));
    if (a <= 0)
      return 0;
    else
      return a;
//      return 1/(1 + 1/a);
  }
  float NonLinFuncD(float a) const {
    return a * (1 - a);
    if (a <= 0)
      return 0;
    else
      return 1;
//      return (a - 1) * (a - 1);
  }
  void Zero() {
    for(int i = 0; i < n3; i++) {
      errf[i] = 0;
    }
    for(int j = 0; j < n2; j++) {
      val[j] = 0;
      err[j] = 0;
    }
  }
 public:
  NeuralNetwork(int n1_, int n2_, int n3_, float training_speed_, int batch_size_) {
    LOG_HERE();
    n1 = n1_;
    n2 = n2_ + 1;
    n3 = n3_;
    batch_size = batch_size_;
    step = 0;
    training_speed = training_speed_;
    Resize();
    LOG_HERE();
    for(int i = 0; i < w1.size(); i++) {
      if (rand() % 50 == 0) {
        w1[i] = SQ(SQ(randf())) * 0.4;
        dw1[i] = 0;
      }
    }
    for(int i = 0; i < w2.size(); i++) {
      if (rand() % 50 == 0) {
        w2[i] = SQ(SQ(randf())) * 0.01;
        dw2[i] = 0;
      }
    }
    LOG_HERE();
  }
  
  void SetTrainingSpeed(float training_speed_) {
    training_speed = training_speed_;
  }
  
  float CalcOutput(const vector<float> &input, 
                   vector<float> &mid,
                   vector<float> &end,
                   const vector<float> &label) {
    mid.resize(0);
    mid.resize(n2, 0);
    // Get first layer.
    for(int j = 0; j + 1 < n2; j++)
      for(int i = 0; i < n1; i++)
        mid[j] += input[i] * w1[j * n1 + i];
    mid.back() = 1;
    for(int j = 0; j < n2; j++)
      mid[j] = NonLinFunc(mid[j]);
    // Get second layer / errors.
    end.resize(0);
    end.resize(n3, 0);
    for(int k = 0; k + 1 < n3; k++)
      for(int j = 0; j < n2; j++)
        end[k] += mid[j] * w2[k * n2 + j];
    end.back() = 1;
    
    for(int k = 0; k < n3; k++)
      errf[k] = label[k] - end[k];
    PRINT(errf);
    float tot_err = 0;
    for(int k = 0; k < n3; k++)
      tot_err += errf[k] * errf[k];
    return 10000 * tot_err / n3;
  }
  
  // Returns total error.
  float Train(const vector<float> &input, const vector<float> &label) {
    Zero();
    float tot_err = CalcOutput(input, val, errf, label);
    PRINT(input);
    PRINT(w1);
    PRINT(val);
    PRINT(w2);
    PRINT(errf);
    // Backpropagate error.
    for(int k = 0; k < n3; k++)
      for(int j = 0; j < n2; j++) {
        err[j] += errf[k] * w2[k * n2 + j];
        dw2[k * n2 + j] += errf[k] * val[j];
      }
    PRINT(err);
    for(int j = 0; j < n2; j++)
      err[j] = NonLinFuncD(val[j]) * err[j];
    PRINT(err);
    // Backpropagate into first layer.
    for(int j = 0; j < n2; j++)
      for(int i = 0; i < n1; i++)
        dw1[j * n1 + i] += err[j] * input[i];
    PRINT(w1);
    PRINT(w2);
    step++;
    if (step % batch_size == 0) {
      double dw_decay_time = 1000;
      for(int i = 0; i < n1; i++)
        for(int j = 0; j < n2; j++) {
          w1[i * n2 + j] += training_speed * dw1[i * n2 + j] / dw_decay_time;
          dw1[i * n2 + j] *= (1 - 1 / dw_decay_time);
        }
      for(int k = 0; k < n3; k++)
        for(int j = 0; j < n2; j++) {
          w2[k * n2 + j] += training_speed * dw2[k * n2 + j] / dw_decay_time;
          dw2[k * n2 + j] *= (1 - 1 / dw_decay_time);
        }
    }
    return tot_err;
  }
  
  void PrintEval(vector<float> &input) {
    CalcOutput(input, val, errf, input);
    print(input);
    print(errf);
    print(val);
    printf("\n");
  }
  
  void Resize() {
    w1.resize(n1 * n2);
    w2.resize(n3 * n2);
    dw1.resize(n1 * n2);
    dw2.resize(n3 * n2);
    val.resize(n2);
    err.resize(n2);
    errf.resize(n3);
    fill(w1.begin(),   w1.end(), 0);
    fill(w2.begin(),   w2.end(), 0);
    fill(dw1.begin(),  dw1.end(), 0);
    fill(dw2.begin(),  dw2.end(), 0);
    fill(val.begin(),  val.end(), 0);
    fill(err.begin(),  err.end(), 0);
    fill(errf.begin(), errf.end(), 0);
  }
  
  void Save(string path) {
    FILE *file = fopen(path.c_str(), "w");
    fprintf(file, "%d %d %d\n", n1, n2, n3);
    for(int i = 0; i < w1.size(); i++)
      fprintf(file, "%f ", w1[i]);
    for(int i = 0; i < w2.size(); i++)
      fprintf(file, "%f ", w2[i]);
    fclose(file);
  }

  void Load(string path) {
    FILE *file = fopen(path.c_str(), "r");
    if (file == nullptr)
      return;
    int n1n, n2n, n3n;
    fscanf(file, "%d", &n1n);
    fscanf(file, "%d", &n2n);
    fscanf(file, "%d", &n3n);
    if (n1n != n1 && n2n != n2 && n3n != n3) {
      fprintf(stderr, "Can't load %s. Incompatible layer sizes.\n", path.c_str());
      fclose(file);
      return;
    }
    Resize();
    for(int i = 0; i < w1.size(); i++)
      fscanf(file, "%f", &w1[i]);
    for(int i = 0; i < w2.size(); i++)
      fscanf(file, "%f", &w2[i]);
    fclose(file);
  }
  
  void WStat() {
    double sw1 = 0;
    double ww1 = 0;
    double sw2 = 0;
    double ww2 = 0;
    for(int i = 0; i < n1; i++)
      for(int j = 0; j < n2; j++) {
        sw1 += w1[i * n2 + j];
        ww1 += w1[i * n2 + j] * w1[i * n2 + j];
      }
    for(int k = 0; k < n3; k++)
      for(int j = 0; j < n2; j++) {
        sw2 += w2[k * n2 + j];
        ww2 += w2[k * n2 + j] * w2[k * n2 + j];
      }
    printf("w1: avg: %e rms: %e  w2: avg: %e rms: %e\n", sw1 / n1 / n2, sqrt(ww1 / n1 / n2), sw2 / n1 / n2, sqrt(ww2 / n3 / n2));
  }
};

void TinyTest() {
  // Second = 2 * first - 1.
  vector<vector<float> > data = 
      {
        {2, 3, 1},
        {2, 3, 1},
        {4, 7, 1},
        {1.5, 2, 1},
      };
  NeuralNetwork nn(3, 1, 3, 0.02, 1);
  for(int i = 0; i < 300; i++) {
    float epoch_error = 0;
    for(int j = 0; j < data.size(); j++)
      epoch_error += nn.Train(data[j], data[j]);
    printf("%lf\n", epoch_error);
  }
}


void SmallTest() {
  vector<vector<float> > data;
  for(int i = 0; i < 100; i++) {
    vector<float> ndat;
    float a = randf();
    float b = randf();
    float c = randf();
    ndat.push_back(a + b);
    ndat.push_back(c * a);
    ndat.push_back(2 * b + c);
    ndat.push_back(sin (a));
    ndat.push_back(3 * c - a);
    ndat.push_back(a + b + c);
    
    ndat.push_back(1);
    data.push_back(ndat);
  }
  printf("Input size: %d\n", data[0].size() - 1);
  NeuralNetwork nn(data[0].size(), 3, data[0].size(), 0.01, 1);
  for(int i = 0; i < 1; i++) {
    float epoch_error = 0;
    for(int j = 0; j < data.size(); j++)
      epoch_error += nn.Train(data[j], data[j]);
    if (i % 300 == 0) {
      for(int j = 0; j < data.size(); j++)
        nn.PrintEval(data[j]);
      printf("%lf\n", epoch_error);
    }
  }
}

void BMPPrint(string bmp) {
  Picture p;
  p.Load(bmp);
  p.Print();
}

int ArgMax(vector<float>& a, int ignore_last = 0) {
  int a_max = 0;
  for(int i = 0; i + ignore_last < a.size(); i++) {
    if (a[a_max] < a[i])
      a_max = i;
  }
  return a_max;
} 

float EvalLabel(NeuralNetwork& nn, vector<Picture>& test, bool save) {
  float err = 0;
  int label_err = 0;
  static vector<float> a, b;
  int err_out_count = 0;
  LOG_HERE();
  for(int j = 0; j < test.size(); j++) {
    err += nn.CalcOutput(test[j].GrayData(), a, b, test[j].LabelData());
    int correct_label = ArgMax(test[j].LabelData(), 1);
    int our_label = ArgMax(b, 1);
    label_err += correct_label != our_label;
    if (correct_label != our_label && 1) {
      if(false) {
        test[j].Print();
        print(b);
        print(test[j].LabelData());
        printf("\n");
      }
      if (save && err_out_count++ < 40) {
        test[j].Save(string("../imgs_out_eval/") + Str(correct_label) + "-" + Str(our_label) + "_" + Str(test[j].Id()) + ".bmp");
      }
    }
  }
  LOG_HERE();
  return 100. * label_err / test.size();
}

float Eval(NeuralNetwork& nn, vector<Picture>& test, bool save) {
  LOG_HERE();
  if (test[0].LabelData().size())
    return EvalLabel(nn, test, save);
  LOG_HERE();
  printf("No label data!!");
  exit(1);
  float err = 0;
  static vector<float> a, b;
  vector<Picture> pics_out = test;
  int zero_comps = 0;
  int max_a = 0;
  for(int j = 0; j < test.size(); j++) {
    double this_err = nn.CalcOutput(test[j].GrayData(), a, pics_out[j].GrayData(), test[j].GrayData());
    err += this_err;
    if (save) {
      max_a = ArgMax(a);
      for(int i = 0; i < a.size(); i++) {
        if (a[i] < 0.0001)
          zero_comps++;
      }
      test[j].UpdateColor();
      pics_out[j].UpdateColor();
      test[j].Save(2 * j, "", "../imgs_out_eval");
      pics_out[j].Save(2 * j + 1, Str(max_a), "../imgs_out_eval");
      test[j].Save(string("../imgs_out_eval/class") + Str(max_a) + "_" + Str(j) + ".bmp");
//      print(a);
    }
  }
//  printf("Zero coefss: %d %d\n", zero_comps, save);
  LOG_HERE();
  return err / test.size();
}

void LearnPictures() {
  ClearFolder();
  ClearFolder("../imgs_out_eval");
  vector<Picture> pics;
  vector<Picture> eval;
//  LoadPictures("../imgs_small", pics, eval);
  LoadPictures("../mnist_train", pics, eval, true, 5000000);
  fprintf(stderr, "%s: %3d: %d images loaded\n", __FILE__, __LINE__, pics.size() + eval.size());
  vector<Picture> pics_out = pics;

  vector<float> tmp_mid;
/*  for(int i = 0; i < pics.size(); i++) {
//    pics[i].Print();
    pics[i].AddToStats();
    pics[i].UpdateColor();
//    pics[i].Save(i);
  }
//  pics[0].PrintStats();
*/

  double training_speed = 3e-2;
  double training_speed_factor = sqrt(sqrt(10));
  double training_speed_max = 1;
  int training_not_saved_for = 0;
  
  int save_every = 3;
  int print_every = 1;
  
  fprintf(stderr, "%s: %3d: Creating neural network\n", __FILE__, __LINE__);
  LOG_HERE();
  NeuralNetwork nn(pics[0].GrayData().size(), 50, pics[0].LabelData().size(), training_speed, 1);
//  nn.Load("model.dat");
  LOG_HERE();
  float lowest_epoch_error = Eval(nn, eval, true);

  printf("Input size: %d\n", pics[0].GrayData().size() - 1);
  printf("Initial error: %lf %lf\n", Eval(nn, pics, false), Eval(nn, eval, true));
  for(int i = 0; i < 5000000; i++) {
    LOG_HERE();
    if (pics[0].LabelData().size())
      for(int j = 0; j < pics.size(); j++)
        nn.Train(pics[j].GrayData(), pics[j].LabelData()) / pics.size();
    else
      for(int j = 0; j < pics.size(); j++)
        nn.Train(pics[j].GrayData(), pics[j].GrayData()) / pics.size();
    LOG_HERE();
    if (i % print_every == 0) {
      ClearFolder();
      ClearFolder("../imgs_out_eval");
      LOG_HERE();
      for(int j = 0; j < pics.size(); j++) {
        nn.CalcOutput(pics[j].GrayData(), tmp_mid, pics_out[j].GrayData(), pics[j].GrayData());
//        print(tmp_mid);
        pics_out[j].UpdateColor();
        pics[j].Save(2 * j);
        pics_out[j].Save(2 * j + 1);
      }
      printf("Epoch, error: %3d %lf %lf\n", i, Eval(nn, pics, false), Eval(nn, eval, true));
    }
    LOG_HERE();
    if (i % save_every == 0) {
      float epoch_error = Eval(nn, eval, false);
      if (lowest_epoch_error >= epoch_error * 0.999999) {
        printf("Saving the model\n");
        nn.Save("model.dat");
        training_not_saved_for = 0;
        if (lowest_epoch_error > epoch_error)
          lowest_epoch_error = epoch_error;
        training_speed *= training_speed_factor;
        if (training_speed > training_speed_max)
          training_speed = training_speed_max;
      } else {
        printf("Not saving the model -> too high error: %lf best: %lf\n", epoch_error, lowest_epoch_error);
        training_speed /= training_speed_factor;
        training_not_saved_for++;
        if (training_not_saved_for > 5) {
          nn.Load("model.dat");
          printf("Nor saved model for %d. Loaded last saved one.\n", training_not_saved_for);
          training_not_saved_for = 0;
        }
      }
      LOG_HERE();
      printf("New training speed: %e    ", training_speed);
      nn.SetTrainingSpeed(training_speed);
      nn.WStat();
    }
    LOG_HERE();
    if (i % 1000 == -1) {
      for(int j = 0; j < 10; j++) {
        nn.CalcOutput(pics[j].GrayData(), tmp_mid, pics_out[j].GrayData(), pics[j].GrayData());
        pics[j].Print();
        print(tmp_mid);
      }
    }
  }
  LOG_HERE();
  nn.Save("model.dat");
}

int main() {
//  SmallTest();
  LearnPictures();
}



