#include "../../include/nn/include_nn.h"

struct Dataset
{
    double **inputs;
    double **targets;
    int num_samples;
    size_t input_size;
    size_t output_size;
};

struct EvaluationMetrics
{
    double mse;         // Mean Squared Error
    double mae;         // Mean Absolute Error
    double rmse;        // Root Mean Squared Error
    double accuracy;    // Accuracy (clasificación)
    int correct_count;  // Número de predicciones correctas
};

EvaluationMetrics evaluate_network(NeuronalNetwork *nn, Dataset *data)
{
    EvaluationMetrics metrics = {0};

    double total_squared_error = 0.0;
    double total_absolute_error = 0.0;
    int correct = 0;

    int output_size = nn->layers[nn->n_layers - 1].n_neurons;
    double *output = malloc(output_size * sizeof(double));

    for (int i = 0; i < data->num_samples; i++)
    {
        // Forward pass
        compute_nn(nn, data->inputs[i], output);

        // Calculate per-sample errors
        double sample_squared_error = 0.0;
        double sample_absolute_error = 0.0;

        for (size_t j = 0; j < data->output_size; j++)
        {
            double error = data->targets[i][j] - output[j];
            sample_squared_error += error * error;
            sample_absolute_error += fabs(error);
        }

        total_squared_error += sample_squared_error;
        total_absolute_error += sample_absolute_error;

        // Classification accuracy
        if (data->output_size > 1)  // Multi-class classification
        {
            int predicted_class = 0;
            int target_class = 0;
            double max_output = output[0];
            double max_target = data->targets[i][0];

            for (size_t j = 1; j < data->output_size; j++)
            {
                if (output[j] > max_output)
                {
                    max_output = output[j];
                    predicted_class = j;
                }
                if (data->targets[i][j] > max_target)
                {
                    max_target = data->targets[i][j];
                    target_class = j;
                }
            }

            if (predicted_class == target_class) { correct++; }
        }
        else  // Binary classification
        {
            int predicted = (output[0] >= 0.5) ? 1 : 0;
            int target = (data->targets[i][0] >= 0.5) ? 1 : 0;
            if (predicted == target) { correct++; }
        }
    }

    // Calculate final metrics
    int total_outputs = data->num_samples * data->output_size;
    metrics.mse = total_squared_error / total_outputs;
    metrics.mae = total_absolute_error / total_outputs;
    metrics.rmse = sqrt(metrics.mse);
    metrics.accuracy = (double)correct / data->num_samples;
    metrics.correct_count = correct;

    free(output);
    return metrics;
}

void print_evaluation(
    EvaluationMetrics metrics,
    int num_samples,
    const char *dataset_name,
    int epoch
)
{
    printf("\n=== Evaluation: %s, epoch: %d ===\n", dataset_name, epoch);
    printf("Total samples: %d\n", num_samples);
    printf("Correct predictions: %d/%d\n", metrics.correct_count, num_samples);
    printf("Accuracy: %.2f%%\n", metrics.accuracy * 100.0);
    printf("MSE:  %.8f\n", metrics.mse);
    printf("RMSE: %.8f\n", metrics.rmse);
    printf("MAE:  %.8f\n", metrics.mae);
    printf("======================================\n\n");
}

void print_xor_nn_predictions(NeuronalNetwork *nn)
{
    // XOR dataset with one-hot encoded outputs
    double inputs[4][2] = {{0.0, 0.0}, {0.0, 1.0}, {1.0, 0.0}, {1.0, 1.0}};

    double expected[4][2] = {
        {1.0, 0.0},  // 0 XOR 0 = 0 -> [1, 0]
        {0.0, 1.0},  // 0 XOR 1 = 1 -> [0, 1]
        {0.0, 1.0},  // 1 XOR 0 = 1 -> [0, 1]
        {1.0, 0.0}   // 1 XOR 1 = 0 -> [1, 0]
    };

    int output_size = nn->layers[nn->n_layers - 1].n_neurons;
    double *output = malloc(output_size * sizeof(double));

    printf("\n=== XOR Predictions ===\n");

    for (int i = 0; i < 4; i++)
    {
        compute_nn(nn, inputs[i], output);

        printf(
            "  %.0f XOR %.0f -> Output: [%.4f, %.4f]  Expected: [%.0f, %.0f]\n",
            inputs[i][0], inputs[i][1], output[0], output[1], expected[i][0],
            expected[i][1]
        );
    }

    printf("=======================\n\n");

    free(output);
}
