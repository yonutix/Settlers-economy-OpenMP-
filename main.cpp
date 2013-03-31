#include "lib_seq.h"

int Pmin, Pmax;
int iterations;
/**
* \param Are the four values agregated
*/
std::vector<Report> reports;

void Year::memAlloc(int n)
{
  this->n = n;
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
}

Year::Year(int n, Year* previous)
{
  this->memAlloc(n);
  this->previous = previous;
  this->updateYear(previous);
}

Year::Year(char* filename):previous(NULL)
{
  FILE *f  = fopen(filename, "r");
  if(!f){
    printf("file not found\n");
    return;
  }
  fscanf(f, "%d %d %d", &Pmin, &Pmax, &n);
  this->memAlloc(n);
  for(int i = 0; i < n; ++i){
    for(int j = 0; j < n; ++j){
      fscanf(f, "%d", &res[i][j]);
    }
  }
  for(int i = 0; i < n; ++i){
    for(int j = 0; j < n; ++j){
      fscanf(f, "%d", &price[i][j]);
    }
  }
  for(int i = 0; i < n; ++i){
    for(int j = 0; j < n; ++j){
      fscanf(f, "%d", &boudget[i][j]);
    }
  }
  fclose(f);
}

Year::~Year()
{
  if(res != NULL){
    for(int i = 0; i < n; ++i){
      (res[i] != NULL)?delete res[i]:void(0);
    }
  }
  if(price != NULL){
    for(int i = 0; i < n; ++i){
      (price[i] != NULL)?delete price[i]:void(0);
    }
  }
  if(boudget != NULL){
    for(int i = 0; i < n; ++i){
      (boudget[i] != NULL)?delete boudget[i]:void(0);
    }
  }
  if(boudgetInventory != NULL){
    for(int i = 0; i < n; ++i){
      (boudgetInventory[i] != NULL)?delete boudgetInventory[i]:void(0);
    }
  }
}

void Year::updateYear(Year* previous)
{
  this->previous = previous;
  resProducers[RESB] = resProducers[RESA] = 0;
  resMaxPrice[RESA] = resMaxPrice[RESB] = MINPRICE;
  resMinPrice[RESA] = resMinPrice[RESB]  = MAXPRICE;

#pragma omp parallel for schedule(runtime)
  for(int k = 0; k < n*n; ++k){
    int i = k/n, j = k%n;
    boudget[i][j] = previous->getCost(i, j);
    res[i][j] = previous->res[i][j];

    price[i][j] = previous->price[i][j] + 
      (abs(previous->boudgetInventory[i][j]) - previous->boudgetInventory[i][j])/2 -
      (abs(previous->boudgetInventory[i][j]) + previous->boudgetInventory[i][j])/4;

    price[i][j] = MAX(price[i][j], Pmin);

    if(!previous->boudgetInventory[i][j]){
      int tmp_p = previous->getCostRes(i, j) + 1;
      price[i][j] = MIN(tmp_p, previous->price[i][j]+1);
    }

    price[i][j] = MAX(price[i][j], Pmin);
    if(price[i][j] > Pmax){
      boudget[i][j] = Pmax;
      price[i][j] = (Pmin + Pmax)/2;
      res[i][j] = 1-res[i][j];
    }
    if(price[i][j] > resMaxPrice[res[i][j]]){
#pragma omp critical
      resMaxPrice[res[i][j]] = 
        (abs(price[i][j]-resMaxPrice[res[i][j]])+price[i][j]-resMaxPrice[res[i][j]])/2 + resMaxPrice[res[i][j]];
    }

    if(price[i][j] < resMinPrice[res[i][j]]){
#pragma omp critical
      resMinPrice[res[i][j]] =
        -((abs(price[i][j] - resMinPrice[res[i][j]])-price[i][j] + resMinPrice[res[i][j]])/2 -  resMinPrice[res[i][j]]);
    }
  }

  resMinPrice[RESA] = (resMinPrice[RESA] == MAXPRICE)?Pmin:resMinPrice[RESA];
  resMinPrice[RESB] = (resMinPrice[RESB] == MAXPRICE)?Pmin:resMinPrice[RESB];
  int nr = 0;
#pragma omp parallel for reduction(+:nr)  
  for (int k = 0; k < n*n; ++k)
    if(res[k/n][k%n] == RESA )
      nr = nr+1;

  resProducers[RESA] = nr;
  resProducers[RESB] = n*n-nr;

  Report r;
  r.resAmaxPrice = resMaxPrice[RESA];
  r.resBmaxPrice = resMaxPrice[RESB];
  r.resAProducers = nr;
  r.resBProducers = n*n-nr;
  reports.push_back(r);
}

