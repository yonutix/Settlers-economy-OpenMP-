#include "lib_seq.h"

std::vector<Year*> frames;
int Pmin, Pmax;
int iterations;
Year::Year(int n, Year* previous)
{
  this->n = n;
  this->previous = previous;
  res = new int*[n];
  price = new int*[n];
  boudget = new int*[n];
  boudgetInventory = new int*[n];
  for(int i = 0; i < n; i++){
    res[i] = new int[n];
    price[i] = new int[n];
    boudget[i] = new int[n];
    boudgetInventory[i] = new int[n];
  }
  resProducers[RESB] = resProducers[RESA] = 0;
  resMaxPrice[RESA] = resMaxPrice[RESB] = MINPRICE;
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      if(previous->boudgetInventory[i][j] < 0){
        boudget[i][j] = previous->getCost(i, j);
        price[i][j] = previous->price[i][j] + previous->getCost(i, j) - previous->boudget[i][j];
        res[i][j] = previous->res[i][j];
      }
      if(previous->boudgetInventory[i][j] > 0){
        boudget[i][j] = previous->getCost(i, j);
        price[i][j] = previous->price[i][j] + (previous->getCost(i, j) - previous->boudget[i][j])/2;
        res[i][j] = previous->res[i][j];
        price[i][j] = std::max(price[i][j], Pmin);
      }
      if(previous->boudgetInventory[i][j] == 0){
        boudget[i][j] = previous->getCost(i, j);
        previous->res[i][j] = 1 - previous->res[i][j];
        int tmp_p = previous->getCheepestPrice(i, j);
        previous->res[i][j] = 1 - previous->res[i][j];
        price[i][j] = (tmp_p < previous->price[i][j])?(tmp_p+1):(previous->price[i][j]+1);
        res[i][j] = previous->res[i][j];
      }
      if(price[i][j] > Pmax){
        res[i][j] = 1-res[i][j];
        boudget[i][j] = Pmax;
        price[i][j] = (Pmin + Pmax)/2;
      }
      if(res[i][j] == RESA){
        resProducers[RESA]++;
        if(price[i][j] > resMaxPrice[RESA])
          resMaxPrice[RESA] = price[i][j];
      }
      else{
        resProducers[RESB]++;
        if(price[i][j] > resMaxPrice[RESB])
          resMaxPrice[RESB] = price[i][j];
      }
    }
  }

}

Year::Year(char* filename):previous(NULL)
{
  FILE *f  = fopen(filename, "r");
  if(!f){
    printf("File not found");
    return;
  }
  fscanf(f, "%d %d %d", &Pmin, &Pmax, &n);
  res = new int*[n];
  price = new int*[n];
  boudget = new int*[n];
  boudgetInventory = new int*[n];
  for(int i = 0; i < n; i++){
    res[i] = new int[n];
    price[i] = new int[n];
    boudget[i] = new int[n];
    boudgetInventory[i] = new int[n];
  }
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      fscanf(f, "%d", &res[i][j]);
    }
  }
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      fscanf(f, "%d", &price[i][j]);
    }
  }
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      fscanf(f, "%d", &boudget[i][j]);
    }
  }

  fclose(f);
}


Year::~Year()
{
  if(res != NULL){
    for(int i = 0; i < n; i++){
      (res[i] != NULL)?delete res[i]:void(0);
    }
  }
  if(price != NULL){
    for(int i = 0; i < n; i++){
      (price[i] != NULL)?delete price[i]:void(0);
    }
  }
  if(boudget != NULL){
    for(int i = 0; i < n; i++){
      (boudget[i] != NULL)?delete boudget[i]:void(0);
    }
  }
}

void Year::deployActivities()
{
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      boudgetInventory[i][j] = boudget[i][j] - getCheepestPrice(i, j);
    }
  }
}

int Year::getCheepestPrice(int x, int y)
{
  int p = MAXPRICE, tmp_p;
  for(int k = 1; k < 2*n-1; k++){
    if(k > p-Pmin+1)
        return p;
    for(int i = 0; i < k; i++){
      if(y+i < n && x-k+i > -1){
        if((tmp_p = getCost(x, y, x-k+i, y+i)) < p && res[x][y]-res[x-k+i][y+i]){
          p = tmp_p;
        }
      }
    }
    
    for(int i = 0; i < k; i++){
      if(x+i < n && y+k-i < n){
        if((tmp_p = getCost(x, y, x+i, y+k-i)) < p && res[x][y]-res[x+i][y+k-i]){
          p = tmp_p;
        }
      }
    }
    
    for(int i = 0; i < k; i++){
      if(y-i > -1 && x+k-i < n){
        if((tmp_p = getCost(x, y, x+k-i, y-i)) < p && res[x][y]-res[x+k-i][y-i]){
          p = tmp_p;
        }
      }
    }
    
    for(int i = 0; i < k; i++){
      if(x-i > -1 && y-k+i > -1){
        if((tmp_p = getCost(x, y, x-i, y-k+i)) < p && res[x][y]-res[x-i][y-k+i]){
          p = tmp_p;
        }
      }
    }
  }

  return p;
}

void Year::exportResults(char* filename, std::vector<Year*> &v)
{
  FILE *f = fopen(filename, "w");
  if(!f){
    printf("Error writing file, please check permissions settings");
    return;
  }
  
  for(uint i = 1; i < v.size(); i++){
    fprintf(f, "%d %d %d %d\n", 
      v[i]->resProducers[RESA],
      v[i]->resMaxPrice[RESA],
      v[i]->resProducers[RESB],
      v[i]->resMaxPrice[RESB]);
  }
  
  for(int i = 0; i < v[0]->n; i++){
    for(int j = 0; j < v[0]->n; j++){
      fprintf(f, "(%d,%d,%d) ", 
        v[v.size()-1]->res[i][j],
        v[v.size()-1]->price[i][j],
        v[v.size()-1]->boudget[i][j]);
    }
    fprintf(f, "\n");
  }

}


int main(int argc, char** argv)
{
  if(argc < 4){
    printf("Insuficient arguments\n");
    return 0;
  }
  iterations = atoi(argv[1]) + 1;
  Year *y = new Year(argv[2]);
  frames.push_back(y);
  frames[0]->deployActivities();
  for(int i = 1; i < iterations; i++){
    frames.push_back(new Year(frames[0]->n, frames[i-1]));
    frames[frames.size()-1]->deployActivities();
  }
  Year::exportResults(argv[3], frames);

  return 0;
}