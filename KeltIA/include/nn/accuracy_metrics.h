#include "network.h"
#include "train.h"
#include <stddef.h>

typedef struct
{
    double mse;         // Mean Squared Error
    double mae;         // Mean Absolute Error
    double rmse;        // Root Mean Squared Error
    double accuracy;    // Accuracy (clasificación)
    int correct_count;  // Number of correct predictions
} EvaluationMetrics;

EvaluationMetrics evaluate_network(NeuronalNetwork *nn, Dataset *data);

void print_evaluation(
    EvaluationMetrics metrics,
    int num_samples,
    const char *dataset_name,
    int epoch
);

void print_xor_nn_predictions(NeuronalNetwork *nn);