void Year::deployActivities()
{
#pragma omp parallel for schedule(runtime)
  for(int k = 0; k < n*n; ++k)
    boudgetInventory[k/n][k%n] = boudget[k/n][k%n] - getCheepestPrice(k/n, k%n);
}

int Year::getCheepestPrice(int x, int y)
{
  int p = 2*n-1, tmp_p, to;
  for(int k = 1; k < p-resMinPrice[1-res[x][y]]+1; ++k){

    to = MIN(k, n-y);
    for(int i = MAX((abs(x-k)-x+k)/2, 0); i < to; ++i){
      tmp_p = getCost(x, y, x-k+i, y+i);
      if(tmp_p < p){
        p = (res[x][y]-res[x-k+i][y+i])?tmp_p:p;
      }
    }

    to = MIN(k, n-x);
    for(int i = MAX((abs(n-y-k-1) - n+y+k+1)/2, 0); i < to; ++i){
      tmp_p = getCost(x, y, x+i, y+k-i);
      if(tmp_p < p){
        p = (res[x][y]-res[x+i][y+k-i])?tmp_p:p;
      }
    }
    to = MIN(k, k-(abs(y-k)-y+k)/2+1);
    for(int i = MAX((abs(n-x-k-1)-n+x+k+1)/2, 0); i < to; ++i){
      tmp_p = getCost(x, y, x+k-i, y-i);

      if(tmp_p < p){
        p = (res[x][y]-res[x+k-i][y-i])?tmp_p:p;
      }
    }
    to = MIN(k, k-(abs(x-k)-x+k)/2+1);
    for(int i = MAX((abs(y-k)-y+k)/2, 0); i < to; ++i){
      tmp_p = getCost(x, y, x-i, y-k+i);
      if(tmp_p < p){
        p = (res[x][y]-res[x-i][y-k+i])?tmp_p:p;
      }
    }
  }

  return p;
}

int Year::getCostRes(int x, int y)
{
  int p = MIN(price[x][y], 2*n-1), tmp_p, to;

  for(int k = 1; k < p-resMinPrice[res[x][y]]+1; ++k){
    to = MIN(k, n-y);
    for(int i = MAX((abs(x-k)-x+k)/2, 0); i < to; ++i){
      if((tmp_p = getCost(x, y, x-k+i, y+i)) < p && !(res[x][y]-res[x-k+i][y+i])){
        p = tmp_p;
      }
    }

    to = MIN(k, n-x);
    for(int i = MAX((abs(n-y-k-1) - n+y+k+1)/2, 0); i < to; ++i){
      if((tmp_p = getCost(x, y, x+i, y+k-i)) < p && !(res[x][y]-res[x+i][y+k-i])){
        p = tmp_p;
      }
    }
    to = MIN(k, k-(abs(y-k)-y+k)/2+1);
    for(int i = MAX((abs(n-x-k-1)-n+x+k+1)/2, 0); i < to; ++i){
      if((tmp_p = getCost(x, y, x+k-i, y-i)) < p && !(res[x][y]-res[x+k-i][y-i])){
        p = tmp_p;
      }
    }
    to = MIN(k, k-(abs(x-k)-x+k)/2+1);
    for(int i = MAX((abs(y-k)-y+k)/2, 0); i < to; ++i){
      if((tmp_p = getCost(x, y, x-i, y-k+i)) < p && !(res[x][y]-res[x-i][y-k+i])){
        p = tmp_p;
      }
    }
  }

  return p;
}

void Year::exportResults(char* filename, Year* y)
{
  FILE *f = fopen(filename, "w");
  if(!f){
    printf("file not found\n");
    return;
  }
  for(uint i = 0; i < reports.size(); ++i){
    fprintf(f, "%d %d %d %d\n", 
      reports[i].resAProducers,
      reports[i].resAmaxPrice,
      reports[i].resBProducers,
      reports[i].resBmaxPrice);
  }

  for(int i = 0; i < y->n; ++i){
    for(int j = 0; j < y->n; ++j){
      fprintf(f, "(%d,%d,%d) ", 
        y->res[i][j],
        y->price[i][j],
        y->boudget[i][j]);
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
  Year* y1 = new Year(argv[2]);
  y1->deployActivities();
  Year* y2 = new Year(y1->n, y1);
  y2->deployActivities();
  Year* aux_y;

  for(int i = 2; i < iterations; ++i){
    y1->updateYear(y2);
    y1->deployActivities();
    aux_y = y1;
    y1 = y2;
    y2 = aux_y;
  }
  Year::exportResults(argv[3], y2);
  delete y1;
  delete y2;
  return 0;
}