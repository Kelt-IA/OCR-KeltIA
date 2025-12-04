// train.c - Versión con OpenMP
#include "../../include/nn/include_nn.h"
#include <cblas.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

volatile sig_atomic_t stop_requested = 0;

void global_sigint_handler(int sig)
{
    (void)sig;
    stop_requested = 1;
    printf("\n\nInterruption received. Stopping training...\n");
}

void train_nn(
    NeuronalNetwork *nn,
    Dataset *dataset,
    size_t epochs,
    size_t batch_size
)
{
    double **deltas;
    double **grad_weights;
    double **grad_biases;

    get_empty_deltas(nn, &deltas);
    get_empty_gradients(nn, &grad_weights, &grad_biases);

#ifdef _OPENMP
    int num_threads = omp_get_max_threads();
    printf("Using OpenMP with %d threads\n", num_threads);

    // Allocate per-thread gradients
    double ***thread_grad_weights = malloc(num_threads * sizeof(double **));
    double ***thread_grad_biases = malloc(num_threads * sizeof(double **));

    for (int t = 0; t < num_threads; t++)
    {
        thread_grad_weights[t] = malloc(nn->n_layers * sizeof(double *));
        thread_grad_biases[t] = malloc(nn->n_layers * sizeof(double *));

        for (size_t l = 0; l < nn->n_layers; l++)
        {
            size_t weight_size =
                nn->layers[l].n_neurons * nn->layers[l].n_inputs;
            thread_grad_weights[t][l] = calloc(weight_size, sizeof(double));
            thread_grad_biases[t][l] =
                calloc(nn->layers[l].n_neurons, sizeof(double));
        }
    }
#endif

    for (size_t epoch = 0; epoch < epochs && !stop_requested; epoch++)
    {
        size_t num_batches =
            (dataset->num_samples + batch_size - 1) / batch_size;

        for (size_t batch = 0; batch < num_batches && !stop_requested; batch++)
        {
            size_t batch_start = batch * batch_size;
            size_t batch_end =
                (batch_start + batch_size > (size_t)dataset->num_samples)
                    ? (size_t)dataset->num_samples
                    : batch_start + batch_size;
            size_t actual_batch_size = batch_end - batch_start;

            // Reset gradients
            for (size_t l = 0; l < nn->n_layers; l++)
            {
                size_t weight_size =
                    nn->layers[l].n_neurons * nn->layers[l].n_inputs;
                memset(grad_weights[l], 0, weight_size * sizeof(double));
                memset(
                    grad_biases[l], 0, nn->layers[l].n_neurons * sizeof(double)
                );
            }

#ifdef _OPENMP
// Parallel batch processing
#pragma omp parallel
            {
                int thread_id = omp_get_thread_num();
                double **local_deltas;
                get_empty_deltas(nn, &local_deltas);

#pragma omp for schedule(dynamic)
                for (size_t i = batch_start; i < batch_end; i++)
                {
                    backpropagation(
                        nn, dataset->inputs[i], dataset->targets[i],
                        local_deltas, thread_grad_weights[thread_id],
                        thread_grad_biases[thread_id]
                    );
                }

                // Free thread-local deltas
                for (size_t l = 0; l < nn->n_layers; l++)
                {
                    free(local_deltas[l]);
                }
                free(local_deltas);
            }

            // Accumulate gradients from all threads
            for (int t = 0; t < num_threads; t++)
            {
                for (size_t l = 0; l < nn->n_layers; l++)
                {
                    size_t weight_size =
                        nn->layers[l].n_neurons * nn->layers[l].n_inputs;
                    cblas_daxpy(
                        weight_size, 1.0, thread_grad_weights[t][l], 1,
                        grad_weights[l], 1
                    );
                    cblas_daxpy(
                        nn->layers[l].n_neurons, 1.0, thread_grad_biases[t][l],
                        1, grad_biases[l], 1
                    );

                    // Reset thread gradients
                    memset(
                        thread_grad_weights[t][l], 0,
                        weight_size * sizeof(double)
                    );
                    memset(
                        thread_grad_biases[t][l], 0,
                        nn->layers[l].n_neurons * sizeof(double)
                    );
                }
            }
#else
            // Serial batch processing
            for (size_t i = batch_start; i < batch_end; i++)
            {
                backpropagation(
                    nn, dataset->inputs[i], dataset->targets[i], deltas,
                    grad_weights, grad_biases
                );
            }
#endif

            // Average gradients and update
            double batch_factor = 1.0 / actual_batch_size;
            for (size_t l = 0; l < nn->n_layers; l++)
            {
                size_t weight_size =
                    nn->layers[l].n_neurons * nn->layers[l].n_inputs;
                cblas_dscal(weight_size, batch_factor, grad_weights[l], 1);
                cblas_dscal(
                    nn->layers[l].n_neurons, batch_factor, grad_biases[l], 1
                );

                update_parameters(
                    &nn->layers[l], grad_weights[l], grad_biases[l],
                    nn->learning_rate
                );
            }
        }
    }

#ifdef _OPENMP
    // Free thread-local gradients
    for (int t = 0; t < num_threads; t++)
    {
        for (size_t l = 0; l < nn->n_layers; l++)
        {
            free(thread_grad_weights[t][l]);
            free(thread_grad_biases[t][l]);
        }
        free(thread_grad_weights[t]);
        free(thread_grad_biases[t]);
    }
    free(thread_grad_weights);
    free(thread_grad_biases);
#endif

    for (size_t i = 0; i < nn->n_layers; i++)
    {
        free(deltas[i]);
        free(grad_weights[i]);
        free(grad_biases[i]);
    }
    free(deltas);
    free(grad_weights);
    free(grad_biases);
}
