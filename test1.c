#include <stdio.h>
#include <stdlib.h>

double calc_trace_cps(double dist, double danger, double alpha){
    long double alpha_less = 1-alpha;
    return (dist*alpha) + (alpha_less * danger);
}

int main() {
    long double dist = 22.8045;
    long double danger = 44.0846;
    long double alpha = 0.95;
    long double r = calc_trace_cps(dist,danger,alpha);
    
    printf("r : %Lf\n",r);
    return 0;
}