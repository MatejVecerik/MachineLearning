#include "img.cc"

int w = 28;
int h = 28;

void CopyMnist() {
  Picture pic;
  pic.Load("../28x28_template.bmp");
  FILE *f = fopen("../mnist.train", "r");
  int label, pos, val;
  int num = 0;
  char n;
  while(true) {
    pic.Zero();
    if(fscanf(f, "%d", &label) == EOF)
      break;
    
    while(true) {
      fscanf(f, "%d", &pos);
      n = fgetc(f);
      fscanf(f, "%d", &val);
      pic.Set(pos % w, pos / w, val / 255.);

      n = fgetc(f);
      if (n == ' ') {
        continue;
      }
      if (n == '\n') {
        break;
      }
      printf("%d Got %c %d\n Exiting\n", __LINE__, n, n);
      exit(0);
    }
    string out_file = StrCat("../mnist_train/", Str(label), "_", Str(num), ".bmp");
    printf("Saving %s\n", out_file.c_str());
    pic.UpdateColor();
    pic.Save(out_file);
    num++;
  }
}

int main() {
  CopyMnist();
}
