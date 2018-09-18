#include <snow.h>
#include <vector>

using namespace snow;
const int NUM = 45;

int main() {
    double *vec0 = snow::alignedMalloc<double>(NUM, 32);
    double *vec1 = snow::alignedMalloc<double>(NUM, 32);
    for (int i = 0; i < NUM; ++i) {vec0[i] = vec1[i] = i / M_PI;}

    double result;

    {
        snow::StopWatch watch("dot_normally");
        for (int i = 0; i < 100000; ++i) result = dot_normally(vec0, vec0, NUM);
    }
    printf("%f\n", result);
    {
        snow::StopWatch watch("dot_avx");
        for (int i = 0; i < 100000; ++i) result = dot(vec0, vec0, NUM);
    }
    printf("%f\n", result);

    std::vector<double> add_res;
    double W = 10.0;
    for (int i = 0; i < NUM; ++i) {
        add_res.push_back(vec0[i] + vec1[i] * W);
    }
    printf("\n");
    addwb(vec0, vec1, W, NUM);
    for (int i = 0; i < NUM; ++i) {
        printf("%d ", abs(vec0[i] - add_res[i]) < 1e-6);
        if (i % 15 == 14) printf("\n");
    }
    printf("\n");

    {
        for (int i = 0; i < NUM; ++i) {vec0[i] = vec1[i] = i;}
        snow::StopWatch watch("addwb");
        for (int i = 0; i < 1000000; ++i) addwb(vec0, vec1, 10.0, NUM);
    }

    {
        for (int i = 0; i < NUM; ++i) {vec0[i] = vec1[i] = i;}
        snow::StopWatch watch("addwb_normally");
        for (int i = 0; i < 1000000; ++i) addwb_normally(vec0, vec1, 10.0, NUM);
    }

    return 0;
}