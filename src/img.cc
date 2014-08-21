#include <random>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cmath>
#include <dirent.h>

FILE* logging_file = fopen("logging.out", "w");
#define LOG_HERE() {fprintf(logging_file, "%s %d\n", __FILE__, __LINE__);fflush(logging_file);}

using namespace std;

void print(vector<float> a, string str = "") {
  printf("%8s [", str.c_str());
  for(int i = 0; i < a.size(); i++)
    printf(" %7.4lf", a[i]);
  printf(" ]\n");
}

void print(vector<int> a, string str = "") {
  printf("%8s [", str.c_str());
  for(int i = 0; i < a.size(); i++)
    printf(" %4d", a[i]);
  printf(" ]\n");
}

float randf() {
  return (rand() % 1000000) / 1000000.;
}

int Cap255(double a) {
  if (a < 0)
    return 0;
  if (a > 255)
    return 255;
  return a;
}

class Picture {
  int w;
  int lw; // Due to padding the actual lenght of the line is rounded up to a multiple of 4.
  int h;
  int c;
  
  int data_offset; // Where the image data starts.
  vector<int> data;
  vector<float> image;
  vector<float> grayscale;
  
  string original_name;
  int id;
  vector<float> label;
  
  static vector<int> histogram;
  static const vector<float> printing_thresholds;
  static const vector<char> printing_chars;
 public://copy constructor
  void Load(string source) {
    FILE *file = fopen(source.c_str(), "r"); 
    do {
      data.push_back(fgetc(file));
    } while (data.back() != EOF);
    fclose(file);

    if(data[0] != 'B' && data[1] != 'M') {
      fprintf(stderr, "Error: Invalid File Format. Bitmap Required. %s\n", source.c_str());
      exit(0);
    }

    if (data[28] != 24 && data[28] != 32) {
      fprintf(stderr, "Error: Invalid File Format. 24 or 32 bit Image Required. %s %d\n", source.c_str(), data[28]);
      exit(0);
    }

    c = data[28] / 8;
    w = data[18] + (data[19] << 8);
    h = data[22] + (data[23] << 8);
    lw = ((w * c + 3) / 4) * 4;
    data_offset = data[10] + (data[11] << 8);
    
    image.resize(w * h * c);
    for(int i = 0; i < h; i++)
      for(int j = 0; j < w; j++)
        for (int k = 0; k < c; k++) {
          image[(i * w + j) * c + k] = data[data_offset + (h - 1 - i) * lw + j * c + k] / 256.;
        }

    grayscale.resize(image.size() / c);
    UpdateGrayscale();
    image.push_back(1);
    grayscale.push_back(1);
    
//    printf("Loaded image %s w: %d h: %d c: %d size: %d\n", source.c_str(), w, h, c, image.size(), grayscale.size());
  }
  
  int W() {return w;}
  int H() {return h;}
  int C() {return c;}
  
  //col = green, blue, red
  float Get(int x, int y, int col) {
    return image[(y * w + x) * c + col];
  }

  float Get(int x, int y) {
    return grayscale[y * w + x];
  }
  
  void Set(int x, int y, int col, float val) {
    image[(y * w + x) * c + col] = val;
  }

  void Set(int x, int y, float val) {
    grayscale[y * w + x] = val;
  }
  
  void UpdateGrayscale() {
    for(int i = 0; i < h; i++)
      for(int j = 0; j < w; j++) {
        grayscale[i * w + j] = 0;
        for (int k = 0; k < c; k++)
          grayscale[i * w + j] += image[(i * w + j) * c + k] / c;
      }
  }

  void UpdateColor() {
    for(int i = 0; i < h; i++)
      for(int j = 0; j < w; j++)
        for (int k = 0; k < c; k++)
          image[(i * w + j) * c + k] = grayscale[i * w + j];
  }
  
  void Zero() {
    for(int i = 0; i < h; i++)
      for(int j = 0; j < w; j++)
        grayscale[i * w + j] = 0;
    UpdateColor();
  }
  
  void Save(int i, string extra = "", string folder = "../imgs_out") {
    char tmp[200];
    if (extra.empty())
      sprintf(tmp, "%s/%05d.bmp", folder.c_str(), i);
    else
      sprintf(tmp, "%s/%05d_%s.bmp", folder.c_str(), i, extra.c_str());
    Save(tmp);
  }
  
  void Save(string name) {
    for(int i = 0; i < h; i++)
      for(int j = 0; j < w; j++)
        for(int k = 0; k < c; k++)
          data[data_offset + (h - 1 - i) * lw + j * c + k] = Cap255(256 * Get(j, i, k));
    FILE *out = fopen(name.c_str(), "w");
//    printf("Saving file %s\n", name.c_str());
    for (int i = 0; i < data.size(); i++)
      fprintf(out, "%c", data[i]);
    fclose(out);
  }

