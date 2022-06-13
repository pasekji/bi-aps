#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char** argv){

string fileName1 = argv[1];

string line;

cout<<fileName1<<endl;

ifstream stream(fileName1, ios::in | ios::binary);
ofstream outfile;
outfile.open("out.ppm");

int width = 0;
int height = 0;
int con = 0;

string head("P6");

typedef struct
{
    unsigned char r, g, b;
} pixel;

if(stream.is_open()){

  getline(stream, line);

  if (head != line){
    cout << "corrupt file" << endl;
    return 0;
  }

  getline(stream, line);
  width = atoi(line.c_str());

  getline(stream, line);
  height = atoi(line.c_str());

  if(height < 1)
  {
    cout << "wrong value" << endl;
    return 0;
  }

  if(width < 1)
  {
    cout << "wrong value" << endl;
    return 0;
  }

  getline(stream, line);
  con = atoi(line.c_str());

  if (con != 255)
  {
    cout << "wrong value" << endl;
    return 0;
  }

  pixel **image = (pixel **) calloc(width, sizeof(pixel *));
  for(int i = 0; i < width; i++) {
    image[i] = (pixel *) calloc(height, sizeof(pixel));
  }

  for (int i = 0; i < width; i++)
   {
       for (int j = 0; j < height; j++)
       {
           stream.read((char*)&image[i][j].r, sizeof(uint8_t));
           stream.read((char*)&image[i][j].g, sizeof(uint8_t));
           stream.read((char*)&image[i][j].b, sizeof(uint8_t));
       }
   }

  stream.close();




  //cout << head << endl;
  //cout << width << endl;
  //cout << height << endl;
  //cout << 255 << endl;

  //for (int i = 0; i < width; i++)
   //{
    //   for (int j = 0; j < height; j++)
      // {
        //   cout << image[i][j].r;
          // cout << image[i][j].g;
           //cout << image[i][j].b;
       //}
   //}

pixel kernel[3][3];

kernel[0][0].r = 0;
kernel[0][1].r = -1;
kernel[0][2].r = 0;
kernel[1][0].r = -1;
kernel[1][1].r = 5;
kernel[1][2].r = -1;
kernel[2][0].r = 0;
kernel[2][1].r = -1;
kernel[2][2].r = 0;


pixel **image_out = (pixel **) calloc(width, sizeof(pixel *));
for(int i = 0; i < width; i++) {
  image_out[i] = (pixel *) calloc(height, sizeof(pixel));
}

int sumR;
int sumG;
int sumB;

for(int y = 1; y < height - 1; y++){
    for(int x = 1; x < width - 1; x++){
        sumR = 0;
        sumG = 0;
        sumB = 0;
        for(int k = -1; k <= 1;k++){
            for(int j = -1; j <=1; j++){
                sumR = sumR + kernel[j+1][k+1].r * image[y - j][x - k].r;
                sumG = sumG + kernel[j+1][k+1].r * image[y - j][x - k].g;
                sumB = sumB + kernel[j+1][k+1].r * image[y - j][x - k].b;

            }
        }
        image_out[y][x].r = sumR;
        image_out[y][x].g = sumG;
        image_out[y][x].b = sumB;
    }
}




//cout << "print out" << endl;
outfile << head << endl;
outfile << width << endl;
outfile << height << endl;
outfile << 255 << endl;

for (int i = 0; i < width; i++)
 {
     for (int j = 0; j < height; j++)
     {
       if(image_out[i][j].r > 255){
         image_out[i][j].r = 255;
       }
       if(image_out[i][j].r < 0){
         image_out[i][j].r = 0;
       }
       if(image_out[i][j].g > 255){
         image_out[i][j].g = 255;
       }
       if(image_out[i][j].g < 0){
         image_out[i][j].g = 0;
       }
       if(image_out[i][j].b > 255){
         image_out[i][j].b = 255;
       }
       if(image_out[i][j].b < 0){
         image_out[i][j].b = 0;
       }

       outfile << image_out[i][j].r;
       outfile << image_out[i][j].g;
       outfile << image_out[i][j].b;

     }
 }

outfile.close();
}



return 0;
}
