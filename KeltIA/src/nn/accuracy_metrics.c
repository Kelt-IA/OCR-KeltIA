#include "../../include/nn/accuracy_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

EvaluationMetrics evaluate_network(NeuronalNetwork *nn, Dataset *dataset)
{
    EvaluationMetrics metrics = {0};
    double total_mse = 0.0;
    int correct = 0;

    for (int i = 0; i < dataset->num_samples; i++)
    {
        double *output = malloc(dataset->output_size * sizeof(double));
        compute_nn(nn, dataset->inputs[i], output);

        // Find predicted class (argmax)
        size_t predicted = 0;
        double max_output = output[0];
        for (size_t j = 1; j < dataset->output_size; j++)
        {
            if (output[j] > max_output)
            {
                max_output = output[j];
                predicted = j;
            }
        }

        // Find actual class
        size_t actual = 0;
        for (size_t j = 0; j < dataset->output_size; j++)
        {
            if (dataset->targets[i][j] == 1.0)
            {
                actual = j;
                break;
            }
        }

        if (predicted == actual) { correct++; }

        // Calculate MSE
        for (size_t j = 0; j < dataset->output_size; j++)
        {
            double error = dataset->targets[i][j] - output[j];
            total_mse += error * error;
        }

        free(output);
    }

    metrics.accuracy = (double)correct / dataset->num_samples;
    metrics.mse = total_mse / (dataset->num_samples * dataset->output_size);
    metrics.correct_predictions = correct;

    return metrics;
}

void log_metrics(const char *filepath, size_t epoch, EvaluationMetrics metrics)
{
    FILE *f = fopen(filepath, "w");
    if (!f)
    {
        fprintf(stderr, "Error opening log file: %s\n", filepath);
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strcspn(timestamp, "\n")] = 0;  // Remove newline

    fprintf(f, "Training Log\n");
    fprintf(f, "=============\n");
    fprintf(f, "Timestamp: %s\n", timestamp);
    fprintf(f, "Epoch: %zu\n", epoch);
    fprintf(f, "Accuracy: %.4f%%\n", metrics.accuracy * 100.0);
    fprintf(f, "MSE: %.6f\n", metrics.mse);
    fprintf(f, "Correct predictions: %d\n", metrics.correct_predictions);

    fclose(f);
}