  void AddToStats() {
    // Crashing - CBA to figure out why.
    int bins = 64;
    if (histogram.size() == 0)
      histogram.resize(bins);
    for(int i = 0; i < h; i++) {
      for(int j = 0; j < w; j++) {
        histogram[bins * Get(j, i)]++;
      }
    }
  }
  
  void PrintStats() {
    for(int i = 0; i < histogram.size(); i++)
      printf("%3d %5d\n", 255 * i / histogram.size(), histogram[i]);
  }

  void Print(bool full = false) {
    printf("%d %d %d\n", w, h, c);
    for(int i = 0; i < h; i++) {
      for(int j = 0; j < w; j++) {
        if (full) {
          printf("%.1lf ", Get(j, i, 0));
        } else {
          int k = 0;
          while (256 * Get(j, i) > printing_thresholds[k])
            k++;
          printf("%c", printing_chars[k]);
        }
      }
      printf("\n");
    }
  }
  
  vector<float>& ColorData() {return image;}
  vector<float>& GrayData() {return grayscale;}
  vector<float>& LabelData() {return label;}
  string& OriginalName() {return original_name;}
  int& Id() {return id;}
};

vector<int> Picture::histogram;
const vector<float> Picture::printing_thresholds = { 40,  70, 100, 135, 190, 220, 250, 256};
const vector<char> Picture::printing_chars =     {'.', '.', '-', ':', '!', 'O', 'X', '#'};

void LoadPictures(string dir_str, vector<Picture> &train, vector<Picture> &eval, bool do_label, int max_pics = 1000000) {
  DIR *dir;
  if ((dir = opendir (dir_str.c_str())) == NULL) {
    fprintf(stderr, "Non existent file %s\n", dir_str.c_str());
    exit(0);
  }
  
  int count = 0;
  while (-2) {
    dirent *ent = readdir (dir);
    if (ent == 0 || count >= max_pics)
      break;
    string f_name = ent->d_name;
    if (f_name[f_name.size() - 4] != '.' || f_name[f_name.size() - 3] != 'b' || f_name[f_name.size() - 2] != 'm' || f_name[f_name.size() - 1] != 'p')
      continue;
    string path = dir_str + "/" + f_name;
    
    Picture pic;
    pic.Load(path);
    pic.OriginalName() = f_name;
    sscanf(f_name.c_str() + 2, "%d", &pic.Id());
    if (do_label) {
      int label = f_name[0] - '0';
      pic.LabelData().resize(10);
      for(int i = 0; i < 10; i++)
        pic.LabelData()[i] = (i == label);
      pic.LabelData().push_back(1);
    }
    
    
    if (count % 5 == 4)
      eval.push_back(pic);
    else
      train.push_back(pic);
    count++;
  }
  random_device rd;
  mt19937 g(rd());
  shuffle(train.begin(), train.end(), g);
  shuffle(eval.begin(), eval.end(), g);
  closedir (dir);
}

void ClearFolder(string folder = "../imgs_out") {
  dirent *next_file;
  DIR *theFolder = opendir(folder.c_str());
  while ( next_file = readdir(theFolder)) {
    remove((folder + "/" + next_file->d_name).c_str());
  }
  closedir(theFolder);
}

string Str(double a) {
  static char tmp[100];
  sprintf(tmp, "%05.1lf", a);
  return tmp; 
}

string Str(int a) {
  static char tmp[100];
  sprintf(tmp, "%d", a);
  return tmp; 
}

string StrCat(string a, string b) {
  return a + b;
}

string StrCat(string a, string b, string c) {
  return a + b + c;
}

string StrCat(string a, string b, string c, string d) {
  return a + b + c + d;
}

string StrCat(string a, string b, string c, string d, string e) {
  return a + b + c + d + e;
}

string StrCat(string a, string b, string c, string d, string e, string f) {
  return a + b + c + d + e + f;
}

string StrCat(string a, string b, string c, string d, string e, string f, string g) {
  return a + b + c + d + e + f + g;
}

/*
Picture LoadBitmap(const char* FilePath) {
    std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

    hFile.seekg(0, std::ios::end);
    int Length = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> FileInfo(Length);
    hFile.read(reinterpret_cast<char*>(FileInfo.data()), 54);

    if(FileInfo[0] != 'B' && FileInfo[1] != 'M') {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. Bitmap Required.");
    }

    if (FileInfo[28] != 24 && FileInfo[28] != 32) {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. 24 or 32 bit Image Required.");
    }

    int bits_per_pixel = FileInfo[28];
    int width = FileInfo[18] + (FileInfo[19] << 8);
    int height = FileInfo[22] + (FileInfo[23] << 8);
    std::uint32_t PixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
    std::uint32_t size = ((width * bits_per_pixel + 31) / 32) * 4 * height;
    Pixels.resize(size);

    hFile.seekg (PixelsOffset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(Pixels.data()), size);
    hFile.close();
}
*/
