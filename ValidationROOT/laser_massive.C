#include "LaserValidation.C"

int main(){
  // read list of run numbers
  ifstream lst("laser_cycle19.list");
  int runnum;

  while(lst >> runnum)UpdateLaserRuns(runnum);
}
